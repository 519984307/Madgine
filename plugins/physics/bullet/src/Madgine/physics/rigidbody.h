#pragma once

#include "Madgine/scene/entity/entitycomponent.h"

#include "bullet3-2.89/src/btBulletDynamicsCommon.h"

#include "Madgine/scene/entity/entitycomponentptr.h"

#include "Madgine/scene/entity/components/transform.h"

#include "collisionshapemanager.h"

namespace Engine {
namespace Physics {

    struct MADGINE_BULLET_EXPORT RigidBody : Engine::Scene::Entity::EntityComponent<RigidBody> {

        RigidBody(const Engine::ObjectPtr &data = {});
        RigidBody(RigidBody &&other);
        ~RigidBody();

        RigidBody &operator=(RigidBody &&other);

        void init(const Scene::Entity::EntityPtr &entity);
        void finalize(const Scene::Entity::EntityPtr &entity);

        btRigidBody *get();
        void activate();

        const Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> &transform();

        float mass() const;
        void setMass(float mass);

        bool kinematic() const;
        void setKinematic(bool kinematic);

        float friction() const;
        void setFriction(float friction);

        void setShape(typename CollisionShapeManager::HandleType handle);
        void setShapeName(const std::string_view &name);
        CollisionShapeManager::ResourceType *getShape() const;

        friend struct PhysicsManager;

    private:
        typename CollisionShapeManager::HandleType mShapeHandle;
        struct Data;
        std::unique_ptr<Data> mData;

        static Scene::SceneManager *sceneMgrFromData(Data *data);
    };

    using RigidBodyPtr = Scene::Entity::EntityComponentPtr<RigidBody>;

}
}

RegisterType(Engine::Physics::RigidBody);