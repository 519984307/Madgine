#pragma once

#include <stddef.h>

namespace Engine {
	namespace Plugins {

		struct BinaryInfo {

			size_t mMajorVersion;
			size_t mMinorVersion;
			size_t mPathNumber;

			const char *mProjectRoot;
			const char *mSourceRoot;
			const char *mBinaryDir;
			const char *mBinaryName;

			const char *mPrecompiledHeaderPath;

		};

#ifndef STATIC_BUILD
		extern "C" extern const BinaryInfo PLUGIN_LOCAL(binaryInfo);
#endif

	}
}