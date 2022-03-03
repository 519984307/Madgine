#pragma once

#include "program.h"

#include "../directx12pixelshaderloader.h"
#include "../directx12vertexshaderloader.h"
#include "directx12buffer.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12Program : Program {

        DirectX12Program() = default;
        DirectX12Program(DirectX12Program &&other);
        ~DirectX12Program();

        DirectX12Program &operator=(DirectX12Program &&other);

        bool link(typename DirectX12VertexShaderLoader::HandleType vertexShader, typename DirectX12PixelShaderLoader::HandleType pixelShader);

        void reset();

        void bind(const DirectX12VertexArray *format) const;

        void setParametersSize(size_t index, size_t size);
        WritableByteBuffer mapParameters(size_t index);

        void setInstanceDataSize(size_t size);
        void setInstanceData(const ByteBuffer &data);

        void setDynamicParameters(size_t index, const ByteBuffer &data);

    private:
        DirectX12VertexShaderLoader::HandleType mVertexShader;
        DirectX12PixelShaderLoader::HandleType mPixelShader;
        std::vector<DirectX12Buffer> mConstantBuffers;
        std::vector<DirectX12Buffer> mDynamicBuffers;
        size_t mInstanceDataSize = 0;
        DirectX12Buffer mInstanceBuffer;
    };

}
}