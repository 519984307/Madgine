#include "Madgine/baselib.h"
#include "Madgine/clientlib.h"

#include "Interfaces/window/windowapi.h"
#include "Madgine/app/application.h"
#include "Madgine/app/appsettings.h"
#include "Madgine/core/root.h"
#include "Madgine/gui/toplevelwindow.h"
#include "Modules/cli/parameter.h"
#include "Modules/threading/scheduler.h"
#include "Modules/threading/workgroup.h"

#include "Madgine/widgetslib.h"
#include "Madgine/widgets/widgetmanager.h"

#include "Modules/filesystem/filemanager.h"
#include "Modules/xml/xmlformatter.h"

#include "Modules/resources/resourcemanager.h"

Engine::CLI::Parameter<bool> toolMode { { "--toolMode", "-t" }, false, "If enabled, no application will be started. Only the root will be initialized and then immediately shutdown again." };

#if EMSCRIPTEN
#    define FIX_LOCAL static
#else
#    define FIX_LOCAL
#endif

int launch(Engine::GUI::TopLevelWindow **topLevelPointer = nullptr)
{

    FIX_LOCAL Engine::App::AppSettings settings;

    settings.mAppName = "Madgine Client";
    FIX_LOCAL Engine::App::Application app { settings };

    FIX_LOCAL Engine::Window::WindowSettings windowSettings;
    windowSettings.mTitle = "Maditor";
    FIX_LOCAL Engine::GUI::TopLevelWindow window { windowSettings };

    if (topLevelPointer)
        *topLevelPointer = &window;

#if !ENABLE_PLUGINS
    window.frameLoop()
        .addSetupSteps([&]() {
            Engine::Filesystem::FileManager mgr("Layout");
            Engine::Serialize::SerializeInStream file = mgr.openRead(Engine::Resources::ResourceManager::getSingleton().findResourceFile("default.layout"), std::make_unique<Engine::XML::XMLFormatter>());

            if (file) {
                window.readState(file);
                window.getWindowComponent<Engine::Widgets::WidgetManager>().openStartupWidget();
            }
        });
#endif

    FIX_LOCAL Engine::Threading::Scheduler scheduler;
    return scheduler.go();
}

#if !EMSCRIPTEN
DLL_EXPORT_TAG int main(int argc, char **argv)
{
    Engine::Threading::WorkGroup workGroup("Launcher");
    Engine::Core::Root root { argc, argv };
    if (!toolMode) {
        return launch();
    } else {
        return root.errorCode();
    }
}
#endif