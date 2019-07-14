#include "../../clientlib.h"

#include "scenewindow.h"

#include "Modules/math/vector4.h"

#include "../vertex.h"

#include "../guisystem.h"

#include "Madgine/scene/camera.h"

#include "toplevelwindow.h"

#include "../../render/renderwindow.h"


namespace Engine
{
	namespace GUI
	{
		SceneWindow::~SceneWindow()
		{
		}

		void SceneWindow::setCamera(Scene::Camera * camera)
		{
			const Engine::Vector3 screenSize = window().getScreenSize();
			mTarget = window().getRenderer()->createRenderTarget(camera, (getAbsoluteSize() * screenSize).xy());
		}

		std::vector<Vertex> SceneWindow::vertices(const Vector3 & screenSize)
		{
			std::vector<Vertex> result;

			Vector3 pos = (getAbsolutePosition() * screenSize) / screenSize;
			Vector3 size = (getAbsoluteSize() * screenSize) / screenSize;

			Vector4 color{1,1,1,1};

			uint32_t texId = mTarget ? mTarget->textureId() : 0;

			Vector3 v = pos;
			v.z = static_cast<float>(depth());
			result.push_back({ v, color, {0,0}, texId });
			v.x += size.x;
			result.push_back({ v, color, {1,0}, texId });
			v.y += size.y;
			result.push_back({ v, color, {1,1}, texId });
			result.push_back({ v, color, {1,1}, texId });
			v.x -= size.x;
			result.push_back({ v, color, {0,1}, texId });
			v.y -= size.y;
			result.push_back({ v, color, {0,0}, texId });
			return result;
		}

		Render::RenderTarget * SceneWindow::getRenderTarget()
		{
			return mTarget.get();
		}

		void SceneWindow::sizeChanged(const Vector3 &pixelSize)
		{
			if (mTarget) {				
				mTarget->resize(pixelSize.xy());
			}
		}

	}
}