#include "../widgetslib.h"

#include "widget.h"

#include "widgetmanager.h"

#include "bar.h"
#include "button.h"
#include "checkbox.h"
#include "combobox.h"
#include "image.h"
#include "label.h"
#include "scenewindow.h"
#include "tabwidget.h"
#include "textbox.h"

#include "vertex.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "font.h"

METATABLE_BEGIN(Engine::Widgets::WidgetBase)
READONLY_PROPERTY(Widgets, children)
READONLY_PROPERTY(Size, getSize)
MEMBER(mVisible)
METATABLE_END(Engine::Widgets::WidgetBase)

SERIALIZETABLE_BEGIN(Engine::Widgets::WidgetBase)
FIELD(mChildren, Serialize::ParentCreator<&Engine::Widgets::WidgetManager::widgetCreationNames, &Engine::Widgets::WidgetBase::createWidgetClassTuple, &Engine::Widgets::WidgetBase::storeWidgetCreationData>)
FIELD(mPos)
FIELD(mSize)
SERIALIZETABLE_END(Engine::Widgets::WidgetBase)

namespace Engine {

namespace Widgets {

    WidgetBase::WidgetBase(const std::string &name, WidgetManager &manager, WidgetBase *parent)
        : mName(name)
        , mParent(parent)
        , mManager(manager)
    {
        mManager.registerWidget(this);
    }

    WidgetBase::~WidgetBase()
    {
        mManager.unregisterWidget(this);
    }

    void WidgetBase::setSize(const Matrix3 &size)
    {
        mSize = size;
        if (mParent)
            updateGeometry(manager().getScreenSpace(), mParent->getAbsoluteSize(), mParent->getAbsolutePosition());
        else
            updateGeometry(manager().getScreenSpace());
    }

    const Matrix3 &WidgetBase::getSize()
    {
        return mSize;
    }

    void WidgetBase::setPos(const Matrix3 &pos)
    {
        mPos = pos;
        if (mParent)
            updateGeometry(manager().getScreenSpace(), mParent->getAbsoluteSize(), mParent->getAbsolutePosition());
        else
            updateGeometry(manager().getScreenSpace());
    }

    const Matrix3 &WidgetBase::getPos() const
    {
        return mPos;
    }

    Matrix3 WidgetBase::getAbsoluteSize() const
    {
        return mAbsoluteSize;
    }

    Matrix3 WidgetBase::getAbsolutePosition() const
    {
        return mAbsolutePos;
    }

    Vector3 WidgetBase::getActualSize() const
    {
        const Rect2i &screenSpace = mManager.getClientSpace();
        return mAbsoluteSize * Vector3 { Vector2 { screenSpace.mSize }, 1.0f };
    }

    Vector3 WidgetBase::getActualPosition() const
    {
        const Rect2i &screenSpace = mManager.getClientSpace();
        return mAbsolutePos * Vector3 { Vector2 { screenSpace.mSize }, 1.0f } + Vector3 { Vector2 { screenSpace.mTopLeft }, 0.0f };
    }

    void WidgetBase::updateGeometry(const Rect2i &screenSpace, const Matrix3 &parentSize, const Matrix3 &parentPos)
    {
        mAbsoluteSize = mSize * parentSize;
        mAbsolutePos = mPos * parentSize + parentPos;

        sizeChanged((mAbsoluteSize * Vector3 { Vector2 { screenSpace.mSize }, 1.0f }).floor());

        for (WidgetBase *child : uniquePtrToPtr(mChildren)) {
            child->updateGeometry(screenSpace, getAbsoluteSize(), getAbsolutePosition());
        }
    }

    WidgetClass WidgetBase::getClass() const
    {
        return WidgetClass::WIDGET_CLASS;
    }

    void WidgetBase::destroy()
    {
        if (mParent)
            mParent->destroyChild(this);
        else
            mManager.destroyTopLevel(this);
    }

    const std::string &WidgetBase::getName() const
    {
        return mName;
    }

    void WidgetBase::setName(const std::string &name)
    {
        mManager.updateWidget(this, name);
        mName = name;
    }

    const std::string &WidgetBase::key() const
    {
        return mName;
    }

    WidgetBase *WidgetBase::createChild(const std::string &name, WidgetClass _class)
    {
        switch (_class) {
        case WidgetClass::WIDGET_CLASS:
            return createChild<WidgetBase>(name);
        case WidgetClass::BAR_CLASS:
            return createChild<Bar>(name);
        case WidgetClass::CHECKBOX_CLASS:
            return createChild<Checkbox>(name);
        case WidgetClass::LABEL_CLASS:
            return createChild<Label>(name);
        case WidgetClass::TABWIDGET_CLASS:
            return createChild<TabWidget>(name);
        case WidgetClass::BUTTON_CLASS:
            return createChild<Button>(name);
        case WidgetClass::COMBOBOX_CLASS:
            return createChild<Combobox>(name);
        case WidgetClass::TEXTBOX_CLASS:
            return createChild<Textbox>(name);
        case WidgetClass::SCENEWINDOW_CLASS:
            return createChild<SceneWindow>(name);
        case WidgetClass::IMAGE_CLASS:
            return createChild<Image>(name);
        default:
            std::terminate();
        }
    }

