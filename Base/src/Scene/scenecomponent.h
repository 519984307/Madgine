#pragma once

#include "uniquecomponent.h"
#include "contextmasks.h"
#include "Serialize/serializableunit.h"
#include "Scripting\Types\globalapi.h"

namespace Engine {
namespace Scene {

class MADGINE_BASE_EXPORT SceneComponentBase : public Serialize::SerializableUnitBase, public MadgineObject{
public:
    virtual ~SceneComponentBase() = default;

    SceneComponentBase(ContextMask context = ContextMask::SceneContext);

    void update(float timeSinceLastFrame, ContextMask mask);
	void fixedUpdate(float timeStep, ContextMask mask);

    virtual bool init();
	virtual void finalize();    

	void setEnabled(bool b);
	bool isEnabled();

	SceneManagerBase *sceneMgr();

protected:
    virtual void update(float);
	virtual void fixedUpdate(float);    

private:
    const ContextMask mContext;

	bool mEnabled;

	SceneManagerBase *mSceneMgr;

};


template <class T>
class SceneComponent : public UniqueComponent<T, SceneComponentBase>, public Scripting::GlobalAPI<T>{

public:
	using UniqueComponent<T, SceneComponentBase>::UniqueComponent;

private:
	virtual size_t getSize() const override final {
		return sizeof(T);
	}

};


}

#ifdef _MSC_VER
template class MADGINE_BASE_EXPORT BaseUniqueComponentCollector<Scene::SceneComponentBase>;
#endif

}


