#include "../../toolslib.h"
#include "OpenGL/opengllib.h"

#include "openglgridpass.h"

#include "OpenGL/openglshaderloader.h"

#include "Modules/math/matrix4.h"

#include "Madgine/scene/camera.h"
#include "Madgine/render/rendertarget.h"

namespace Engine {
namespace Tools {

    OpenGlGridPass::OpenGlGridPass()
    {
        std::shared_ptr<Render::OpenGLShader> vertexShader = Render::OpenGLShaderLoader::load("grid_VS");
        std::shared_ptr<Render::OpenGLShader> pixelShader = Render::OpenGLShaderLoader::load("grid_PS");

        if (!mProgram.link(vertexShader.get(), pixelShader.get()))
            throw 0;

        mVAO.bind();

        mVertexBuffer.bind(GL_ARRAY_BUFFER);
        GL_CHECK();
        glVertexAttribPointer(
            0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
            4, // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            sizeof(Vector4), // stride
            (void *)0 // array buffer offset
        );
        glEnableVertexAttribArray(0);
        GL_CHECK();

        Vector4 vertices[] = {
            { 0, 0, 0, 1 }, { 1, 0, 0, 0 }, { 0, 0, 1, 0 }, { -1, 0, 0, 0 }, { 0, 0, -1, 0 }/*,
            { 0, 0, 0, 1 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, -1, 0, 0 }, { 0, 0, -1, 0 },
            { 0, 0, 0, 1 }, { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { -1, 0, 0, 0 }, { 0, -1, 0, 0 }*/
        };
        mVertexBuffer.setData(GL_ARRAY_BUFFER, sizeof(vertices), vertices);

        unsigned int indices[] = {
            0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1/*,
            5, 6, 7, 5, 7, 8, 5, 8, 9, 5, 9, 6,
            10, 11, 12, 10, 12, 13, 10, 13, 14, 10, 14, 11*/
        };
        mIndexBuffer.setData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices);
    }

    void OpenGlGridPass::render(Render::RenderTarget *target, Scene::Camera *camera)
    {
        const Vector2 &size = target->getSize();
        float aspectRatio = size.x / size.y;

        mProgram.setUniform("mvp", camera->getViewProjectionMatrix(aspectRatio));

        mVAO.bind();

        mVertexBuffer.bind(GL_ARRAY_BUFFER);
        mIndexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
        GL_CHECK();
        glVertexAttribPointer(
            0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
            4, // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            sizeof(Vector4), // stride
            (void *)0 // array buffer offset
        );
        glEnableVertexAttribArray(0);
        GL_CHECK();

        glDrawElements(
            GL_TRIANGLES, // mode
            12, // count
            GL_UNSIGNED_INT, // type
            (void *)0 // element array buffer offset
        );


    }
}
}