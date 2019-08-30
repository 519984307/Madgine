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

        target->renderTriangles(context->mTriangleVertices.data(), context->mTriangleVertices.size(), context->mTriangleIndices.data(), context->mTriangleIndices.size());        
    }

}
}