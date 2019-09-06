#include "../apislib.h"

#include "im3drenderpass.h"

#include "Madgine/render/rendertarget.h"

#include "im3d.h"
#include "im3d_internal.h"

namespace Engine {
namespace Render {

    void Im3DRenderPass::render(RenderTarget *target, Scene::Camera *camera)
    {
        Im3D::Im3DContext *context = Im3D::GetCurrentContext();

        target->clearDepthBuffer();

        for (const std::pair<Im3DNativeMesh, std::vector<Matrix4>> &p : context->mNativeMeshes)
            target->renderInstancedMesh(RenderPassFlags_NoLighting, p.first, p.second);

		for (size_t i = 0; i < IM3D_MESHTYPE_COUNT; ++i) {
            target->renderVertices(RenderPassFlags_NoLighting, i + 1, context->mVertices[i].data(), context->mVertices[i].size(), context->mIndices[i].data(), context->mIndices[i].size());
            target->renderVertices(RenderPassFlags_NoLighting, i + 1, context->mVertices2[i].data(), context->mVertices2[i].size(), context->mIndices2[i].data(), context->mIndices2[i].size());
        }

    }

}
}
