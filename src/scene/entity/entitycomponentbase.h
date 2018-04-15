#pragma once

#include "../../serialize/serializableunit.h"
#include "../../scripting/types/scopebase.h"

namespace Engine
{
	namespace Scene
	{
		namespace Entity
		{
			class MADGINE_BASE_EXPORT EntityComponentBase : public Serialize::SerializableUnitBase, public Scripting::ScopeBase
			{
			public:
				EntityComponentBase(Entity& entity, const Scripting::LuaTable& initTable = {});
				virtual ~EntityComponentBase() = 0;

				virtual void init();
				virtual void finalize();

				void moveToEntity(Entity* ent);
				Entity& getEntity() const;

				virtual const char* key() const = 0;

				template <class T>
				T &getSceneComponent()
				{
					return static_cast<T&>(getSceneComponent(T::component_index()));
				}

				SceneComponentBase &getSceneComponent(size_t i);
				

			private:

				Entity* mEntity;
				Scripting::LuaTable mInitTable;
			};
		}
	}
}