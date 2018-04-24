#pragma once

#include "transform.h"

#include "../../../serialize/container/serialized.h"

namespace Engine
{
	namespace Scene
	{
		namespace Entity
		{
			class MADGINE_SERVER_EXPORT ServerTransform : public EntityComponentVirtualImpl<ServerTransform, Transform>
			{
			public:
				ServerTransform(Entity& entity, const Scripting::LuaTable& table = {});
				virtual ~ServerTransform();

				Vector3 getPosition() const;
				Vector3 getScale() const;
				std::array<float, 4> getOrientation() const;

				void setPosition(const Vector3& v);
				void setScale(const Vector3& scale);
				void setOrientation(const std::array<float, 4>& orientation);

				void translate(const Vector3& v);
				void rotate(const std::array<float, 4>& q);

			private:
				Serialize::Serialized<Vector3> mPosition;
				Vector3 mScale;
				std::array<float, 4> mOrientation;
			};
		}
	}
}
