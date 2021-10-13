#pragma once

#include "render/attributedescriptor.h"
#include "openglvertexarrayobject.h"

namespace Engine {
namespace Render {
    

    struct MADGINE_OPENGL_EXPORT OpenGLVertexArray {

        OpenGLVertexArray() = default;
        OpenGLVertexArray(const OpenGLVertexArray &) = delete;
        OpenGLVertexArray(OpenGLVertexArray &&);
        OpenGLVertexArray(OpenGLBuffer &vertex, OpenGLBuffer &index, std::array<AttributeDescriptor, 7> (*attributes)());
        ~OpenGLVertexArray();

        OpenGLVertexArray &operator=(OpenGLVertexArray &&other);

		explicit operator bool() const;

        void reset();        

        void bind(OpenGLProgram *program, OpenGLBuffer &instanceDataBuffer, size_t instanceDataSize);
        void unbind(OpenGLProgram *program);
        

        uint8_t mFormat = 0;
    private:
        OpenGLBuffer *mVertexBuffer = nullptr;
        OpenGLBuffer *mIndexBuffer = nullptr;
        std::array<AttributeDescriptor, 7> (*mAttributes)();
        std::map<OpenGLProgram *, OpenGLVertexArrayObject> mInstances;
    };

}
}