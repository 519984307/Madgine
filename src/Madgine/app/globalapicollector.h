#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"
#include "globalapibase.h"

DECLARE_UNIQUE_COMPONENT(Engine::App, GlobalAPI, MADGINE_BASE, GlobalAPIBase, Application &);

namespace Engine
{
	namespace App
	{
		template <class T>
		using GlobalAPI = GlobalAPIComponent<T>;
	}
}
