#pragma once

namespace Engine {
	namespace Render {

		struct MADGINE_OPENGL_EXPORT OpenGLShaderProgram {			
			
			~OpenGLShaderProgram();

			bool link(OpenGLShader *vertexShader, OpenGLShader *pixelShader);

			void reset();

			void bind();

			void setUniform(const std::string &var, int value);
            void setUniform(const std::string &var, const Matrix3 &value);
			void setUniform(const std::string &var, const Matrix4 &value);
			void setUniform(const std::string &var, const Vector3 &value);

		private:
			GLuint mHandle = 0;
		};

	}
}