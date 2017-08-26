#pragma once

#include "uniquecomponent.h"
#include "contextmasks.h"
#include "serialize/serializableunit.h"
#include "scripting/types/scope.h"
#include "madgineobject.h"
#include "serialize/serializableunitheapcreator.h"

namespace Engine {
namespace Scene {

class MADGINE_BASE_EXPORT SceneComponentBase : public Serialize::SerializableUnitBase, public Scripting::ScopeBase, public MadgineObject{
public:
    virtual ~SceneComponentBase() = default;

	SceneComponentBase(SceneManagerBase *sceneMgr, ContextMask context = ContextMask::SceneContext);

    void update(float timeSinceLastFrame, ContextMask mask);
	void fixedUpdate(float timeStep, ContextMask mask);

    virtual bool init() override;
	virtual void finalize() override;    

	void setEnabled(bool b);
	bool isEnabled();

	SceneManagerBase *sceneMgr();

	virtual KeyValueMapList maps() override;

protected:
    virtual void update(float);
	virtual void fixedUpdate(float);    

private:
    const ContextMask mContext;

	bool mEnabled;

	SceneManagerBase *mSceneMgr;

};


template <class T>
class SceneComponent : public Scripting::Scope<T, BaseUniqueComponent<T, SceneComponentBase, Serialize::SerializableUnitHeapCreator, Scene::SceneManagerBase*>>{

public:
	using Scripting::Scope<T, BaseUniqueComponent<T, SceneComponentBase, Serialize::SerializableUnitHeapCreator, SceneManagerBase*>>::Scope;

private:
	virtual size_t getSize() const override final {
		return sizeof(T);
	}

};


}

#ifdef _MSC_VER
template class MADGINE_BASE_EXPORT BaseUniqueComponentCollector<Scene::SceneComponentBase, Serialize::SerializableUnitHeapCreator, Scene::SceneManagerBase*>;
#endif

}


