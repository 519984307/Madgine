#include "../../interfaceslib.h"

#include "profiler.h"

#include "../../threading/workgroup.h"


namespace Engine
{

	namespace Debug {

		namespace Profiler
		{

			thread_local ProfilerThread *sThread = nullptr;

			Profiler::Profiler(Threading::WorkGroup &workGroup)
			{
				registerCurrentThread();
				workGroup.addThreadInitializer([this]() { registerCurrentThread(); });
			}

			Profiler::~Profiler()
			{
			}

			const std::list<ProfilerThread> &Profiler::getThreadStats()
			{
				return mThreads;
			}

			void Profiler::registerCurrentThread()
			{
				mThreads.emplace_back(this);
			}

			Profiler & Profiler::getCurrent()
			{
				return *sThread->mProfiler;
			}

			std::string idToString(std::thread::id id)
			{
				std::stringstream ss;
				ss << id;
				return ss.str();
			}

			ProfilerThread::ProfilerThread(Profiler *profiler) :
				mId(idToString(std::this_thread::get_id())),
				mStats(mId.c_str()),
				mProfiler(profiler)
			{
				assert(!sThread);
				sThread = this;

				mCurrent = &mStats;
				mStats.start();
			}

			ProfilerThread::~ProfilerThread()
			{
				mStats.stop();

				assert(sThread == this);
				sThread = nullptr;
			}

			void StaticProcess::start()
			{
				if (mStats.start()) {
					mPrevious = sThread->mCurrent;
					sThread->mCurrent = &mStats;
				}
			}

			void StaticProcess::stop()
			{
				std::optional<ProcessStats::Data> d = mStats.stop();
				if (d) {
					assert(sThread->mCurrent == &mStats);
					sThread->mCurrent = mPrevious;
					if (mPrevious)
					{
						mPrevious = nullptr;
						if (!mParent) {
							mParent = sThread->mCurrent->updateChild(&mStats, *d);
						}
						else {
							mParent->second += *d;
						}
					}
				}
			}

		}
	}
}
