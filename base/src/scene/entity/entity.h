#pragma once

#include "scripting/types/scope.h"

#include "entitycomponentbase.h"

#include "serialize/container/set.h"

namespace Engine {
namespace Scene {
namespace Entity {




class MADGINE_BASE_EXPORT Entity : public Serialize::SerializableUnit<Entity>, public Scripting::Scope<Entity>
{
private:
    typedef std::function<std::unique_ptr<EntityComponentBase>(Entity &, const Scripting::LuaTable &)> ComponentBuilder;

public:
	Entity(const Entity&);
	Entity(Entity &&);

    Entity(SceneManagerBase *sceneMgr, const std::string &name, const std::string &behaviour = "");
    ~Entity();

	void setup();
    
	void remove();

	const char *key() const;

	template <class T, class... Ty>
	T *addComponent_t(Ty&&... args) {
		if (!hasComponent<T>())
			addComponentImpl(createComponent_t<T>(*this, std::forward<Ty>(args)...));
		return getComponent<T>();
	}

	template <class T>
	void removeComponent_t() {
		if (hasComponent<T>())
			removeComponent(T::componentName());
	}

	template <class T>
	T *getComponent() {
		return static_cast<T*>(getComponent(T::componentName()));
	}

	EntityComponentBase *getComponent(const std::string &name);

	template <class T>
	bool hasComponent() {
		return hasComponent(T::componentName());
	}

	bool hasComponent(const std::string &name);

	void addComponent(const std::string &name, const Scripting::LuaTable &table = {});
	void removeComponent(const std::string &name);

	static bool existsComponent(const std::string &name);

	static std::set<std::string> registeredComponentNames();

	virtual void writeState(Serialize::SerializeOutStream &of) const override;
	virtual void readState(Serialize::SerializeInStream &ifs) override;
	virtual void writeCreationData(Serialize::SerializeOutStream &of) const override;

	SceneManagerBase &sceneMgr() const;

protected:

	virtual KeyValueMapList maps() override;

private:

	template <class T, class... Ty>
	static std::unique_ptr<EntityComponentBase> createComponent_t(Entity &e, Ty&&... args) {
		return std::make_unique<T>(e, std::forward<Ty>(args)...);
	}	

	std::tuple<std::unique_ptr<EntityComponentBase>> createComponent(const std::string &name, const Scripting::LuaTable &table = {});
	std::tuple<std::unique_ptr<EntityComponentBase>> createComponentSimple(const std::string &name) { return createComponent(name); }
	EntityComponentBase *addComponentImpl(std::unique_ptr<EntityComponentBase> &&component);

	template <class T>
	class ComponentRegistrator {
	public:
		ComponentRegistrator() {
			const std::string name = T::componentName();
			assert(sRegisteredComponentsByName().find(name) == sRegisteredComponentsByName().end());
			sRegisteredComponentsByName()[name] = &createComponent_t<T, const Scripting::LuaTable&>;			
		}
		~ComponentRegistrator() {
			const std::string name = T::componentName();
			assert(sRegisteredComponentsByName().find(name) != sRegisteredComponentsByName().end());
			sRegisteredComponentsByName().erase(name);
		}
	};

	static std::map<std::string, ComponentBuilder> &sRegisteredComponentsByName() {
		static std::map<std::string, ComponentBuilder> dummy;
		return dummy;
	}

	template <class T, class Base>
	friend class EntityComponent;


	const std::string mName;

	Serialize::ObservableSet<std::unique_ptr<EntityComponentBase>, Serialize::ContainerPolicy::masterOnly, Serialize::ParentCreator<decltype(&Entity::createComponentSimple), &Entity::createComponentSimple>> mComponents;

	SceneManagerBase *mSceneManager;


};

}
}
}
