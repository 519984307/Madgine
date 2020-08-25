#include "oislib.h"
#include "oisinputhandler.h"

#include "Madgine/input/inputevents.h"

#include "Interfaces/window/windowapi.h"

#include "Modules/debug/profiler/profile.h"

#include "Modules/keyvalue/metatable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

UNIQUECOMPONENT(Engine::Input::OISInputHandler);

METATABLE_BEGIN(Engine::Input::OISInputHandler)
METATABLE_END(Engine::Input::OISInputHandler)



namespace Engine {
namespace Input {
    OISInputHandler::OISInputHandler(Window::MainWindow &topLevel, Window::OSWindow *window, InputListener *listener)
        : UniqueComponent(topLevel, listener)
        , mWindow(window)
    {
        //LOG("*** Initializing OIS ***");
        OIS::ParamList pl;
        std::ostringstream windowHndStr;

        windowHndStr << (size_t)mWindow->mHandle;
        pl.insert(make_pair("WINDOW"s, windowHndStr.str()));

#if WINDOWS
        pl.insert(std::make_pair("w32_mouse"s, "DISCL_FOREGROUND"s));
        pl.insert(std::make_pair("w32_mouse"s, "DISCL_NONEXCLUSIVE"s));
        pl.insert(std::make_pair("w32_keyboard"s, "DISCL_FOREGROUND"s));
        pl.insert(std::make_pair("w32_keyboard"s, "DISCL_NONEXCLUSIVE"s));
#elif LINUX
        pl.insert(std::make_pair("x11_mouse_grab"s, "false"s));
        pl.insert(std::make_pair("x11_mouse_hide"s, "false"s));
#endif

        mInputManager = OIS::InputManager::createInputSystem(pl);

        mKeyboard = static_cast<OIS::Keyboard *>(mInputManager->createInputObject(
            OIS::OISKeyboard, true));
        mMouse = static_cast<OIS::Mouse *>(mInputManager->createInputObject(
            OIS::OISMouse, true));

        mMouse->setEventCallback(this);
        mKeyboard->setEventCallback(this);

        //window->addListener(this);
        //Set initial mouse clipping size
        onResize(mWindow->width(), mWindow->height());
    }

    OISInputHandler::~OISInputHandler()
    {
        //mWindow->removeListener(this);

        mInputManager->destroyInputObject(mMouse);
        mInputManager->destroyInputObject(mKeyboard);

        OIS::InputManager::destroyInputSystem(mInputManager);
        mInputManager = nullptr;
    }

    bool OISInputHandler::isKeyDown(Key key)
    {
        return mKeyboard->isKeyDown(static_cast<OIS::KeyCode>(key));
    }

    MouseButton::MouseButton OISInputHandler::convertMouseButton(OIS::MouseButtonID id)
    {
        switch (id) {
        case OIS::MB_Left:
            return MouseButton::LEFT_BUTTON;
        case OIS::MB_Right:
            return MouseButton::RIGHT_BUTTON;
        case OIS::MB_Middle:
            return MouseButton::MIDDLE_BUTTON;
        default:
            std::terminate();
        }
    }

    bool OISInputHandler::keyPressed(const OIS::KeyEvent &arg)
    {
        return injectKeyPress({ static_cast<Key>(arg.key), static_cast<char>(arg.text) });
    }

    bool OISInputHandler::keyReleased(const OIS::KeyEvent &arg)
    {
        return injectKeyRelease({ static_cast<Key>(arg.key) });
    }

    bool OISInputHandler::mousePressed(const OIS::MouseEvent &arg,
        OIS::MouseButtonID id)
    {
        return injectPointerPress({ { static_cast<float>(arg.state.X.abs + mWindow->renderX()), static_cast<float>(arg.state.Y.abs + mWindow->renderY()) },
            convertMouseButton(id) });
    }

    bool OISInputHandler::mouseMoved(const OIS::MouseEvent &arg)
    {
        return injectPointerMove({ { static_cast<float>(arg.state.X.abs + mWindow->renderX()), static_cast<float>(arg.state.Y.abs + mWindow->renderY()) },
            { static_cast<float>(arg.state.X.rel * mMouseScale), static_cast<float>(arg.state.Y.rel * mMouseScale) },
            arg.state.Z.rel / 120.0f });
    }

    bool OISInputHandler::mouseReleased(const OIS::MouseEvent &arg,
        OIS::MouseButtonID id)
    {
        return injectPointerRelease({ { static_cast<float>(arg.state.X.abs + mWindow->renderX()), static_cast<float>(arg.state.Y.abs + mWindow->renderY()) },
            convertMouseButton(id) });        
    }

    bool OISInputHandler::frameStarted(std::chrono::microseconds timeSinceLastFrame)
    {
        PROFILE();
        //Need to capture/update each device
        mKeyboard->capture();
        mMouse->capture();
        return true;
    }

    bool OISInputHandler::frameRenderingQueued(std::chrono::microseconds timeSinceLastFrame, Threading::ContextMask context)
    {        
        return true;
    }

    void OISInputHandler::onResize(size_t width, size_t height)
    {
        const OIS::MouseState &ms = mMouse->getMouseState();
        ms.width = static_cast<int>(width);
        ms.height = static_cast<int>(height);

        mMouseScale = (width / 640.0f + height / 480.0f) / 2;
    }

}
}

