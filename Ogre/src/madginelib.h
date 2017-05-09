#pragma once

/// @cond

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#if defined(OgreMadgine_EXPORTS)
#define OGREMADGINE_EXPORT __declspec(dllexport)

#include <unordered_map>
namespace std {
	namespace tr1 {
		template <class Key>
		using hash = std::hash<Key>;

		template <class Key, class T>
		using unordered_map = std::unordered_map<Key, T>;
	}
}

#else
#define OGREMADGINE_EXPORT __declspec(dllimport)
#endif

#include "baselib.h"

#include <stack>
#include <memory>
#include <map>
#include <array>
#include <sstream>
#include <vector>
#include <typeindex>
#include <chrono>

#include "forward.h"


#include <Ogre.h>

#include <Overlay/OgreFont.h>
#include <Overlay/OgreFontManager.h>

#include <OgreTerrainGroup.h>


#ifdef _MSC_VER
#pragma warning(pop)
#endif

/// @endcond
