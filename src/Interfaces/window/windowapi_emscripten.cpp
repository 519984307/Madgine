#include "../interfaceslib.h"

#if EMSCRIPTEN

#    include "windowapi.h"
#    include "windowsettings.h"

#    include <EGL/egl.h>

#    include <emscripten/html5.h>

#    include "../threading/systemvariable.h"

#    include "../input/inputevents.h"

namespace Engine {
namespace Window {

    DLL_EXPORT const PlatformCapabilities platformCapabilities {
        false
    };

    //DLL_EXPORT Threading::SystemVariable<ANativeWindow*> sNativeWindow = nullptr;

    DLL_EXPORT EGLDisplay sDisplay = EGL_NO_DISPLAY;

    static struct DisplayGuard {
        DisplayGuard()
        {
            sDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (sDisplay != EGL_NO_DISPLAY) {
                if (!eglInitialize(sDisplay, nullptr, nullptr))
                    sDisplay = EGL_NO_DISPLAY;
            }
        }

        ~DisplayGuard()
        {
            if (sDisplay != EGL_NO_DISPLAY)
                eglTerminate(sDisplay);
        }
    } sDisplayGuard;

    struct EmscriptenWindow : OSWindow {
        EmscriptenWindow(EGLSurface surface)
            : OSWindow((uintptr_t)surface)
            , mKeyDown {}
        {
            EGLint width;
            EGLint height;
            if (!eglQuerySurface(sDisplay, surface, EGL_WIDTH, &width) || !eglQuerySurface(sDisplay, surface, EGL_HEIGHT, &height))
                std::terminate();
            mWidth = width;
            mHeight = height;

            //Input
            emscripten_set_mousemove_callback("#canvas", this, 0, EmscriptenWindow::handleMouseEvent);

            emscripten_set_mousedown_callback("#canvas", this, 0, EmscriptenWindow::handleMouseEvent);
            emscripten_set_mouseup_callback("#document", this, 0, EmscriptenWindow::handleMouseEvent);

            emscripten_set_keydown_callback("#window", this, 0, EmscriptenWindow::handleKeyEvent);
            emscripten_set_keyup_callback("#window", this, 0, EmscriptenWindow::handleKeyEvent);
        }

        virtual void update() override
        {
        }

        virtual int width() override
        {
            return mWidth;
        }

        virtual int height() override
        {
            return mHeight;
        }

        virtual int renderWidth() override
        {
            //TODO
            return width();
        }

        virtual int renderHeight() override
        {
            //TODO
            return height();
        }

        virtual void swapBuffers() override
        {
            eglSwapBuffers(sDisplay, (EGLSurface)mHandle);
        }

        virtual int x() override
        {
            return 0;
        }

        virtual int y() override
        {
            return 0;
        }

        virtual int renderX() override
        {
            return 0;
        }

        virtual int renderY() override
        {
            return 0;
        }

        virtual void setSize(int width, int height) override
        {
            mWidth = width;
            mHeight = height;
            emscripten_set_canvas_element_size("#canvas", mWidth, mHeight);
        }

        virtual void setRenderSize(int width, int height) override
        {
            setSize(width, height);
        }

        virtual void setPos(int width, int height) override
        {
        }

        virtual void setRenderPos(int width, int height) override
        {
        }

        virtual void show() override
        {
        }

        virtual bool isMinimized() override
        {
            return false;
        }

        virtual void focus() override
        {
        }

        virtual bool hasFocus() override
        {
            return true;
        }

        virtual void setTitle(const char *title) override
        {
        }

        virtual void destroy() override;

        //Input
        virtual bool isKeyDown(Input::Key::Key key) override
        {
            return mKeyDown[key];
        }

        static Input::MouseButton::MouseButton convertMouseButton(unsigned short id)
        {
            switch (id) {
            case 0:
                return Input::MouseButton::LEFT_BUTTON;
            case 1:
                return Input::MouseButton::MIDDLE_BUTTON;
            case 2:
                return Input::MouseButton::RIGHT_BUTTON;
            default:
                std::terminate();
            }
        }

