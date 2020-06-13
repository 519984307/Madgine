#pragma once

#include "../toolscollector.h"

namespace Engine {
namespace Tools {

    struct ImGuiDemo : Tool<ImGuiDemo> {
        ImGuiDemo(ImRoot &root);

        void render() override;

        std::string_view key() const override;
    };

}
}

RegisterType(Engine::Tools::ImGuiDemo);