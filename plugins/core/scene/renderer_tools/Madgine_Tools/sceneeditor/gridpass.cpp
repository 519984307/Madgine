#include "../scenerenderertoolslib.h"

#include "gridpass.h"

//#include "OpenGL/openglshaderloader.h"

#include "Modules/math/matrix4.h"

#include "Madgine/render/camera.h"
#include "Madgine/render/rendertarget.h"
#include "Modules/render/vertex.h"

#include "meshloader.h"

#include "program.h"
#include "programloader.h"

namespace Engine {
namespace Tools {

    GridPass::GridPass(Render::Camera *camera, int priority)
        : mCamera(camera)
        , mPriority(priority)
    {

        mProgram.create("grid");

        mMesh = Resources::MeshLoader::loadManual("grid", {}, [](Resources::MeshLoader *loader, Resources::MeshData &data, Resources::MeshLoader::ResourceType * res) {
            Compound<Render::VertexPos_4D> vertices[] = {
                { { 0, 0, 0, 1 } },
                { { 1, 0, 0, 0 } },
                { { 0, 0, 1, 0 } },
                { { -1, 0, 0, 0 } },
                { { 0, 0, -1, 0 } }
            };

            unsigned short indices[] = {
                0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1
            };

            return loader->generate(data, 3, vertices, 5, indices, 12);
        });
    }

    void GridPass::render(Render::RenderTarget *target)
    {
        Vector2i size = target->size();
        float aspectRatio = float(size.x) / size.y;

        mProgram.setUniform("mvp", mCamera->getViewProjectionMatrix(aspectRatio));
        mProgram.bind();

        target->renderMesh(mMesh);
    }

    int GridPass::priority() const
    {
        return mPriority;
    }
}
}