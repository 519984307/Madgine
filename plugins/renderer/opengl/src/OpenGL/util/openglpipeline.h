#pragma once

#include "pipeline.h"

#include "../openglshaderloader.h"

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLPipeline : Pipeline {

        OpenGLPipeline() = default;
        OpenGLPipeline(OpenGLPipeline &&other);
        ~OpenGLPipeline();

        OpenGLPipeline &operator=(OpenGLPipeline &&other);

        bool link(typename OpenGLShaderLoader::HandleType vertexShader, typename OpenGLShaderLoader::HandleType geometryShader, typename OpenGLShaderLoader::HandleType pixelShader = {});

        void reset();

        void bind() const;

        GLuint handle() const;

    private:
        mutable GLuint mHandle = 0;
    };

}
}