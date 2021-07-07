#include "../clientlib.h"

#include "rendercontext.h"
#include "rendertarget.h"
#include "renderpass.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::Render::RenderTarget)
METATABLE_END(Engine::Render::RenderTarget)

namespace Engine {
namespace Render {

    RenderTarget::RenderTarget(RenderContext *context)
        : mContext(context)
    {
        mContext->addRenderTarget(this);
    }

    RenderTarget::~RenderTarget()
    {
        for (RenderPass* pass : mRenderPasses) {
            pass->shutdown();
        }
        mContext->removeRenderTarget(this);
    }

    void RenderTarget::render()
    {
        beginFrame();
        for (RenderPass *pass : mRenderPasses)
            pass->render(this);
        endFrame();
    }

    void RenderTarget::addRenderPass(RenderPass *pass)
    {
        mRenderPasses.insert(
            std::upper_bound(mRenderPasses.begin(), mRenderPasses.end(), pass,
                [](RenderPass *first, RenderPass *second) { return first->priority() < second->priority(); }),
            pass);
        pass->setup(mContext);
    }

    void RenderTarget::removeRenderPass(RenderPass *pass)
    {
        pass->shutdown();
        mRenderPasses.erase(std::find(mRenderPasses.begin(), mRenderPasses.end(), pass));
    }

    const std::vector<RenderPass *> &RenderTarget::renderPasses()
    {
        return mRenderPasses;
    }

    void RenderTarget::beginFrame()
    {
    }

    void RenderTarget::endFrame()
    {
    }

}
}