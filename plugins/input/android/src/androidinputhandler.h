#pragma once

struct ALooper;
struct AInputEvent;

#include "input/inputcollector.h"

namespace Engine
{
	namespace Input
	{

		struct MADGINE_ANDROIDINPUT_EXPORT AndroidInputHandler :
			public InputHandlerComponent<AndroidInputHandler>
		{
                AndroidInputHandler(Window::Window *window, App::Application &app, InputListener *listener);
			~AndroidInputHandler();

			virtual bool isKeyDown(Key key) override;

		private:
			bool frameRenderingQueued(std::chrono::microseconds timeSinceLastFrame, Scene::ContextMask context) override;

			bool handleMotionEvent(const AInputEvent *event);

		private:
		};

	}
}
