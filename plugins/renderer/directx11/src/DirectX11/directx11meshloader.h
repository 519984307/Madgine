#pragma once

#include "Modules/resources/resourceloader.h"

#include "meshloader.h"

#include "directx11meshdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX11_EXPORT DirectX11MeshLoader : Resources::VirtualResourceLoaderImpl<DirectX11MeshLoader, DirectX11MeshData, MeshLoader> {
        DirectX11MeshLoader();

        virtual bool generateImpl(MeshData &mesh, const std::vector<std::optional<AttributeDescriptor>> &attributeList, const AABB &bb, size_t groupSize, Render::ByteBuffer vertices, size_t vertexSize, std::vector<unsigned short> indices = {}, const Filesystem::Path &texturePath = {}) override;

        virtual void updateImpl(MeshData &data, const AABB &bb, size_t groupSize, Render::ByteBuffer vertices, size_t vertexSize, std::vector<unsigned short> indices = {}) override;

		virtual void resetImpl(MeshData &data) override;
    };
}
}