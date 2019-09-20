#include "../opengllib.h"

#include "opengltexture.h"

namespace Engine {
namespace Render {

    OpenGLTexture::OpenGLTexture()
    {
        glGenTextures(1, &mHandle);
        glCheck();
        bind();
#if !OPENGL_ES
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glCheck();
#endif
    }

    OpenGLTexture::OpenGLTexture(OpenGLTexture &&other)
        : mHandle(std::exchange(other.mHandle, 0))
    {
    }

    OpenGLTexture::~OpenGLTexture()
    {
        if (mHandle) {
            glDeleteTextures(1, &mHandle);
            glCheck();
        }
    }

    void OpenGLTexture::bind() const
    {
        glBindTexture(GL_TEXTURE_2D, mHandle);
        glCheck();
    }

    void OpenGLTexture::setData(Vector2i size, void *data, GLenum type)
    {
        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, type, data);
        glCheck();
        mSize = size;
    }

    void OpenGLTexture::setSubData(Vector2i offset, Vector2i size, void *data, GLenum type)
    {
        bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, size.x, size.y, GL_RGBA, type, data);
        glCheck();
    }

    void OpenGLTexture::resize(Vector2i size)
    {
        throw "TODO";
        /*Vector2i commonSize = min(size, mSize);
        GLuint tempTex;
        glGenTextures(1, &tempTex);
        GL_CHECK();
        std::swap(tempTex, mHandle);

        setData(size, nullptr);

        glCopyImageSubData(tempTex, GL_TEXTURE_2D, 0, 0, 0, 0, mHandle, GL_TEXTURE_2D, 0, 0, 0, 0, commonSize.x, commonSize.y, 1);
        GL_CHECK();

        glDeleteTextures(1, &tempTex);
        GL_CHECK();*/
    }

    GLuint OpenGLTexture::handle() const
    {
        return mHandle;
    }

    void OpenGLTexture::setWrapMode(GLint mode)
    {
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
        glCheck();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
        glCheck();
    }

    void OpenGLTexture::setFilter(GLint filter)
    {
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glCheck();
    }

}
}