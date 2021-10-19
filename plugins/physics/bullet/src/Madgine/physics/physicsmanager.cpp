#include "../bulletlib.h"

#include "physicsmanager.h"

#include "rigidbody.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "physicslistener.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/scene/scenemanager.h"

#include "bullet3-2.89/src/BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "bullet3-2.89/src/btBulletDynamicsCommon.h"

UNIQUECOMPONENT(Engine::Physics::PhysicsManager)

METATABLE_BEGIN(Engine::Physics::PhysicsManager)
PROPERTY(AirDensity, airDensity, setAirDensity)
METATABLE_END(Engine::Physics::PhysicsManager)

SERIALIZETABLE_BEGIN(Engine::Physics::PhysicsManager)
SERIALIZETABLE_END(Engine::Physics::PhysicsManager)

namespace Engine {
namespace Physics {

    bool PhysicsManager::sContactCallback(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0, int partId0, int index0, const btCollisionObjectWrapper *colObj1, int partId1, int index1)
    {
        RigidBody *rigidBody0 = static_cast<RigidBody*>(colObj0->m_collisionObject->getUserPointer());
        RigidBody *rigidBody1 = static_cast<RigidBody *>(colObj1->m_collisionObject->getUserPointer());
        
        Scene::SceneManager *sceneMgr = nullptr;
        if (rigidBody0)
            sceneMgr = rigidBody0->sceneMgr();
        else if (rigidBody1)
            sceneMgr = rigidBody1->sceneMgr();
        else
            return false;

        ContactPoint p { cp, colObj0->m_collisionObject, colObj1->m_collisionObject };
        return sceneMgr->getComponent<PhysicsManager>().contactCallback(p, rigidBody0, rigidBody1);
    }

    bool PhysicsManager::contactCallback(ContactPoint &p, RigidBody *body0, RigidBody *body1)
    {

        for (PhysicsListener *listener : mListener)
            listener->onContact(p, body0, body1);
        return true;
    }

    struct PhysicsData {
        btDefaultCollisionConfiguration mCollisionConfiguration;
        btCollisionDispatcher mDispatcher;
        btDbvtBroadphase mBroadphase;
        btSequentialImpulseConstraintSolver mSolver;
        btSoftRigidDynamicsWorld mWorld;
        btSoftBodyWorldInfo mWorldInfo;

        PhysicsData()
            : mDispatcher(&mCollisionConfiguration)
            , mWorld(&mDispatcher, &mBroadphase, &mSolver, &mCollisionConfiguration)
        {
            mWorld.setGravity({ 0, -9.81f, 0 });

            mWorld.getSolverInfo().m_solverMode |= SOLVER_ENABLE_FRICTION_DIRECTION_CACHING | SOLVER_USE_2_FRICTION_DIRECTIONS;

            mWorldInfo.m_dispatcher = &mDispatcher;
            mWorldInfo.m_broadphase = &mBroadphase;
            mWorldInfo.m_gravity.setValue(0, -9.81, 0);
        }
    };

    PhysicsManager::PhysicsManager(Scene::SceneManager &sceneMgr)
        : VirtualScope(sceneMgr)
    {
    }

    PhysicsManager::~PhysicsManager()
    {
    }

    btSoftRigidDynamicsWorld &PhysicsManager::world()
    {
        return mData->mWorld;
    }

    btSoftBodyWorldInfo &PhysicsManager::worldInfo()
    {
        return mData->mWorldInfo;
    }

    ContactPointList PhysicsManager::contactPoints()
    {
        return { mData.get() };
    }

    void PhysicsManager::addListener(PhysicsListener *listener)
    {
        mListener.push_back(listener);
    }

    void PhysicsManager::removeListener(PhysicsListener *listener)
    {
        mListener.erase(std::find(mListener.begin(), mListener.end(), listener));
    }

    bool PhysicsManager::init()
    {
        mData = std::make_unique<PhysicsData>();

        gContactAddedCallback = &PhysicsManager::sContactCallback;

        return VirtualScope::init();
    }

