#include "directx12lib.h"

#include "directx12geometryshaderloader.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "directx12shadercodegen.h"

#include "Interfaces/filesystem/api.h"

#include "Madgine/codegen/resolveincludes.h"

#include "Madgine/render/shadinglanguage/slloader.h"

#include "directx12rendercontext.h"

UNIQUECOMPONENT(Engine::Render::DirectX12GeometryShaderLoader);

namespace Engine {
namespace Render {

    std::wstring GetLatestGeometryProfile()
    {
        return L"gs_6_0";
    }

    DirectX12GeometryShaderLoader::DirectX12GeometryShaderLoader()
        : ResourceLoader({ ".GS_hlsl" })
    {
        HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&mLibrary));
        //if(FAILED(hr)) Handle error...

        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&mCompiler));
        //if(FAILED(hr)) Handle error
    }

    /* void DirectX12PixelShaderLoader::Handle::create(const std::string &name, const CodeGen::ShaderFile &file, DirectX12PixelShaderLoader *loader)
    {
        *this = DirectX12PixelShaderLoader::loadManual(
            name, {}, [=, &file](DirectX12PixelShaderLoader *loader, DirectX12PixelShader &shader, const DirectX12PixelShaderLoader::ResourceDataInfo &info) { return loader->create(shader, info.resource(), file); }, loader);
    }*/

    bool DirectX12GeometryShaderLoader::loadImpl(ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info)
    {
        std::string source = info.resource()->readAsText();

        return loadFromSource(shader, info.resource()->path().stem(), source);
    }

    void DirectX12GeometryShaderLoader::unloadImpl(ReleasePtr<IDxcBlob> &shader)
    {
        shader.reset();
    }

    /* bool DirectX12PixelShaderLoader::create(DirectX12PixelShader &shader, Resource *res, const CodeGen::ShaderFile &file)
    {
        if (res->path().empty()) {
            Filesystem::Path dir = Filesystem::appDataPath() / "generated/shader/directx12";

            Filesystem::createDirectories(dir);

            res->setPath(dir / (std::string { res->name() } + ".PS_hlsl"));
        }

        std::ostringstream ss;
        DirectX12ShaderCodeGen::generate(ss, file, 1);

        {
            std::ofstream f { res->path() };
            f << ss.str();
        }

        return loadFromSource(shader, res->name(), ss.str());
    }*/

    bool DirectX12GeometryShaderLoader::loadFromSource(ReleasePtr<IDxcBlob> &shader, std::string_view name, std::string source)
    {
        std::wstring profile = L"latest";
        if (profile == L"latest")
            profile = GetLatestGeometryProfile();

        std::map<std::string, size_t> files;

        CodeGen::resolveIncludes(
            source, [](const Filesystem::Path &path, size_t line, std::string_view filename) {
                Resources::ResourceBase *res = SlLoader::get(path.stem());
                return "#line 1 \"" + path.filename().str() + "\"\n" + res->readAsText() + "\n#line " + std::to_string(line + 1) + " \"" + std::string { filename } + "\"";
            },
            name, files);

       
        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = source.c_str();
        sourceBuffer.Size = source.size();
        sourceBuffer.Encoding = CP_UTF8;

        std::vector<LPCWSTR> arguments;
        arguments.push_back(L"-E");
        arguments.push_back(L"main");

        arguments.push_back(L"-T");
        arguments.push_back(profile.c_str());

        ReleasePtr<IDxcResult> result;
        HRESULT hr = mCompiler->Compile(
            &sourceBuffer, // pSource
            arguments.data(), // pSourceName
            arguments.size(), // pEntryPoint
            nullptr,
            IID_PPV_ARGS(&result)); // ppResult
        if (SUCCEEDED(hr))
            result->GetStatus(&hr);
        if (FAILED(hr)) {
            LOG_ERROR("Loading of Shader '" << name << "' failed:");

            if (result) {
                ReleasePtr<IDxcBlobUtf8> pErrorBlob;
                hr = result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrorBlob), nullptr);
                if (SUCCEEDED(hr) && pErrorBlob) {
                    LOG_ERROR((char *)pErrorBlob->GetBufferPointer());
                }
            }
            return false;
        }
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr);

        return true;
    }

    Threading::TaskQueue *DirectX12GeometryShaderLoader::loadingTaskQueue() const
    {
        return DirectX12RenderContext::renderQueue();
    }
}
}

METATABLE_BEGIN(Engine::Render::DirectX12GeometryShaderLoader)
METATABLE_END(Engine::Render::DirectX12GeometryShaderLoader)

METATABLE_BEGIN_BASE(Engine::Render::DirectX12GeometryShaderLoader::Resource, Engine::Resources::ResourceBase)
METATABLE_END(Engine::Render::DirectX12GeometryShaderLoader::Resource)
