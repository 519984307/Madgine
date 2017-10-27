#pragma once

#include "util/process.h"
#include "imagesets/imagesetmanager.h"
#include "scripting/parsing/ogrescriptparser.h"

namespace Engine {
	namespace Resources {

		class OGREMADGINE_EXPORT ResourceLoader : public Engine::Util::Process, public Ogre::ResourceGroupListener, public Ogre::Singleton<ResourceLoader>, public Ogre::GeneralAllocatedObject {
		public:
			ResourceLoader(App::OgreApplication *app, const std::string &mediaPath);
			~ResourceLoader();

			bool load_async();
			bool load();
			void loadScripts();

			std::string getMediaPath(const std::string &filename, const std::string &folder = "");

			void resourceGroupScriptingStarted(const Ogre::String &groupName,
				size_t scriptCount);
			void scriptParseStarted(const Ogre::String &scriptName, bool &skipThisScript);
			void scriptParseEnded(const Ogre::String &scriptName, bool skipped);
			void resourceGroupScriptingEnded(const Ogre::String &groupName);
			void resourceGroupLoadStarted(const Ogre::String &groupName,
				size_t resourceCount);
			void resourceLoadStarted(const Ogre::ResourcePtr &resource);
			void resourceLoadEnded(void);
			void worldGeometryStageStarted(const Ogre::String &description);
			void worldGeometryStageEnded(void);
			void resourceGroupLoadEnded(const Ogre::String &groupName);

			Scripting::Parsing::ScriptParser *scriptParser();

		private:
			Ogre::ResourceGroupManager *mRgm;

			App::Application *mApp;

			std::string mMediaPath;

			std::unique_ptr<Scripting::Parsing::OgreScriptParser> mParser;
			std::unique_ptr<ImageSets::ImageSetManager> mImageSetManager;
		};

	}
}