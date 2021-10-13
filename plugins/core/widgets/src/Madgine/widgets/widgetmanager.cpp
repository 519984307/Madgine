#include "../widgetslib.h"

#include "widgetmanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "gpumeshloader.h"
#include "programloader.h"

#include "widget.h"

#include "bar.h"
#include "button.h"
#include "checkbox.h"
#include "combobox.h"
#include "image.h"
#include "label.h"
#include "scenewindow.h"
#include "tabwidget.h"
#include "textbox.h"

#include "imagedata.h"

#include "vertex.h"

#include "Madgine/window/mainwindow.h"

#include "Interfaces/window/windowapi.h"

#include "Meta/math/atlas2.h"

#include "imageloader.h"
#include "meshloader.h"
#include "programloader.h"
#include "textureloader.h"

#include "Madgine/render/shadinglanguage/sl.h"

#define SL_SHADER ui
#include INCLUDE_SL_SHADER

UNIQUECOMPONENT(Engine::Widgets::WidgetManager)

METATABLE_BEGIN(Engine::Widgets::WidgetManager)
READONLY_PROPERTY(Widgets, widgets)
MEMBER(mStartupWidget)
METATABLE_END(Engine::Widgets::WidgetManager)

SERIALIZETABLE_BEGIN(Engine::Widgets::WidgetManager)
FIELD(mStartupWidget)
FIELD(mTopLevelWidgets, Serialize::ParentCreator<&Engine::Widgets::WidgetManager::widgetCreationNames, &Engine::Widgets::WidgetManager::createWidgetClassTuple, &Engine::Widgets::WidgetManager::storeWidgetCreationData>)
SERIALIZETABLE_END(Engine::Widgets::WidgetManager)

namespace Engine {
namespace Widgets {

    struct WidgetManager::WidgetManagerData {

        Render::ProgramLoader::HandleType mProgram;
        Render::GPUMeshLoader::HandleType mMesh;

        Resources::ImageLoader::HandleType mDefaultTexture;
        Render::TextureLoader::HandleType mUIAtlasTexture;
        Atlas2 mUIAtlas { { 2048, 2048 } };
        int mUIAtlasSize = 0;
        std::map<Resources::ImageLoader::ResourceType *, Atlas2::Entry> mUIAtlasEntries;

        void expandUIAtlas()
        {
            if (mUIAtlasSize == 0) {
                mUIAtlasSize = 1;
                mUIAtlasTexture.setData({ mUIAtlasSize * 2048, mUIAtlasSize * 2048 }, {});
                for (int x = 0; x < mUIAtlasSize; ++x) {
                    for (int y = 0; y < mUIAtlasSize; ++y) {
                        mUIAtlas.addBin({ 2048 * x, 2048 * y });
                    }
                }
            } else {
                /*for (int x = 0; x < mUIAtlasSize; ++x) {
                for (int y = 0; y < mUIAtlasSize; ++y) {
                    mUIAtlas.addBin({ 512 * x, 512 * (y + mUIAtlasSize) });
                    mUIAtlas.addBin({ 512 * (x + mUIAtlasSize), 512 * y });
                    mUIAtlas.addBin({ 512 * (x + mUIAtlasSize), 512 * (y + mUIAtlasSize) });
                }
            }
            mUIAtlasSize *= 2;
            mUIAtlasTexture.resize({ 512 * mUIAtlasSize, 512 * mUIAtlasSize });*/
                throw "TODO";
            }
        }
    };

    WidgetManager::WidgetManager(Window::MainWindow &window)
        : VirtualUnit(window, 20)
        , mData(std::make_unique<WidgetManagerData>())
    {
    }

    WidgetManager::~WidgetManager()
    {
        assert(mWidgets.empty());
    }

    std::string_view WidgetManager::key() const
    {
        return "WidgetManager";
    }

