#include "toolslib.h"

#include "toolbase.h"

#include "renderer/imroot.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "imgui/imgui.h"

namespace Engine {
namespace Tools {
    ToolBase::ToolBase(ImRoot &root)
        : mRoot(root)
    {
    }

    void ToolBase::render()
    {
    }

    void ToolBase::renderMenu()
    {
        if (ImGui::BeginMenu("Views")) {
            ImGui::MenuItem(key().data(), "", &mVisible);
            ImGui::EndMenu();
        }
    }

    void ToolBase::renderStatus()
    {
    }

    void ToolBase::update()
    {
        if (mVisible)
            render();
    }

    bool ToolBase::isVisible() const
    {
        return mVisible;
    }

    void ToolBase::setVisible(bool v)
    {
        mVisible = v;
    }

    bool ToolBase::isEnabled() const
    {
        return mEnabled;
    }

    void ToolBase::setEnabled(bool e)
    {
        mEnabled = e;
    }

    ToolBase &ToolBase::getToolComponent(size_t index)
    {
        return mRoot.getToolComponent(index);
    }

    ImRoot &ToolBase::root()
    {
        return mRoot;
    }

    Threading::TaskQueue *ToolBase::taskQueue() const
    {
        return mRoot.taskQueue();
    }

    Threading::Task<bool> ToolBase::init()
    {
        co_return true;
    }

    Threading::Task<void> ToolBase::finalize()
    {
        co_return;
    }
}
}

METATABLE_BEGIN(Engine::Tools::ToolBase)
PROPERTY(visible, isVisible, setVisible)
METATABLE_END(Engine::Tools::ToolBase)

SERIALIZETABLE_BEGIN(Engine::Tools::ToolBase)
FIELD(mVisible)
SERIALIZETABLE_END(Engine::Tools::ToolBase)
