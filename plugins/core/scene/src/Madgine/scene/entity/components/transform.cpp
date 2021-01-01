#include "../../../scenelib.h"

#include "transform.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/serialize/serializetable_impl.h"

#include "Modules/math/transformation.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "../entitycomponentptr.h"

#include "../../scenemanager.h"


ENTITYCOMPONENT_IMPL(Transform, Engine::Scene::Entity::Transform);

METATABLE_BEGIN(Engine::Scene::Entity::Transform)
PROPERTY(Position, getPosition, setPosition)
PROPERTY(Scale, getScale, setScale)
PROPERTY(Orientation, getOrientation, setOrientation)
METATABLE_END(Engine::Scene::Entity::Transform)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Transform)
FIELD(mParent)
FIELD(mPosition)
FIELD(mScale)
FIELD(mOrientation)
SERIALIZETABLE_END(Engine::Scene::Entity::Transform)

namespace Engine {

namespace Scene {
    namespace Entity {

        Matrix4 Transform::matrix() const
        {
            return TransformMatrix(mPosition, mScale, mOrientation);
        }

        Matrix4 Transform::worldMatrix(const EntityComponentList<Transform> &transforms) const
        {
            return parentMatrix(transforms) * matrix();
        }

        Matrix4 Transform::parentMatrix(const EntityComponentList<Transform> &transforms) const
        {
            if (mParent)
                return transforms.get(mParent)->worldMatrix(transforms);
            else
                return Matrix4::IDENTITY;
        }

        void Transform::setParent(const EntityComponentPtr<Transform> &parent)
        {
            if (parent == this)
                return;
            EntityComponentPtr<Transform> ptr = parent;
            while (ptr) {
                EntityComponentPtr<Transform> next { ptr->mParent, ptr.sceneMgr() };
                if (next == this) {
                    ptr->setParent({});
                    ptr = {};
                } else {
                    ptr = std::move(next);
                }
            }
            mParent = static_cast<EntityComponentHandle<Transform>>(parent);
        }

        const EntityComponentHandle<Transform> &Transform::parent() const
        {
            return mParent;
        }

        const Vector3 &Transform::getPosition() const
        {
            return mPosition;
        }
        const Vector3 &Transform::getScale() const
        {
            return mScale;
        }
        const Quaternion &Transform::getOrientation() const
        {
            return mOrientation;
        }
        void Transform::setPosition(const Vector3 &position)
        {
            mPosition = position;
        }
        void Transform::setScale(const Vector3 &scale)
        {
            mScale = scale;
        }
        void Transform::setOrientation(const Quaternion &orientation)
        {
            mOrientation = orientation;
        }
        void Transform::translate(const Vector3 &v)
        {
            *mPosition += v;
        }
        void Transform::rotate(const Quaternion &q)
        {
            mOrientation *= q;
        }

        Matrix4 EntityComponentPtr<Transform>::worldMatrix() const
        {
            return (*this)->worldMatrix(mSceneMgr->entityComponentList<Transform>());
        }

        Matrix4 EntityComponentPtr<Transform>::parentMatrix() const
        {
            return (*this)->parentMatrix(mSceneMgr->entityComponentList<Transform>());
        }
    }
}
}