    bool WidgetManager::init()
    {
        mData->mProgram.create("ui");

        mData->mProgram.setParametersSize(2, sizeof(WidgetsPerObject));

        mData->mMesh = Render::GPUMeshLoader::loadManual("widgetMesh", {}, [](Render::GPUMeshLoader *loader, Render::GPUMeshData &mesh, Render::GPUMeshLoader::ResourceDataInfo &info) {
            return loader->generate(mesh, { 3, std::vector<Vertex> {} });
        });

        mData->mUIAtlasTexture.create("widgetUIAtlas", Render::TextureType_2D, Render::FORMAT_RGBA8);

        mData->mDefaultTexture.load("default_tex");

        mWindow.getRenderWindow()->addRenderPass(this);

        return true;
    }

    void WidgetManager::finalize()
    {
        mWindow.getRenderWindow()->removeRenderPass(this);

        mTopLevelWidgets.clear();

        mData->mDefaultTexture.reset();

        mData->mUIAtlasTexture.reset();

        mData->mMesh.reset();

        mData->mProgram.reset();
    }

    template <typename WidgetType>
    std::unique_ptr<WidgetType> WidgetManager::create(const std::string &name, WidgetBase *parent)
    {
        return std::make_unique<WidgetType>(name, *this, parent);
    }

    template <typename WidgetType>
    WidgetType *WidgetManager::createTopLevel(const std::string &name)
    {
        std::unique_ptr<WidgetType> p = create<WidgetType>(name);
        WidgetType *w = p.get();
        w->hide();
        w->updateGeometry(mClientSpace);
        mTopLevelWidgets.emplace_back(std::move(p));
        return w;
    }

    template DLL_EXPORT WidgetBase *WidgetManager::createTopLevel<WidgetBase>(const std::string &);

    static constexpr std::array<const char *, 2> sWidgetCreationNames {
        "name",
        "type"
    };

    const char *WidgetManager::widgetCreationNames(size_t index)
    {
        
        return sWidgetCreationNames[index];
    }

    std::unique_ptr<WidgetBase> WidgetManager::createWidgetClass(const std::string &name, WidgetClass _class, WidgetBase *parent)
    {
        std::unique_ptr<WidgetBase> w = [=]() -> std::unique_ptr<WidgetBase> {
            switch (_class) {
            case WidgetClass::WIDGET_CLASS:
                return create<WidgetBase>(name, parent);
            case WidgetClass::BAR_CLASS:
                return create<Bar>(name, parent);
            case WidgetClass::CHECKBOX_CLASS:
                return create<Checkbox>(name, parent);
            case WidgetClass::LABEL_CLASS:
                return create<Label>(name, parent);
            case WidgetClass::TABWIDGET_CLASS:
                return create<TabWidget>(name, parent);
            case WidgetClass::BUTTON_CLASS:
                return create<Button>(name, parent);
            case WidgetClass::COMBOBOX_CLASS:
                return create<Combobox>(name, parent);
            case WidgetClass::TEXTBOX_CLASS:
                return create<Textbox>(name, parent);
            case WidgetClass::SCENEWINDOW_CLASS:
                return create<SceneWindow>(name, parent);
            case WidgetClass::IMAGE_CLASS:
                return create<Image>(name, parent);
            default:
                std::terminate();
            }
        }();
        if (!parent) {
            w->hide();
            w->updateGeometry(mClientSpace);
        }
        return w;
    }

    std::tuple<std::unique_ptr<WidgetBase>> WidgetManager::createWidgetClassTuple(const std::string &name, WidgetClass _class)
    {
        return { createWidgetClass(name, _class) };
    }

    std::tuple<std::string, WidgetClass> WidgetManager::storeWidgetCreationData(const std::unique_ptr<WidgetBase> &widget) const
    {
        return std::make_tuple(widget->getName(), widget->getClass());
    }

    bool WidgetManager::propagateInput(WidgetBase *w, const Input::PointerEventArgs &arg, bool (WidgetBase::*f)(const Input::PointerEventArgs &))
    {
        if (!w->mVisible)
            return false;

        if (!w->containsPoint(Vector2 { static_cast<float>(arg.windowPosition.x), static_cast<float>(arg.windowPosition.y) }, { { 0, 0 }, mClientSpace.mSize }))
            return false;

        for (WidgetBase *c : w->children()) {
            if (propagateInput(c, arg, f))
                return true;
        }
        return (w->*f)(arg);
    }

