#include "../clientlib.h"

#include "uimanager.h"

#include "../gui/widgets/toplevelwindow.h"
#include "../gui/guisystem.h"

#include "../gui/widgets/widget.h"

#include "Madgine/app/application.h"
#include "Madgine/app/appsettings.h"

#include "Modules/debug/profiler/profiler.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/reflection/classname.h"

namespace Engine {

namespace UI {

    UIManager::UIManager(GUI::TopLevelWindow &window)
        : UniqueComponent(window)
        , mGuiHandlers(*this)
        , mGameHandlers(*this)
    {
    }

    UIManager::~UIManager()
    {
    }

    UIManager &UIManager::getSelf(bool init)
    {
        if (init) {
            checkDependency();
        }
        return *this;
    }

    bool UIManager::init()
    {
        App::Application::getSingleton().addFrameListener(this);

        markInitialized();

        if (App::Application::getSingleton().settings().mRunMain) {
            /*std::optional<Scripting::ArgumentList> res = app(false).callMethodIfAvailable("afterViewInit", {});
				if (res && !res->empty() && (!res->front().is<bool>() || !res->front().as<bool>()))
					return false;*/
        }

        for (const std::unique_ptr<GuiHandlerBase> &handler : mGuiHandlers)
            if (!handler->callInit())
                return false;

        for (const std::unique_ptr<GameHandlerBase> &handler : mGameHandlers) {
            if (!handler->callInit())
                return false;
        }

        return true;
    }

    void UIManager::finalize()
    {
        for (const std::unique_ptr<GameHandlerBase> &handler : mGameHandlers) {
            handler->callFinalize();
        }

        for (const std::unique_ptr<GuiHandlerBase> &handler : mGuiHandlers)
            handler->callFinalize();

        App::Application::getSingleton().removeFrameListener(this);
    }

    void UIManager::clear()
    {
        /*while (!mModalWindowList.empty()) {
            closeModalWindow(mModalWindowList.top());
        }*/
    }

    void UIManager::hideCursor(bool keep)
    {
        if (!isCursorVisible())
            return;
        mKeepingCursorPos = keep;
        if (keep) {
            /*const OIS::MouseState &mouseState = mMouse->getMouseState();
				mKeptCursorPosition = { (float)mouseState.X.abs, (float)mouseState.Y.abs };*/
        }
        //mGUI.hideCursor();
        for (const std::unique_ptr<GameHandlerBase> &h : mGameHandlers) {
            h->onMouseVisibilityChanged(false);
        }
        for (const std::unique_ptr<GuiHandlerBase> &h : mGuiHandlers) {
            h->onMouseVisibilityChanged(false);
        }
    }

    void UIManager::showCursor()
    {
        if (isCursorVisible())
            return;
        if (mKeepingCursorPos) {
            /*OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
				mutableMouseState.X.abs = mKeptCursorPosition.x;
				mutableMouseState.Y.abs = mKeptCursorPosition.y;
				callSafe([&]() {
					mouseMoved(OIS::MouseEvent(mMouse, mutableMouseState));*/
            //mGUI.showCursor();
            /*});*/
        } else {
            //mGUI.showCursor();
        }
        for (const std::unique_ptr<GameHandlerBase> &h : mGameHandlers) {
            h->onMouseVisibilityChanged(true);
        }
        for (const std::unique_ptr<GuiHandlerBase> &h : mGuiHandlers) {
            h->onMouseVisibilityChanged(true);
        }
    }

    bool UIManager::isCursorVisible() const
    {
        return /* mGUI.isCursorVisible()*/ true;
    }

    std::set<GameHandlerBase *> UIManager::getGameHandlers()
    {
        std::set<GameHandlerBase *> result;
        for (const std::unique_ptr<GameHandlerBase> &h : mGameHandlers) {
            result.insert(h.get());
        }
        return result;
    }

    std::set<GuiHandlerBase *> UIManager::getGuiHandlers()
    {
        std::set<GuiHandlerBase *> result;
        for (const std::unique_ptr<GuiHandlerBase> &h : mGuiHandlers) {
            result.insert(h.get());
        }
        return result;
    }

    /*App::Application &UIManager::app(bool init)
    {
        if (init) {
            checkInitState();
        }
        return mWindow.gui().app(init);
    }*/

    const MadgineObject *UIManager::parent() const
    {
        return &mWindow.gui();
    }

    bool UIManager::frameRenderingQueued(std::chrono::microseconds timeSinceLastFrame, Scene::ContextMask context)
    {
        PROFILE();
        for (const std::unique_ptr<GameHandlerBase> &h : mGameHandlers) {
            h->update(timeSinceLastFrame, context);
        }
        return true;
    }

    bool UIManager::frameFixedUpdate(std::chrono::microseconds timeSinceLastFrame, Scene::ContextMask context)
    {
        for (const std::unique_ptr<GameHandlerBase> &h : mGameHandlers) {
            h->fixedUpdate(timeSinceLastFrame, context);
        }
        return true;
    }

    /*Scene::ContextMask UIManager::currentContext()
    {
        return mModalWindowList.empty()
            ? (mCurrentRoot ? mCurrentRoot->context() : Scene::ContextMask::NoContext)
            : mModalWindowList.top()->context();
    }*/

    GUI::TopLevelWindow &UIManager::window(bool init) const
    {
        if (init) {
            checkInitState();
        }
        return mWindow;
    }

    /*KeyValueMapList UIManager::maps()
		{
			return Scope::maps().merge(mGuiHandlers, mGameHandlers, MAP_F(showCursor));
		}*/

    const char *UIManager::key() const
    {
        return "UI";
    }

    Scene::SceneComponentBase &UIManager::getSceneComponent(size_t i, bool init)
    {
        if (init) {
            checkInitState();
        }
        return mWindow.gui().getSceneComponent(i, init);
    }

    App::GlobalAPIBase &UIManager::getGlobalAPIComponent(size_t i, bool init)
    {
        if (init) {
            checkInitState();
        }
        return mWindow.gui().getGlobalAPIComponent(i, init);
    }

    Scene::SceneManager &UIManager::sceneMgr(bool init)
    {
        if (init) {
            checkInitState();
        }
        return mWindow.gui().sceneMgr(init);
    }

    GameHandlerBase &UIManager::getGameHandler(size_t i, bool init)
    {
        GameHandlerBase &handler = mGameHandlers.get(i);
        if (init) {
            checkInitState();
            handler.callInit();
        }
        return handler.getSelf(init);
    }

    GuiHandlerBase &UIManager::getGuiHandler(size_t i, bool init)
    {
        GuiHandlerBase &handler = mGuiHandlers.get(i);
        if (init) {
            checkInitState();
            handler.callInit();
        }
        return handler.getSelf(init);
    }

}
}

UNIQUECOMPONENT(Engine::UI::UIManager)

METATABLE_BEGIN(Engine::UI::UIManager)
METATABLE_END(Engine::UI::UIManager)

RegisterType(Engine::UI::UIManager);