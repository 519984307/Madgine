#include <Madgine/baselib.h>
#include <Madgine/serverlib.h>
#include <Madgine/clientlib.h>
#include "mmolobbylib.h"

#include "Madgine/core/root.h"
#include "Modules/threading/workgroup.h"
#include "server.h"
#include "Modules/threading/scheduler.h"

#include "Madgine/window/mainwindow.h"

#include "Madgine/app/application.h"

#include "Interfaces/window/windowsettings.h"

int main() {

	Engine::Threading::WorkGroup group("MMOLobby-Server");
	Engine::Core::Root root;

	Engine::Util::StandardLog::setLogLevel(Engine::Util::MessageType::DEBUG_TYPE);

	MMOLobby::Server server;

	Engine::App::Application app;

	Engine::Window::WindowSettings windowSettings;
	windowSettings.mTitle = "MMOLobby-Server";
	windowSettings.mRestoreGeometry = false;
	Engine::Window::MainWindow window{ windowSettings };

	return Engine::Threading::Scheduler{}.go();
}