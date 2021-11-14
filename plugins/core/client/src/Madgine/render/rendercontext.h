#pragma once

#include "Meta/math/vector2i.h"

#include "rendertextureconfig.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

namespace Engine {
namespace Render {

    struct MADGINE_CLIENT_EXPORT RenderContext {
        RenderContext(Threading::TaskQueue *queue, bool supportsMultisampling = true);
        virtual ~RenderContext();

        virtual std::unique_ptr<RenderTarget> createRenderWindow(Window::OSWindow *w, size_t samples = 1) = 0;
        virtual std::unique_ptr<RenderTarget> createRenderTexture(const Vector2i &size = { 1, 1 }, const RenderTextureConfig &config = {}) = 0;

        void addRenderTarget(RenderTarget *target);
        void removeRenderTarget(RenderTarget *target);

        void render();

        virtual void beginFrame();
        virtual void endFrame();

		static RenderContext &getSingleton();

        size_t frame() const;

        static Threading::TaskQueue *renderQueue();

        bool supportsMultisampling() const;

    protected:
        void checkThread();

        static bool isRenderThread();

    protected:
        std::vector<RenderTarget *> mRenderTargets;

        Threading::TaskQueue *mRenderQueue = nullptr;
        std::thread::id mRenderThread;

        size_t mFrame = 1;

        bool mSupportsMultisampling;
    };

}
}


RegisterType(Engine::Render::RenderContext);