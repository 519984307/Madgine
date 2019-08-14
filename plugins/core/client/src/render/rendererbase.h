#pragma once

#include "Modules/reflection/classname.h"
#include "Madgine/core/madgineobject.h"

namespace Engine {
	namespace Render {

		class MADGINE_CLIENT_EXPORT RendererBase : public Core::MadgineObject {
		public:			
			RendererBase(GUI::GUISystem *gui);
			RendererBase(const RendererBase&) = delete;
			virtual ~RendererBase();

			void operator=(const RendererBase&) = delete;

			virtual std::unique_ptr<RenderWindow> createWindow(Window::Window *w, GUI::TopLevelWindow *topLevel = nullptr, RenderWindow *sharedResources = nullptr) = 0;

			virtual App::Application &app(bool = true) override;
			virtual const Core::MadgineObject *parent() const override;

		private:
			GUI::GUISystem *mGui;

		};

	}
}