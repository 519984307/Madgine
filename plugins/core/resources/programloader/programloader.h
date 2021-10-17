#pragma once

#include "Madgine/resources/resourceloader.h"
#include "Modules/threading/workgroupstorage.h"

#include "Generic/bytebuffer.h"

#include "program.h"

namespace Engine {
namespace Render {

    struct MADGINE_PROGRAMLOADER_EXPORT ProgramLoader : Resources::VirtualResourceLoaderBase<ProgramLoader, Program, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {

        using Base = Resources::VirtualResourceLoaderBase<ProgramLoader, Program, std::list<Placeholder<0>>, Threading::WorkGroupStorage>;

        struct MADGINE_PROGRAMLOADER_EXPORT HandleType : Base::HandleType {

            using Base::HandleType::HandleType;
            HandleType(Base::HandleType handle)
                : Base::HandleType(std::move(handle))
            {
            }

            Threading::TaskFuture<bool> create(const std::string &name, const std::vector<size_t> &bufferSizes = {}, size_t instanceDataSize = 0, ProgramLoader *loader = nullptr);
            void create(const std::string &name, CodeGen::ShaderFile &&file, ProgramLoader *loader = nullptr);
            void createUnnamed(CodeGen::ShaderFile &&file, ProgramLoader *loader = nullptr);

            WritableByteBuffer mapParameters(size_t index, ProgramLoader *loader = nullptr);

            void setInstanceData(const ByteBuffer &data, ProgramLoader *loader = nullptr);

			void setDynamicParameters(size_t index, const ByteBuffer &data, ProgramLoader *loader = nullptr);
        };

        ProgramLoader();

        virtual bool create(Program &program, const std::string &name, const std::vector<size_t> &bufferSizes = {}, size_t instanceDataSize = 0) = 0;
        virtual bool create(Program &program, const std::string &name, const CodeGen::ShaderFile &file) = 0;

        virtual WritableByteBuffer mapParameters(Program &program, size_t index) = 0;
 
        virtual void setInstanceData(Program &program, const ByteBuffer &data) = 0;

		virtual void setDynamicParameters(Program &program, size_t index, const ByteBuffer &data) = 0;
    };

}
}

RegisterType(Engine::Render::ProgramLoader);