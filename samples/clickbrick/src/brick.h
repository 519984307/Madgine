#pragma once

#include "Madgine/scene/entity/entitycomponent.h"

#include "Modules/math/vector3.h"
#include "Modules/math/quaternion.h"

namespace ClickBrick {
namespace Scene {

    struct Brick : Engine::Scene::Entity::EntityComponent<Brick> {
        using Engine::Scene::Entity::EntityComponent<Brick>::EntityComponent;

        Brick(Brick &&) = default;

        Brick &operator=(Brick &&) = default;

        float mSpeed;
        Engine::Vector3 mDir;

		Engine::Quaternion mQ0, mQ1;
        float mQAcc, mQSpeed;
    };

    using BrickPtr = Engine::Scene::Entity::EntityComponentPtr<Brick>;


}
}

RegisterType(ClickBrick::Scene::Brick);