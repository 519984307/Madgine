#pragma once

#include "Serialize\serializableunit.h"
#include "Serialize\Container\observed.h"
#include "Serialize\Container\action.h"



namespace Engine {
	namespace Util {

		class TrackerAccessor : public Ogre::MemoryTracker {
		public:
			using MemoryTracker::AllocationMap;
			using MemoryTracker::Alloc;

			using MemoryTracker::mAllocations;
		};

		class MADGINE_EXPORT AppStats : public Serialize::SerializableUnit {
		public:
			AppStats(Ogre::RenderWindow *window);

			
			void update();

		protected:
			void startTrackImpl();
			void stopTrackImpl();

		private:
			Engine::Serialize::Observed<float> mAverageFPS;
			
#if OGRE_MEMORY_TRACKER
			TrackerAccessor::AllocationMap mMemoryImage;
			TrackerAccessor &mTracker;
			Engine::Serialize::Observed<size_t> mOgreMemory;
			Engine::Serialize::Action<Engine::Serialize::ActionPolicy::standard> startTrack, stopTrack;
#endif		

			Ogre::RenderWindow *mWindow;

		};

	}
}