    bool WidgetManager::injectPointerPress(const Input::PointerEventArgs &arg)
    {
        Input::PointerEventArgs widgetArg = arg;
        widgetArg.windowPosition = { widgetArg.windowPosition.x - mClientSpace.mTopLeft.x, widgetArg.windowPosition.y - mClientSpace.mTopLeft.y };

        for (WidgetBase *modalWidget : mModalWidgetList) {
            if (propagateInput(modalWidget, widgetArg, &WidgetBase::injectPointerPress))
                return true;
        }

        for (WidgetBase *w : uniquePtrToPtr(mTopLevelWidgets)) {
            if (propagateInput(w, widgetArg, &WidgetBase::injectPointerPress))
                return true;
        }

        return false;
    }

    bool WidgetManager::injectPointerRelease(const Input::PointerEventArgs &arg)
    {
        Input::PointerEventArgs widgetArg = arg;
        widgetArg.windowPosition = { widgetArg.windowPosition.x - mClientSpace.mTopLeft.x, widgetArg.windowPosition.y - mClientSpace.mTopLeft.y };

        for (WidgetBase *modalWidget : mModalWidgetList) {
            if (propagateInput(modalWidget, widgetArg, &WidgetBase::injectPointerRelease))
                return true;
        }

        for (WidgetBase *w : uniquePtrToPtr(mTopLevelWidgets)) {
            if (propagateInput(w, widgetArg, &WidgetBase::injectPointerRelease))
                return true;
        }

        return false;
    }

    WidgetBase *WidgetManager::getHoveredWidgetUp(const Vector2 &pos, WidgetBase *current)
    {
        if (!current) {
            return nullptr;
        } else if (!current->mVisible || !current->containsPoint(pos, { { 0, 0 }, mClientSpace.mSize })) {
            return getHoveredWidgetUp(pos, current->getParent());
        } else {
            return current;
        }
    }

    WidgetBase *WidgetManager::getHoveredWidgetDown(const Vector2 &pos, WidgetBase *current)
    {
        LOG_WARNING_ONCE("Handle modal widgets for hover");

        const auto &widgets = current ? current->children() : uniquePtrToPtr(static_cast<const std::vector<std::unique_ptr<WidgetBase>> &>(mTopLevelWidgets));

        for (WidgetBase *w : widgets) {
            if (w->mVisible && w->containsPoint(pos, { { 0, 0 }, mClientSpace.mSize })) {
                return getHoveredWidgetDown(pos, w);
            }
        }
        return current;
    }

    WidgetBase *WidgetManager::getHoveredWidget(const Vector2 &pos, WidgetBase *current)
    {
        return getHoveredWidgetDown(pos, getHoveredWidgetUp(pos, current));
    }

    bool WidgetManager::injectPointerMove(const Input::PointerEventArgs &arg)
    {
        if (std::find_if(mWidgets.begin(), mWidgets.end(), [&](const std::pair<const std::string, WidgetBase *> &p) { return p.second == mHoveredWidget; }) == mWidgets.end())
            mHoveredWidget = nullptr;

        Input::PointerEventArgs widgetArg = arg;
        widgetArg.windowPosition = { widgetArg.windowPosition.x - mClientSpace.mTopLeft.x, widgetArg.windowPosition.y - mClientSpace.mTopLeft.y };

        WidgetBase *hoveredWidget = getHoveredWidget(Vector2 { static_cast<float>(widgetArg.windowPosition.x), static_cast<float>(widgetArg.windowPosition.y) }, mHoveredWidget);

        if (mHoveredWidget != hoveredWidget) {

            if (mHoveredWidget)
                mHoveredWidget->injectPointerLeave(widgetArg);

            mHoveredWidget = hoveredWidget;

            if (mHoveredWidget)
                mHoveredWidget->injectPointerEnter(widgetArg);
        }

        if (mHoveredWidget)
            return mHoveredWidget->injectPointerMove(widgetArg);

        return false;
    }

