#include "directx12lib.h"

#include "directx12rendertarget.h"

#include "meshdata.h"

#include "directx12meshdata.h"
#include "directx12meshloader.h"
#include "util/directx12program.h"

#include "directx12rendercontext.h"

#include "Meta/math/rect2i.h"

#include "render/material.h"

namespace Engine {
namespace Render {

    DirectX12RenderTarget::DirectX12RenderTarget(DirectX12RenderContext *context, bool global, std::string name)
        : RenderTarget(context, global, name)
    {
    }

    DirectX12RenderTarget::~DirectX12RenderTarget()
    {
    }

    void DirectX12RenderTarget::setup(OffsetPtr targetView, const Vector2i &size)
    {
        /*if (mTargetView) {
            mTargetView->Release();
            mTargetView = nullptr;
        }*/

        mTargetView = targetView;

        D3D12_HEAP_PROPERTIES depthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R24G8_TYPELESS, size.x, size.y);
        depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        CD3DX12_CLEAR_VALUE depthOptimizedClearValue(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0);

        HRESULT hr = sDevice->CreateCommittedResource(
            &depthHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &depthResourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(&mDepthStencilBuffer));
        DX12_CHECK(hr);

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        mDepthStencilView = DirectX12RenderContext::getSingleton().mDepthStencilDescriptorHeap.allocate();
        sDevice->CreateDepthStencilView(mDepthStencilBuffer, &dsvDesc, DirectX12RenderContext::getSingleton().mDepthStencilDescriptorHeap.cpuHandle(mDepthStencilView));

        // Setup depth/stencil state.
        ZeroMemory(&mDepthStencilStateDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));

        mDepthStencilStateDesc.DepthEnable = TRUE;
        mDepthStencilStateDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        mDepthStencilStateDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        mDepthStencilStateDesc.StencilEnable = FALSE;

        // Setup rasterizer state.
        ZeroMemory(&mRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

       

        ZeroMemory(&mBlendState, sizeof(D3D12_BLEND_DESC));

        

        /*if (!mSamplers[0]) {

            D3D12_SAMPLER_DESC samplerDesc;

            samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_WRAP;
            samplerDesc.MipLODBias = 0.0f;
            samplerDesc.MaxAnisotropy = 1;
            samplerDesc.ComparisonFunc = D3D12_COMPARISON_ALWAYS;
            samplerDesc.BorderColor[0] = 0;
            samplerDesc.BorderColor[1] = 0;
            samplerDesc.BorderColor[2] = 0;
            samplerDesc.BorderColor[3] = 0;
            samplerDesc.MinLOD = 0;
            samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

            hr = sDevice->CreateSamplerState(&samplerDesc, mSamplers + 0);
            DX12_CHECK(hr);

            samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_CLAMP;

            hr = sDevice->CreateSamplerState(&samplerDesc, mSamplers + 1);
            DX12_CHECK(hr);
        }*/

        DX12_CHECK();
    }

    void DirectX12RenderTarget::shutdown()
    {
        /*for (size_t i = 0; i < 2; ++i) {
            if (mSamplers[i]) {
                mSamplers[i]->Release();
                mSamplers[i] = nullptr;
            }
        }*/

        if (mDepthStencilBuffer) {
            mDepthStencilBuffer->Release();
            mDepthStencilBuffer = nullptr;
        }

        /*if (mTargetView) {
            mTargetView->Release();
            mTargetView = nullptr;
        }*/
    }

    void DirectX12RenderTarget::beginIteration(size_t iteration) const
    {
        context()->mCommandList.mList->SetGraphicsRootSignature(context()->mRootSignature);

        const Vector2i &screenSize = size();

        D3D12_VIEWPORT viewport;
        viewport.Width = static_cast<float>(screenSize.x);
        viewport.Height = static_cast<float>(screenSize.y);
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        DirectX12RenderContext::getSingleton().mCommandList.mList->RSSetViewports(1, &viewport);

        D3D12_RECT scissorRect;
        scissorRect.left = 0.0f;
        scissorRect.top = 0.0f;
        scissorRect.right = static_cast<float>(screenSize.x);
        scissorRect.bottom = static_cast<float>(screenSize.y);

        DirectX12RenderContext::getSingleton().mCommandList.mList->RSSetScissorRects(1, &scissorRect);

        //TransitionBarrier();

        constexpr FLOAT color[4] = { 0.2f, 0.3f, 0.3f, 1.0f };

        D3D12_CPU_DESCRIPTOR_HANDLE targetView = DirectX12RenderContext::getSingleton().mRenderTargetDescriptorHeap.cpuHandle(mTargetView);
        D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DirectX12RenderContext::getSingleton().mDepthStencilDescriptorHeap.cpuHandle(mDepthStencilView);

        DirectX12RenderContext::getSingleton().mCommandList.mList->ClearRenderTargetView(targetView, color, 0, nullptr);
        DirectX12RenderContext::getSingleton().mCommandList.mList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        DirectX12RenderContext::getSingleton().mCommandList.mList->OMSetRenderTargets(1, &targetView, false, &depthStencilView);
        /*sDeviceContext->OMSetDepthStencilState(mDepthStencilState, 1);
        sDeviceContext->OMSetBlendState(mBlendState, 0, 0xffffffff);*/

        //sDeviceContext->PSSetSamplers(0, 2, mSamplers);

        RenderTarget::beginIteration(iteration);
    }

