#include "Interfaces/commonlib.h"

#include "Interfaces/plugins/binaryinfo.h"

#include "Interfaces/macros.h"

/*namespace Engine{
	namespace Plugins {
	*/

		extern "C" DLL_EXPORT const Engine::Plugins::BinaryInfo PLUGIN_LOCAL(binaryInfo) {

			BINARY_MAJOR_VERSION,
				BINARY_MINOR_VERSION,
				BINARY_PATCH_NUMBER,

#ifdef PLUGIN_BUILD
				"PLUGIN_"
#endif
				STRINGIFY2(PROJECT_NAME),
				
				BINARY_PROJECT_ROOT,
				BINARY_SOURCE_ROOT,
				BINARY_BINARY_DIR,
				BINARY_FILENAME,

				PRECOMPILED_HEADER_PATH

		};

		/*
	}
}*/