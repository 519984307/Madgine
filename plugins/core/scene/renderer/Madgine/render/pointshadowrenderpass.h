#pragma once

#include "Madgine/render/renderpass.h"

#include "programloader.h"

#include "Meta/math/frustum.h"


namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT PointShadowRenderPass : RenderPass {
        PointShadowRenderPass(size_t index, Scene::SceneManager &scene, int priority);

        virtual void setup(RenderTarget *target) override;
        virtual void shutdown() override;
        virtual void render(Render::RenderTarget *target, size_t iteration) override;

        virtual int priority() const override;

    private:
        ProgramLoader::HandleType mProgram;

        Scene::SceneManager &mScene;        

        size_t mIndex;

        int mPriority;        
    };

}
}