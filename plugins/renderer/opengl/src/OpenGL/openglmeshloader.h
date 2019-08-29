#pragma once

#include "Madgine/resources/resourceloader.h"

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLMeshLoader : Resources::ResourceLoader<OpenGLMeshLoader, OpenGLMeshData, Resources::ThreadLocalResource> {
        OpenGLMeshLoader();

        std::shared_ptr<OpenGLMeshData> loadImpl(ResourceType *res) override;

        static OpenGLMeshData generate(Vertex *vertices, size_t vertexCount, unsigned int *indices = nullptr, size_t indexCount = 0);

        static void update(OpenGLMeshData &data, Vertex *vertices, size_t vertexCount, unsigned int *indices = nullptr, size_t indexCount = 0);
    };

}
}