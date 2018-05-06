#pragma once


#include "../entitycomponent.h"

namespace Engine
{
	namespace Scene
	{
		namespace Entity
		{
			class MADGINE_CLIENT_EXPORT OgreSkeletonVisualizer : public EntityComponent<OgreSkeletonVisualizer>
			{
			public:
				OgreSkeletonVisualizer(Entity& entity, const Scripting::LuaTable& table = {});
				virtual ~OgreSkeletonVisualizer();

				void init() override;
				void finalize() override;

			protected:
				void showBones(bool b);

				void createBoneMaterial();
				void createBoneMesh();

			private:
				OgreMesh* mMesh;

				std::vector<Ogre::Entity*> mBoneEntities;

				//float mScaleAxes;

				float mBoneSize;

				bool mShowBones;
				Ogre::MaterialPtr mBoneMatPtr;
				Ogre::MeshPtr mBoneMeshPtr;
			};
		}
	}
}
