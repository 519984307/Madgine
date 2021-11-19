#include "../moduleslib.h"

#include "taskqueue.h"
#include "workgroup.h"

namespace Engine {
namespace Threading {

    TaskQueue::TaskQueue(const std::string &name, bool wantsMainThread)
        : mName(name)
        , mSetupState(mSetupSteps.begin())
        , mWantsMainThread(wantsMainThread)
    {
        WorkGroup::self().addTaskQueue(this);
    }

    TaskQueue::~TaskQueue()
    {
        assert(mQueue.empty());
        WorkGroup::self().removeTaskQueue(this);
    }

    bool TaskQueue::wantsMainThread() const
    {
        return mWantsMainThread;
    }

    void TaskQueue::queueInternal(ScheduledTask &&task)
    {
        {
            //TODO: priority Queue
            std::lock_guard<std::mutex> lock(mMutex);
            mQueue.emplace_back(std::move(task));
        }
        mCv.notify_one();
    }

    std::optional<TaskTracker> TaskQueue::fetch_on_idle()
    {
        return {};
    }

    TaskTracker TaskQueue::wrapTask(TaskHandle &&task)
    {
        return TaskTracker { std::move(task), mTaskCount };
    }

    const std::string &TaskQueue::name() const
    {
        return mName;
    }

    std::chrono::steady_clock::time_point TaskQueue::update(int idleCount, int repeatedCount)
    {
        std::chrono::steady_clock::time_point nextAvailableTaskTime = std::chrono::steady_clock::time_point::max();
        while (std::optional<Threading::TaskTracker> f = fetch(nextAvailableTaskTime, idleCount, repeatedCount)) {
            f->mTask();
            assert(!f->mTask);
        }
        return nextAvailableTaskTime;
    }

    void TaskQueue::waitForTasks(std::chrono::steady_clock::time_point until)
    {
        std::unique_lock<std::mutex> lock(mMutex);

        if (until == std::chrono::steady_clock::time_point::max()) {
            auto cond = [=]() { return !mQueue.empty() || !mRunning || !mRepeatedTasks.empty(); };
            mCv.wait(lock, cond);
        } else {
            auto cond = [=]() { return !mQueue.empty() || !mRunning; };
            mCv.wait_until(lock, until, cond);
        }
    }

    void TaskQueue::notify()
    {
        mCv.notify_one();
    }

    bool TaskQueue::running() const
    {
        return mRunning;
    }

    void TaskQueue::stop()
    {
        mRunning = false;
        mCv.notify_all();
    }

    void TaskQueue::queueHandle(TaskHandle &&task, const std::vector<Threading::DataMutex *> &dependencies)
    {
        queueInternal({ std::move(task) });
    }

    void TaskQueue::queue_after(TaskHandle &&task, std::chrono::steady_clock::duration duration, const std::vector<Threading::DataMutex *> &dependencies)
    {
        queue_for(std::move(task), std::chrono::steady_clock::now() + duration);
    }

    void TaskQueue::queue_for(TaskHandle &&task, std::chrono::steady_clock::time_point time_point, const std::vector<Threading::DataMutex *> &dependencies)
    {
        queueInternal({ std::move(task), time_point });
    }

