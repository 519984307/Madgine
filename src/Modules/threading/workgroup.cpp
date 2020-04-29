#include "../moduleslib.h"

#if ENABLE_THREADING

#    include "threadlocal.h"
#    include "threadstorage.h"
#    include "workgroup.h"
#    include "workgroupstorage.h"

namespace Engine {
namespace Threading {

    static THREADLOCAL(WorkGroup *) sSelf = nullptr;

    static std::atomic<size_t> sWorkgroupInstanceCounter = 0;

    static std::vector<Threading::TaskHandle> &sThreadInitializers()
    {
        static std::vector<Threading::TaskHandle> dummy;
        return dummy;
    }

    WorkGroup::WorkGroup(const std::string &name)
        : mInstanceCounter(sWorkgroupInstanceCounter.fetch_add(1))
        , mName(name.empty() ? "Workgroup_" + std::to_string(mInstanceCounter) : name)
    {
        ThreadStorage::init(true);
        ThreadStorage::init(false);

        assert(!sSelf);
        sSelf = this;

        setCurrentThreadName(mName + "_Main");

        WorkGroupStorage::init(true);
        WorkGroupStorage::init(false);

        for (const Threading::TaskHandle &task : sThreadInitializers()) {
            task();
        }

        //Is that useful at all?
        for (const Threading::TaskHandle &task : mThreadInitializers) {
            task();
        }
    }

    WorkGroup::~WorkGroup()
    {
        assert(singleThreaded());

        WorkGroupStorage::finalize(false);
        WorkGroupStorage::finalize(true);

        finalizeThread();
    }

    void WorkGroup::addThreadInitializer(Threading::TaskHandle &&task)
    {
        assert(mSubThreads.empty());
        mThreadInitializers.emplace_back(std::move(task));
    }

    void WorkGroup::addStaticThreadInitializer(Threading::TaskHandle &&task)
    {
        sThreadInitializers().emplace_back(std::move(task));
    }

    bool WorkGroup::singleThreaded()
    {
        return mSubThreads.empty();
    }

    void WorkGroup::checkThreadStates()
    {
        auto pivot = std::remove_if(mSubThreads.begin(), mSubThreads.end(),
            [](std::future<int> &f) {
                return f.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
            });

        for (auto it = pivot; it != mSubThreads.end(); ++it) {
            it->get();
        }

        mSubThreads.erase(pivot, mSubThreads.end());
    }

    const std::string &WorkGroup::name() const
    {
        return mName;
    }

    void WorkGroup::initThread(const std::string &name)
    {

        ThreadStorage::init(true);
        ThreadStorage::init(false);

        assert(!sSelf);
        sSelf = this;

        setCurrentThreadName(mName + "_" + name);

        for (const Threading::TaskHandle &task : sThreadInitializers()) {
            task();
        }

        for (const Threading::TaskHandle &task : mThreadInitializers) {
            task();
        }
    }

    void WorkGroup::finalizeThread()
    {

        assert(sSelf == this);
        sSelf = nullptr;

        ThreadStorage::finalize(false);
        ThreadStorage::finalize(true);
    }

    WorkGroup &WorkGroup::self()
    {
        assert(isInitialized());
        return *sSelf;
    }

    bool WorkGroup::isInitialized()
    {
        return sSelf != nullptr;
    }

    void WorkGroup::addTaskQueue(TaskQueue *taskQueue)
    {
        mTaskQueues.push_back(taskQueue);
    }

    const std::vector<TaskQueue *> WorkGroup::taskQueues() const
    {
        return mTaskQueues;
    }

}
}

#endif
