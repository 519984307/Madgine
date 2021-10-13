#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

#include "util/openglshader.h"

namespace Engine {
namespace Render {

    struct OpenGLShaderLoader : Resources::ResourceLoader<OpenGLShaderLoader, OpenGLShader, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        OpenGLShaderLoader();

        struct HandleType : Base::HandleType {

            using Base::HandleType::HandleType;
            HandleType(Base::HandleType handle)
                : Base::HandleType(std::move(handle))
            {
            }

            void create(std::string name, const CodeGen::ShaderFile &file, ShaderType type, OpenGLShaderLoader *loader = nullptr);
            void load(std::string name, ShaderType type, OpenGLShaderLoader *loader = nullptr);
        };

        bool loadImpl(OpenGLShader &shader, ResourceDataInfo &info);
        void unloadImpl(OpenGLShader &shader, ResourceDataInfo &info);

        bool create(OpenGLShader &shader, ResourceType *res, const CodeGen::ShaderFile &file, ShaderType type);

        bool loadFromSource(OpenGLShader &shader, std::string_view name, std::string source, ShaderType type);
    };

}
}

RegisterType(Engine::Render::OpenGLShaderLoader);