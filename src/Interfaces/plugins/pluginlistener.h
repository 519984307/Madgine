#pragma once

namespace Engine {
	namespace Plugins {


		struct PluginListener {
			virtual bool aboutToUnloadPlugin(const Plugin *p) { return true; }
			virtual bool aboutToLoadPlugin(const Plugin *p) { return true; }
			virtual void onPluginUnload(const Plugin *p) {}
			virtual void onPluginLoad(const Plugin *p) {}
		};

	}
}