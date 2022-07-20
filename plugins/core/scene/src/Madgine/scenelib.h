#pragma once

/// @cond

#include "Madgine/baselib.h"
#include "Madgine/meshloaderlib.h"
#include "Madgine/skeletonloaderlib.h"
#include "Madgine/animationloaderlib.h"

#if defined(Scene_EXPORTS)
#    define MADGINE_SCENE_EXPORT DLL_EXPORT
#else
#    define MADGINE_SCENE_EXPORT DLL_IMPORT
#endif

#include "sceneforward.h"

#include <queue>
#include <set>

/// @endcond
