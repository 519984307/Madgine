#pragma once

#include "Modules/serialize/toplevelserializableunit.h"

#include "scenecomponentcollector.h"

#include "Modules/serialize/container/list.h"

#include "entity/entity.h"

#include "../app/globalapicollector.h"
#include "../core/madgineobjectobserver.h"
#include "Modules/keyvalue/observablecontainer.h"
#include "Modules/serialize/container/noparent.h"
#include "Modules/threading/framelistener.h"

#include "scenecomponentset.h"

#include "camera.h"

#include "Modules/threading/datamutex.h"

#include "Modules/serialize/container/offset.h"

namespace Engine {
namespace Scene {
    class MADGINE_BASE_EXPORT SceneManager : public Serialize::TopLevelSerializableUnit<SceneManager>,
                                             public App::GlobalAPI<Serialize::NoParentUnit<SceneManager>>,
                                             public Threading::FrameListener {
    public:
        SceneManager(App::Application &app);
        SceneManager(const SceneManager &) = delete;
        virtual ~SceneManager() = default;

        /*void readState(Serialize::SerializeInStream &in) override;
        void writeState(Serialize::SerializeOutStream &out) const override;*/

        bool frameRenderingQueued(std::chrono::microseconds timeSinceLastFrame, Scene::ContextMask context) override;
        bool frameFixedUpdate(std::chrono::microseconds timeStep, ContextMask context) override final;

        Entity::Entity *createEntity(const std::string &behavior = "", const std::string &name = "",
            const std::function<void(Entity::Entity &)> &init = {});
        Entity::Entity *createLocalEntity(const std::string &behavior = "", const std::string &name = "");
        Entity::Entity *findEntity(const std::string &name);
        void removeLater(Entity::Entity *e);

        Entity::Entity *makeLocalCopy(Entity::Entity &e);
        Entity::Entity *makeLocalCopy(Entity::Entity &&e);

        Scene::Camera *createCamera();
        void destroyCamera(Scene::Camera *camera);

        void clear();

        template <class T>
        T &getComponent(bool init = true)
        {
            return static_cast<T &>(getComponent(T::component_index(), init));
        }
        SceneComponentBase &getComponent(size_t i, bool = true);
        size_t getComponentCount();

        template <class T>
        T &getGlobalAPIComponent(bool init = true)
        {
            return static_cast<T &>(getGlobalAPIComponent(T::component_index(), init));
        }

        App::GlobalAPIBase &getGlobalAPIComponent(size_t i, bool = true);

        SceneManager &getSelf(bool = true);

        virtual App::Application &app(bool = true) override;
        virtual const Core::MadgineObject *parent() const override;

        Threading::DataMutex &mutex();

					SignalSlot::SignalStub<> &clearedSignal();

    protected:
        virtual bool init() final;
        virtual void finalize() final;

        //KeyValueMapList maps() override;

    private:
        void updateCamera(Camera &camera);

        void removeQueuedEntities();

        std::string generateUniqueName();

        std::tuple<SceneManager &, bool, std::string> createNonLocalEntityData(const std::string &name);
        std::tuple<SceneManager &, bool, std::string> createEntityData(const std::string &name, bool local);

    private:
        App::Application &mApp;
        size_t mItemCount;
					
        SceneComponentContainer<PartialObservableContainer<Serialize::PartialSerializableContainer<SceneComponentSet, Engine::Serialize::SerializableOffsetPtr<Self, 100>>::type, Core::MadgineObjectObserver>::type> mSceneComponents;        DEFINE_SERIALIZABLE_OFFSET(mSceneComponents);

        Serialize::ObservableList<::Engine::Serialize::CombinedOffsetPtr<Self, __LINE__>, Entity::Entity, Serialize::ContainerPolicies::masterOnly> mEntities;        DEFINE_COMBINED_OFFSET(mEntities);              
        std::list<Serialize::NoParentUnit<Entity::Entity>> mLocalEntities;
        std::list<Entity::Entity *> mEntityRemoveQueue;

        std::list<Scene::Camera> mCameras;

        SignalSlot::Signal<> mStateLoadedSignal;
        SignalSlot::Signal<> mClearedSignal;

        Threading::DataMutex mMutex;

    public:
        Serialize::ObservableList<::Engine::Serialize::CombinedOffsetPtr<SceneManager, 102>, Entity::Entity, Serialize::ContainerPolicies::masterOnly> &entities();

        SignalSlot::SignalStub<const decltype(mEntities)::iterator &, int> &entitiesSignal();
    };

}
}