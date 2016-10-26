#include "libinclude.h"

#include "GUISystem.h"
#include "Windows\WindowContainer.h"
#include "Windows\window.h"


namespace Engine {
	namespace GUI {

		GUISystem::GUISystem() :
			mRootWindow(0),
			mWindowSizesDirty(false)
		{
		}


		GUISystem::~GUISystem()
		{
			assert(mWindows.empty());
		}

		void GUISystem::init()
		{
		}



		void GUISystem::showCursor()
		{
			setCursorVisibility(true);
		}

		void GUISystem::hideCursor()
		{
			setCursorVisibility(false);
		}

		void GUISystem::injectTimePulse(float time)
		{
			if (mWindowSizesDirty) {
				mWindowSizesDirty = false;
				updateWindowSizes();
			}
		}

		void GUISystem::notifyDisplaySizeChanged(const Ogre::Vector2 & size)
		{
			updateWindowSizes();
		}

		void GUISystem::renderSingleFrame()
		{
		}

		void GUISystem::printHierarchy() {
			std::list<std::pair<WindowContainer*, size_t>> windows{ {mRootWindow, 0} };
			
			while (!windows.empty()) {
				std::pair<WindowContainer*, size_t> p = windows.front();
				windows.pop_front();
				WindowContainer *w = p.first;
				for (WindowContainer *win : w->getChildren()) {
					windows.push_front({ win, p.second + 1 });
				}				
				std::cout << std::string(4 * p.second, ' ') << "'" << w->getName() << "', " << w->getPixelSize() << std::endl;
			}
		}

		void GUISystem::addWindow(const std::string & name, WindowContainer * w)
		{
			//assert(mWindows.find(name) == mWindows.end());
			mWindows[name] = w;
		}

		void GUISystem::removeWindow(const std::string & name)
		{
			mWindows.erase(name);
		}

		void GUISystem::updateWindowSizes()
		{
			if (mRootWindow)
				mRootWindow->updateSize(getScreenSize());
		}

		void GUISystem::setDirtyWindowSizes()
		{
			mWindowSizesDirty = true;
		}

		Window * GUISystem::getWindowByName(const std::string & name, Class _class)
		{
			return mWindows.find(name)->second->as(_class);
		}

		Window * GUISystem::loadLayout(const std::string & name, const std::string & parent)
		{
			return mWindows.find(parent)->second->loadLayout(name)->as();
		}

		Window * GUISystem::getRootWindow()
		{
			return mRootWindow->as();
		}

}
}