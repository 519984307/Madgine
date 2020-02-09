#pragma once

#include "Modules/moduleslib.h"
#include "skeletonloaderlib.h"

#if defined(AnimationLoader_EXPORTS)
#    define MADGINE_ANIMATIONLOADER_EXPORT DLL_EXPORT
#else
#    define MADGINE_ANIMATIONLOADER_EXPORT DLL_IMPORT
#endif

#include "animationloaderforward.h"
