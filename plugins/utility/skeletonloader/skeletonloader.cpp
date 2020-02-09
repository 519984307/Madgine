#include "skeletonloaderlib.h"

#include "skeletonloader.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/math/transformation.h"
#include "Modules/reflection/classname.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "../assimptools.h"

UNIQUECOMPONENT(Engine::Render::SkeletonLoader)

METATABLE_BEGIN(Engine::Render::SkeletonLoader)
MEMBER(mResources)
METATABLE_END(Engine::Render::SkeletonLoader)

METATABLE_BEGIN_BASE(Engine::Render::SkeletonLoader::ResourceType, Engine::Resources::ResourceBase)
READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Render::SkeletonLoader::ResourceType)

METATABLE_BEGIN(Engine::Render::Bone)
MEMBER(mName)
MEMBER(mOffsetMatrix)
MEMBER(mTTransform)
MEMBER(mParent)
MEMBER(mFirstChild)
METATABLE_END(Engine::Render::Bone)

METATABLE_BEGIN(Engine::Render::SkeletonDescriptor)
MEMBER(mBones)
MEMBER(mBaseTransform)
METATABLE_END(Engine::Render::SkeletonDescriptor)

RegisterType(Engine::Render::SkeletonLoader)

    namespace Engine
{
    namespace Render {

        SkeletonLoader::SkeletonLoader()
            : ResourceLoader({ ".fbx", ".dae" })
        {
        }

        bool SkeletonLoader::loadImpl(SkeletonDescriptor &skeleton, ResourceType *res)
        {
            Assimp::Importer importer;

            std::vector<unsigned char> buffer = res->readAsBlob();

            const aiScene *scene = importer.ReadFileFromMemory(buffer.data(), buffer.size(), 0);

            if (!scene) {
                LOG_ERROR(importer.GetErrorString());
                return false;
            }

            if (scene->mNumMeshes == 0) {
                LOG_ERROR("No mesh in file '" << res->path().str() << "'");
                return false;
            }

            std::map<aiNode *, size_t> indices;

            for (size_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
                aiMesh *mesh = scene->mMeshes[meshIndex];

                for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                    aiBone *bone = mesh->mBones[boneIndex];
                    aiNode *node = scene->mRootNode->FindNode(bone->mName);

                    bool newBone = indices.try_emplace(node, skeleton.mBones.size()).second;
                    if (newBone) {

                        skeleton.mBones.push_back(
                            Bone {
                                bone->mName.C_Str(),
                                assimpConvertMatrix(bone->mOffsetMatrix) });
                    }
                }
            }

            assimpTraverseTree(
                scene, [&](const aiNode *parentNode, const Matrix4 &matrix) {
                    for (size_t meshIndex = 0; meshIndex < parentNode->mNumMeshes; ++meshIndex) {
                        skeleton.mBaseTransform = matrix;

                        aiMesh *mesh = scene->mMeshes[parentNode->mMeshes[meshIndex]];

                        if (mesh->mNumBones > 0) {

                            for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                                aiBone *bone = mesh->mBones[boneIndex];
                                aiNode *node = scene->mRootNode->FindNode(bone->mName);

                                size_t index = indices.at(node);

                                if (node->mParent && indices.count(node->mParent) > 0) {
                                    size_t parentIndex = indices.at(node->mParent);
                                    skeleton.mBones[index].mParent = parentIndex;
                                    assert(parentIndex < index);
                                    skeleton.mBones[index].mTTransform = skeleton.mBones[parentIndex].mTTransform * assimpConvertMatrix(node->mTransformation);
                                } else {
                                    skeleton.mBones[index].mTTransform = matrix * assimpConvertMatrix(node->mTransformation);
                                }

                                if (node->mNumChildren > 0 && indices.count(node->mChildren[0]) > 0) {
                                    skeleton.mBones[index].mFirstChild = indices.at(node->mChildren[0]);
                                }                                
                            }
                        }
                    }
                },
                Matrix4 { ScalingMatrix(0.01 * Vector3 { Vector3::UNIT_SCALE }) });

            return true;
        }

        void SkeletonLoader::unloadImpl(SkeletonDescriptor &data, ResourceType *res)
        {
            data.mBones.clear();
        }

    }
}