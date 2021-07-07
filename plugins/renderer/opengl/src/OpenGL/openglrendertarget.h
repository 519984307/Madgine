#pragma once

#include "Madgine/render/rendertarget.h"

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLRenderTarget : RenderTarget {

        OpenGLRenderTarget(OpenGLRenderContext *context);
        ~OpenGLRenderTarget();

        virtual void beginFrame() override;

        virtual void setRenderSpace(const Rect2i &space) override;
        virtual void renderVertices(Program *program, size_t groupSize, std::vector<Vertex> vertices, std::vector<unsigned short> indices = {}) override;
        virtual void renderVertices(Program *program, size_t groupSize, std::vector<Vertex2> vertices, std::vector<unsigned short> indices = {}, TextureHandle texture = 0) override;
        virtual void renderMesh(GPUMeshData *mesh, Program *program) override;
        virtual void clearDepthBuffer() override;

        virtual void bindTexture(const std::vector<TextureHandle> &tex) override;

        //void setupProgram(RenderPassFlags flags = RenderPassFlags_None, unsigned int textureId = 0) override;
    };

}
}