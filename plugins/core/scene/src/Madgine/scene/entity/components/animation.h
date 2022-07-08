#pragma once

#include "../entitycomponent.h"

#include "animationloader.h"
#include "skeletonloader.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Animation : EntityComponent<Animation> {
            SERIALIZABLEUNIT(Animation);

            using EntityComponent<Animation>::EntityComponent;
            virtual ~Animation() = default;

            std::string_view getName() const;
            void setName(std::string_view name);

            Render::AnimationLoader::ResourceType *get() const;
            void set(Render::AnimationLoader::HandleType handle);

            void setCurrentAnimationName(std::string_view name);
            void setCurrentAnimationPtr(Render::AnimationDescriptor *desc);
            Render::AnimationDescriptor *currentAnimationPtr() const;

            IndexType<uint32_t> currentAnimation() const;
            void setCurrentAnimation(IndexType<uint32_t> index);

            void step(float delta);
            void setStep(float step);
            float currentStep() const;

            void updateRender(Entity *entity);

        private:
            void refreshCache();

        private:
            Render::AnimationLoader::HandleType mAnimationList;
            Render::SkeletonLoader::HandleType mSkeletonCache;
            int *mBoneIndexMapping = nullptr;
            IndexType<uint32_t> mCurrentAnimation;
            float mCurrentStep = 0.0f;
        };

        using AnimationPtr = EntityComponentPtr<Animation>;

    }
}
}

RegisterType(Engine::Scene::Entity::Animation);
RegisterType(Engine::Scene::Entity::EntityComponentList<Engine::Scene::Entity::Animation>);