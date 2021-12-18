#include "Madgine/baselib.h"
#include "Madgine/clientlib.h"

#include "../launcher.h"
#include "Interfaces/filesystem/api.h"
#include "Interfaces/util/standardlog.h"
#include "Madgine/core/root.h"
#include "Modules/threading/workgroup.h"
#include "cli/cli.h"
#include "cli/parameter.h"
#include <emscripten.h>

extern Engine::CLI::Parameter<Engine::Util::MessageType> logLevel;

std::unique_ptr<Engine::CLI::CLICore> sTempCLI;

void mainImpl()
{
    emscripten_cancel_main_loop();
    static Engine::Threading::WorkGroup workGroup { "Launcher" };
    static Engine::Core::Root root { std::move(sTempCLI) };
    Engine::Util::StandardLog::setLogLevel(logLevel);
    launch();
}

DLL_EXPORT_TAG int main(int argc, char **argv)
{
    sTempCLI = std::make_unique<Engine::CLI::CLICore>(argc, argv);

    void (*callback)() = &mainImpl;
    Engine::Filesystem::setup(&callback);

    emscripten_set_main_loop_arg([](void *) {}, nullptr, 0, false);
    return 0;
}
