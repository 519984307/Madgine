#include "Madgine/clientlib.h"
#include "Madgine/baselib.h"

#include "androidlauncher.h"

#include "Madgine/core/root.h"

#include "Madgine/window/mainwindow.h"

#include <android/native_activity.h>

#include "Generic/systemvariable.h"

#include "Interfaces/filesystem/api.h"
#include "../launcher.h"

#include "cli/parameter.h"
#include "Interfaces/util/standardlog.h"

extern Engine::CLI::Parameter<Engine::Util::MessageType> logLevel;

namespace Engine {

namespace Window {
    extern SystemVariable<ANativeWindow *> sNativeWindow;
    extern AInputQueue *sQueue;
}

namespace Android {

    template <auto f, typename... Args>
    static void delegate(ANativeActivity *activity, Args... args)
    {
        (static_cast<AndroidLauncher *>(activity->instance)->*f)(args...);
    }

    AndroidLauncher::AndroidLauncher(ANativeActivity *activity)
        : mActivity(activity)
    {
        activity->instance = this;

        activity->callbacks->onDestroy = delegate<&AndroidLauncher::onDestroy>;
        activity->callbacks->onNativeWindowCreated = delegate<&AndroidLauncher::onNativeWindowCreated, ANativeWindow *>;
        activity->callbacks->onNativeWindowDestroyed = delegate<&AndroidLauncher::onNativeWindowDestroyed, ANativeWindow *>;
        activity->callbacks->onInputQueueCreated = delegate<&AndroidLauncher::onInputQueueCreated, AInputQueue *>;
        activity->callbacks->onInputQueueDestroyed = delegate<&AndroidLauncher::onInputQueueDestroyed, AInputQueue *>;

        Engine::Filesystem::setup(activity);

        mThread = Threading::WorkGroupHandle(&AndroidLauncher::go, this);
    }

    void AndroidLauncher::go()
    {
        ANativeActivity *activity = mActivity;

        static Engine::Core::Root root;

        Engine::Util::StandardLog::setLogLevel(logLevel);

		launch(&mWindow);
		
		ANativeActivity_finish(activity);
    }

    void AndroidLauncher::onDestroy()
    {
        mWindow->shutdown();
        mThread.detach();
        mActivity->instance = nullptr;
        delete this;
    }

    void AndroidLauncher::onNativeWindowCreated(ANativeWindow *window)
    {
        assert(!Window::sNativeWindow);
        Window::sNativeWindow = window;
    }

    void AndroidLauncher::onNativeWindowDestroyed(ANativeWindow *window)
    {
        assert(Window::sNativeWindow == window);
        Window::sNativeWindow = nullptr;
    }

    void AndroidLauncher::onInputQueueCreated(AInputQueue *queue)
    {
        assert(!Window::sQueue);
        Window::sQueue = queue;
    }

    void AndroidLauncher::onInputQueueDestroyed(AInputQueue *queue)
    {
        assert(Window::sQueue == queue);
        Window::sQueue = nullptr;
    }

}
}