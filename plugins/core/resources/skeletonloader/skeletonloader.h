#pragma once

#include "Madgine/resources/resourceloader.h"

#include "skeletondescriptor.h"

namespace Engine {
namespace Render {

    struct MADGINE_SKELETONLOADER_EXPORT SkeletonLoader : Resources::ResourceLoader<SkeletonLoader, SkeletonDescriptor, std::list<Placeholder<0>>> {

        using Base = ResourceLoader<SkeletonLoader, SkeletonDescriptor, std::list<Placeholder<0>>>;

        struct HandleType : Base::HandleType {
            using Base::HandleType::HandleType;
            HandleType(Base::HandleType handle)
                : Base::HandleType(std::move(handle))
            {
            }

        };

        SkeletonLoader();

        bool loadImpl(SkeletonDescriptor &data, ResourceDataInfo &info);
        void unloadImpl(SkeletonDescriptor &data, ResourceDataInfo &info);

    };

}
}

RegisterType(Engine::Render::SkeletonLoader);