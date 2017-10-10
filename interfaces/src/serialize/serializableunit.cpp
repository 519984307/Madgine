#include "interfaceslib.h"
#include "serializableunit.h"

#include "streams/bufferedstream.h"

#include "serializemanager.h"

#include "toplevelserializableunit.h"

#include "observable.h"

#include "serializeexception.h"

#include "serializable.h"

namespace Engine {
namespace Serialize {

	thread_local std::list<SerializableUnitBase*> SerializableUnitBase::intern::stack;

	SerializableUnitBase::SerializableUnitBase(SerializableUnitBase *parent, size_t masterId) :
	mSlaveId(0),
		mActive(false),
		mParent(parent)
	{
		if (masterId == 0) {
			mMasterId = SerializeManager::addMasterMapping(this);
		}
		else {
			mMasterId = SerializeManager::addStaticMasterMapping(this, masterId);
		}
		insertInstance();
	}

	SerializableUnitBase::SerializableUnitBase(SerializableUnitBase *parent, const SerializableUnitBase & other) :
		mSlaveId(0),
		mActive(false),
		mParent(parent)
	{
		mMasterId = SerializeManager::addMasterMapping(this);
		insertInstance();
	}

	SerializableUnitBase::SerializableUnitBase(SerializableUnitBase *parent, SerializableUnitBase && other) :
		mSlaveId(0),
		mActive(false),
		mParent(parent)
	{
		std::tie(mMasterId, other.mMasterId) = SerializeManager::updateMasterMapping(other.mMasterId, this);
		insertInstance();
	}

SerializableUnitBase::~SerializableUnitBase()
{
	removeInstance();
	clearSlaveId();
	SerializeManager::removeMasterMapping(mMasterId, this);
}

void SerializableUnitBase::writeState(SerializeOutStream & out) const
{
	for (Serializable *val : mStateValues) {
		val->writeState(out);
	}
}

void SerializableUnitBase::writeId(SerializeOutStream & out) const
{
	out << mMasterId.first;
}

size_t SerializableUnitBase::readId(SerializeInStream & in)
{
	size_t id;
	in >> id;
	if (&in.manager() == topLevel()->getSlaveManager()) {
		setSlaveId(id);
	}
	return id;
}

void SerializableUnitBase::readState(SerializeInStream & in)
{
	for (Serializable *val : mStateValues) {
		val->readState(in);
	}
	
}

void SerializableUnitBase::readAction(BufferedInOutStream & in)
{
	size_t index;
	in >> index;
	try {
		mObservedValues.at(index)->readAction(in);
	}
	catch (const std::out_of_range &) {
		throw SerializeException("Unknown Observed-Id used! Possible binary mismatch!");
	}
}

void SerializableUnitBase::readRequest(BufferedInOutStream & in)
{
	size_t index;
	in >> index;
	try {
		mObservedValues.at(index)->readRequest(in);
	}
	catch (const std::out_of_range &) {
		throw SerializeException("Unknown Observed-Id used! Possible binary mismatch!");
	}
}

size_t SerializableUnitBase::addObservable(Observable * val)
{	
	mObservedValues.push_back(val);
	return mObservedValues.size()-1;
}

void SerializableUnitBase::addSerializable(Serializable * val)
{
	mStateValues.push_back(val);
}

std::set<BufferedOutStream*, CompareStreamId> SerializableUnitBase::getMasterMessageTargets()
{
	std::set<BufferedOutStream *, CompareStreamId> result;
	if (mActive) {
		result = mParent->getMasterMessageTargets();
	}
	return result;
}

BufferedOutStream * SerializableUnitBase::getSlaveMessageTarget()
{
	BufferedOutStream* result = nullptr;
	if (mActive) {
		result = topLevel()->getSlaveMessageTarget();
	}
	return result;
}


void SerializableUnitBase::writeCreationData(SerializeOutStream & out) const
{
}

const TopLevelSerializableUnitBase * SerializableUnitBase::topLevel() const
{
	return mParent->topLevel();
}

void SerializableUnitBase::clearSlaveId()
{
	if (mSlaveId != 0) {
		topLevel()->getSlaveManager()->removeSlaveMapping(this);
		mSlaveId = 0;
	}
}

void SerializableUnitBase::postConstruct()
{
	removeInstance();
}

void SerializableUnitBase::insertInstance() {
	findParentIt(this, this + 1);
	intern::stack.emplace_front(this);
	//std::cout << "Stack size: " << sStack.size() << std::endl;
}

void SerializableUnitBase::removeInstance() {
	auto it = std::find(intern::stack.begin(), intern::stack.end(), this);
	if (it != intern::stack.end()) {
		intern::stack.erase(intern::stack.begin(), ++it);
		//std::cout << "Stack size: " << sStack.size() << std::endl;
	}
}

SerializableUnitBase * SerializableUnitBase::findParent(void * from, void * to) {
	auto it = findParentIt(from, to);
	if (it != intern::stack.end()) {
		return *it;
	}
	else {
		return nullptr;
	}

}

std::list<SerializableUnitBase*>::iterator SerializableUnitBase::findParentIt(void * from, void * to)
{
	auto it = std::find_if(intern::stack.begin(), intern::stack.end(), [&](SerializableUnitBase * const p) {return p <= from && to <= reinterpret_cast<char*>(p) + p->getSize(); });
	if (it != intern::stack.end()) {
		intern::stack.erase(intern::stack.begin(), it);
	}
	return it;
}

size_t SerializableUnitBase::getSize() const
{
	return sizeof(SerializableUnitBase);
}


size_t SerializableUnitBase::slaveId()
{
	return mSlaveId;
}

size_t SerializableUnitBase::masterId()
{
	return mMasterId.first;
}

void SerializableUnitBase::setSlaveId(size_t id)
{
	if (mSlaveId != id) {
		if (mSlaveId != 0) {
			clearSlaveId();
		}
		mSlaveId = id;
		topLevel()->getSlaveManager()->addSlaveMapping(this);		
	}
}

void SerializableUnitBase::applySerializableMap(const std::map<size_t, SerializableUnitBase*>& map)
{
	for (Serializable *ser : mStateValues) {
		ser->applySerializableMap(map);
	}
}

void SerializableUnitBase::setActive(bool b) {
	assert(mActive != b);
	if (b)
		mActive = true;
	for (Serializable *ser : mStateValues) {
		ser->setActive(b);
	}
	if (!b)
		mActive = false;
}

void SerializableUnitBase::activate()
{
	setActive(true);
}

void SerializableUnitBase::deactivate()
{
	setActive(false);
}

bool SerializableUnitBase::isActive() const
{
	return mActive;
}

} // namespace Serialize
} // namespace Core

