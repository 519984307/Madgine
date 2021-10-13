#include "opengllib.h"

#include "openglprogramloader.h"

#include "openglshaderloader.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "codegen/codegen_shader.h"

VIRTUALUNIQUECOMPONENT(Engine::Render::OpenGLProgramLoader);

METATABLE_BEGIN_BASE(Engine::Render::OpenGLProgramLoader, Engine::Render::ProgramLoader)
MEMBER(mResources)
METATABLE_END(Engine::Render::OpenGLProgramLoader)

METATABLE_BEGIN_BASE(Engine::Render::OpenGLProgramLoader::ResourceType, Engine::Render::ProgramLoader::ResourceType)
READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Render::OpenGLProgramLoader::ResourceType)

namespace Engine {
namespace Render {

    OpenGLProgramLoader::OpenGLProgramLoader()
    {
    }

    bool OpenGLProgramLoader::loadImpl(OpenGLProgram &program, ResourceDataInfo &info)
    {
        throw 0;
    }

    void OpenGLProgramLoader::unloadImpl(OpenGLProgram &program, ResourceDataInfo &info)
    {
        program.reset();
    }

    bool OpenGLProgramLoader::create(Program &_program, const std::string &name)
    {
        OpenGLProgram &program = static_cast<OpenGLProgram &>(_program);

        OpenGLShaderLoader::HandleType vertexShader;
        vertexShader.load(name, VertexShader);
        if (!vertexShader) {
            LOG_ERROR("Failed to load VS '" << name << "'!");
        }

        OpenGLShaderLoader::HandleType pixelShader;
        pixelShader.load(name, PixelShader);

        OpenGLShaderLoader::HandleType geometryShader;
        geometryShader.load(name, GeometryShader);

        if (!program.link(vertexShader, pixelShader, geometryShader)) {
            LOG_ERROR("Failed to link Program '" << name << "'!");
            std::terminate();
        }

        return true;
    }

    bool OpenGLProgramLoader::create(Program &_program, const std::string &name, const CodeGen::ShaderFile &file)
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
    }

    void OpenGLProgramLoader::setParametersSize(Program &program, size_t index, size_t size)
    {
        static_cast<OpenGLProgram &>(program).setParametersSize(index, size);
    }

    WritableByteBuffer OpenGLProgramLoader::mapParameters(Program &program, size_t index)
    {
        return static_cast<OpenGLProgram &>(program).mapParameters(index);
    }

    void OpenGLProgramLoader::setInstanceDataSize(Program &program, size_t size)
    {
        static_cast<OpenGLProgram &>(program).setInstanceDataSize(size);
    }

    void OpenGLProgramLoader::setInstanceData(Program &program, const ByteBuffer &data)
    {
        static_cast<OpenGLProgram &>(program).setInstanceData(data);
    }

    void OpenGLProgramLoader::setDynamicParameters(Program &program, size_t index, const ByteBuffer &data)
    {
        static_cast<OpenGLProgram &>(program).setDynamicParameters(index, data);
    }

}
}
