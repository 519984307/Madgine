#include "directx11toolslib.h"

#include "directx11toolconfig.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "inspector/inspector.h"

#include "imgui/imgui.h"

#include "imageloaderlib.h"
#include "imageloader.h"

#include "fontloader.h"

#include "imagedata.h"

namespace Engine {
namespace Tools {

    DirectX11ToolConfig::DirectX11ToolConfig(ImRoot &root)
        : Tool<DirectX11ToolConfig>(root)
    {
    }

    Threading::Task<bool> DirectX11ToolConfig::init()
    {
        mImageTexture = { Render::TextureType_2D, Render::FORMAT_RGBA8, D3D11_BIND_SHADER_RESOURCE, 100, 100 };

        getTool<Inspector>().addPreviewDefinition<Render::FontLoader::ResourceType>([](Render::FontLoader::ResourceType *font) {
            Render::FontLoader::HandleType handle = font->loadData();
            handle.info()->setPersistent(true);
            ImGui::Image((void *)handle->mTexture->mTextureHandle, { 100, 100 });
        });

        getTool<Inspector>().addPreviewDefinition<Resources::ImageLoader::ResourceType>([this](Resources::ImageLoader::ResourceType *image) {
            Resources::ImageLoader::HandleType data = image->loadData();
            data.info()->setPersistent(true);

            mImageTexture.setData({ data->mWidth, data->mHeight }, { data->mBuffer, static_cast<size_t>(data->mWidth * data->mHeight) });
            ImGui::Image((void *)mImageTexture.mTextureHandle, { static_cast<float>(data->mWidth), static_cast<float>(data->mHeight) });
        });

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> DirectX11ToolConfig::finalize()
    {
        mImageTexture.reset();

        co_await ToolBase::finalize();
    }

    void DirectX11ToolConfig::renderMenu()
    {
    }

    std::string_view DirectX11ToolConfig::key() const
    {
        return "DirectX11ToolConfig";
    }

}
}

UNIQUECOMPONENT(Engine::Tools::DirectX11ToolConfig);

METATABLE_BEGIN_BASE(Engine::Tools::DirectX11ToolConfig, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::DirectX11ToolConfig)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::DirectX11ToolConfig, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::DirectX11ToolConfig)