    bool WidgetManager::injectAxisEvent(const Input::AxisEventArgs &arg)
    {
        if (std::find_if(mWidgets.begin(), mWidgets.end(), [&](const std::pair<const std::string, WidgetBase *> &p) { return p.second == mHoveredWidget; }) == mWidgets.end())
            mHoveredWidget = nullptr;

        if (mHoveredWidget)
            return mHoveredWidget->injectAxisEvent(arg);

        return false;
    }

    WidgetBase *WidgetManager::currentRoot()
    {
        return mCurrentRoot;
    }

    void WidgetManager::destroyTopLevel(WidgetBase *w)
    {
        auto it = std::find_if(mTopLevelWidgets.begin(), mTopLevelWidgets.end(), [=](const std::unique_ptr<WidgetBase> &ptr) { return ptr.get() == w; });
        assert(it != mTopLevelWidgets.end());
        mTopLevelWidgets.erase(it);
    }

    void WidgetManager::clear()
    {
        mTopLevelWidgets.clear();
    }

    bool WidgetManager::isHovered(WidgetBase *w)
    {
        WidgetBase *hovered = mHoveredWidget;
        while (hovered) {
            if (hovered == w)
                return true;
            hovered = hovered->getParent();
        }
        return false;
    }

    WidgetBase *WidgetManager::hoveredWidget()
    {
        return mHoveredWidget;
    }

    WidgetBase *WidgetManager::getWidget(const std::string &name)
    {
        auto it = mWidgets.find(name);
        if (it == mWidgets.end())
            return nullptr;
        return it->second;
    }

    void WidgetManager::registerWidget(WidgetBase *w)
    {
        if (!w->getName().empty()) {
            mWidgets.try_emplace(w->getName(), w);
        }
    }

    void WidgetManager::updateWidget(WidgetBase *w, const std::string &newName)
    {
        unregisterWidget(w);
        if (!newName.empty()) {
            mWidgets.try_emplace(newName, w);
        }
    }

    void WidgetManager::unregisterWidget(WidgetBase *w)
    {
        if (!w->getName().empty()) {
            auto it = mWidgets.find(w->getName());
            assert(it != mWidgets.end());
            if (it->second == w) {
                mWidgets.erase(it);
            }
        }
    }

    void WidgetManager::swapCurrentRoot(WidgetBase *newRoot)
    {
        if (mCurrentRoot)
            mCurrentRoot->hide();
        mHoveredWidget = nullptr;
        mCurrentRoot = newRoot;
        newRoot->show();
    }

    void WidgetManager::openModalWidget(WidgetBase *widget)
    {
        widget->show();
        mModalWidgetList.emplace(mModalWidgetList.begin(), widget);
    }

    void WidgetManager::openWidget(WidgetBase *widget)
    {
        widget->show();
    }

    void WidgetManager::closeModalWidget(WidgetBase *widget)
    {
        assert(mModalWidgetList.size() > 0 && mModalWidgetList.front() == widget);
        widget->hide();
        mModalWidgetList.erase(mModalWidgetList.begin());
    }

    void WidgetManager::closeWidget(WidgetBase *widget)
    {
        widget->hide();
    }

    void WidgetManager::openStartupWidget()
    {
        if (mStartupWidget)
            swapCurrentRoot(mStartupWidget);
    }

    void WidgetManager::onResize(const Rect2i &space)
    {
        MainWindowComponentBase::onResize(space);
        for (WidgetBase *topLevel : uniquePtrToPtr(mTopLevelWidgets)) {
            topLevel->updateGeometry(space);
        }
    }

