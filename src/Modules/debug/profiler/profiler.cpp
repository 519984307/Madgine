#include "../../moduleslib.h"

#include "profiler.h"

#include "../../threading/workgroupstorage.h"

#include "../../threading/threadlocal.h"

#include "../../threading/taskguard.h"

#include "profilerthread.h"

namespace Engine {

namespace Debug {

    namespace Profiler {

        THREADLOCAL(ProfilerThread) sThread;
        Threading::WorkgroupLocal<Profiler> sProfiler;
#if ENABLE_THREADING
        Threading::TaskGuard _reg { []() { Threading::WorkGroup::addStaticThreadInitializer([]() { sProfiler->registerThread(&sThread); sThread->updateName(); }); }, []() {} };
#endif

        Profiler::Profiler()
        {
#if !ENABLE_THREADING
            registerThread(&sThread);
#endif
        }

        Profiler::~Profiler()
        {
        }

        const std::list<ProfilerThread *> &Profiler::getThreadStats()
        {
            return mThreads;
        }

        void Profiler::registerThread(ProfilerThread *thread)
        {
            mThreads.emplace_back(thread);
        }

        void Profiler::unregisterThread(ProfilerThread *thread)
        {
            mThreads.remove(thread);
        }

        Profiler &Profiler::getCurrent()
        {
            return sProfiler;
        }

    }
}
}
