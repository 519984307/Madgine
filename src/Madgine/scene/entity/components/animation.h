#pragma once

#include "../entitycomponent.h"
#include "Interfaces/serialize/container/serializedmapper.h"

namespace Engine
{
	namespace Scene
	{
		namespace Entity
		{

			class MADGINE_BASE_EXPORT Animation : public EntityComponentVirtualBase<Animation>
			{
			public:
				using EntityComponentVirtualBase<Animation>::EntityComponentVirtualBase;
				virtual ~Animation() = default;

				virtual std::string getDefaultAnimation() const = 0;
				void resetDefaultAnimation();
				virtual void setDefaultAnimation(const std::string &name) = 0;

			private:
				std::string getDefaultAnimation2() const
				{
					return getDefaultAnimation();
				}

				void setDefaultAnimation2(const std::string &name)
				{
					setDefaultAnimation(name);
				}

				Serialize::SerializedMapper<&Animation::getDefaultAnimation2, 
				&Animation::setDefaultAnimation2>
					mSerializedObject;

			};

		}
	}
}

RegisterType(Engine::Scene::Entity::Animation);