#include "../toolslib.h"

#include "imroot.h"

#ifdef BUILD_PLUGIN_OpenGL
#    include "opengl/openglimguimanager.h"
#endif

#include "Modules/math/vector3.h"

#include "Modules/debug/profiler/profiler.h"

#include "../toolbase.h"

#include "Modules/plugins/pluginmanager.h"

#include "Modules/signalslot/taskqueue.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/reflection/classname.h"

#include "Modules/keyvalue/scopeiterator.h"

#include "Modules/serialize/streams/serializestream.h"

#include "Modules/ini/iniformatter.h"

namespace Engine {

namespace Tools {

    void *ToolReadOpen(ImGuiContext *ctx, ImGuiSettingsHandler *handler, const char *name) // Read: Called when entering into a new ini entry e.g. "[Window][Name]"
    {
        ImRoot *root = static_cast<ImRoot *>(handler->UserData);
        for (ToolBase *tool : uniquePtrToPtr(root->tools())) {
            if (streq(tool->key(), name))
                return tool;
        }
        return nullptr;
    }

    void ToolReadLine(ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry, const char *line) // Read: Called for every line of text within an ini entry
    {
        if (strlen(line) > 0) {
            auto buf = std::make_unique<Serialize::WrappingSerializeStreambuf<std::stringbuf>>(std::make_unique<Ini::IniFormatter>(), line);
            Serialize::SerializeInStream in { std::move(buf) };

            ToolBase *tool = static_cast<ToolBase *>(entry);
            tool->readState(in, nullptr, true);
        }
    }

    void ToolWriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *out_buf) // Write: Output every entries into 'out_buf'
    {

        auto buf = std::make_unique<Serialize::WrappingSerializeStreambuf<std::stringbuf>>(std::make_unique<Ini::IniFormatter>());
        std::stringbuf *outBuffer = buf.get();
        Serialize::SerializeOutStream out { std::move(buf) };

        ImRoot *root = static_cast<ImRoot *>(handler->UserData);
        for (ToolBase *tool : uniquePtrToPtr(root->tools())) {
            out_buf->appendf("[Tool][%s]\n", tool->key());

            tool->writeState(out, nullptr, true);
            out_buf->append(outBuffer->str().c_str());
            outBuffer->str("");

            out_buf->append("\n");
        }
    }

    ImRoot::ImRoot(const MadgineObject *parent)
        : mParent(parent)
        , mCollector(*this)
    {
    }

    ImRoot::~ImRoot()
    {
    }

    bool ImRoot::init()
    {

        ImGuiSettingsHandler ini_handler;
        ini_handler.TypeName = "Tool";
        ini_handler.TypeHash = ImHashStr("Tool");
        ini_handler.ReadOpenFn = ToolReadOpen;
        ini_handler.ReadLineFn = ToolReadLine;
        ini_handler.WriteAllFn = ToolWriteAll;
        ini_handler.UserData = this;
        GImGui->SettingsHandlers.push_back(ini_handler);

        for (const std::unique_ptr<ToolBase> &tool : mCollector) {
            tool->callInit();
        }

        return true;
    }

    void ImRoot::finalize()
    {

        for (const std::unique_ptr<ToolBase> &tool : mCollector) {
            tool->callFinalize();
        }
    }

    bool ImRoot::frame()
    {
        PROFILE_NAMED("ImGui - Rendering");

        bool running = true;

		ImGuiID dockspace_id = ImGui::GetID("MadgineDockSpace");

        ImGuiDockNodeFlags dockspace_flags = /*ImGuiDockNodeFlags_NoDockingInCentralNode | */ ImGuiDockNodeFlags_PassthruCentralNode;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        bool open = true;
        ImGui::Begin("Madgine Root Window", &open, window_flags);
        ImGui::PopStyleVar();

        ImGui::PopStyleVar(2);

        // DockSpace
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        ImGuiDockNode *node = ImGui::DockBuilderGetNode(dockspace_id);

        if (ImGui::BeginMainMenuBar()) {

            //mManager->setMenuHeight(ImGui::GetWindowSize().y);

            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Quit")) {
                    running = false;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools")) {
                for (const std::unique_ptr<ToolBase> &tool : mCollector) {
                    bool visible = tool->isVisible();
                    ImGui::MenuItem(tool->key(), "", &visible);
                    tool->setVisible(visible);
                }
                ImGui::EndMenu();
            }
            for (const std::unique_ptr<ToolBase> &tool : mCollector) {
                tool->renderMenu();
            }
            ImGui::EndMainMenuBar();
        }

        for (const std::unique_ptr<ToolBase> &tool : mCollector) {
            tool->update();
        }

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::End();
        }

        return running;
    }

    const ToolsContainer<std::vector> &ImRoot::tools()
    {
        return mCollector;
    }

    ToolBase &ImRoot::getToolComponent(size_t index, bool init)
    {
        ToolBase &tool = mCollector.get(index);
        if (init) {
            checkInitState();
            tool.callInit();
        }
        return tool.getSelf(init);
    }

    const MadgineObject *ImRoot::parent() const
    {
        return mParent;
    }

}
}

METATABLE_BEGIN(Engine::Tools::ImRoot)
READONLY_PROPERTY(Tools, tools)
METATABLE_END(Engine::Tools::ImRoot)

RegisterType(Engine::Tools::ImRoot);