    void WidgetManager::render(Render::RenderTarget *target, size_t iteration)
    {
        target->pushAnnotation("WidgetManager");

        Rect2i screenSpace = mClientSpace;

        target->setRenderSpace(screenSpace);

        std::map<TextureSettings, std::vector<Vertex>> vertices;

        std::queue<Widgets::WidgetBase *> q;
        for (Widgets::WidgetBase *w : widgets()) {
            if (w->mVisible) {
                q.push(w);
            }
        }
        while (!q.empty()) {

            Widgets::WidgetBase *w = q.front();
            q.pop();

            for (Widgets::WidgetBase *c : w->children()) {
                if (c->mVisible)
                    q.push(c);
            }

            std::vector<std::pair<std::vector<Vertex>, TextureSettings>> localVerticesList = w->vertices(Vector3 { Vector2 { screenSpace.mSize }, 1.0f });

            Resources::ImageLoader::ResourceType *resource = w->resource();
            auto it = mData->mUIAtlasEntries.find(resource);
            if (it == mData->mUIAtlasEntries.end()) {
                Resources::ImageLoader::HandleType data = resource ? resource->loadData() : mData->mDefaultTexture;
                it = mData->mUIAtlasEntries.try_emplace(resource, mData->mUIAtlas.insert({ data->mWidth, data->mHeight }, [this]() { mData->expandUIAtlas(); })).first;
                mData->mUIAtlasTexture.setSubData({ it->second.mArea.mTopLeft.x, it->second.mArea.mTopLeft.y }, it->second.mArea.mSize, { data->mBuffer, static_cast<size_t>(data->mWidth * data->mHeight) });
            }

            for (std::pair<std::vector<Vertex>, TextureSettings> &localVertices : localVerticesList) {
                if (!localVertices.second.mTexture.mTextureHandle) {
                    std::transform(localVertices.first.begin(), localVertices.first.end(), std::back_inserter(vertices[localVertices.second]), [&](const Vertex &v) {
                        return Vertex {
                            v.mPos,
                            v.mColor,
                            { (it->second.mArea.mSize.x / (2048.f * mData->mUIAtlasSize)) * v.mUV.x + it->second.mArea.mTopLeft.x / (2048.f * mData->mUIAtlasSize),
                                (it->second.mArea.mSize.y / (2048.f * mData->mUIAtlasSize)) * v.mUV.y + it->second.mArea.mTopLeft.y / (2048.f * mData->mUIAtlasSize) }
                        };
                    });
                } else {
                    std::move(localVertices.first.begin(), localVertices.first.end(), std::back_inserter(vertices[localVertices.second]));
                }
            }
        }

        for (std::pair<const TextureSettings, std::vector<Vertex>> &p : vertices) {
            if (!p.second.empty()) {

                {
                    auto parameters = mData->mProgram.mapParameters(2).cast<WidgetsPerObject>();
                    parameters->hasDistanceField = bool(p.first.mFlags & TextureFlag_IsDistanceField);
                }

                if (p.first.mTexture.mTextureHandle)
                    target->bindTextures({ p.first.mTexture });
                else
                    target->bindTextures({ { mData->mUIAtlasTexture->mTextureHandle, Render::TextureType_2D } });

                mData->mMesh.update({ 3, std::move(p.second) });

                target->renderMesh(mData->mMesh, mData->mProgram);
            }
        }

        target->popAnnotation();
    }

    void WidgetManager::preRender() {
        std::queue<Widgets::WidgetBase *> q;
        for (Widgets::WidgetBase *w : widgets()) {
            if (w->mVisible) {
                q.push(w);
            }
        }
        while (!q.empty()) {
            Widgets::WidgetBase *w = q.front();
            q.pop();

            for (Widgets::WidgetBase *c : w->children()) {
                if (c->mVisible)
                    q.push(c);
            }

            w->preRender();
        }
    }

    Render::Texture &WidgetManager::uiTexture() const
    {
        return *mData->mUIAtlasTexture;
    }

    void WidgetManager::onActivate(bool active)
    {
        if (active)
            openStartupWidget();
    }

    int WidgetManager::priority() const
    {
        return mPriority;
    }

}
}