        static EM_BOOL handleMouseEvent(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
        {
            EmscriptenWindow *_this = static_cast<EmscriptenWindow *>(userData);

            switch (eventType) {
            case EMSCRIPTEN_EVENT_MOUSEMOVE:
                return _this->injectPointerMove({ { static_cast<float>(mouseEvent->canvasX), static_cast<float>(mouseEvent->canvasY) },
                    { static_cast<float>(mouseEvent->movementX), static_cast<float>(mouseEvent->movementY) } });
            case EMSCRIPTEN_EVENT_MOUSEDOWN:
                return _this->injectPointerPress({ { static_cast<float>(mouseEvent->canvasX), static_cast<float>(mouseEvent->canvasY) },
                    convertMouseButton(mouseEvent->button) });
            case EMSCRIPTEN_EVENT_MOUSEUP:
                return _this->injectPointerRelease({ { static_cast<float>(mouseEvent->canvasX), static_cast<float>(mouseEvent->canvasY) },
                    convertMouseButton(mouseEvent->button) });
            }

            return EM_FALSE;
        }
        static EM_BOOL handleKeyEvent(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
        {
            EmscriptenWindow *_this = static_cast<EmscriptenWindow *>(userData);

            switch (eventType) {
            case EMSCRIPTEN_EVENT_KEYDOWN:
                _this->mKeyDown[keyEvent->keyCode] = true;
                return _this->injectKeyPress({ static_cast<Input::Key::Key>(keyEvent->code[0]), keyEvent->key[0] });
            case EMSCRIPTEN_EVENT_KEYUP:
                _this->mKeyDown[keyEvent->keyCode] = false;
                return _this->injectKeyRelease({ static_cast<Input::Key::Key>(keyEvent->code[0]) });
            }

            return EM_FALSE;
        }

    private:
        int mWidth;
        int mHeight;

        //Input
        bool mKeyDown[512];
    };

    static std::unordered_map<EGLSurface, EmscriptenWindow> sWindows;

    void EmscriptenWindow::destroy()
    {
        eglDestroySurface(sDisplay, (EGLSurface)mHandle);
        sWindows.erase((EGLSurface)mHandle);
    }

    EM_BOOL eventCallback(int type, const EmscriptenUiEvent *event, void *userData)
    {
        double w;
        double h;

        emscripten_get_element_css_size(nullptr, &w, &h);

        static_cast<EmscriptenWindow *>(userData)->setSize(static_cast<int>(w), static_cast<int>(h));
        return true;
    }

    OSWindow *sCreateWindow(const WindowSettings &settings)
    {
        assert(sDisplay);

        EGLSurface handle = (EGLSurface)settings.mHandle;
        if (!handle) {

            const EGLint attribs[] = {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
                EGL_NONE
            };

            EGLConfig config;
            EGLint numConfigs;
            EGLint format;

            if (!eglChooseConfig(sDisplay, attribs, &config, 1, &numConfigs))
                return nullptr;

            if (!eglGetConfigAttrib(sDisplay, config, EGL_NATIVE_VISUAL_ID, &format))
                return nullptr;

            //sNativeWindow.wait();

            //ANativeWindow_setBuffersGeometry(sNativeWindow, 0, 0, format);

            handle = eglCreateWindowSurface(sDisplay, config, 0, 0);
            if (!handle)
                return nullptr;
        }

        auto pib = sWindows.try_emplace(handle, handle);
        assert(pib.second);

        EmscriptenWindow *window = &pib.first->second;

        emscripten_set_resize_callback("#window", window, false, &eventCallback);

        double w;
        double h;

        emscripten_get_element_css_size(nullptr, &w, &h);
        window->setSize(static_cast<int>(w), static_cast<int>(h));

        return window;
    }

    OSWindow *sFromNative(uintptr_t handle)
    {
        return handle ? &sWindows.at((EGLSurface)handle) : nullptr;
    }

    std::vector<MonitorInfo> listMonitors()
    {
        double w;
        double h;

        emscripten_get_element_css_size(nullptr, &w, &h);

        MonitorInfo info { 0, 0, static_cast<int>(w), static_cast<int>(h) };

        return { info };
    }

}
}

#endif