#pragma once

#include "Interfaces/scripting/types/scope.h"

#include "Interfaces/serialize/toplevelserializableunit.h"

#include "scenecomponentcollector.h"

#include "Interfaces/serialize/container/list.h"

#include "entity/entity.h"

#include "Interfaces/serialize/container/noparent.h"
#include "Interfaces/threading/framelistener.h"
#include "Interfaces/generic/observablecontainer.h"
#include "../core/madgineobjectobserver.h"
#include "../app/globalapicollector.h"

#include "scenecomponentset.h"

#include "camera.h"

namespace Engine
{
	namespace Scene
	{
		class MADGINE_BASE_EXPORT SceneManager : public Serialize::TopLevelSerializableUnit<SceneManager>,
			public Scripting::Scope<SceneManager, GlobalAPIComponent<Serialize::NoParentUnit<SceneManager>>>,
			public Threading::FrameListener
		{
		public:
			SceneManager(App::Application &app);
			virtual ~SceneManager() = default;

			void readState(Serialize::SerializeInStream& in) override;
			void writeState(Serialize::SerializeOutStream& out) const override;

			bool frameRenderingQueued(std::chrono::microseconds timeSinceLastFrame, Scene::ContextMask context) override;
			bool frameFixedUpdate(std::chrono::microseconds timeStep, ContextMask context) override final;


			Entity::Entity* createEntity(const std::string& behavior = "", const std::string& name = "",
			                             const std::function<void(Entity::Entity&)> &init = {});
			Entity::Entity* createLocalEntity(const std::string& behavior = "", const std::string& name = "");
			Entity::Entity* findEntity(const std::string& name);
			std::list<Entity::Entity*> entities();
			void removeLater(Entity::Entity* e);


			Entity::Entity* makeLocalCopy(Entity::Entity& e);
			Entity::Entity* makeLocalCopy(Entity::Entity&& e);			

			Scene::Camera *createCamera();
			void destroyCamera(Scene::Camera *camera);

			void clear();

			template <class T>
			T &getComponent(bool init = true)
			{
				return static_cast<T&>(getComponent(T::component_index(), init));
			}
			SceneComponentBase &getComponent(size_t i, bool = true);
			size_t getComponentCount();

			template <class T>
			T &getGlobalAPIComponent(bool init = true)
			{
				return static_cast<T&>(getGlobalAPIComponent(T::component_index(), init));
			}

			App::GlobalAPIBase &getGlobalAPIComponent(size_t i, bool = true);


			SceneManager &getSelf(bool = true);

			virtual App::Application &app(bool = true) override;
			virtual const Core::MadgineObject *parent() const override;

		protected:

			virtual bool init() final;
			virtual void finalize() final;

			KeyValueMapList maps() override;

		private:

			void updateCamera(Camera &camera);

			void removeQueuedEntities();

			std::string generateUniqueName();

			std::tuple<SceneManager &, bool, std::string> createNonLocalEntityData(const std::string& name);
			std::tuple<SceneManager &, bool, std::string> createEntityData(const std::string& name, bool local);

		private:
			App::Application &mApp;
			size_t mItemCount;

			SceneComponentContainer<PartialObservableContainer<SceneComponentSet, Core::MadgineObjectObserver>> mSceneComponents;

			Serialize::ObservableList<Entity::Entity, Serialize::ContainerPolicies::masterOnly, Serialize::ParentCreator<&SceneManager::createNonLocalEntityData>> mEntities;
			std::list<Serialize::NoParentUnit<Entity::Entity>> mLocalEntities;
			std::list<Entity::Entity *> mEntityRemoveQueue;

			std::list<Scene::Camera> mCameras;

			SignalSlot::Signal<> mStateLoadedSignal;
			SignalSlot::Signal<> mClearedSignal;

		public:
			SignalSlot::SignalStub<const decltype(mEntities)::iterator &, int> &entitiesSignal();

			
		};

	}
}

RegisterType(Engine::Scene::SceneManager);