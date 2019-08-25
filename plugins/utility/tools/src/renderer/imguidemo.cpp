#include "../toolslib.h"

#include "imguidemo.h"

#include "../imgui/imgui.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/serialize/serializetable_impl.h"
#include "Modules/reflection/classname.h"

namespace Engine {
namespace Tools {

    ImGuiDemo::ImGuiDemo(ImGuiRoot &root)
        : Tool<ImGuiDemo>(root)
    {
    }

    void ImGuiDemo::render()
    {
        ImGui::ShowDemoWindow(&mVisible);
    }

    const char *ImGuiDemo::key() const
    {
        return "ImGuiDemo";
    }

}
}

UNIQUECOMPONENT(Engine::Tools::ImGuiDemo);

METATABLE_BEGIN(Engine::Tools::ImGuiDemo)
METATABLE_END(Engine::Tools::ImGuiDemo)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::ImGuiDemo, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::ImGuiDemo)

RegisterType(Engine::Tools::ImGuiDemo);