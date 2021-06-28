#include "skeletonloaderlib.h"

#include "skeletonloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/math/transformation.h"

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
//MEMBER(mParent)
//MEMBER(mFirstChild)
METATABLE_END(Engine::Render::Bone)

METATABLE_BEGIN(Engine::Render::SkeletonDescriptor)
MEMBER(mBones)
METATABLE_END(Engine::Render::SkeletonDescriptor)

namespace Engine {
namespace Render {

    SkeletonLoader::SkeletonLoader()
        : ResourceLoader({ ".fbx", ".dae" })
    {
    }

    bool SkeletonLoader::loadImpl(SkeletonDescriptor &skeleton, ResourceDataInfo &info)
    {
        Assimp::Importer importer;

        std::vector<unsigned char> buffer = info.resource()->readAsBlob();

        const aiScene *scene = importer.ReadFileFromMemory(buffer.data(), buffer.size(), 0);

        if (!scene) {
            LOG_ERROR(importer.GetErrorString());
            return false;
        }

        if (scene->mNumMeshes == 0) {
            LOG_ERROR("No mesh in file '" << info.resource()->path().str() << "'");
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
                    skeleton.mBones.push_back(Bone { bone->mName.C_Str() });
                }
            }
        }



        bool matrixSet = false;

        assimpTraverseTree(
            scene, [&](const aiNode *parentNode, const Matrix4 &t) {
                Matrix4 anti_t = t.Inverse();

                for (size_t meshIndex = 0; meshIndex < parentNode->mNumMeshes; ++meshIndex) {

                    aiMesh *mesh = scene->mMeshes[parentNode->mMeshes[meshIndex]];

                    if (mesh->mNumBones > 0) {

                        std::set<size_t> parentTransformToDos;

                        for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                            aiBone *bone = mesh->mBones[boneIndex];
                            aiNode *node = scene->mRootNode->FindNode(bone->mName);

                            if (!matrixSet) {
                                matrixSet = true;
                                skeleton.mMatrix = t;
                            }

                            size_t index = indices.at(node);

                            skeleton.mBones[index].mTTransform = assimpConvertMatrix(node->mTransformation);
                            skeleton.mBones[index].mOffsetMatrix = assimpConvertMatrix(bone->mOffsetMatrix) * anti_t;

                            if (node->mParent && indices.count(node->mParent) > 0) {
                                size_t parentIndex = indices.at(node->mParent);
                                skeleton.mBones[index].mParent = parentIndex;
                                if (parentIndex < index && parentTransformToDos.count(parentIndex) == 0)
                                    skeleton.mBones[index].mTTransform = skeleton.mBones[parentIndex].mTTransform * skeleton.mBones[index].mTTransform;
                                else
                                    parentTransformToDos.emplace(index);
                            } 

                            if (node->mNumChildren > 0 && indices.count(node->mChildren[0]) > 0) {
                                skeleton.mBones[index].mFirstChild = indices.at(node->mChildren[0]);
                            }
                        }
                        while (!parentTransformToDos.empty()) {
                            for (std::set<size_t>::iterator it = parentTransformToDos.begin(); it != parentTransformToDos.end();) {
                                size_t parentIndex = skeleton.mBones[*it].mParent;
                                if (parentTransformToDos.count(parentIndex) == 0) {
                                    skeleton.mBones[*it].mTTransform = skeleton.mBones[parentIndex].mTTransform * skeleton.mBones[*it].mTTransform;
                                    it = parentTransformToDos.erase(it);
                                } else {
                                    ++it;
                                }
                            }
                        }
                    }
                }
            });

        for (Bone &bone : skeleton.mBones) {
            bone.mTTransform = skeleton.mMatrix * bone.mTTransform * bone.mOffsetMatrix;
        }

        return true;
    }

    void SkeletonLoader::unloadImpl(SkeletonDescriptor &data, ResourceDataInfo &info)
    {
        data.mBones.clear();
    }

}
}