#pragma once

#include "Profiler.h"
#include "AppStats.h"

#include "Serialize\toplevelserializableunit.h"

#include "Serialize\Container\serialized.h"

#include "OgreUtilLog.h"

namespace Engine {
	namespace Util {

		class MADGINE_EXPORT Util : public Ogre::Singleton<Util>, public Ogre::GeneralAllocatedObject, public Serialize::TopLevelSerializableUnit {
		public:
			Util(Ogre::RenderWindow *window);

			Profiler *profiler();

			void update();

		private:
			OgreUtilLog mLog;

			Serialize::Serialized<Profiler> mProfiler;
			Serialize::Serialized<AppStats> mStats;

		};

	}
}