#pragma once

#include "../filesystem/path.h"
#include "../filesystem/filequery.h"

namespace Engine {
namespace Dl {

    INTERFACES_EXPORT Filesystem::FileQuery listSharedLibraries();
    //INTERFACES_EXPORT std::set<std::string> listLoadedLibraries();
}
}