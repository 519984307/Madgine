#pragma once

#include "Modules/math/vector2.h"
#include "Modules/math/matrix4.h"
#include "renderpass.h"
#include "Modules/render/vertex.h"
#include "Modules/render/renderpassflags.h"

#include "textureloader.h"

namespace Engine {
namespace Render {

    struct MADGINE_CLIENT_EXPORT RenderTarget {
        RenderTarget(RenderContext *context);
        RenderTarget(const RenderTarget &) = delete;
        RenderTarget(RenderTarget &&) = default;
        virtual ~RenderTarget();

        void render();
        virtual void setRenderSpace(const Rect2i &space) = 0;
        virtual void renderVertices(RenderPassFlags flags, size_t groupSize, Vertex *vertices, size_t vertexCount, unsigned short *indices = nullptr, size_t indexCount = 0) = 0;
        virtual void renderVertices(RenderPassFlags flags, size_t groupSize, Vertex2 *vertices, size_t vertexCount, unsigned short *indices = nullptr, size_t indexCount = 0, unsigned int textureId = 0) = 0;
        //virtual void renderInstancedMesh(RenderPassFlags flags, Resources::MeshData *meshData, const std::vector<Matrix4> &transforms) = 0;
        virtual void renderMesh(Resources::MeshData *mesh) = 0;
        virtual void clearDepthBuffer() = 0;

		//virtual void setupProgram(RenderPassFlags flags = RenderPassFlags_None, unsigned int textureId = 0) = 0;

        virtual const Texture *texture() const = 0;

        virtual bool resize(const Vector2i &size) = 0;
        virtual Vector2i size() const = 0;

        void addRenderPass(RenderPass *pass);
        void removeRenderPass(RenderPass *pass);

        const std::vector<RenderPass*> &renderPasses();

        virtual void beginFrame();
        virtual void endFrame();

    private:
        RenderContext *mContext;

        std::vector<RenderPass*> mRenderPasses;
    };

}
}