#include "../clientlib.h"
#include "serialize/filesystem/filesystemlib.h"

#include "mainwindow.h"

#include "Interfaces/window/windowapi.h"

#include "../render/rendercontext.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Meta/serialize/serializetable_impl.h"

#include "toolwindow.h"

#include "Interfaces/util/exception.h"

#include "Generic/container/reverseIt.h"

#include "../render/rendertarget.h"

#include "mainwindowcomponent.h"

#include "Meta/serialize/configs/controlled.h"

#include "Interfaces/filesystem/api.h"
#include "Interfaces/window/windowsettings.h"

#include "serialize/filesystem/filemanager.h"

#include "Meta/serialize/formatter/iniformatter.h"

#include "Meta/serialize/serializableunitptr.h"

METATABLE_BEGIN(Engine::Window::MainWindow)
READONLY_PROPERTY(Components, components)
METATABLE_END(Engine::Window::MainWindow)

SERIALIZETABLE_BEGIN(Engine::Window::MainWindow)
FIELD(mComponents, Serialize::ControlledConfig<KeyCompare<std::unique_ptr<Engine::Window::MainWindowComponentBase>>>)
SERIALIZETABLE_END(Engine::Window::MainWindow)

namespace Engine {
namespace Window {

    bool MainWindowComponentComparator::operator()(const std::unique_ptr<MainWindowComponentBase> &first, const std::unique_ptr<MainWindowComponentBase> &second) const
    {
        return first->mPriority < second->mPriority;
    }

    int MainWindowComponentComparator::traits::to_cmp_type(const item_type &value)
    {
        return value->mPriority;
    }

    MainWindow::MainWindow(const WindowSettings &settings)
        : mSettings(settings)
        , mComponents(*this)
    {

        mLoop.addSetupSteps(
            [this]() {
                if (!callInit())
                    return false;
                sync();
                return true;
            },
            [this]() {
                unsync();
                callFinalize();
            });
    }

    MainWindow::~MainWindow()
    {
    }

    Rect2i MainWindow::getScreenSpace()
    {
        if (!mOsWindow)
            return { { 0, 0 }, { 0, 0 } };
        InterfacesVector pos = mOsWindow->renderPos(), size = mOsWindow->renderSize();
        return {
            { pos.x, pos.y }, { size.x, size.y }
        };
    }

    bool MainWindow::init()
    {
        WindowSettings settings = mSettings;

        Filesystem::FileManager mgr { "MainWindow-Layout" };

        if (Serialize::SerializeInStream in = mgr.openRead(Filesystem::appDataPath() / "mainwindow.ini", std::make_unique<Serialize::IniFormatter>())) {
            in >> settings.mData;
        }

        mOsWindow = sCreateWindow(settings);

        mOsWindow->addListener(this);

        assert(!mRenderContext);
        mRenderContext.emplace(&mLoop);
        mRenderWindow = (*mRenderContext)->createRenderWindow(mOsWindow);

        addFrameListener(this);

        for (const std::unique_ptr<MainWindowComponentBase> &comp : components()) {
            bool result = comp->callInit();
            assert(result);
        }

        return true;
    }

    void MainWindow::finalize()
    {
        Filesystem::FileManager mgr { "MainWindow-Layout" };

        if (Serialize::SerializeOutStream out = mgr.openWrite(Filesystem::appDataPath() / "mainwindow.ini", std::make_unique<Serialize::IniFormatter>())) {
            out << mOsWindow->data();
        }

        for (const std::unique_ptr<MainWindowComponentBase> &comp : reverseIt(components())) {
            comp->callFinalize();
        }

        removeFrameListener(this);

        mRenderWindow.reset();
        mRenderContext.reset();

        mOsWindow->removeListener(this);
        mOsWindow->destroy();
        mOsWindow = nullptr;
    }

    ToolWindow *MainWindow::createToolWindow(const WindowSettings &settings)
    {
        return mToolWindows.emplace_back(std::make_unique<ToolWindow>(*this, settings)).get();
    }

    void MainWindow::destroyToolWindow(ToolWindow *w)
    {
        auto it = std::find_if(mToolWindows.begin(), mToolWindows.end(), [=](const std::unique_ptr<ToolWindow> &ptr) { return ptr.get() == w; });
        assert(it != mToolWindows.end());
        mToolWindows.erase(it);
    }

