#include "opengllib.h"

#include "openglpipelineloader.h"

#include "openglshaderloader.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "codegen/codegen_shader.h"

#include "openglrendercontext.h"

#include "util/openglpipelineinstance.h"

VIRTUALUNIQUECOMPONENT(Engine::Render::OpenGLPipelineLoader);

METATABLE_BEGIN_BASE(Engine::Render::OpenGLPipelineLoader, Engine::Render::PipelineLoader)
MEMBER(mResources)
METATABLE_END(Engine::Render::OpenGLPipelineLoader)

METATABLE_BEGIN_BASE(Engine::Render::OpenGLPipelineLoader::ResourceType, Engine::Render::PipelineLoader::ResourceType)
READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Render::OpenGLPipelineLoader::ResourceType)

namespace Engine {
namespace Render {

    OpenGLPipelineLoader::OpenGLPipelineLoader()
    {
    }

    bool OpenGLPipelineLoader::loadImpl(OpenGLPipeline &pipeline, ResourceDataInfo &info)
    {
        throw 0;
    }

    void OpenGLPipelineLoader::unloadImpl(OpenGLPipeline &pipeline)
    {
        pipeline.reset();
    }

    Threading::Task<bool> OpenGLPipelineLoader::create(Instance &instance, PipelineConfiguration config, bool dynamic)
    {

        char buffer[256];
        sprintf_s(buffer, 256, "%s|%s|%s", config.vs.data(), config.gs.data(), config.ps.data());

        HandleType pipeline;
        if (!co_await pipeline.create(buffer, {}, [&](OpenGLPipelineLoader *loader, OpenGLPipeline &pipeline, ResourceDataInfo &info) -> Threading::Task<bool> {
                OpenGLShaderLoader::HandleType vertexShader;
                if (!co_await vertexShader.load(config.vs, VertexShader)) {
                    LOG_ERROR("Failed to load VS '" << config.vs << "'!");
                    co_return false;
                }

                OpenGLShaderLoader::HandleType pixelShader;
                if (!co_await pixelShader.load(config.ps, PixelShader) && pixelShader) {
                    LOG_ERROR("Failed to load PS '" << config.ps << "'!");
                    co_return false;
                }

                OpenGLShaderLoader::HandleType geometryShader;
                if (!co_await geometryShader.load(config.gs, GeometryShader) && geometryShader) {
                    LOG_ERROR("Failed to load GS '" << config.gs << "'!");
                    co_return false;
                }

                if (!pipeline.link(std::move(vertexShader), std::move(geometryShader), std::move(pixelShader))) {
                    LOG_ERROR("Failed to link Program '" << config.vs << "|" << config.gs << "|" << config.ps
                                                         << "'!");
                    co_return false;
                }

                co_return true;
            }))
            co_return false;

        instance = std::make_unique<OpenGLPipelineInstance>(config, std::move(pipeline));

        co_return true;
    }

    /* bool OpenGLPipelineLoader::create(Program &_program, const std::string &name, const CodeGen::ShaderFile &file)
    {
        assert(file.mInstances.size() == 2);

        OpenGLProgram &program = static_cast<OpenGLProgram &>(_program);

        OpenGLShaderLoader::HandleType vertexShader;
        vertexShader.create(name, file, VertexShader);
        OpenGLShaderLoader::HandleType pixelShader;
        pixelShader.create(name, file, PixelShader);

        if (!program.link(std::move(vertexShader), std::move(pixelShader)))
            std::terminate();

        return true;
    }*/

    Threading::TaskQueue *OpenGLPipelineLoader::loadingTaskQueue() const
    {
        return OpenGLRenderContext::renderQueue();
    }

}
}
