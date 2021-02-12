#pragma once

#include "Modules/moduleslib.h"
#include "Madgine/resourceslib.h"

#if defined(TextureLoader_EXPORTS)
#    define MADGINE_TEXTURELOADER_EXPORT DLL_EXPORT
#else
#    define MADGINE_TEXTURELOADER_EXPORT DLL_IMPORT
#endif

#include "textureloaderforward.h"