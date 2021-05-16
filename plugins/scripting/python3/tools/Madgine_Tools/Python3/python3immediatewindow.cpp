#include "python3toolslib.h"

#include "python3immediatewindow.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/app/application.h"
#include "Python3/python3env.h"

METATABLE_BEGIN_BASE(Engine::Tools::Python3ImmediateWindow, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::Python3ImmediateWindow)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::Python3ImmediateWindow, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::Python3ImmediateWindow)

UNIQUECOMPONENT(Engine::Tools::Python3ImmediateWindow)

namespace Engine {
namespace Tools {

    Python3ImmediateWindow::Python3ImmediateWindow(ImRoot &root)
        : Tool<Python3ImmediateWindow>(root)
    {
    }

    std::string_view Python3ImmediateWindow::key() const
    {
        return "Python3ImmediateWindow";
    }

    bool Python3ImmediateWindow::init()
    {

        mEnv = &Engine::App::Application::getSingleton().getGlobalAPIComponent<Scripting::Python3::Python3Environment>();        

        return ToolBase::init();
    }

    void Python3ImmediateWindow::finalize()
    {
        ToolBase::finalize();

        
    }

    void Python3ImmediateWindow::renderMenu()
    {
    }

    void Python3ImmediateWindow::render()
    {
        if (ImGui::Begin("Python3ImmediateWindow", &mVisible)) {

            ImVec2 size = ImGui::GetContentRegionAvail();
            size.y -= 30;

            ImGui::InputTextMultiline("Log", const_cast<char *>(mCommandLog.str().c_str()), mCommandLog.tellp(), size, ImGuiInputTextFlags_ReadOnly);

            bool exec = false;
            if (ImGui::InputText("Command", &mCommandBuffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
                exec = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Go"))
                exec = true;

            if (exec) {
                mCommandLog << ">>> " << mCommandBuffer << "\n";
                mEnv->lock(mCommandLog.rdbuf());
                mEnv->execute(mCommandBuffer);
                mEnv->unlock();
                mCommandBuffer.clear();
                ImGui::SetKeyboardFocusHere(-1);
            }
        }
        ImGui::End();
    }

}
}
