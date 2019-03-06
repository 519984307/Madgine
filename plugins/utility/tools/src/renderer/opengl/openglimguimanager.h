#pragma once

#include "../imguimanager.h"

namespace Engine {
	namespace Tools {

		class OpenGLImGuiManager : public ImGuiManager {
		public:
			OpenGLImGuiManager(App::Application &app);

			// Geerbt �ber ImGuiManager
			virtual void init() override;
			virtual void finalize() override;
			virtual void newFrame(float timeSinceLastFrame) override;
		};

	}
}