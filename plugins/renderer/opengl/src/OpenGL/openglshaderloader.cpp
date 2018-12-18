#include "opengllib.h"

#include "openglshaderloader.h"

#include "Interfaces/util/stringutil.h"

#include "openglshader.h"

#include "glad.h"

namespace Engine
{
	namespace Render 
	{

		OpenGLShaderLoader::OpenGLShaderLoader(Resources::ResourceManager & mgr) :
			ResourceLoader(mgr, { ".glsl" })
		{
		}

		std::shared_ptr<OpenGLShader> OpenGLShaderLoader::load(ResourceType * res)
		{
			std::string filename = res->path().stem().generic_string();
			
			ShaderType type;
			int glType;
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

			std::ifstream ifs(res->path());
			std::string source((std::istreambuf_iterator<char>(ifs)),
				std::istreambuf_iterator<char>());

			const char *cSource = source.c_str();

			std::shared_ptr<OpenGLShader> shader = std::make_shared<OpenGLShader>(glType, type);

			unsigned int handle = shader->mHandle;

			glShaderSource(handle, 1, &cSource, NULL);
			glCompileShader(handle);
			// check for shader compile errors
			int success;
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

