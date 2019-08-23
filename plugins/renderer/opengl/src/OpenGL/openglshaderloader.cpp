#include "opengllib.h"

#include "openglshaderloader.h"

#include "Interfaces/stringutil.h"

#include "util/openglshader.h"

#include "Modules/reflection/classname.h"
#include "Modules/keyvalue/metatable_impl.h"

UNIQUECOMPONENT(Engine::Render::OpenGLShaderLoader);

namespace Engine
{
	namespace Render 
	{

		OpenGLShaderLoader::OpenGLShaderLoader() :
			ResourceLoader({ ".glsl" })
		{
		}

		std::shared_ptr<OpenGLShader> OpenGLShaderLoader::loadImpl(ResourceType * res)
		{
			std::string filename = res->path().stem();
			
			ShaderType type;
			GLenum glType;
			if (StringUtil::endsWith(filename, "_VS")) 
			{
				type = VertexShader;
				glType = GL_VERTEX_SHADER;
			}
			else if (StringUtil::endsWith(filename, "_PS"))
			{
				type = PixelShader;
				glType = GL_FRAGMENT_SHADER;
			}
			else
				throw 0;

			std::string source = res->readAsText();

			const char *cSource = source.c_str();

			std::shared_ptr<OpenGLShader> shader = std::make_shared<OpenGLShader>(glType, type);

			GLuint handle = shader->mHandle;

			glShaderSource(handle, 1, &cSource, NULL);
			glCompileShader(handle);
			// check for shader compile errors
			GLint success;
			char infoLog[512];
			glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(handle, 512, NULL, infoLog);
				LOG_ERROR("Loading of Shader '"s + filename + "' failed:");
				LOG_ERROR(infoLog);			
				return {};
			}
			
			return shader;
		}

	}
}

METATABLE_BEGIN(Engine::Render::OpenGLShaderLoader)
METATABLE_END(Engine::Render::OpenGLShaderLoader)

RegisterType(Engine::Render::OpenGLShaderLoader);