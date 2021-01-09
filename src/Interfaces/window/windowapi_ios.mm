#include "../interfaceslib.h"

#include "windowapi.h"
#include "windowsettings.h"

#include <UIKit/UIKit.h>

#if __has_feature(objc_arc)
#error "ARC is on!"
#endif

namespace Engine{
namespace Window{
struct IOSWindow;
}
}

/*@interface Cocoa_WindowListener : NSObject <UIWindowDelegate>{
    Engine::Window::OSXWindow *mWindow;
}

- (void)listen:(Engine::Window::OSXWindow *)window;
@end*/

namespace Engine {
namespace Window {

    DLL_EXPORT const PlatformCapabilities platformCapabilities {
        true,
        static_cast<float>([[UIScreen mainScreen] scale])
    };


    struct IOSWindow : OSWindow {
        IOSWindow(UIWindow *handle)
            : OSWindow((uintptr_t)handle)
            //, mListener([[Cocoa_WindowListener alloc] init])
        {
            //[mListener listen:this];
        }
        
        ~IOSWindow(){
            //[mListener dealloc];
        }

        /*bool handle(const XEvent &e)
        {
            switch (e.type) {
            case ConfigureNotify: {
                const XConfigureEvent &xce = e.xconfigure;

                /* This event type is generated for a variety of
                       happenings, so check whether the window has been
                       resized. */
/*
                if (xce.width != mWidth || xce.height != mHeight) {
                    mWidth = xce.width;
                    mHeight = xce.height;
                    onResize(mWidth, mHeight);
                }
                break;
            }
            case ClientMessage: {
                const XClientMessageEvent &xcme = e.xclient;
                if (xcme.data.l[0] == WM_DELETE_WINDOW) {
                    onClose();
                }
                break;
            }
            case DestroyNotify:
                return false;
            }
            return true;
        }*/

        virtual InterfacesVector size() override
        {
            CGRect rect = [reinterpret_cast<UIWindow*>(mHandle) frame];
            return {static_cast<int>(rect.size.width), static_cast<int>(rect.size.height)};
        }

        virtual InterfacesVector renderSize() override
        {
            //TODO
            return {static_cast<int>(size().x * platformCapabilities.mScalingFactor), static_cast<int>(size().y * platformCapabilities.mScalingFactor)};
        }

        virtual InterfacesVector pos() override
        {
            CGRect rect = [reinterpret_cast<UIWindow*>(mHandle) frame];
            return {static_cast<int>(rect.origin.x), static_cast<int>([[reinterpret_cast<UIWindow*>(mHandle) screen] bounds].size.height - rect.origin.y - rect.size.height)};
        }

        virtual InterfacesVector renderPos() override
        {
            return {static_cast<int>(pos().x * platformCapabilities.mScalingFactor), static_cast<int>(pos().y * platformCapabilities.mScalingFactor)};
        }

        virtual void setSize(const InterfacesVector &size) override
        {
            CGRect rect{static_cast<double>(pos().x), static_cast<double>(pos().y), static_cast<double>(size.x), static_cast<double>(size.y)};
            [reinterpret_cast<UIWindow*>(mHandle) setBounds:rect];
        }

        virtual void setRenderSize(const InterfacesVector &size) override
        {
            setSize({static_cast<int>(size.x / platformCapabilities.mScalingFactor), static_cast<int>(size.y / platformCapabilities.mScalingFactor)});
        }

        virtual void setPos(const InterfacesVector &size) override
        {
        }

        virtual void setRenderPos(const InterfacesVector &size) override
        {
        }

        virtual void show() override
        {
            [reinterpret_cast<UIWindow*>(mHandle) makeKeyAndVisible];
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

        virtual void destroy() override
        {
            [reinterpret_cast<UIWindow*>(mHandle) close];
        }
        
        virtual void captureInput() override
        {
            
        }
        
        virtual void releaseInput() override
        {
            
        }

        virtual bool isMaximized() override
        {
            return false;
        }
        
        virtual bool isFullscreen() override
        {
            return false;
        }
        
        virtual void update() override
        {
            const CFTimeInterval seconds = 0.000002;

            SInt32 result;
            do {
                result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, TRUE);
            } while (result == kCFRunLoopRunHandledSource);

            do {
                result = CFRunLoopRunInMode((CFStringRef)UITrackingRunLoopMode, seconds, TRUE);
            } while(result == kCFRunLoopRunHandledSource);
        }
        
        virtual bool isKeyDown(Input::Key::Key key) override
        {
            return false;
        }
        
        using OSWindow::onClose;
        using OSWindow::onRepaint;
        using OSWindow::onResize;

    private:
        InterfacesVector mLastKnownMousePos;
        //Cocoa_WindowListener *mListener;
    };


    static std::unordered_map<UIWindow *, IOSWindow> sWindows;

    OSWindow *sCreateWindow(const WindowSettings &settings)
    {
        CGRect rect = CGRectMake(0, 0, settings.mData.mSize.x, settings.mData.mSize.y);
        if (settings.mData.mPosition.x != -1 || settings.mData.mPosition.y != -1) {
            rect.origin.x = settings.mData.mPosition.x;
            rect.origin.y = settings.mData.mPosition.y;
        }
        
        UIWindow *handle = [[UIWindow alloc] initWithFrame:rect];


        //[handle setTitle:[NSString stringWithUTF8String:settings.mTitle]];
        
        //[handle cascadeTopLeftFromPoint:NSMakePoint(20,20)];
        [handle makeKeyAndVisible];
        
        auto pib = sWindows.try_emplace(handle, handle);
        assert(pib.second);
        
        return &pib.first->second;
    }

    OSWindow *sFromNative(uintptr_t handle)
    {
        return handle ? &sWindows.at((UIWindow*)handle) : nullptr;
    }

    static std::vector<MonitorInfo> sBuffer;

    static void updateMonitors()
    {
        sBuffer.clear();
        for (UIScreen *screen in [UIScreen screens]){
            CGRect frame = [screen bounds];
            sBuffer.push_back({static_cast<int>(frame.origin.x), static_cast<int>(frame.origin.y), static_cast<int>(frame.size.width), static_cast<int>(frame.size.height)});
        }
    }
    
    std::vector<MonitorInfo> listMonitors()
    {
        if (sBuffer.empty())
            updateMonitors();
        return sBuffer;
    }


}
}