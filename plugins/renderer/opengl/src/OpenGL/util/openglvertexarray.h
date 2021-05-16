#pragma once

#include "render/attributedescriptor.h"

namespace Engine {
namespace Render {
    

    struct MADGINE_OPENGL_EXPORT OpenGLVertexArray {

        OpenGLVertexArray() = default;
        OpenGLVertexArray(const OpenGLVertexArray &) = delete;
        OpenGLVertexArray(OpenGLVertexArray &&);
        OpenGLVertexArray(create_t);
        ~OpenGLVertexArray();

        OpenGLVertexArray &operator=(OpenGLVertexArray &&other);

		explicit operator bool() const;

        static unsigned int getCurrent();

        void reset();

        void bind();
        void unbind();

#if OPENGL_ES
        static void onBindVBO(GLuint buffer);
        static void onBindEBO(GLuint buffer);

        static std::pair<unsigned int, unsigned int> getCurrentBindings();
#else
        GLuint handle();
#endif

        void setVertexAttribute(unsigned int index, AttributeDescriptor attribute);

    private:
#if !OPENGL_ES
        GLuint mHandle = 0;
#else
        GLuint mVBO = 0;
        GLuint mEBO = 0;

        struct VertexArrayAttribute {
            bool mEnabled = false;
            AttributeDescriptor mAttribute;
        };

        std::vector<VertexArrayAttribute> mAttributes;
#endif
    };

}
}