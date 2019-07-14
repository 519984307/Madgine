#include "Madgine/baselib.h"


#include "Madgine/app/application.h"
#include "Madgine/app/appsettings.h"
#include "Madgine/core/root.h"
#include "Modules/threading/workgroup.h"

#if WINDOWS
#	include <conio.h>
#endif

#if EMSCRIPTEN
#	define FIX_LOCAL static
#else
#	define FIX_LOCAL
#endif

DLL_EXPORT_TAG int main() {
	int result;
	{
		FIX_LOCAL Engine::Threading::WorkGroup workGroup("Launcher");
		FIX_LOCAL Engine::Core::Root root;
		FIX_LOCAL Engine::App::AppSettings settings;
		settings.mRunMain = false;
		settings.mAppName = "Madgine Client";
		settings.mWindowSettings.mTitle = "Maditor";
		FIX_LOCAL Engine::App::Application app(settings);
		FIX_LOCAL Engine::Threading::Scheduler scheduler(workGroup, { &app.frameLoop() });
		result = scheduler.go();
	}
#if WINDOWS
	while (!_kbhit());
#endif
	return result;
}


#if ANDROID

#include "android/androidlauncher.h"

extern "C" DLL_EXPORT void ANativeActivity_onCreate(ANativeActivity* activity,
	void* savedState, size_t savedStateSize) {
	new Engine::Android::AndroidLauncher(activity);
}

#endif
