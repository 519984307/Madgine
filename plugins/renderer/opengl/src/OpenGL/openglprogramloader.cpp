#include "opengllib.h"

#include "openglprogramloader.h"

#include "openglshaderloader.h"

#include "Meta/keyvalue/metatable_impl.h"

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

    bool OpenGLProgramLoader::loadImpl(OpenGLProgram &program, ResourceType *res)
    {
        throw 0;
    }

    void OpenGLProgramLoader::unloadImpl(OpenGLProgram &program, ResourceType *res)
    {
        program.reset();
    }

    bool OpenGLProgramLoader::create(Program &_program, const std::string &name)
    {
        OpenGLProgram &program = static_cast<OpenGLProgram &>(_program);

        OpenGLShaderLoader::HandleType vertexShader;
        vertexShader.load(name + "_VS");
        if (!vertexShader) {
            LOG_ERROR("Failed to load VS '" << name << "_VS'!");
        }
        
        OpenGLShaderLoader::HandleType pixelShader;
        pixelShader.load(name + "_PS");
        if (!pixelShader) {
            LOG_ERROR("Failed to load VS '" << name << "_PS'!");
        }
        
        if (!vertexShader || !pixelShader) {
            LOG_ERROR("Failed to load Program '" << name << "'!");
            std::terminate();
        }

        if (!program.link(vertexShader, pixelShader)) {
            LOG_ERROR("Failed to link Program '" << name << "'!");
            std::terminate();
        }        

        return true;
    }

    void OpenGLProgramLoader::setParameters(Program &program, const ByteBuffer &data, size_t index)
    {
        static_cast<OpenGLProgram &>(program).setParameters(data, index);
    }

    void OpenGLProgramLoader::setDynamicParameters(Program &program, const ByteBuffer &data, size_t index)
    {
        static_cast<OpenGLProgram &>(program).setDynamicParameters(data, index);
    }

}
}
