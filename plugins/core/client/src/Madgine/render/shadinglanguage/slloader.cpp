#include "../../clientlib.h"

#include "slloader.h"

#include "Meta/keyvalue/metatable_impl.h"

UNIQUECOMPONENT(Engine::Render::SlLoader);

METATABLE_BEGIN(Engine::Render::SlLoader)
METATABLE_END(Engine::Render::SlLoader)

METATABLE_BEGIN_BASE(Engine::Render::SlLoader::Resource, Engine::Resources::ResourceBase)
METATABLE_END(Engine::Render::SlLoader::Resource)

namespace Engine {
namespace Render {

    SlLoader::SlLoader()
        : ResourceLoader({
            ".sl",
            ".hlsl"
        })
    {
    }

    bool SlLoader::loadImpl(SlFile &file, ResourceDataInfo &info)
    {
        throw 0;
    }

    void SlLoader::unloadImpl(SlFile &file)
    {
        throw 0;
    }

}
}
