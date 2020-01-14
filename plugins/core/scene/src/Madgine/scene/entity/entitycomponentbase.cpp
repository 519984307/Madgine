#include "../../scenelib.h"
#include "entitycomponentbase.h"
#include "entity.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/reflection/classname.h"

#include "Modules/serialize/serializetable_impl.h"

namespace Engine {

namespace Scene {
    namespace Entity {
        EntityComponentBase::EntityComponentBase(Entity &entity, const ObjectPtr &initTable)
            : mEntity(&entity)
            , mInitTable(initTable)
        {
        }

        EntityComponentBase::~EntityComponentBase()
        {
        }

        void EntityComponentBase::init()
        {
        }

        void EntityComponentBase::finalize()
        {
        }

        Entity &EntityComponentBase::getEntity() const
        {
            return *mEntity;
        }

        EntityComponentBase *EntityComponentBase::getComponent(const std::string &name)
        {
            return mEntity->getComponent(name);
        }

        EntityComponentBase *EntityComponentBase::addComponent(const std::string &name, const ObjectPtr &init)
        {
            return mEntity->addComponent(name, init);
        }

        SceneComponentBase &EntityComponentBase::getSceneComponent(size_t i, bool init)
        {
            return mEntity->getSceneComponent(i, init);
        }

        App::GlobalAPIBase &EntityComponentBase::getGlobalAPIComponent(size_t i, bool init)
        {
            return mEntity->getGlobalAPIComponent(i, init);
        }

        const ObjectPtr &EntityComponentBase::initTable()
        {
            return mInitTable;
        }

        void EntityComponentBase::moveToEntity(Entity *entity)
        {
            mEntity = entity;
        }

        void EntityComponentBase::writeCreationData(Serialize::SerializeOutStream &out) const
        {
            out << key();
        }

    }
}
}

METATABLE_BEGIN(Engine::Scene::Entity::EntityComponentBase)
METATABLE_END(Engine::Scene::Entity::EntityComponentBase)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::EntityComponentBase)
SERIALIZETABLE_END(Engine::Scene::Entity::EntityComponentBase)

RegisterType(Engine::Scene::Entity::EntityComponentBase);