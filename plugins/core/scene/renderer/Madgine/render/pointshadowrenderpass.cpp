#include "../scenerendererlib.h"

#include "pointshadowrenderpass.h"

#include "Madgine/scene/scenemanager.h"

#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/pointlight.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entity.h"

#include "Madgine/render/rendertarget.h"

#include "Madgine/render/shadinglanguage/sl.h"

#include "Meta/math/transformation.h"

#include "Meta/math/frustum.h"

//#include "Madgine/render/rendercontext.h"

#define SL_SHADER pointshadow
#include INCLUDE_SL_SHADER

namespace Engine {
namespace Render {

    PointShadowRenderPass::PointShadowRenderPass(size_t index, Scene::SceneManager &scene, int priority)
        : mScene(scene)
        , mIndex(index)
        , mPriority(priority)
    {
    }

    void PointShadowRenderPass::setup(RenderTarget *target)
    {
        mProgram.create("pointshadow", { sizeof(PointShadowPerApplication), sizeof(PointShadowPerFrame), sizeof(PointShadowPerObject) }, sizeof(PointShadowInstanceData));
    }

    void PointShadowRenderPass::shutdown()
    {
        mProgram.reset();
    }

    void PointShadowRenderPass::render(Render::RenderTarget *target, size_t iteration)
    {
        Scene::Entity::EntityComponentList<Scene::Entity::PointLight> &lights = mScene.entityComponentList<Scene::Entity::PointLight>();
        if (mIndex >= lights.size())
            return;

        //TODO Culling

        Threading::DataLock lock { mScene.mutex(), Threading::AccessMode::READ };

        mScene.updateRender();

        std::map<std::tuple<GPUMeshData *, Scene::Entity::Skeleton *>, std::vector<Matrix4>> instances;

        for (const auto &[mesh, e] : mScene.entityComponentList<Scene::Entity::Mesh>().data()) {
            if (!mesh.isVisible())
                continue;

            GPUMeshData *meshData = mesh.data();
            if (!meshData)
                continue;

            Scene::Entity::Transform *transform = e->getComponent<Scene::Entity::Transform>();
            if (!transform)
                continue;

            Scene::Entity::Skeleton *skeleton = e->getComponent<Scene::Entity::Skeleton>();
            
            instances[std::tuple<GPUMeshData *, Scene::Entity::Skeleton *> { meshData, skeleton }].push_back(transform->worldMatrix());
        }

        target->pushAnnotation("PointShadow");

        {
            auto perApplication = mProgram.mapParameters(0).cast<PointShadowPerApplication>();

            Frustum f {
                Vector3::ZERO,
                Quaternion {},
                0.01f, 0.01f,
                0.01f, 100.0f,
                false
            };
            perApplication->p = ProjectionMatrix(f);
        }

        {
            auto perFrame = mProgram.mapParameters(1).cast<PointShadowPerFrame>();

            perFrame->position = lights.getEntity(mIndex)->getComponent<Scene::Entity::Transform>()->getPosition();
        }

        for (std::pair<const std::tuple<GPUMeshData *, Scene::Entity::Skeleton *>, std::vector<Matrix4>> &instance : instances) {
            GPUMeshData *meshData = std::get<0>(instance.first);
            Scene::Entity::Skeleton *skeleton = std::get<1>(instance.first);

            {
                auto perObject = mProgram.mapParameters(2).cast<PointShadowPerObject>();

                perObject->hasSkeleton = skeleton != nullptr;
            }

            size_t instanceCount = instance.second.size();
            mProgram.setInstanceData(std::move(instance.second));

            if (skeleton) {
                mProgram.setDynamicParameters(0, skeleton->matrices());
            } else {
                mProgram.setDynamicParameters(0, {});
            }

            target->renderMeshInstanced(instanceCount, meshData, mProgram);
        }

        target->popAnnotation();
    }

    int PointShadowRenderPass::priority() const
    {
        return mPriority;
    }

}
}
