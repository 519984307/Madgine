#include "../interfaceslib.h"

#if WINDOWS

#    include "api.h"
#    include "path.h"

#    define NOMINMAX
#    include <Windows.h>

#    include <cctype>

#    include <direct.h>

#    include <ShlObj.h>

namespace Engine {
namespace Filesystem {

    void setup(void*) {

    }

    bool isDir(const Path &p)
    {
        return (GetFileAttributesA(p.c_str()) & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    Path executablePath()
    {
        char buffer[512];

        auto result = GetModuleFileName(nullptr, buffer, sizeof(buffer));
        assert(result > 0);

        return Path(buffer).parentPath();
    }

    std::string executableName()
    {
        char buffer[512];

        auto result = GetModuleFileName(nullptr, buffer, sizeof(buffer));
        assert(result > 0);

        return std::string { Path(buffer).stem() };
    }

    Path appDataPath()
    {
        wchar_t *wBuffer;
        auto ret = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &wBuffer);
        assert(SUCCEEDED(ret));

        std::string buffer;
        buffer.reserve(wcslen(wBuffer));
        std::copy(wBuffer, wBuffer + wcslen(wBuffer), std::back_inserter(buffer));

        Path result { std::move(buffer) };

        CoTaskMemFree(wBuffer);

        result /= executableName();

        if (!exists(result))
            createDirectory(result);

        return result;
    }

    void createDirectory(const Path &p)
    {
        auto result = CreateDirectory(p.c_str(), NULL);
        assert(result);
    }

    bool exists(const Path &p)
    {
        return GetFileAttributes(p.c_str()) != INVALID_FILE_ATTRIBUTES;
    }

    bool remove(const Path &p)
    {
        return DeleteFile(p.c_str());
    }

    void makeNormalized(std::string &p)
    {
        if (p.find('\\') == std::string::npos && p.find('/') == std::string::npos)
            return;

        if (GetFileAttributes(p.c_str()) == INVALID_FILE_ATTRIBUTES)
            return; //TODO ???

        char buffer[1024];
        auto result = GetLongPathNameA(p.c_str(), buffer, sizeof(buffer));
        if (result == 0) {
            auto error = GetLastError();
            assert(false);
        }
        assert(result < sizeof(buffer));
        buffer[0] = toupper(buffer[0]);
        p = buffer;
    }

    bool isValidPath(const std::string &p)
    {
        if (p.empty())
            return true;
        const char *c = p.data();
        if (p[0] != '.') {
            if (!std::isalnum(p[0]) && p[0] != '_')
                return false;
            if (p.size() == 1)
                return true;
            if (!std::isalnum(p[1]) && !isSeparator(p[1]) && !std::ispunct(p[1]) && p[1] != ':')
                return false;
            c = p.data() + 2;
        }
        for (; c < p.data() + p.size(); ++c)
            if ((!std::isalnum(*c) && !isSeparator(*c) && !std::ispunct(*c) && *c != ' ')
                || *c == '<'
                || *c == '>'
                || *c == ':'
                || *c == '"'
                || *c == '|'
                || *c == '?'
                || *c == '*')
                return false;
        return true;
    }

    bool isAbsolute(const Path &p)
    {
        auto colon = p.str().find(':');
        return colon != std::string::npos;
    }

    static bool compareChar(char c1, char c2)
    {
        return std::toupper(c1) == std::toupper(c2);
    }

    bool isEqual(const Path &p1, const Path &p2)
    {
        return (p1.str().size() == p2.str().size()) && std::equal(p1.str().begin(), p1.str().end(), p2.str().begin(), &compareChar);
    }

    InStream openFileRead(const Path &p, bool isBinary)
    {
        std::unique_ptr<std::filebuf> buffer = std::make_unique<std::filebuf>();
        if (buffer->open(p.c_str(), static_cast<std::ios_base::openmode>(std::ios_base::in | (isBinary ? std::ios_base::binary : 0))))
            return { std::move(buffer) };
        else
            return {};
    }

    OutStream openFileWrite(const Path &p, bool isBinary)
    {
        std::unique_ptr<std::filebuf> buffer = std::make_unique<std::filebuf>();
        if (buffer->open(p.c_str(), static_cast<std::ios_base::openmode>(std::ios_base::out | (isBinary ? std::ios_base::binary : 0))))
            return { std::move(buffer) };
        else
            return {};
    }

    void setCwd(const Path &p)
    {
        auto result = _chdir(p.c_str());
        assert(result == 0);
    }

    Path getCwd()
    {
        char buffer[256];
        auto result = _getcwd(buffer, sizeof(buffer));
        assert(result);
        return buffer;
    }

    FileInfo fileInfo(const Path &path)
    {
        struct _stat64 stats;
        _stat64(path.c_str(), &stats);
        return {
            static_cast<size_t>(stats.st_size)
        };
    }

}
}

#endif