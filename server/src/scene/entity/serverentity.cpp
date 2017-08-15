#include "serverlib.h"

#include "serverentity.h"


#include "scene/entity/masks.h"

namespace Engine {

	

namespace Scene {
namespace Entity {



	ServerEntity::ServerEntity(SceneManagerBase *sceneMgr, const std::string &name, const std::string &object, const std::string &behaviour) :
	Entity(sceneMgr, name),
		mScale{ 1, 1, 1 },
		mOrientation{ {1, 0, 0, 0} },
		mObject(object)
{
		init(behaviour);
}

	ServerEntity::~ServerEntity()
{
}

std::string ServerEntity::getObjectName() const
{
	return mObject;
}

void ServerEntity::setObjectVisible(bool b)
{
	//mObject->setVisible(b);
}

void ServerEntity::setPosition(const Vector3 &v)
{
	mPosition = v;
}

void ServerEntity::setScale(const Vector3 & scale)
{
	mScale = scale;
}

void ServerEntity::setOrientation(const std::array<float, 4>& orientation)
{
	mOrientation = orientation;
}

void ServerEntity::translate(const Vector3 & v)
{
//	mNode->translate(Ogre::Vector3(v.data()));
}

void ServerEntity::rotate(const std::array<float, 4> &q)
{
  //  mNode->rotate(Ogre::Quaternion(const_cast<float*>(q.data())));
}

Vector3 ServerEntity::getPosition() const
{
	return mPosition;
}

Vector3 ServerEntity::getCenter() const
{
	return mPosition;
}


std::array<float, 4> ServerEntity::getOrientation() const
{
	return mOrientation;
}

Vector3 ServerEntity::getScale() const
{
	return mScale;
}

}
}


}
