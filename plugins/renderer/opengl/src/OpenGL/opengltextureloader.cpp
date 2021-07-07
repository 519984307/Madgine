#include "opengllib.h"

#include "opengltextureloader.h"

#include "util/opengltexture.h"

#include "Meta/keyvalue/metatable_impl.h"

VIRTUALUNIQUECOMPONENT(Engine::Render::OpenGLTextureLoader);

METATABLE_BEGIN(Engine::Render::OpenGLTextureLoader)
MEMBER(mResources)
METATABLE_END(Engine::Render::OpenGLTextureLoader)

METATABLE_BEGIN_BASE(Engine::Render::OpenGLTextureLoader::ResourceType, Engine::Render::TextureLoader::ResourceType)
METATABLE_END(Engine::Render::OpenGLTextureLoader::ResourceType)



namespace Engine {
namespace Render {

    OpenGLTextureLoader::OpenGLTextureLoader()
    {
    }

    bool OpenGLTextureLoader::loadImpl(OpenGLTexture &tex, ResourceDataInfo &info)
    {
        throw 0;
    }

    void OpenGLTextureLoader::unloadImpl(OpenGLTexture &tex, ResourceDataInfo &info)
    {
        tex.reset();
    }

    bool OpenGLTextureLoader::create(Texture &tex, DataFormat format)
    {
        GLenum type;
        switch (format) {
        case FORMAT_FLOAT8:
            type = GL_UNSIGNED_BYTE;
            break;
        default:
            std::terminate();
        }

        static_cast<OpenGLTexture &>(tex) = OpenGLTexture { type };

        return true;
    }

    void OpenGLTextureLoader::setData(Texture &tex, Vector2i size, const ByteBuffer &data)
    {
        static_cast<OpenGLTexture &>(tex).setData(size, data);
    }

    void OpenGLTextureLoader::setSubData(Texture &tex, Vector2i offset, Vector2i size, const ByteBuffer &data)
    {
        static_cast<OpenGLTexture &>(tex).setSubData(offset, size, data);
    }

}
}
