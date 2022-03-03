#pragma once

#include "Madgine/resources/resourceloader.h"

#include "gpumeshdata.h"

#include "Modules/threading/workgroupstorage.h"

namespace Engine {
namespace Render {

    struct MADGINE_MESHLOADER_EXPORT GPUMeshLoader : Resources::VirtualResourceLoaderBase<GPUMeshLoader, GPUMeshData, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {

        using Base = VirtualResourceLoaderBase<GPUMeshLoader, GPUMeshData, std::list<Placeholder<0>>, Threading::WorkGroupStorage>;

        struct HandleType : Base::HandleType {
            using Base::HandleType::HandleType;
            HandleType(Base::HandleType handle)
                : Base::HandleType(std::move(handle))
            {
            }

            void update(const MeshData &mesh, GPUMeshLoader *loader = nullptr)
            {
                if (!loader)
                    loader = &GPUMeshLoader::getSingleton();
                loader->update(*getDataPtr(*this, loader), mesh);
            }

            void update(MeshData &&mesh, GPUMeshLoader *loader = nullptr)
            {
                if (!loader)
                    loader = &GPUMeshLoader::getSingleton();
                loader->update(*getDataPtr(*this, loader), std::move(mesh));
            }

        };

        GPUMeshLoader();

        Threading::Task<bool> loadImpl(GPUMeshData &data, ResourceDataInfo &info);
        void unloadImpl(GPUMeshData &data);

        virtual bool generate(GPUMeshData &data, const MeshData &mesh) = 0;
        virtual bool generate(GPUMeshData &data, MeshData &&mesh) = 0;

        void generateMaterials(GPUMeshData &data, const MeshData &mesh);

        virtual void update(GPUMeshData &data, const MeshData &mesh) = 0;
        virtual void update(GPUMeshData &data, MeshData &&mesh) = 0;

        virtual void reset(GPUMeshData &data) = 0;
    };

}
}

RegisterType(Engine::Render::GPUMeshLoader);