#include "opengllib.h"

#include "openglshaderloader.h"

#include "Interfaces/stringutil.h"

#include "util/openglshader.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/reflection/classname.h"

UNIQUECOMPONENT(Engine::Render::OpenGLShaderLoader);

namespace Engine {
namespace Render {

    OpenGLShaderLoader::OpenGLShaderLoader()
        : ResourceLoader({ ".glsl" })
    {
    }

    bool OpenGLShaderLoader::loadImpl(OpenGLShader &shader, ResourceType *res)
    {
        std::string filename = res->path().stem();

        ShaderType type;
        GLenum glType;
        if (StringUtil::endsWith(filename, "_VS")) {
            type = VertexShader;
        } else if (StringUtil::endsWith(filename, "_PS")) {
            type = PixelShader;            
        } else
            std::terminate();

        std::string source = res->readAsText();

        const char *cSource = source.c_str();

        OpenGLShader tempShader { type };

        GLuint handle = tempShader.mHandle;

        glShaderSource(handle, 1, &cSource, NULL);
        glCompileShader(handle);
        // check for shader compile errors
        GLint success;
        char infoLog[512];
        glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(handle, 512, NULL, infoLog);
            LOG_ERROR("Loading of Shader '"s + filename + "' failed:");
            LOG_ERROR(infoLog);
            return false;
        }

        shader = std::move(tempShader);

        return true;
    }

    void OpenGLShaderLoader::unloadImpl(OpenGLShader &shader, ResourceType *res)
    {
        shader.reset();
    }

}
}

METATABLE_BEGIN(Engine::Render::OpenGLShaderLoader)
METATABLE_END(Engine::Render::OpenGLShaderLoader)

METATABLE_BEGIN_BASE(Engine::Render::OpenGLShaderLoader::ResourceType, Engine::Resources::ResourceBase)
METATABLE_END(Engine::Render::OpenGLShaderLoader::ResourceType)

RegisterType(Engine::Render::OpenGLShaderLoader);