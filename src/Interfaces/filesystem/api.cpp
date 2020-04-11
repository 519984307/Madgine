#include "../interfaceslib.h"
#include "api.h"

namespace Engine {
namespace Filesystem {

    void createDirectories(const Path &p)
    {
        if (!exists(p)) {
            Path parent = p.parentPath();
            if (!parent.empty())
                createDirectories(parent);
            createDirectory(p);
        }
    }

    FileQuery listFilesRecursive(const Path &path)
    {
        return { path, true };
    }

    FileQuery listFilesAndDirsRecursive(const Path &path)
    {
        return { path, true, true };
    }

    FileQuery listFiles(const Path &path)
    {
        return { path, false };
    }

    FileQuery listDirs(const Path &path)
    {
        return { path, false, true, false };
    }

    FileQuery listFilesAndDirs(const Path &path)
    {
        return { path, false, true };
    }

    bool isSeparator(char c)
    {
        return c == '/' || c == '\\';
    }

}
}
