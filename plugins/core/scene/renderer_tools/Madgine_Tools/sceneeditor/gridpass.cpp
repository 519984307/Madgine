#include "../scenerenderertoolslib.h"

#include "gridpass.h"

//#include "OpenGL/openglshaderloader.h"

#include "meshloader.h"

#include "Meta/math/matrix4.h"

#include "Madgine/render/camera.h"
#include "Madgine/render/rendertarget.h"
#include "render/vertex.h"

#include "gpumeshloader.h"

namespace Engine {
namespace Tools {

    GridPass::GridPass(Render::Camera *camera, int priority)
        : mCamera(camera)
        , mPriority(priority)
    {

        mMesh.load("Plane");

        mPipeline.createStatic({ .vs = "grid", .ps = "grid", .format = type_holder<Compound<Render::VertexPos_4D>>, .bufferSizes = { 0, sizeof(GridPerFrame) } });

    }

    void GridPass::render(Render::RenderTarget *target, size_t iteration)
    {
        /* if (!mProgram.available())
            return;*/
        if (!mMesh.available())
            return;

        target->pushAnnotation("Grid");

        Vector2i size = target->size();
        float aspectRatio = float(size.x) / size.y;
        {
            auto parameters = mPipeline.mapParameters(1).cast<GridPerFrame>();

            parameters->vp = mCamera->getViewProjectionMatrix(aspectRatio);
        }

        target->renderMesh(mMesh, mPipeline);

        target->popAnnotation();
    }

    int GridPass::priority() const
    {
        return mPriority;
    }
}
}