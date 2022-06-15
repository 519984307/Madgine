#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

#include "render/vertexformat.h"

namespace Engine {
namespace Render {

    struct DirectX12VertexShaderLoader : Resources::ResourceLoader<DirectX12VertexShaderLoader, ReleasePtr<IDxcBlob>, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        DirectX12VertexShaderLoader();

        struct HandleType : Base::HandleType {

            using Base::HandleType::HandleType;
            HandleType(Base::HandleType handle)
                : Base::HandleType(std::move(handle))
            {
            }

            Threading::TaskFuture<bool> load(std::string_view name, VertexFormat format);

            void create(const std::string &name, const CodeGen::ShaderFile &file, DirectX12VertexShaderLoader *loader = nullptr);
        };


        Threading::Task<bool> loadImpl(ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info);
        void unloadImpl(ReleasePtr<IDxcBlob> &shader);

        bool create(ReleasePtr<IDxcBlob> &shader, ResourceType *res, const CodeGen::ShaderFile &file);

        bool loadFromSource(ReleasePtr<IDxcBlob> &shader, std::string_view name, std::string source, VertexFormat format);

        virtual Threading::TaskQueue *loadingTaskQueue() const override;

    private:
        ReleasePtr<IDxcLibrary> mLibrary;
        ReleasePtr<IDxcCompiler3> mCompiler;
    };

}
}

REGISTER_TYPE(Engine::Render::DirectX12VertexShaderLoader)