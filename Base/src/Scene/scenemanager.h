#pragma once

#include "MadgineObject.h"
#include "Scripting\Types\refscopetoplevelserializableunit.h"
#include "Scripting\Types\globalapi.h"


#include "Scene\scenecomponent.h"

namespace Engine {
	namespace Scene {

		class MADGINE_BASE_EXPORT SceneManagerBase : public Singleton<SceneManagerBase>,
			public Scripting::RefScopeTopLevelSerializableUnitBase,
			public MadgineObject,
			public Scripting::GlobalAPI<SceneManagerBase>
		{
		public:
			SceneManagerBase();
			virtual ~SceneManagerBase() = default;

			Scripting::ScopeBase *createSceneArray(size_t size);
			Scripting::ScopeBase *createSceneStruct();
			Scripting::ScopeBase *createSceneList();

			virtual Entity::Entity *createEntity(const std::string &behaviour = "", const std::string &name = "", const std::string &mesh = "", const Scripting::ArgumentList &args = {}, std::function<void(Entity::Entity&)> init = {}) = 0;
			virtual Entity::Entity *createLocalEntity(const std::string &behaviour = "", const std::string &name = "", const std::string &mesh = "", const Scripting::ArgumentList &args = {}) = 0;
			virtual Entity::Entity *findEntity(const std::string &name) = 0;
			virtual std::list<Entity::Entity*> entities() = 0;

			virtual Light *createLight() = 0;
			virtual std::list<Light*> lights() = 0;

			virtual bool init() override;
			virtual void finalize() override;
			
			virtual void update(float timeSinceLastFrame, ContextMask context);
			virtual void fixedUpdate(float timeStep, ContextMask context);

			virtual void clear() = 0;

			size_t getComponentCount();

			void saveComponentData(Serialize::SerializeOutStream &out) const;
			void loadComponentData(Serialize::SerializeInStream &in);


			virtual void readState(Serialize::SerializeInStream &in);
			virtual void writeState(Serialize::SerializeOutStream &out) const;
			

			void removeLater(Entity::Entity *e);

			template <class T>
			void connectStateLoaded(T &slot) {
				mStateLoadedSignal.connect(slot);
			}

		protected:
			std::string generateUniqueName();

			virtual void removeQueuedEntities() = 0;

			std::list<Entity::Entity *> mEntityRemoveQueue;

		private:
			size_t mItemCount;

			BaseUniqueComponentCollector<SceneComponentBase> mSceneComponents;

			Engine::SignalSlot::Signal<> mStateLoadedSignal;

		};

		template <class T>
		class SceneManager : public Singleton<T>, public SceneManagerBase {
		public:
			using SceneManagerBase::SceneManagerBase;

		private:
			virtual size_t getSize() const override final {
				return sizeof(T);
			}
		};
		

	}
}