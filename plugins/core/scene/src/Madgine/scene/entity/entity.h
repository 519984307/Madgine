#pragma once

#include "Modules/keyvalue/scopebase.h"

#include "entitycomponentbase.h"

#include "Modules/serialize/container/syncablecontainer.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct EntityComponentObserver {
            void operator()(const SetIterator<std::unique_ptr<EntityComponentBase>, KeyCompare<std::unique_ptr<EntityComponentBase>>, std::set<std::unique_ptr<EntityComponentBase>, KeyCompare<std::unique_ptr<EntityComponentBase>>>::iterator> &it, int op);
        };

        class MADGINE_SCENE_EXPORT Entity : public Serialize::SerializableUnit<Entity>, public ScopeBase {
            SERIALIZABLEUNIT;

        public:
            Entity(const Entity &, bool local);
            Entity(Entity &&, bool local);

            Entity(SceneManager &sceneMgr, bool local, const std::string &name, const ObjectPtr &behavior = {});
            Entity(const Entity &) = delete;
            ~Entity();

            void remove();

            const char *key() const;

            template <class T>
            T *addComponent(const ObjectPtr &table = {})
            {
                return static_cast<T *>(addComponent(T::componentName(), table));
            }

            template <class T>
            void removeComponent()
            {
                removeComponent(T::componentName());
            }

            template <class T>
            T *getComponent()
            {
                return static_cast<T *>(getComponent(T::componentName()));
            }

            EntityComponentBase *getComponent(const std::string &name);

            decltype(auto) components()
            {
                return uniquePtrToPtr(mComponents);
            }

            template <class T>
            bool hasComponent()
            {
                return hasComponent(T::componentName());
            }

            bool hasComponent(const std::string &name);

            EntityComponentBase *addComponent(const std::string &name, const ObjectPtr &table = {});
            void removeComponent(const std::string &name);

            void writeCreationData(Serialize::SerializeOutStream &of) const;

            SceneManager &sceneMgr(bool = true) const;

            bool isLocal() const;

            template <class T>
            T &getSceneComponent(bool init = true)
            {
                return static_cast<T &>(getSceneComponent(T::component_index(), init));
            }

            SceneComponentBase &getSceneComponent(size_t i, bool = true);

            template <class T>
            T &getGlobalAPIComponent(bool init = true)
            {
                return static_cast<T &>(getGlobalAPIComponent(T::component_index(), init));
            }

            App::GlobalAPIBase &getGlobalAPIComponent(size_t i, bool = true);

            //App::Application &app(bool = true);

        protected:
            EntityComponentBase *addComponentSimple(const std::string &name, const ObjectPtr &table = {});

        public:
            std::string mName;

        private:
            std::tuple<std::unique_ptr<EntityComponentBase>> createComponentTuple(const std::string &name);

            bool mLocal;

            SYNCABLE_CONTAINER(mComponents, std::set<std::unique_ptr<EntityComponentBase>, KeyCompare<std::unique_ptr<EntityComponentBase>>>, Serialize::ContainerPolicies::masterOnly, EntityComponentObserver);

            SceneManager &mSceneManager;
        };

    }
}
}