    void TaskQueue::addRepeatedTask(std::function<void()> &&task, std::chrono::steady_clock::duration interval, void *owner)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mRepeatedTasks.emplace_back(RepeatedTask { std::move(task), owner, interval });
        }
        mCv.notify_one();
    }

    void TaskQueue::removeRepeatedTasks(void *owner)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mRepeatedTasks.erase(std::remove_if(mRepeatedTasks.begin(), mRepeatedTasks.end(), [owner](const RepeatedTask &task) { return task.mOwner == owner; }), mRepeatedTasks.end());
    }

    std::optional<TaskTracker> TaskQueue::fetch(std::chrono::steady_clock::time_point &nextTask, int &idleCount, int &repeatedCount)
    {
        std::chrono::steady_clock::time_point nextTaskTimepoint = nextTask;

        if (mRunning) {
            while (mSetupState != mSetupSteps.end()) {
                TaskHandle init = std::move(mSetupState->first);
                ++mSetupState;
                if (init) {
                    return wrapTask(std::move(init));
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(mMutex);
            for (auto it = mQueue.begin(); it != mQueue.end(); ++it) {
                if (it->mScheduledFor <= std::chrono::steady_clock::now()) {
                    TaskTracker f = wrapTask(std::move(it->mTask));
                    mQueue.erase(it);
                    return f;
                } else {
                    nextTaskTimepoint = std::min(it->mScheduledFor, nextTaskTimepoint);
                }
            }
            while (!mAwaiterStack.empty()) {
                TaskHandle handle = std::move(mAwaiterStack.top());
                mAwaiterStack.pop();
                if (handle.queue() == this) {
                    return wrapTask(std::move(handle));
                }
                handle.resumeInQueue();
            }
            if (mRunning && repeatedCount != 0) {
                RepeatedTask *nextTask = nullptr;
                for (RepeatedTask &task : mRepeatedTasks) {
                    if (task.mNextExecuted < nextTaskTimepoint) {
                        nextTask = &task;
                        nextTaskTimepoint = task.mNextExecuted;
                    }
                }
                if (nextTask && nextTaskTimepoint <= std::chrono::steady_clock::now()) {
                    if (repeatedCount > 0)
                        --repeatedCount;
                    nextTask->mNextExecuted = std::chrono::steady_clock::now() + nextTask->mInterval;
                    return wrapTask(make_task([=]() { nextTask->mTask(); }).assign(this));
                }
            }
        }

        if (mRunning && idleCount != 0) {
            size_t zero = 0;
            if (mTaskCount.compare_exchange_strong(zero, 1)) {
                std::optional<TaskTracker> task = fetch_on_idle();
                --mTaskCount;
                if (task) {
                    if (idleCount > 0)
                        --idleCount;
                    return task;
                }
            }
        }

        if (!mRunning && nextTaskTimepoint == std::chrono::steady_clock::time_point::max()) {

            while (mSetupState != mSetupSteps.begin()) {
                --mSetupState;
                TaskHandle finalize = std::move(mSetupState->second);
                if (finalize) {
                    return wrapTask(std::move(finalize));
                }
            }
        }

        nextTask = nextTaskTimepoint;
        return {};
    }

    bool TaskQueue::idle() const
    {
        if (mRunning) {
            if (mSetupState != mSetupSteps.end())
                return false;
        } else {
            if (mSetupState != mSetupSteps.begin())
                return false;
        }

        return mQueue.empty() && mTaskCount == 0;
    }

    void TaskQueue::addSetupStepTasks(TaskHandle init, TaskHandle finalize)
    {
        bool isItEnd = mSetupState == mSetupSteps.end();
        mSetupSteps.emplace_back(std::move(init), std::move(finalize));
        if (isItEnd) {
            mSetupState = std::prev(mSetupState);
        }
    }

    bool TaskQueue::await_ready()
    {
        return idle();
    }

    void TaskQueue::await_suspend(TaskHandle handle)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mAwaiterStack.emplace(std::move(handle));
    }

    void TaskQueue::await_resume()
    {
    }

    TaskTracker::TaskTracker(TaskHandle &&task, std::atomic<size_t> &tracker)
        : mTask(std::move(task))
        , mTracker(&tracker)
    {
        assert(mTask);
        ++tracker;
    }

    TaskTracker::TaskTracker(TaskTracker &&other)
        : mTask(std::move(other.mTask))
        , mTracker(std::exchange(other.mTracker, nullptr))
    {
    }

    TaskTracker::~TaskTracker()
    {
        if (mTracker)
            --(*mTracker);
    }

}
}