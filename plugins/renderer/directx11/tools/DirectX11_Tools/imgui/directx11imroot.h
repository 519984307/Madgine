#pragma once

#include "Madgine_Tools/imgui/clientimroot.h"

namespace Engine {
namespace Tools {

    struct DirectX11ImRoot : VirtualScope<DirectX11ImRoot, VirtualUniqueComponentImpl<DirectX11ImRoot, ClientImRoot>> {

        DirectX11ImRoot(Window::MainWindow &window);
        ~DirectX11ImRoot();

		virtual bool init() override;
        virtual void finalize() override;

        virtual void newFrame(float timeSinceLastFrame) override;        
    };

}
}

RegisterType(Engine::Tools::DirectX11ImRoot);