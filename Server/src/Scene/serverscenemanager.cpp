#include "serverlib.h"

#include "serverSceneManager.h"

#include "exceptionmessages.h"

#include "Serialize\Streams\serializestream.h"

#include "Scripting\Parsing\scriptparser.h"

namespace Engine {

namespace Scene {


ServerSceneManager::ServerSceneManager()
{
}

ServerSceneManager::~ServerSceneManager()
{
}

bool ServerSceneManager::init()
{
	return SceneManager::init();
}

void ServerSceneManager::finalize()
{
	SceneManager::finalize();
}

std::list<Entity::Entity *> ServerSceneManager::entities()
{
    std::list<Entity::Entity*> result;
    for (Entity::Entity &e : mEntities){
        if (std::find(mEntityRemoveQueue.begin(), mEntityRemoveQueue.end(), &e) == mEntityRemoveQueue.end())
            result.push_back(&e);
    }
    return result;
}


void ServerSceneManager::writeState(Serialize::SerializeOutStream &out) const
{
	out << mStaticSceneName << ValueType::EOL() << ValueType::EOL();

	SceneManager::writeState(out);    
}

void ServerSceneManager::readState(Serialize::SerializeInStream &in)
{
	ValueType dummy;
	in >> mStaticSceneName >> dummy >> dummy;
	
	SceneManager::readState(in);		
}

void ServerSceneManager::readScene(Serialize::SerializeInStream & in, bool callInit)
{
	in.process().startSubProcess(1, "Loading Level...");

	readState(in);
	
	applySerializableMap(in.manager().slavesMap());

	if (callInit) {
		for (Entity::Entity &e : mEntities) {
			e.init();
		}
	}

	in.process().endSubProcess();
}

void ServerSceneManager::writeScene(Serialize::SerializeOutStream & out) const
{
	writeState(out);
}

void ServerSceneManager::makeLocalCopy(Entity::ServerEntity & e)
{
	mLocalEntities.emplace_back(e);
}

void ServerSceneManager::makeLocalCopy(Entity::ServerEntity && e)
{
	mLocalEntities.emplace_back(std::forward<Entity::ServerEntity>(e));
}


std::tuple<const Scripting::Parsing::EntityNode *, std::string, std::string> ServerSceneManager::createEntityData(const std::string & behaviour, const std::string & name, const std::string & meshName)
{

	std::string actualName = name.empty() ? generateUniqueName() : name;

	const Scripting::Parsing::EntityNode *behaviourNode = 0;
	if (!behaviour.empty()) {
		behaviourNode = &Scripting::Parsing::ScriptParser::getSingleton().getEntityDescription(behaviour);
	}

	return std::make_tuple(behaviourNode, actualName, meshName);
	
}

Entity::Entity *ServerSceneManager::createEntity(const std::string &behaviour, const std::string &name, const std::string &meshName, const Scripting::ArgumentList &args, std::function<void(Entity::Entity&)> init)
{
	mEntities.emplace_tuple_back_safe([&](const decltype(mEntities)::iterator &it) {
		it->init(args);
		if (init)
			init(*it);
	}, createEntityData(behaviour, name, meshName));
	return &mEntities.back();
}

Entity::Entity * ServerSceneManager::createLocalEntity(const std::string & behaviour, const std::string & name, const std::string & meshName, const Scripting::ArgumentList & args)
{
	throw 0;
	const std::tuple<const Scripting::Parsing::EntityNode *, std::string, std::string> &data = createEntityData(behaviour, name, meshName);
	mLocalEntities.emplace_back(std::get<0>(data), std::get<1>(data), std::get<2>(data));
	Entity::Entity &e = mLocalEntities.back();
	e.init(args);
	return &e;
}

void ServerSceneManager::removeQueuedEntities()
{
    std::list<Entity::Entity *>::iterator it = mEntityRemoveQueue.begin();

	auto find = [&](const Entity::Entity &ent) {return &ent == *it; };

    while (it != mEntityRemoveQueue.end()) {

		auto ent = std::find_if(mEntities.begin(), mEntities.end(), find);
        if (ent == mEntities.end()) throw 0;

		mEntities.erase(ent);

        it = mEntityRemoveQueue.erase(it);
    }
}

Light * ServerSceneManager::createLight()
{
	return &*mLights.emplace_back();
}

std::list<Light*> ServerSceneManager::lights()
{
	std::list<Light*> result;
	for (Light &light : mLights) {
		result.push_back(&light);
	}
	return result;
}

void ServerSceneManager::clear()
{

	for (Entity::Entity &e : mEntities) {
		e.clear();
	}
    mEntities.clear();
    mEntityRemoveQueue.clear();
	mLights.clear();

}


Entity::Entity * ServerSceneManager::findEntity(const std::string & name)
{
	auto it = std::find_if(mEntities.begin(), mEntities.end(), [&](const Entity::Entity &e) {
		return e.getName() == name;
	});
	if (it == mEntities.end()) {
		throw 0;
	}
	else
		return &*it;
}

}
}

