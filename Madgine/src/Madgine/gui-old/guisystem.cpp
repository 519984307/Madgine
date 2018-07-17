#include "../clientlib.h"

#include "guisystem.h"
#include "windows/windowcontainer.h"
#include "windows/window.h"

#include "../app/application.h"
#include "../ui/uimanager.h"

#include "../util/profile.h"

namespace Engine
{

	SINGLETON_IMPL(GUI::GUISystem);


	namespace GUI
	{
		GUISystem::GUISystem(App::Application &app) :
			ScopeBase(app.createTable()),
			mRootWindow(nullptr),
			mWindowSizesDirty(false),
			mApp(app)
		{
		}


		GUISystem::~GUISystem()
		{
			assert(mWindows.empty());
		}

		void GUISystem::showCursor()
		{
			setCursorVisibility(true);
		}

		void GUISystem::hideCursor()
		{
			setCursorVisibility(false);
		}

		/*void GUISystem::update(float time)
		{
			if (mWindowSizesDirty)
			{
				mWindowSizesDirty = false;
				updateWindowSizes();
			}
		}*/

		void GUISystem::notifyDisplaySizeChanged(const Ogre::Vector2& size)
		{
			updateWindowSizes();
		}


		bool GUISystem::preInit()
		{
			mUI = std::make_unique<UI::UIManager>(*this);
			return mUI->preInit();
		}

		bool GUISystem::init()
		{
			return MadgineObject::init() && mUI->init();
		}

		void GUISystem::finalize()
		{
			mUI->finalize();
			MadgineObject::finalize();
		}

		/*void GUISystem::printHierarchy() {
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
		}*/

		void GUISystem::addWindow(const std::string& name, WindowContainer* w)
		{
			//assert(mWindows.find(name) == mWindows.end());
			mWindows[name] = w;
		}

		void GUISystem::removeWindow(const std::string& name)
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

		Window* GUISystem::getWindowByName(const std::string& name, Class _class)
		{
			return mWindows.at(name)->as(_class);
		}

		Window* GUISystem::loadLayout(const std::string& name, const std::string& parent)
		{
			auto it = mWindows.find(parent);
			if (it == mWindows.end())
				return nullptr;
			WindowContainer* w = it->second->loadLayout(name);
			if (!w)
				return nullptr;
			return w->as();
		}

		Window* GUISystem::getRootWindow() const
		{
			return mRootWindow->as();
		}

		void GUISystem::windowResized(Ogre::RenderWindow* rw)
		{
			unsigned int width, height, depth;
			int left, top;
			rw->getMetrics(width, height, depth, left, top);

			notifyDisplaySizeChanged(Ogre::Vector2(width, height));
		}

		const char* GUISystem::key() const
		{
			return "Gui";
		}

		App::Application& GUISystem::app()
		{
			return mApp;
		}

		Scene::SceneComponentBase& GUISystem::getSceneComponent(size_t i)
		{
			return mApp.getSceneComponent(i);
		}

		Scripting::GlobalAPIComponentBase& GUISystem::getGlobalAPIComponent(size_t i)
		{
			return mApp.getGlobalAPIComponent(i);
		}

		Scene::SceneManager& GUISystem::sceneMgr()
		{
			return mApp.sceneMgr();
		}

		UI::UIManager& GUISystem::ui()
		{
			return *mUI;
		}
	}
}
