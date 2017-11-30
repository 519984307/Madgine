#pragma once


#include "signalslot/connectionstore.h"

#include "scripting/types/scope.h"
#include "madgineobject.h"

#include "scripting/types/globalscopebase.h"

#include "uniquecomponentcollector.h"

#include "scripting/types/globalapicomponent.h"

namespace Engine
{
	namespace App
	{
		class MADGINE_BASE_EXPORT Application : public Singleton<Application>,
		                                        public Scripting::Scope<Application, Scripting::GlobalScopeBase>,
		                                        public MadgineObject
		{
		public:
			/**
			* Creates the Application.
			*
			*/
			Application(const Scripting::LuaTable& table = {});
			/**
			* Deletes all objects created by the Application.
			*
			*/
			virtual ~Application();

			/**
			* Loads all plugins listed in <code>settings.mPluginsFile</code>.
			* If no name is given "plugins.cfg" is used by default.
			* Creates a RenderWindow with the given name and all Madgine-Components.
			* If an InputHandler is passed, it will be used instead of the default one.
			* The root-directory for the Application-Resources will be set according to the <code>settings.mRootDir</code>
			*
			* @param settings all necessary information to setup the Application
			*/
			void setup(const AppSettings& settings);

			/**
			* May only be called after a call to setup().
			* Initializes all Madgine-Components.
			*/
			bool init() override;

			void finalize() override;

			/**
			* Tries to call the script-method "init", which must be implemented in a script-file or in a Scripting::GlobalAPI, and to start the Ogre-Renderloop.
			* If "init" is not found, <code>-1</code> is returned.
			* Otherwise the Renderloop will be started.
			* After the Renderloop finished, all game components will be cleared.
			* Note: this method will <b>not</b> return, until the Application is shutdown.
			*
			* @return <code>0</code>, if the Application is started and shutdown properly; <code>-1</code> otherwise
			*/
			virtual int go() = 0;

			/**
			* Marks the Application as shutdown. This causes the Renderloop to return within the next frame.
			*/
			virtual void shutdown();


			/**
			* Convenience method, that creates the Application of type T (which defaults to Application), calls setup(), init() and go() with the given <code>settings</code> and returns the result of the call to go().
			*
			* @return result of the call to go()
			* @param settings the settings for the Application
			*/
			template <class App, class Settings>
			static int run(const Settings& settings)
			{
				App app;
				app.setup(settings);
				if (!app.init()) return -1;
				int result = app.go();
				app.finalize();
				return result;
			}


			/**
			* This will be called by Ogre whenever a new frame was sent to and gets rendered by the GPU. It returns <code>false</code>, if the Application was shutdown().
			* Otherwise it will update all Profilers for frame-profiling, capture input from the Input::InputHandler, update the UI::UIManager and perform all tasks given by callSafe().
			* This is the toplevel method of the Madgine, that should recursively update all elements that need update per frame.
			*
			* @return <code>true</code>, if the Application is not shutdown, <code>false</code> otherwise
			* @param timeSinceLastFrame holds the time since the last frame
			*/
			virtual bool update(float timeSinceLastFrame);

			float fixedRemainder() const;

			bool isShutdown() const;

			float getFPS();

			void addFrameListener(FrameListener* listener);
			void removeFrameListener(FrameListener* listener);

			KeyValueMapList maps() override;

			bool singleFrame(float timeSinceLastFrame);

			using Engine::Singleton<Application>::getSingleton;
			using Engine::Singleton<Application>::getSingletonPtr;

		protected:
			virtual void _clear();

			virtual bool fixedUpdate(float timeStep);

			bool sendFrameStarted(float timeSinceLastFrame);
			bool sendFrameRenderingQueued(float timeSinceLastFrame);
			bool sendFrameEnded(float timeSinceLastFrame);

		private:

			bool mShutDown;

			SignalSlot::ConnectionManager mConnectionManager;

			Scripting::GlobalAPICollector mGlobalAPIs;

			std::unique_ptr<Util::StandardLog> mLog;

			float mTimeBank;

			std::list<FrameListener*> mListeners;

			static constexpr float FIXED_TIMESTEP = 0.015f;
		};
	}
}
