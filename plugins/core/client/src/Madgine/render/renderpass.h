#pragma once

namespace Engine {
namespace Render {

	struct RenderPass {
        virtual ~RenderPass() = default;

        virtual void render(Render::RenderTarget *target) = 0;
	};

}
}