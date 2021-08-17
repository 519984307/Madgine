#pragma once

#include "Madgine/window/mainwindowcomponentcollector.h"
#include "Madgine/window/mainwindowcomponent.h"

#include "Meta/serialize/container/serializablecontainer.h"


#include "Madgine/render/renderpass.h"

#include "Generic/container/transformIt.h"



namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETS_EXPORT WidgetManager : Window::MainWindowComponent<WidgetManager>,
                                                 Render::RenderPass {

        SERIALIZABLEUNIT(WidgetManager);

        WidgetManager(Window::MainWindow &window);
        WidgetManager(const WidgetManager &) = delete;
        ~WidgetManager();

        void swapCurrentRoot(WidgetBase *newRoot);
        void openModalWidget(WidgetBase *widget);
        void closeModalWidget(WidgetBase *widget);
        void openWidget(WidgetBase *widget);
        void closeWidget(WidgetBase *widget);

        bool isHovered(WidgetBase *w);
        WidgetBase *hoveredWidget();

        WidgetBase *getWidget(const std::string &name);

        void registerWidget(WidgetBase *w);
        void updateWidget(WidgetBase *w, const std::string &newName);
        void unregisterWidget(WidgetBase *w);

        virtual std::string_view key() const override;

        virtual bool init() override;
        virtual void finalize() override;

        WidgetBase *currentRoot();

        void clear();

        void destroyTopLevel(WidgetBase *w);
        template <typename WidgetType = WidgetBase>
        WidgetType *createTopLevel(const std::string &name);

        decltype(auto) widgets()
        {
            return uniquePtrToPtr(mTopLevelWidgets);
        }

        WidgetBase *mStartupWidget = nullptr;
        void openStartupWidget();

        virtual bool injectPointerPress(const Input::PointerEventArgs &arg) override;
        virtual bool injectPointerRelease(const Input::PointerEventArgs &arg) override;
        virtual bool injectPointerMove(const Input::PointerEventArgs &arg) override;
        virtual bool injectAxisEvent(const Input::AxisEventArgs &arg) override;

        virtual void onResize(const Rect2i &space) override;
        virtual void render(Render::RenderTarget *target) override;

		virtual int priority() const override;

        Render::Texture &uiTexture() const;

        static const char *widgetCreationNames(size_t index);

    protected:
        WidgetBase *getHoveredWidget(const Vector2 &pos, WidgetBase *current);
        WidgetBase *getHoveredWidgetUp(const Vector2 &pos, WidgetBase *current);
        WidgetBase *getHoveredWidgetDown(const Vector2 &pos, WidgetBase *current);
        bool propagateInput(WidgetBase *w, const Input::PointerEventArgs &arg, bool (WidgetBase::*f)(const Input::PointerEventArgs &));

        
        std::unique_ptr<WidgetBase> createWidgetClass(const std::string &name, WidgetClass _class, WidgetBase *parent = nullptr);
        std::tuple<std::unique_ptr<WidgetBase>> createWidgetClassTuple(const std::string &name, WidgetClass _class);
        std::tuple<std::string, WidgetClass> storeWidgetCreationData(const std::unique_ptr<WidgetBase> &widget) const;

        template <typename WidgetType = WidgetBase>
        std::unique_ptr<WidgetType> create(const std::string &name, WidgetBase *parent = nullptr);
        
        friend struct WidgetBase;
        
    private:
        std::map<std::string, WidgetBase *> mWidgets;

        SERIALIZABLE_CONTAINER(mTopLevelWidgets, std::vector<std::unique_ptr<WidgetBase>>, NoOpFunctor);

        WidgetBase *mHoveredWidget = nullptr;

        WidgetBase *mCurrentRoot = nullptr;

        std::vector<WidgetBase *> mModalWidgetList;

        void expandUIAtlas();
        void onActivate(bool active);

        struct WidgetManagerData;
        std::unique_ptr<WidgetManagerData> mData;

    };

}
}

RegisterType(Engine::Widgets::WidgetManager);