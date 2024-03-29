#pragma once

#include "../toolscollector.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

namespace Engine {

struct MadgineObjectState;

namespace Tools {

    struct MADGINE_TOOLS_EXPORT ImRoot {
        ImRoot();
        ~ImRoot();

        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        const std::vector<std::unique_ptr<ToolBase>> &tools();
        ToolBase &getTool(size_t index);
        template <typename T>
        T &getTool()
        {
            return static_cast<T &>(getTool(::Engine::UniqueComponent::component_index<T>()));
        }

        void render();

        unsigned int dockSpaceId() const;

        std::stringstream mToolReadBuffer;
        ToolBase *mToolReadTool = nullptr;

        void finishToolRead();

        virtual Threading::TaskQueue *taskQueue() const = 0;


    protected:
        unsigned int mDockSpaceId;

    private:
        ToolsContainer<std::vector<Placeholder<0>>> mCollector;
    };

}
}

REGISTER_TYPE(Engine::Tools::ImRoot)
