#include "../interfaceslib.h"

#if WINDOWS

#include "windowapi.h"
#include "windoweventlistener.h"

#define NOMINMAX
#include <Windows.h>

namespace Engine {
namespace Window {

	DLL_EXPORT const PlatformCapabilities platformCapabilities {
        true
    };

    struct WindowsWindow : Window {
        WindowsWindow(HWND hwnd)
            : Window((uintptr_t)hwnd)
        {
        }

        bool handle(UINT msg, WPARAM wParam, LPARAM lParam)
        {
            switch (msg) {
            case WM_SIZE:
                onResize(LOWORD(lParam), HIWORD(lParam));
                break;
            case WM_CLOSE:
                onClose();
                break;
            case WM_DESTROY:
                return false;
                break;
            case WM_PAINT:
                onRepaint();
                break;
            }
            return true;
        }

        virtual int width() override
        {
            RECT rect;
            auto result = GetWindowRect((HWND)mHandle, &rect);
            assert(result);
            return rect.right - rect.left;
        }

        virtual int height() override
        {
            RECT rect;
            auto result = GetWindowRect((HWND)mHandle, &rect);
            assert(result);
            return rect.bottom - rect.top;
        }

        virtual int renderWidth() override
        {
            RECT rect;
            auto result = GetClientRect((HWND)mHandle, &rect);
            assert(result);
            return rect.right - rect.left;
        }

        virtual int renderHeight() override
        {
            RECT rect;
            auto result = GetClientRect((HWND)mHandle, &rect);
            assert(result);
            return rect.bottom - rect.top;
        }

        virtual void beginFrame() override
        {
            BeginPaint((HWND)mHandle, &mPs);
        }

        virtual void endFrame() override
        {
            EndPaint((HWND)mHandle, &mPs);
            SwapBuffers(GetDC((HWND)mHandle));
        }

        virtual void destroy() override
        {
            DestroyWindow((HWND)mHandle);
        }

        virtual int x() override
        {
            RECT rect;
            auto result = GetWindowRect((HWND)mHandle, &rect);
            assert(result);
            return rect.left;
        }
        virtual int y() override
        {
            RECT rect;
            auto result = GetWindowRect((HWND)mHandle, &rect);
            assert(result);
            return rect.top;
        }
        virtual int renderX() override
        {
            POINT p { 0, 0 };
            auto result = ClientToScreen((HWND)mHandle, &p);
            assert(result);
            return p.x;
        }
        virtual int renderY() override
        {
            POINT p { 0, 0 };
            auto result = ClientToScreen((HWND)mHandle, &p);
            assert(result);
            return p.y;
        }
        virtual void setSize(int width, int height) override
        {
            MoveWindow((HWND)mHandle, x(), y(), width, height, true);
        }
        virtual void setRenderSize(int width, int height) override
        {
            RECT r;
            auto result = GetClientRect((HWND)mHandle, &r);
            assert(result);
            RECT r2;
            result = GetWindowRect((HWND)mHandle, &r2);
            assert(result);
            MoveWindow((HWND)mHandle, x(), y(), width + ((r2.right - r2.left) - (r.right - r.left)), height + ((r2.bottom - r2.top) - (r.bottom - r.top)), true);
        }
        virtual void setPos(int x, int y) override
        {
            MoveWindow((HWND)mHandle, x, y, width(), height(), true);
        }
        virtual void setRenderPos(int x, int y) override
        {
            RECT r;
            auto result = GetWindowRect((HWND)mHandle, &r);
            assert(result);
            POINT p { 0, 0 };
            result = ClientToScreen((HWND)mHandle, &p);
            assert(result);
            result = MoveWindow((HWND)mHandle, x - (p.x - r.left), y - (p.y - r.top), width(), height(), true);
            assert(result);
        }

        virtual void show() override
        {
            ShowWindow((HWND)mHandle, SW_SHOW);
        }

        virtual bool isMinimized() override
        {
            WINDOWPLACEMENT placement;
            auto result = GetWindowPlacement((HWND)mHandle, &placement);
            assert(result);
            return placement.showCmd == SW_MINIMIZE;
        }

        virtual void focus() override
        {
            SetFocus((HWND)mHandle);
        }
        virtual bool hasFocus() override
        {
            return GetFocus() == (HWND)mHandle;
        }

        virtual void setTitle(const char *title) override
        {
            SetWindowTextA((HWND)mHandle, title);
        }

        PAINTSTRUCT mPs;
    };

    static std::unordered_map<HWND, WindowsWindow> sWindows;

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static const char *CreateWindowClass()
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        // Register class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = "InterfacesWindowClass";
        wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_APPLICATION);
        if (!RegisterClassEx(&wcex))
            return nullptr;
        return wcex.lpszClassName;
    }

    Window *sCreateWindow(const WindowSettings &settings)
    {
        HWND handle = (HWND)settings.mHandle;
        if (!handle) {
            static const char *windowClass = CreateWindowClass();

            // Create window
            HINSTANCE hInstance = GetModuleHandle(nullptr);
            RECT rc = { 0, 0, (LONG)settings.mSize.x, (LONG)settings.mSize.y };
            AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
            int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
            if (settings.mPosition) {
                x = settings.mPosition->x;
                y = settings.mPosition->y;
            }
            DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;
            if (settings.mHeadless) {
                style = WS_POPUP;
            }
            handle = CreateWindow(windowClass, settings.mTitle,
                style,
                x, y, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                nullptr);
            if (!handle)
                return nullptr;
        }

        if (!settings.mHidden)
            ShowWindow(handle, SW_SHOW);

        auto pib = sWindows.try_emplace(handle, handle);
        assert(pib.second);

        return &pib.first->second;
    }

    void sUpdate()
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    Window *sFromNative(uintptr_t handle)
    {
        return handle ? &sWindows.at((HWND)handle) : nullptr;
    }

    static std::vector<MonitorInfo> sBuffer;

    static void updateMonitors()
    {
        sBuffer.clear();
        auto result = EnumDisplayMonitors(
            NULL, NULL, [](HMONITOR, HDC, LPRECT rect, LPARAM) {
                sBuffer.push_back({ rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top });
                return BOOL(true);
            },
            NULL);
        assert(result);
    }

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto it = sWindows.find(hwnd);
        if (it != sWindows.end()) {
            if (msg == WM_DISPLAYCHANGE) {
                if (it == sWindows.begin())
                    updateMonitors();
            } else if (!it->second.handle(msg, wParam, lParam))
                sWindows.erase(it);
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    std::vector<MonitorInfo> listMonitors()
    {
        if (sBuffer.empty())
            updateMonitors();
        return sBuffer;
    }

}
}

#endif