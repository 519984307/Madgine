#include "../baselib.h"
#include "application.h"

#include "appsettings.h"

#include "globalapibase.h"

#include "../scene/scenemanager.h"

//#include "Modules/scripting/types/luastate.h"

#include "Interfaces/exception.h"

#include "Modules/debug/profiler/profiler.h"

#include "Modules/keyvalue/metatable_impl.h"

#include "Modules/threading/workgroup.h"

namespace Engine {

namespace App {

    static Threading::WorkgroupLocal<Application *> sApp;

    Application::Application(const AppSettings &settings)
        : //Scripting::GlobalScopeBase(Scripting::LuaState::getSingleton()),
        mSettings(settings)
        , mGlobalAPIInitCounter(0)
        , mGlobalAPIs(*this)
    {
        assert(!sApp);
        sApp = this;

        mLoop.addSetupSteps(
            [this]() {
                if (!callInit())
                    throw exception("App Init Failed!");
            },
            [this]() {
                callFinalize();
            });
    }

    Application::~Application()
    {
        assert(sApp == this);
        sApp = nullptr;
    }

    bool Application::init()
    {

        markInitialized();

        for (const std::unique_ptr<GlobalAPIBase> &api : mGlobalAPIs) {
            if (!api->callInit(mGlobalAPIInitCounter))
                return false;
        }

        return true;
    }

    void Application::finalize()
    {
        for (; mGlobalAPIInitCounter > 0; --mGlobalAPIInitCounter) {
            for (const std::unique_ptr<GlobalAPIBase> &api : mGlobalAPIs) {
                api->callFinalize(mGlobalAPIInitCounter);
            }
        }
    }

    void Application::shutdown()
    {
        mLoop.shutdown();
    }

    bool Application::isShutdown() const
    {
        return mLoop.isShutdown();
    }

    float Application::getFPS()
    {
        return 1.0f;
    }

    /*KeyValueMapList Application::maps()
		{
			return Scope::maps().merge(mGlobalAPIs, this, MAP_F(shutdown));
		}*/

    GlobalAPIBase &Application::getGlobalAPIComponent(size_t i, bool init)
    {
        GlobalAPIBase &api = mGlobalAPIs.get(i);
        if (init) {
            checkInitState();
            api.callInit(mGlobalAPIInitCounter);
        }
        return api.getSelf(init);
    }

    Scene::SceneComponentBase &Application::getSceneComponent(size_t i, bool init)
    {
        Scene::SceneManager &sceneMgr = mGlobalAPIs.get<Scene::SceneManager>();
        if (init) {
            checkInitState();
            sceneMgr.callInit(mGlobalAPIInitCounter);
        }
        return sceneMgr.getComponent(i, init);
    }

    Scene::SceneManager &Application::sceneMgr(bool init)
    {
        Scene::SceneManager &sceneMgr = mGlobalAPIs.get<Scene::SceneManager>();
        if (init) {
            checkInitState();
            sceneMgr.callInit(mGlobalAPIInitCounter);
        }
        return sceneMgr.getSelf(init);
    }

    Application &Application::getSelf(bool init)
    {
        if (init) {
            checkDependency();
        }
        return *this;
    }

    void Application::addFrameListener(Threading::FrameListener *listener)
    {
        mLoop.addFrameListener(listener);
    }

    void Application::removeFrameListener(Threading::FrameListener *listener)
    {
        mLoop.removeFrameListener(listener);
    }

    void Application::singleFrame()
    {
        mLoop.singleFrame();
    }

    Threading::FrameLoop &Application::frameLoop()
    {
        return mLoop;
    }

    const AppSettings &Application::settings()
    {
        return mSettings;
    }

    Debug::Profiler::Profiler &Application::profiler()
    {
        return *mProfiler;
    }

    const MadgineObject *Application::parent() const
    {
        return nullptr;
    }

    /*Application &Application::app(bool init)
    {
        return getSelf(init);
    }*/

    Application &Application::getSingleton()
    {
        return *sApp;
    }
}

}

METATABLE_BEGIN(Engine::App::Application)
MEMBER(mGlobalAPIs)
METATABLE_END(Engine::App::Application)

RegisterType(Engine::App::Application);