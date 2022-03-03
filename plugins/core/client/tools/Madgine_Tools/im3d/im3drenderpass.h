#pragma once

#include "Madgine/render/renderpass.h"

#include "programloader.h"



namespace Engine {
namespace Render {

    struct MADGINE_CLIENT_TOOLS_EXPORT Im3DRenderPass : RenderPass {
        Im3DRenderPass(Camera *camera, int priority);

        virtual void render(RenderTarget *target, size_t iteration) override;

        virtual int priority() const override;

    private:
        ProgramLoader::PtrType mProgram;

        Camera *mCamera;

        int mPriority;
    };

}
}