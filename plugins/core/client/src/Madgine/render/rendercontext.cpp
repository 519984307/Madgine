#include "../clientlib.h"

#include "rendercontext.h"

#include "Modules/generic/reverseIt.h"

#include "Modules/generic/safeiterator.h"

#include "Modules/threading/workgroupstorage.h"

#include "Modules/reflection/classname.h"

#include "rendertarget.h"

RegisterType(Engine::Render::RenderContext)

    namespace Engine
{
    namespace Render {

        Threading::WorkgroupLocal<RenderContext *> sContext = nullptr;

        RenderContext::RenderContext(SignalSlot::TaskQueue *queue)
            : mRenderQueue(queue)
            , mRenderThread(std::this_thread::get_id())
        {
            assert(!sContext);
            sContext = this;
        }

        RenderContext::~RenderContext()
        {
            assert(mRenderTargets.empty());
            assert(sContext == this);
            sContext = nullptr;
        }

        void RenderContext::addRenderTarget(RenderTarget *target)
        {
            mRenderTargets.push_back(target);
        }

        void RenderContext::removeRenderTarget(RenderTarget *target)
        {
            mRenderTargets.erase(std::find(mRenderTargets.begin(), mRenderTargets.end(), target));
        }

        void RenderContext::render()
        {
            for (RenderTarget *target : safeIterate(reverseIt(mRenderTargets)))
                target->render();
        }

        void RenderContext::checkThread()
        {
            assert(mRenderThread == std::this_thread::get_id());
        }

        void RenderContext::queueRenderTask(SignalSlot::TaskHandle &&task)
        {
            assert(sContext);
            sContext->mRenderQueue->queue(std::move(task));
        }

        bool RenderContext::isRenderThread()
        {
            assert(sContext);
            return sContext->mRenderThread == std::this_thread::get_id();
        }

    }
}