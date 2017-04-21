#pragma once


namespace Engine{
namespace App{

	class FrameListener {
	public:
		virtual bool frameStarted(float timeSinceLastFrame) { return true; }
		virtual bool frameRenderingQueued(float timeSinceLastFrame) { return true; }
		virtual bool frameEnded(float timeSinceLastFrame) { return true; }
	};


}
}