    void DirectX12RenderTarget::endIteration(size_t iteration) const
    {
        RenderTarget::endIteration(iteration);
        context()->ExecuteCommandList(context()->mCommandList);
    }
    
    void DirectX12RenderTarget::pushAnnotation(const char *tag)
    {
        PIXBeginEvent(DirectX12RenderContext::getSingleton().mCommandList.mList, PIX_COLOR(255, 255, 255), tag);
    }

    void DirectX12RenderTarget::popAnnotation()
    {
        PIXEndEvent(DirectX12RenderContext::getSingleton().mCommandList.mList);
    }

    void DirectX12RenderTarget::setRenderSpace(const Rect2i &space)
    {
        D3D12_VIEWPORT viewport;
        viewport.Width = static_cast<float>(space.mSize.x);
        viewport.Height = static_cast<float>(space.mSize.y);
        viewport.TopLeftX = static_cast<float>(space.mTopLeft.x);
        viewport.TopLeftY = static_cast<float>(space.mTopLeft.y);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        DirectX12RenderContext::getSingleton().mCommandList.mList->RSSetViewports(1, &viewport);

        D3D12_RECT scissorRect;
        scissorRect.left = static_cast<float>(space.mTopLeft.x);
        scissorRect.top = static_cast<float>(space.mTopLeft.y);
        scissorRect.right = static_cast<float>(space.mTopLeft.x + space.mSize.x);
        scissorRect.bottom = static_cast<float>(space.mTopLeft.y + space.mSize.y);

        DirectX12RenderContext::getSingleton().mCommandList.mList->RSSetScissorRects(1, &scissorRect);
    }

    void DirectX12RenderTarget::renderMesh(const GPUMeshData *m, const Program *p, const Material *material)
    {
        const DirectX12MeshData *mesh = static_cast<const DirectX12MeshData *>(m);
        const DirectX12Program *program = static_cast<const DirectX12Program *>(p);

        if (!mesh->mVAO)
            return;

        program->bind(&mesh->mVAO);

        if (material)
            bindTextures({ { material->mDiffuseTexture, TextureType_2D } });

        if (mesh->mIndices) {
            DirectX12RenderContext::getSingleton().mCommandList.mList->DrawIndexedInstanced(mesh->mElementCount, 1, 0, 0, 0);
        } else {
            DirectX12RenderContext::getSingleton().mCommandList.mList->DrawInstanced(mesh->mElementCount, 1, 0, 0);
        }
    }

    void DirectX12RenderTarget::renderMeshInstanced(size_t count, const GPUMeshData *m, const Program *p, const Material *material)
    {
        const DirectX12MeshData *mesh = static_cast<const DirectX12MeshData *>(m);
        const DirectX12Program *program = static_cast<const DirectX12Program *>(p);

        if (!mesh->mVAO)
            return;

        program->bind(&mesh->mVAO);

        if (material)
            bindTextures({ { material->mDiffuseTexture, TextureType_2D } });

        if (mesh->mIndices) {
            DirectX12RenderContext::getSingleton().mCommandList.mList->DrawIndexedInstanced(mesh->mElementCount, count, 0, 0, 0);
        } else {
            DirectX12RenderContext::getSingleton().mCommandList.mList->DrawInstanced(mesh->mElementCount, count, 0, 0);
        }
    }

    void DirectX12RenderTarget::renderVertices(const Program *program, size_t groupSize, std::vector<Vertex> vertices, std::vector<unsigned short> indices)
    {
        if (!vertices.empty()) {
            DirectX12MeshData tempMesh;
            DirectX12MeshLoader::getSingleton().generate(tempMesh, { groupSize, std::move(vertices), std::move(indices) });

            //setupProgram(flags);

            renderMesh(&tempMesh, program);
        }
    }

    void DirectX12RenderTarget::renderVertices(const Program *program, size_t groupSize, std::vector<Vertex2> vertices, std::vector<unsigned short> indices, const Material *material)
    {
        if (!vertices.empty()) {
            DirectX12MeshData tempMesh;
            DirectX12MeshLoader::getSingleton().generate(tempMesh, { groupSize, std::move(vertices), std::move(indices) });

            renderMesh(&tempMesh, program, material);
        }
    }

    void DirectX12RenderTarget::clearDepthBuffer()
    {
        DirectX12RenderContext::getSingleton().mCommandList.mList->ClearDepthStencilView(DirectX12RenderContext::getSingleton().mDepthStencilDescriptorHeap.cpuHandle(mDepthStencilView), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    void DirectX12RenderTarget::bindTextures(const std::vector<TextureDescriptor> &tex, size_t offset) const
    {        
        //DirectX12RenderContext::getSingleton().mCommandList->PSSetShaderResources(0, tex.size(), reinterpret_cast<ID3D12ShaderResourceView *const *>(tex.data()));
    }

    DirectX12RenderContext* DirectX12RenderTarget::context() const {
        return static_cast<DirectX12RenderContext *>(RenderTarget::context());
    }

}
}