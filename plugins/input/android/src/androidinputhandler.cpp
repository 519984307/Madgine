#include "androidinputlib.h"

#include "androidinputhandler.h"

#include "Interfaces/debug/profiler/profiler.h"

#include "Interfaces/math/vector2.h"

#include <android/input.h>

#include "input/inputevents.h"

UNIQUECOMPONENT(Engine::Input::AndroidInputHandler);

namespace Engine
{
	namespace Input
	{

		DLL_EXPORT AInputQueue *sQueue = nullptr;

		AndroidInputHandler::AndroidInputHandler(Window::Window * window)
		{			
		}

		AndroidInputHandler::~AndroidInputHandler()
		{			
		}

		bool AndroidInputHandler::frameRenderingQueued(std::chrono::microseconds timeSinceLastFrame, Scene::ContextMask context)
		{
			PROFILE();
			if (sQueue)
			{
				AInputEvent* event = NULL;
				while (AInputQueue_getEvent(sQueue, &event) >= 0) {
					if (AInputQueue_preDispatchEvent(sQueue, event)) {
						continue;
					}
					bool handled = false;
					switch (AInputEvent_getType(event))
					{
					case AINPUT_EVENT_TYPE_KEY:
						//TODO
						throw 0;
						break;
					case AINPUT_EVENT_TYPE_MOTION:
						handled = handleMotionEvent(event);
						break;
					default:
						LOG_ERROR("Unknown Event Type: " << AInputEvent_getType(event));
						break;
					}
					AInputQueue_finishEvent(sQueue, event, handled);
				}
			}
			
			return true;
		}

		bool AndroidInputHandler::handleMotionEvent(const AInputEvent *event)
		{
			int32_t action = AMotionEvent_getAction(event);
			size_t pointer_index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			Vector2 position{
				AMotionEvent_getX(event, pointer_index),
				AMotionEvent_getY(event, pointer_index)
			};
			std::stringstream ss;
			ss << position;

			switch (action & AMOTION_EVENT_ACTION_MASK)
			{
			case AMOTION_EVENT_ACTION_DOWN:
				injectPointerMove({
					position,
					Vector2{},
					0.0f
				});
				injectPointerPress({
					position,
					MouseButton::LEFT_BUTTON
				});
				break;
			case AMOTION_EVENT_ACTION_UP:
				injectPointerRelease({
					position,
					MouseButton::LEFT_BUTTON
				});
				break;
			case AMOTION_EVENT_ACTION_MOVE:
				injectPointerMove({
					position,
					Vector2{},
					0.0f
				});
				break;
			case AMOTION_EVENT_ACTION_CANCEL:
				LOG("Motion Cancel");
				break;
			case AMOTION_EVENT_ACTION_OUTSIDE:
				LOG("Motion Outside");
				break;
			case AMOTION_EVENT_ACTION_POINTER_DOWN:
				LOG("Motion Pointer Down");
				break;
			case AMOTION_EVENT_ACTION_POINTER_UP:
				LOG("Motion Pointer Up");
				break;
			case AMOTION_EVENT_ACTION_HOVER_MOVE:
				LOG("Motion Hover Move");
				break;
			case AMOTION_EVENT_ACTION_SCROLL:
				LOG("Motion Scroll");
				break;
			case AMOTION_EVENT_ACTION_HOVER_ENTER:
				LOG("Motion Hover Enter");
				break;
			case AMOTION_EVENT_ACTION_HOVER_EXIT:
				LOG("Motion Hover Exit");
				break;
			default:
				LOG_ERROR("Unknown Motion Event Type: " << (action & AMOTION_EVENT_ACTION_MASK));
				break;
			}
			return true;
		}


	}
}