#include "../../clientlib.h"

#include "bloompass.h"

#include "../rendercontext.h"
#include "../rendertarget.h"

#include "../shadinglanguage/sl.h"
#define SL_SHADER bloom
#include "../shadinglanguage/sl_include.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::Render::BloomPass)
MEMBER(mExposure)
METATABLE_END(Engine::Render::BloomPass)


namespace Engine {
namespace Render {

    BloomPass::BloomPass(int priority)
        : mPriority(priority)
        , mBlur(50)
    {
    }

    BloomPass::~BloomPass()
    {
    }

    void BloomPass::setup(RenderTarget *target)
    {
        mProgram.create("bloom");

        mProgram.setParametersSize(0, sizeof(BloomData));

        mBlurTarget = target->context()->createRenderTexture(target->size(), { .mIterations = 10, .mFormat = FORMAT_RGBA16F });

        mBlurTarget->addRenderPass(&mBlur);
    }

    void BloomPass::shutdown()
    {
        mBlurTarget.reset();

        mProgram.reset();
    }

    void BloomPass::render(RenderTarget *target, size_t iteration)
    {
        if (iteration == 0)
            target->pushAnnotation("Bloom");

        target->bindTextures({ mInput->texture(mInputIndex), mBlurTarget->texture(0) });

        mProgram.mapParameters(0).cast<BloomData>()->exposure = mExposure;

        target->renderQuad(mProgram);

        if (iteration == target->iterations() - 1)
            target->popAnnotation();
    }

    void BloomPass::preRender()
    {
        mInput->render();
        mBlurTarget->render();
    }

    void BloomPass::onResize(const Vector2i& size) 
    {
        mBlurTarget->resize(size);
        mInput->resize(size);
    }

    int BloomPass::priority() const
    {
        return mPriority;
    }

    void BloomPass::setInput(RenderTarget *input, size_t inputIndex, RenderTarget *blurInput, size_t blurIndex)
    {
        mInput = input;
        mInputIndex = inputIndex;
        mBlur.setInput(blurInput ? blurInput : input, blurIndex);
    }

}
}