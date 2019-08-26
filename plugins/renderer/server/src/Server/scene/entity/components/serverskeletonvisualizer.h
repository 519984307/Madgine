#pragma once


#include "Madgine/scene/entity/entitycomponent.h"

namespace Engine
{
	namespace Scene
	{
		namespace Entity
		{
                class MADGINE_SERVERRENDERER_EXPORT ServerSkeletonVisualizer : public EntityComponent<ServerSkeletonVisualizer>
			{
			public:
                    ServerSkeletonVisualizer(Entity &entity, const ObjectPtr &table = {});
				virtual ~ServerSkeletonVisualizer();
			};
		}
	}
}
