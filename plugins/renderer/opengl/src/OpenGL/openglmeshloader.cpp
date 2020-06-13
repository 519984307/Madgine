#include "opengllib.h"

#include "openglmeshloader.h"

#include "openglmeshdata.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/reflection/classname.h"

#include "imageloaderlib.h"
#include "imagedata.h"
#include "imageloader.h"

#include "openglrendercontext.h"

VIRTUALUNIQUECOMPONENT(Engine::Render::OpenGLMeshLoader);

METATABLE_BEGIN_BASE(Engine::Render::OpenGLMeshLoader, Engine::Render::MeshLoader)
MEMBER(mResources)
METATABLE_END(Engine::Render::OpenGLMeshLoader)

METATABLE_BEGIN_BASE(Engine::Render::OpenGLMeshLoader::ResourceType, Engine::Render::MeshLoader::ResourceType)
METATABLE_END(Engine::Render::OpenGLMeshLoader::ResourceType)



namespace Engine {
namespace Render {

    OpenGLMeshLoader::OpenGLMeshLoader()
    {
    }

    bool OpenGLMeshLoader::generateImpl(MeshData &_data, const std::vector<std::optional<AttributeDescriptor>> &attributeList, const AABB &bb, size_t groupSize, ByteBuffer vertices, size_t vertexSize, std::vector<unsigned short> indices, const Filesystem::Path &texturePath)
    {
        OpenGLMeshData &data = static_cast<OpenGLMeshData &>(_data);
        data.mAABB = bb;

        OpenGLRenderContext::execute([=, &data, vertices { std::move(vertices) }, indices { std::move(indices) }]() mutable {
            data.mVAO = create;
            data.mVAO.bind();

            data.mVertices = GL_ARRAY_BUFFER;

            if (!indices.empty()) {
                data.mIndices = GL_ELEMENT_ARRAY_BUFFER;
            }

            if (!texturePath.empty()) {
                std::string imageName = texturePath.stem();
                Resources::ImageLoader::HandleType tex;
                tex.load(imageName);

                data.mTexture = { GL_UNSIGNED_BYTE };
                data.mTexture.setFilter(GL_NEAREST);
                data.mTexture.setData({ tex->mWidth, tex->mHeight }, { tex->mBuffer, static_cast<size_t>(tex->mWidth * tex->mHeight) });
                data.mTextureHandle = data.mTexture.handle();
            }

            updateImpl(data, bb, groupSize, std::move(vertices), vertexSize, std::move(indices));

            for (int i = 0; i < attributeList.size(); ++i) {
                if (attributeList[i])
                    data.mVAO.enableVertexAttribute(i, *attributeList[i]);
                else
                    data.mVAO.disableVertexAttribute(i);
            }

            data.mVAO.unbind();
        });

        return true;
    }

    void OpenGLMeshLoader::updateImpl(MeshData &_data, const AABB &bb, size_t groupSize, ByteBuffer vertices, size_t vertexSize, std::vector<unsigned short> indices)
    {
        OpenGLMeshData &data = static_cast<OpenGLMeshData &>(_data);

        data.mAABB = bb;

        OpenGLRenderContext::execute([=, &data, vertices { std::move(vertices) }, indices { std::move(indices) }]() mutable {
            data.mGroupSize = groupSize;

            data.mVertices.setData(vertices);

            if (!indices.empty()) {
                data.mIndices.setData(indices);
                data.mElementCount = indices.size();
            } else {
                data.mElementCount = vertices.mSize / vertexSize;
            }
        });
    }

    void OpenGLMeshLoader::resetImpl(MeshData &data)
    {
        static_cast<OpenGLMeshData &>(data).reset();
    }

}
}
