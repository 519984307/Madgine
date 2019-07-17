#pragma once

#include "Madgine/app/globalapicollector.h"
#include "Modules/threading/framelistener.h"

#include "../toolscollector.h"

#include "Modules/plugins/pluginlistener.h"

namespace Engine {
namespace Tools {

    class MADGINE_TOOLS_EXPORT ImGuiRoot : public App::GlobalAPI<ImGuiRoot>, public Threading::FrameListener
#ifndef STATIC_BUILD
        ,
                                           public Plugins::PluginListener
#endif
    {
    public:
        ImGuiRoot(App::Application &app);
        ~ImGuiRoot();

        bool init() override;
        void finalize() override;

        bool frameStarted(std::chrono::microseconds timeSinceLastFrame) override;
        bool frameRenderingQueued(std::chrono::microseconds timeSinceLastFrame, Scene::ContextMask context) override;

#ifndef STATIC_BUILD
        bool aboutToUnloadPlugin(const Plugins::Plugin *p) override;
        void onPluginLoad(const Plugins::Plugin *p) override;
#endif

        const ToolsContainer<std::vector> &tools();
        ToolBase &getToolComponent(size_t index, bool = true);
        template <typename T>
        T &getTool()
        {
            return static_cast<T &>(getToolComponent(::Engine::component_index<T>()));
        }

    private:
        void createManager();
        void destroyManager();

    private:
        std::unique_ptr<ImGuiManager> mManager;

        ToolsContainer<std::vector> mCollector;
    };

}
}
