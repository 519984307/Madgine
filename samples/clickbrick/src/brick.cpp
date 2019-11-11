#include "Madgine/clientlib.h"
#include "Madgine/scenelib.h"

#include "brick.h"

#include "Modules/keyvalue/metatable_impl.h"
#include "Modules/serialize/serializetable_impl.h"

ENTITYCOMPONENT_IMPL(Brick, ClickBrick::Scene::Brick)

METATABLE_BEGIN(ClickBrick::Scene::Brick)
METATABLE_END(ClickBrick::Scene::Brick)

SERIALIZETABLE_BEGIN(ClickBrick::Scene::Brick)
SERIALIZETABLE_END(ClickBrick::Scene::Brick)

namespace ClickBrick {
namespace Scene {


}
}