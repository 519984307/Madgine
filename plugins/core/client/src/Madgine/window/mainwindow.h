#pragma once

#include "Interfaces/window/windoweventlistener.h"
#include "Modules/uniquecomponent/uniquecomponentselector.h"

#include "Modules/generic/container/transformIt.h"

#include "Modules/madgineobject/madgineobjectobserver.h"
#include "Modules/serialize/toplevelserializableunit.h"

#include "../render/rendercontextcollector.h"

#include "../threading/frameloop.h"

#include "mainwindowcomponentcollector.h"

#include "Modules/keyvalue/keyvalueset.h"

#include "Modules/keyvalue/scopebase.h"

#include "Modules/madgineobject/madgineobject.h"

#include "Modules/serialize/container/serializablecontainer.h"

#include "../threading/framelistener.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

namespace Engine {
namespace Window {

    template <typename OffsetPtr = TaggedPlaceholder<OffsetPtrTag, 0>>
    struct MainWindowComponentObserver : MadgineObjectObserver {
        template <typename It>
        void operator()(const It &it, int event)
        {
            MadgineObjectObserver::operator()(it, event);

            if (event == (AFTER | ACTIVATE_ITEM)) {
                auto end = OffsetPtr::parent(this)->mComponents.end();
                auto next = std::next(it);
                if (end == next)
                    OffsetPtr::parent(this)->applyClientSpaceResize();
                else
                    OffsetPtr::parent(this)->applyClientSpaceResize(next->get());
            }
            if (event == (AFTER | DEACTIVATE_ITEM)) {
                auto end = OffsetPtr::parent(this)->mComponents.end();
                if (end == it)
                    OffsetPtr::parent(this)->applyClientSpaceResize();
                else
                    OffsetPtr::parent(this)->applyClientSpaceResize(it->get());
            }
        }
    };

    struct MADGINE_CLIENT_EXPORT MainWindowComponentComparator {

        bool operator()(const std::unique_ptr<MainWindowComponentBase> &first, const std::unique_ptr<MainWindowComponentBase> &second) const;

        struct traits {
            using type = int;
            using cmp_type = MainWindowComponentComparator;
            using item_type = std::unique_ptr<MainWindowComponentBase>;

            static int to_cmp_type(const item_type &value);
        };
    };

    struct MADGINE_CLIENT_EXPORT MainWindow : ScopeBase,
                                                  WindowEventListener,
                                                  Threading::FrameListener,
                                                  Serialize::TopLevelSerializableUnit<MainWindow>,
                                                  MadgineObject<MainWindow> {
        SERIALIZABLEUNIT;

        MainWindow(const WindowSettings &settings);
        virtual ~MainWindow();

        Rect2i getScreenSpace();

        bool injectKeyPress(const Input::KeyEventArgs &arg) override;
        bool injectKeyRelease(const Input::KeyEventArgs &arg) override;
        bool injectPointerPress(const Input::PointerEventArgs &arg) override;
        bool injectPointerRelease(const Input::PointerEventArgs &arg) override;
        bool injectPointerMove(const Input::PointerEventArgs &arg) override;

        OSWindow *osWindow() const;

        decltype(auto) components()
        {
            return uniquePtrToPtr(mComponents);
        }

        Render::RenderContext *getRenderer();
        Render::RenderTarget *getRenderWindow();

        ToolWindow *createToolWindow(const WindowSettings &settings);
        void destroyToolWindow(ToolWindow *w);

        virtual bool frameStarted(std::chrono::microseconds) override;
        virtual bool frameRenderingQueued(std::chrono::microseconds, Threading::ContextMask) override;
        virtual bool frameEnded(std::chrono::microseconds) override;

        template <typename T>
        T &getWindowComponent(bool init = true)
        {
            return static_cast<T &>(getWindowComponent(component_index<T>(), init));
        }

        MainWindowComponentBase &getWindowComponent(size_t i, bool = true);

        bool init();
        void finalize();

        /**
			* \brief Adds a FrameListener to the application.
			* \param listener the FrameListener to be added.
			*/
        void addFrameListener(Threading::FrameListener *listener);
        /**
			* \brief Removes a FrameListener from the application.
			* \param listener the FrameListener to be removed.
			*/
        void removeFrameListener(Threading::FrameListener *listener);

        void singleFrame();

        void shutdown();

        Threading::FrameLoop &frameLoop();

        void applyClientSpaceResize(MainWindowComponentBase *component = nullptr);

    protected:
        void onClose() override;
        void onRepaint() override;
        void onResize(const InterfacesVector &size) override;

    private:
        const WindowSettings &mSettings;

        OFFSET_CONTAINER(mComponents, MainWindowComponentContainer<Serialize::SerializableContainer<KeyValueSet<Placeholder<0>, MainWindowComponentComparator>, MainWindowComponentObserver<>, std::true_type>>);

        std::vector<std::unique_ptr<ToolWindow>> mToolWindows;

        Threading::FrameLoop mLoop;

        OSWindow *mOsWindow = nullptr;
        std::optional<Render::RenderContextSelector<>> mRenderContext;
        std::unique_ptr<Render::RenderTarget> mRenderWindow;

        template <typename OffsetPtr>
        friend struct MainWindowComponentObserver;
    };

}
}
