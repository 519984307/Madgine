#include "../clientlib.h"
#include "renderwindow.h"
#include "../gui/widgets/toplevelwindow.h"
#include "../gui/guisystem.h"
#include "../app/clientapplication.h"
#include "Interfaces/window/windowapi.h"
#include "rendertarget.h"

namespace Engine {
	namespace Render {



		RenderWindow::RenderWindow(GUI::TopLevelWindow * w) :
			mWindow(w)
		{
			w->gui().app(false).addFrameListener(this);
		}

		RenderWindow::~RenderWindow()
		{
			mWindow->gui().app(false).removeFrameListener(this);
		}

		bool RenderWindow::frameStarted(std::chrono::microseconds)
		{
			mWindow->window()->beginFrame();
			return true;
		}

		bool RenderWindow::frameRenderingQueued(std::chrono::microseconds, Scene::ContextMask)
		{
			render();
			return true;
		}

		bool RenderWindow::frameEnded(std::chrono::microseconds)
		{
			mWindow->renderOverlays();
			mWindow->window()->endFrame();
			return true;
		}

		void RenderWindow::addRenderTarget(RenderTarget * target)
		{
			mRenderTargets.push_back(target);
		}

		void RenderWindow::removeRenderTarget(RenderTarget * target)
		{
			mRenderTargets.erase(std::find(mRenderTargets.begin(), mRenderTargets.end(), target));
		}

		void RenderWindow::updateRenderTargets()
		{
			for (RenderTarget *target : mRenderTargets)
				target->render();
		}

		GUI::TopLevelWindow * RenderWindow::window()
		{
			return mWindow;
		}

	}
}