#include "toolslib.h"

#include "toolbase.h"

#include "Modules/reflection/classname.h"

#include "renderer/imguiroot.h"

#include "Modules/keyvalue/metatable_impl.h"

namespace Engine {
namespace Tools {
    ToolBase::ToolBase(ImGuiRoot &root)
        : mRoot(root)
    {
    }

    void ToolBase::render()
    {
    }

    void ToolBase::renderMenu()
    {
    }

    void ToolBase::update()
    {
        if (mVisible)
            render();
    }

    bool ToolBase::isVisible()
    {
        return mVisible;
    }

    void ToolBase::setVisible(bool v)
    {
        mVisible = v;
    }

    ToolBase &ToolBase::getToolComponent(size_t index, bool init)
    {
        if (init) {
            checkInitState();
        }
        return mRoot.getToolComponent(index, init);
    }

    ToolBase &ToolBase::getSelf(bool init)
    {
        if (init) {
            checkDependency();
        }
        return *this;
    }

    const Core::MadgineObject *ToolBase::parent() const
    {
        return &mRoot;
    }

    App::Application &ToolBase::app(bool init)
    {
        if (init) {
            checkDependency();
        }
        return mRoot.app(init);
    }

    bool ToolBase::init()
    {
        return true;
    }

    void ToolBase::finalize()
    {
    }
}
}


METATABLE_BEGIN(Engine::Tools::ToolBase)
PROPERTY(visible, isVisible, setVisible)
METATABLE_END(Engine::Tools::ToolBase)

RegisterType(Engine::Tools::ToolBase);