    void PhysicsManager::finalize()
    {
        mData.reset();

        VirtualScope::finalize();
    }

    void PhysicsManager::update(std::chrono::microseconds timeSinceLastFrame, bool paused)
    {
        //TODO
        if (!paused)
            mData->mWorld.stepSimulation(timeSinceLastFrame.count() / 1000000.0f, 1, 1.0f / 30.0f);
        //else {
        //    mData->mWorld.synchronizeMotionStates();
        //}
    }

    float PhysicsManager::airDensity() const {
        return mData->mWorldInfo.air_density;
    }

    void PhysicsManager::setAirDensity(float density) {
        mData->mWorldInfo.air_density = density;
    }

    ContactPointIterator ContactPointList::begin()
    {
        return { mData, 0, 0 };
    }

    ContactPointIterator ContactPointList::end()
    {
        return { mData, mData->mDispatcher.getNumManifolds(), 0 };
    }

    ContactPointIterator::ContactPointIterator(PhysicsData *data, int manifold, int index)
        : mData(data)
        , mManifold(manifold)
        , mIndex(index)
    {
        update();
    }

    ContactPoint ContactPointIterator::operator*()
    {
        btPersistentManifold *contactManifold = mData->mDispatcher.getManifoldByIndexInternal(mManifold);

        const btCollisionObject *obj0 = contactManifold->getBody0();
        const btCollisionObject *obj1 = contactManifold->getBody1();

        btManifoldPoint &pt = contactManifold->getContactPoint(mIndex);

        return { pt, obj0, obj1 };
    }

    Proxy<ContactPoint> Engine::Physics::ContactPointIterator::operator->()
    {
        return { **this };
    }

    void ContactPointIterator::operator++()
    {
        assert(mManifold < mData->mDispatcher.getNumManifolds());
        ++mIndex;
        update();
    }

    bool ContactPointIterator::operator!=(const ContactPointIterator &other) const
    {
        assert(mData == other.mData);
        return mManifold != other.mManifold || mIndex != other.mIndex;
    }

    void ContactPointIterator::update()
    {
        while (mManifold < mData->mDispatcher.getNumManifolds() && mIndex == mData->mDispatcher.getManifoldByIndexInternal(mManifold)->getNumContacts()) {
            ++mManifold;
            mIndex = 0;
        }
    }

    void ContactPoint::setFriction(float motion, const Vector3 &dir, const Vector3 &dir2)
    {
        btManifoldResult result;

        mPoint.m_contactPointFlags |= BT_CONTACT_FLAG_LATERAL_FRICTION_INITIALIZED;

        mPoint.m_lateralFrictionDir1.setValue(dir.x, dir.y, dir.z);
        //Im3D::Arrow(0.1f, Vector3 { mPoint.m_positionWorldOnA }, Vector3 { mPoint.m_positionWorldOnA + mPoint.m_lateralFrictionDir1 }, Vector4 { 0, 1, 0, 1 });
        ///downscale the friction direction 2 for lower (anisotropic) friction (rather than a unit vector)
        //mPoint.m_lateralFrictionDir2 = 0.9f * mPoint.m_lateralFrictionDir1.cross(mPoint.m_normalWorldOnB);
        mPoint.m_lateralFrictionDir2.setValue(dir2.x, dir2.y, dir2.z);
        //Im3D::Arrow(0.1f, Vector3 { mPoint.m_positionWorldOnA }, Vector3 { mPoint.m_positionWorldOnA + mPoint.m_lateralFrictionDir2 }, Vector4 { 0, 0, 1, 1 });
        ///choose a target surface velocity in the friction dir1 direction, for a conveyor belt effect
        mPoint.m_contactMotion1 = motion;

        mPoint.m_combinedFriction = result.calculateCombinedFriction(mObj0, mObj1);
        mPoint.m_combinedRestitution = result.calculateCombinedRestitution(mObj0, mObj1);
    }

}
}
