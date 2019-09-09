#pragma once

#include "Madgine/render/rendertarget.h"
#include "Modules/math/matrix4.h"
#include "openglshaderprogram.h"
#include "util/openglbuffer.h"
#include "util/opengltexture.h"

#include "Madgine/render/renderpassflags.h"

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLRenderTexture : RenderTarget {

        OpenGLRenderTexture(OpenGLRenderWindow *window, uint32_t index, Scene::Camera *camera, const Vector2 &size);
        ~OpenGLRenderTexture();

        uint32_t textureId() const override;

        bool resize(const Vector2 &size) override;

        virtual void render() override;
        virtual void renderVertices(RenderPassFlags flags, size_t groupSize, Vertex *vertices, size_t vertexCount, unsigned int *indices = nullptr, size_t indexCount = 0) override;
        virtual void renderVertices(RenderPassFlags flags, size_t groupSize, Vertex2 *vertices, size_t vertexCount, unsigned int *indices = nullptr, size_t indexCount = 0, unsigned int textureId = 0) override;
        virtual void renderInstancedMesh(RenderPassFlags flags, void *meshData, const std::vector<Matrix4> &transforms) override;
        virtual void clearDepthBuffer() override;

        const OpenGLTexture &texture() const;

    protected:
        void setupProgram(RenderPassFlags flags = RenderPassFlags_None, unsigned int textureId = 0);

        void renderMesh(OpenGLMeshData *mesh, const Matrix4 &transformMatrix = Matrix4::IDENTITY);

    private:
        uint32_t mIndex;

        GLuint mFramebuffer;
        GLuint mDepthRenderbuffer;

        OpenGLShaderProgram mProgram;

        OpenGLTexture mTexture;

        OpenGLBuffer mTempBuffer;
    };

}
}