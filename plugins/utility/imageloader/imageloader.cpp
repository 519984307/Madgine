#include "imageloaderlib.h"

#include "imageloader.h"

#include "Interfaces/streams/streams.h"

#include "Interfaces/filesystem/api.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/reflection/classname.h"

#include "imagedata.h"

#include "stb_image.h"

UNIQUECOMPONENT(Engine::Resources::ImageLoader)

using LoaderImpl = Engine::Resources::ResourceLoaderImpl<Engine::Resources::ImageData, Engine::Resources::Resource>;
METATABLE_BEGIN(LoaderImpl)
MEMBER(mResources)
METATABLE_END(LoaderImpl)

METATABLE_BEGIN_BASE(Engine::Resources::ImageLoader, LoaderImpl)
METATABLE_END(Engine::Resources::ImageLoader)

METATABLE_BEGIN_BASE(Engine::Resources::ImageLoader::ResourceType, Engine::Resources::ResourceBase)
METATABLE_END(Engine::Resources::ImageLoader::ResourceType)

RegisterType(Engine::Resources::ImageLoader)

    namespace Engine
{
    namespace Resources {

        ImageData::ImageData(const std::vector<unsigned char> &buffer)
        {

            mBuffer = stbi_load_from_memory(buffer.data(), buffer.size(),
                &mWidth,
                &mHeight,
                &mChannels,
                STBI_rgb_alpha);
        }

        ImageData::~ImageData()
        {
            stbi_image_free(mBuffer);
        }

        ImageLoader::ImageLoader()
            : ResourceLoader({ ".png", ".jpg" })
        {
        }

        std::shared_ptr<ImageData> ImageLoader::loadImpl(ResourceType *res)
        {
            return std::make_shared<ImageData>(res->readAsBlob());
        }

    }
}