    template DLL_EXPORT WidgetBase *WidgetBase::createChild<WidgetBase>(const std::string &);
    template DLL_EXPORT Button *WidgetBase::createChild<Button>(const std::string &);

    template <typename WidgetType>
    WidgetType *WidgetBase::createChild(const std::string &name)
    {
        std::unique_ptr<WidgetType> p = mManager.create<WidgetType>(name, this);
        WidgetType *w = p.get();
        mChildren.emplace_back(std::move(p));
        w->updateGeometry(mManager.getScreenSpace(), getAbsoluteSize());
        return w;
    }

    size_t WidgetBase::depth()
    {
        return mParent ? mParent->depth() + 1 : 0;
    }

    WidgetManager &WidgetBase::manager()
    {
        return mManager;
    }

    void WidgetBase::destroyChild(WidgetBase *w)
    {
        auto it = std::find_if(mChildren.begin(), mChildren.end(), [=](const std::unique_ptr<WidgetBase> &ptr) { return ptr.get() == w; });
        assert(it != mChildren.end());
        mChildren.erase(it);
    }

    /*KeyValueMapList Widget::maps()
		{
			return Scope::maps().merge(mChildren, MAP_RO(AbsolutePos, getAbsolutePosition), MAP_RO(AbsoluteSize, getAbsoluteSize), MAP(Visible, isVisible, setVisible), MAP(Size, getSize, setSize), MAP(Position, getPos, setPos));
		}*/

    void WidgetBase::show()
    {
        mVisible = true;
    }

    void WidgetBase::hide()
    {
        mVisible = false;
    }

    void WidgetBase::setEnabled(bool b)
    {
    }

    WidgetBase *WidgetBase::getChildRecursive(const std::string &name)
    {
        if (name == getName() || name.empty())
            return this;
        for (const std::unique_ptr<WidgetBase> &w : mChildren)
            if (WidgetBase *f = w->getChildRecursive(name))
                return f;
        return nullptr;
    }

    void WidgetBase::setParent(WidgetBase *parent)
    {
        auto it = std::find_if(mParent->mChildren.begin(), mParent->mChildren.end(), [this](std::unique_ptr<WidgetBase> &p) { return p.get() == this; });
        parent->mChildren.emplace_back(mParent->mChildren.extract(it));
        mParent = parent;
        updateGeometry(mManager.getScreenSpace(), parent->getAbsoluteSize());
    }

    WidgetBase *WidgetBase::getParent() const
    {
        return mParent;
    }

    bool WidgetBase::injectPointerPress(const Input::PointerEventArgs &arg)
    {
        mPointerDownSignal.emit(arg);
        return true;
    }

    bool WidgetBase::injectPointerRelease(const Input::PointerEventArgs &arg)
    {
        mPointerUpSignal.emit(arg);
        return true;
    }

    bool WidgetBase::injectPointerMove(const Input::PointerEventArgs &arg)
    {
        mPointerMoveSignal.emit(arg);
        return true;
    }

    bool WidgetBase::injectPointerEnter(const Input::PointerEventArgs &arg)
    {
        mPointerEnterSignal.emit(arg);
        return true;
    }

    bool WidgetBase::injectPointerLeave(const Input::PointerEventArgs &arg)
    {
        mPointerLeaveSignal.emit(arg);
        return true;
    }

    bool WidgetBase::injectAxisEvent(const Input::AxisEventArgs &arg)
    {
        mAxisEventSignal.emit(arg);
        return true;
    }

    Threading::SignalStub<const Input::PointerEventArgs &> &WidgetBase::pointerMoveEvent()
    {
        return mPointerMoveSignal;
    }

    Threading::SignalStub<const Input::PointerEventArgs &> &WidgetBase::pointerDownEvent()
    {
        return mPointerDownSignal;
    }

    Threading::SignalStub<const Input::PointerEventArgs &> &WidgetBase::pointerUpEvent()
    {
        return mPointerUpSignal;
    }

    Threading::SignalStub<const Input::PointerEventArgs &> &WidgetBase::pointerEnterEvent()
    {
        return mPointerEnterSignal;
    }

    Threading::SignalStub<const Input::PointerEventArgs &> &WidgetBase::pointerLeaveEvent()
    {
        return mPointerLeaveSignal;
    }

    Threading::SignalStub<const Input::AxisEventArgs &> &WidgetBase::axisEvent()
    {
        return mAxisEventSignal;
    }

