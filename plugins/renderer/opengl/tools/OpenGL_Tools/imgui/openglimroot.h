#pragma once

#include "Madgine_Tools/imgui/clientimroot.h"

namespace Engine {
namespace Tools {

    struct OpenGLImRoot : VirtualScope<OpenGLImRoot, VirtualUniqueComponentImpl<OpenGLImRoot, ClientImRoot>> {

        OpenGLImRoot(Window::MainWindow &window);
        ~OpenGLImRoot();

		virtual bool init() override;
        virtual void finalize() override;

        virtual void newFrame(float timeSinceLastFrame) override;
    };

}
}

RegisterType(Engine::Tools::OpenGLImRoot);