    bool MainWindow::injectKeyPress(const Input::KeyEventArgs &arg)
    {
        for (const std::unique_ptr<MainWindowComponentBase> &comp : reverseIt(components())) {
            if (comp->injectKeyPress(arg))
                return true;
        }
        return false;
    }

    bool MainWindow::injectKeyRelease(const Input::KeyEventArgs &arg)
    {
        for (const std::unique_ptr<MainWindowComponentBase> &comp : reverseIt(components())) {
            if (comp->injectKeyRelease(arg))
                return true;
        }
        return false;
    }

    bool MainWindow::injectPointerPress(const Input::PointerEventArgs &arg)
    {

        for (const std::unique_ptr<MainWindowComponentBase> &comp : reverseIt(components())) {
            if (comp->injectPointerPress(arg))
                return true;
        }

        return false;
    }

    bool MainWindow::injectPointerRelease(const Input::PointerEventArgs &arg)
    {

        for (const std::unique_ptr<MainWindowComponentBase> &comp : reverseIt(components())) {
            if (comp->injectPointerRelease(arg))
                return true;
        }

        return false;
    }

    bool MainWindow::injectPointerMove(const Input::PointerEventArgs &arg)
    {

        for (const std::unique_ptr<MainWindowComponentBase> &comp : reverseIt(components())) {
            if (comp->injectPointerMove(arg))
                return true;
        }

        return false;
    }

    bool MainWindow::injectAxisEvent(const Input::AxisEventArgs &arg)
    {
        for (const std::unique_ptr<MainWindowComponentBase> &comp : reverseIt(components())) {
            if (comp->injectAxisEvent(arg))
                return true;
        }

        return false;
    }

    OSWindow *MainWindow::osWindow() const
    {
        return mOsWindow;
    }

    Render::RenderContext *MainWindow::getRenderer()
    {
        return *mRenderContext;
    }

    void MainWindow::onClose()
    {
        callFinalize();
    }

    void MainWindow::onRepaint()
    {
        //update();
    }

    void MainWindow::onResize(const InterfacesVector &size)
    {
        mRenderWindow->resize({ size.x, size.y });
        applyClientSpaceResize();
    }

    void MainWindow::applyClientSpaceResize(MainWindowComponentBase *component)
    {
        if (!mOsWindow)
            return;

        Rect2i space;
        if (!component) {
            InterfacesVector size = mOsWindow->renderSize();
            space = {
                { 0, 0 }, { size.x, size.y }
            };
        } else
            space = component->getChildClientSpace();

        for (const std::unique_ptr<MainWindowComponentBase> &comp : reverseIt(components())) {
            if (component) {
                if (component == comp.get()) {
                    component = nullptr;
                }
            } else {
                comp->onResize(space);
                space = comp->getChildClientSpace();
            }
        }
    }

    bool MainWindow::frameStarted(std::chrono::microseconds)
    {
        return true;
    }

    bool MainWindow::frameRenderingQueued(std::chrono::microseconds, Threading::ContextMask)
    {

        return mOsWindow;
    }

    bool MainWindow::frameEnded(std::chrono::microseconds)
    {
        //TODO
        if (mRenderContext)
            (*mRenderContext)->render();
        mOsWindow->update();
        for (std::unique_ptr<ToolWindow> &window : mToolWindows)
            window->osWindow()->update();
        return mOsWindow;
    }

    MainWindowComponentBase &MainWindow::getWindowComponent(size_t i, bool init)
    {
        return getChild(mComponents.get(i), init);
    }

    void MainWindow::addFrameListener(Threading::FrameListener *listener)
    {
        mLoop.addFrameListener(listener);
    }

    void MainWindow::removeFrameListener(Threading::FrameListener *listener)
    {
        mLoop.removeFrameListener(listener);
    }

    void MainWindow::singleFrame()
    {
        mLoop.singleFrame();
    }

    Threading::FrameLoop &MainWindow::frameLoop()
    {
        return mLoop;
    }

    void MainWindow::shutdown()
    {
        mLoop.shutdown();
    }

    Render::RenderTarget *MainWindow::getRenderWindow()
    {
        return mRenderWindow.get();
    }

}
}