    bool WidgetBase::containsPoint(const Vector2 &point, const Rect2i &screenSpace, float extend) const
    {
        Vector3 min = mAbsolutePos * Vector3 { Vector2 { screenSpace.mSize }, 1.0f } + Vector3 { Vector2 { screenSpace.mTopLeft }, 0.0f } - extend;
        Vector3 max = mAbsoluteSize * Vector3 { Vector2 { screenSpace.mSize }, 1.0f } + min + 2 * extend;
        return min.x <= point.x && min.y <= point.y && max.x >= point.x && max.y >= point.y;
    }

    std::vector<std::pair<std::vector<Vertex>, TextureSettings>> WidgetBase::vertices(const Vector3 &screenSize)
    {
        return {};
    }

    void *WidgetBase::userData()
    {
        return mUserData;
    }

    void WidgetBase::setUserData(void *userData)
    {
        mUserData = userData;
    }

    std::unique_ptr<WidgetBase> WidgetBase::createWidgetClass(const std::string &name, WidgetClass _class)
    {
        return mManager.createWidgetClass(name, _class, this);
    }
    std::tuple<std::unique_ptr<WidgetBase>> WidgetBase::createWidgetClassTuple(const std::string &name, WidgetClass _class)
    {
        return {
            createWidgetClass(name, _class)
        };
    }
    std::tuple<std::string, WidgetClass> WidgetBase::storeWidgetCreationData(const std::unique_ptr<WidgetBase> &widget) const
    {
        return mManager.storeWidgetCreationData(widget);
    }
    void WidgetBase::sizeChanged(const Vector3i &pixelSize)
    {
    }

    Resources::ResourceType<Resources::ImageLoader> *WidgetBase::resource() const
    {
        return nullptr;
    }

    std::pair<std::vector<Vertex>, TextureSettings> WidgetBase::renderText(const std::string &text, Vector3 pos, Render::Font *font, float fontSize, Vector2 pivot, const Vector3 &screenSize)
    {
        std::vector<Vertex> result;

        size_t textLen = text.size();

        if (textLen == 0)
            return {};

        float scaleX = fontSize / 5.0f / screenSize.x;
        float scaleY = fontSize / 5.0f / screenSize.y;

        const float padding = 1.0f * scaleX;

        float fullWidth = padding * (textLen - 1);
        float minY = 0.0f;
        float maxY = 0.0f;

        for (size_t i = 0; i < textLen; ++i) {
            Render::Glyph &g = font->mGlyphs[text[i]];

            fullWidth += g.mSize.x * scaleX;
            maxY = max(maxY, g.mBearingY * scaleY);
            minY = min(minY, (g.mBearingY - g.mSize.y) * scaleY);
        }

        float fullHeight = maxY - minY;

        float xLeft = -fullWidth * pivot.x;
        float yTop = -fullHeight * pivot.y;

        float cursorX = xLeft;

        for (size_t i = 0; i < textLen; ++i) {
            Render::Glyph &g = font->mGlyphs[text[i]];

            float width = g.mSize.x * scaleX;
            float height = g.mSize.y * scaleY;

            float vPosX1 = cursorX;
            float vPosX2 = cursorX + width;
            float vPosY1 = yTop + fullHeight - g.mBearingY * scaleY;
            float vPosY2 = vPosY1 + height;

            Vector3 v11 = { vPosX1, vPosY1, pos.z + 0.5f }, v12 = { vPosX2, vPosY1, pos.z + 0.5f }, v21 = { vPosX1, vPosY2, pos.z + 0.5f }, v22 = { vPosX2, vPosY2, pos.z + 0.5f };

            int uvWidth = g.mSize.x;
            int uvHeight = g.mSize.y;

            if (g.mFlipped)
                std::swap(uvWidth, uvHeight);

            Vector2 uvTopLeft = { float(g.mUV.x) / font->mTextureSize.x, float(g.mUV.y) / font->mTextureSize.y };
            Vector2 uvBottomRight = { float(g.mUV.x + uvWidth) / font->mTextureSize.x,
                float(g.mUV.y + uvHeight) / font->mTextureSize.y };

            Vector2 uvTopRight = { uvBottomRight.x, uvTopLeft.y };
            Vector2 uvBottomLeft = { uvTopLeft.x, uvBottomRight.y };

            if (g.mFlipped)
                std::swap(uvTopRight, uvBottomLeft);

            result.push_back({ v11 + pos, { 1, 1, 1, 1 }, uvTopLeft });
            result.push_back({ v12 + pos, { 1, 1, 1, 1 }, uvTopRight });
            result.push_back({ v21 + pos, { 1, 1, 1, 1 }, uvBottomLeft });
            result.push_back({ v21 + pos, { 1, 1, 1, 1 }, uvBottomLeft });
            result.push_back({ v12 + pos, { 1, 1, 1, 1 }, uvTopRight });
            result.push_back({ v22 + pos, { 1, 1, 1, 1 }, uvBottomRight });

            cursorX += g.mAdvance / 64.0f * scaleX;
        }
        return { result, { { font->mTexture->mTextureHandle, Render::TextureType_2D }, TextureFlag_IsDistanceField } };
    }

    void WidgetBase::preRender() {

    }

}
}
