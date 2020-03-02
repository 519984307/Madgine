#include "../interfaceslib.h"

#if ANDROID

#    include "api.h"

#    include <dirent.h>
#    include <sys/stat.h>
#    include <unistd.h>

#    include "../stringutil.h"

#    include <android/asset_manager.h>

namespace Engine {
namespace Filesystem {

    DLL_EXPORT AAssetManager *sAssetManager = nullptr;

    static const char sAssetPrefix[] = "assets:";

    static bool isAssetPath(const Path &p)
    {
        return StringUtil::startsWith(p.str(), sAssetPrefix);
    }

    static const char *assetDir(const Path &p)
    {
        assert(isAssetPath(p));
        const char *dir = p.c_str() + sizeof(sAssetPrefix);
        if (isSeparator(*dir))
            ++dir;
        return dir;
    }

    Path configPath()
    {
        //TODO
        char buffer[512];

        auto result = readlink("/proc/self/exe", buffer, sizeof(buffer));
        assert(result > 0);

        return Path(buffer).parentPath();
    }

    struct FileQueryState {
        union {
            dirent *mData;
            const char *mAssetData;
        };
        Path mPath;
    };

    static bool isAssetQuery(const FileQueryState &data)
    {
        return isAssetPath(data.mPath);
    }

    static bool skipSymbolic(void *handle, FileQueryState &data)
    {
        while (strcmp(data.mData->d_name, ".") == 0 || strcmp(data.mData->d_name, "..") == 0) {
            data.mData = readdir((DIR *)handle);
            if (!data.mData)
                return false;
        }

        return true;
    }

    FileQueryHandle::FileQueryHandle(Path p, FileQueryState &data)
        : mPath(std::move(p))
        , mHandle(isAssetPath(mPath) ? (void *)AAssetManager_openDir(sAssetManager, assetDir(mPath)) : (void *)opendir(mPath.c_str()))
    {
        if (mHandle) {
            if (!advance(data)) {
                close();
            }
        }
    }

    void FileQueryHandle::close()
    {
        if (mHandle) {
            if (isAssetPath(mPath))
                AAssetDir_close((AAssetDir *)mHandle);
            else
                closedir((DIR *)mHandle);
            mHandle = nullptr;
        }
    }

    bool FileQueryHandle::advance(FileQueryState &data)
    {
        data.mPath = mPath;
        if (isAssetPath(mPath)) {
            data.mAssetData = AAssetDir_getNextFileName((AAssetDir *)mHandle);
            return data.mAssetData != nullptr;
        } else {
            data.mData = readdir((DIR *)mHandle);

            if (!data.mData)
                return false;
            return skipSymbolic(mHandle, data);
        }
    }

    FileQueryState *createQueryState()
    {
        return new FileQueryState;
    }

    void destroyQueryState(FileQueryState *state)
    {
        delete state;
    }

    bool isDir(const FileQueryState &data)
    {
        if (isAssetPath(data.mPath))
            return false;

        struct stat statbuffer;
        auto result = stat((data.mPath / data.mData->d_name).c_str(), &statbuffer);
        assert(result != -1);
        return (statbuffer.st_mode & S_IFMT) == S_IFDIR;
    }

    const char *filename(const FileQueryState &data)
    {
        if (isAssetPath(data.mPath))
            return data.mAssetData;
        else
            return data.mData->d_name;
    }

    void createDirectory(const Path &p)
    {
        assert(!isAssetPath(p));
        auto result = mkdir(p.c_str(), 0700);
        assert(result == 0);
    }

    bool exists(const Path &p)
    {
        if (isAssetPath(p)) {
            AAssetDir *dir = AAssetManager_openDir(sAssetManager, assetDir(p));
            bool result = AAssetDir_getNextFileName(dir) != nullptr;
            AAssetDir_close(dir);
            return result;
        } else {
            return access(p.c_str(), F_OK) != -1;
        }
    }

    bool remove(const Path &p)
    {
        assert(!isAssetPath(p));
        return ::remove(p.c_str());
    }

    Path makeNormalized(const char *p)
    {
        return p;
    }

    bool isAbsolute(const Path &p)
    {
        return isAssetPath(p) || p.c_str()[0] == '/';
    }

    bool isEqual(const Path &p1, const Path &p2)
    {
        return p1 == p2;
    }

    struct AAsset_Streambuf : public std::basic_streambuf<char> {
        AAsset_Streambuf(const char *path)
            : mAsset(AAssetManager_open(sAssetManager, path, AASSET_MODE_RANDOM))
        {
        }

        ~AAsset_Streambuf()
        {
            AAsset_close(mAsset);
        }

        pos_type seekoff(off_type off, std::ios_base::seekdir dir,
            std::ios_base::openmode mode = std::ios_base::in) override
        {
            assert(mode & std::ios_base::in);

            int whence;
            switch (dir) {
            case std::ios_base::beg:
                whence = SEEK_SET;
                break;
            case std::ios_base::cur:
                whence = SEEK_CUR;
                off += gptr() - egptr();
                if (off >= eback() - egptr() && off <= 0) {
                    setg(eback(), egptr() + off, egptr());
                    off_t cur = AAsset_seek(mAsset, 0, SEEK_CUR);
                    return pos_type(off_type(cur + off));
                }
                break;
            case std::ios_base::end:
                whence = SEEK_END;
                break;
            default:
                std::terminate();
            }

            off_t pos = AAsset_seek(mAsset, off, whence);
            if (pos != (off_t)-1) {
                fetch();
            }
            return pos_type(off_type(pos));
        }

        pos_type seekpos(pos_type pos,
            std::ios_base::openmode mode = std::ios_base::in) override
        {
            assert(mode & std::ios_base::in);

            off_t newPos = AAsset_seek(mAsset, pos, SEEK_SET);
            if (newPos != (off_t)-1) {
                fetch();
            }

            return pos_type(off_type(pos));
        }

        int underflow() override
        {
            return fetch();
        }

        int fetch()
        {
            int count = AAsset_read(mAsset, mBuffer.data(), BUFFER_SIZE);
            if (count < 0)
                std::terminate();
            setg(mBuffer.data(), mBuffer.data(), mBuffer.data() + count);
            if (count == 0)
                return EOF;
            return int((unsigned char)(*gptr()));
        }

    private:
        static const constexpr size_t BUFFER_SIZE = 100;
        std::array<char, BUFFER_SIZE> mBuffer;
        AAsset *mAsset;
    };

    InStream openFileRead(const Path &p, bool isBinary)
    {
        if (isAssetPath(p)) {
            return { std::make_unique<AAsset_Streambuf>(assetDir(p)) };
        } else {
            std::unique_ptr<std::filebuf> buffer = std::make_unique<std::filebuf>();
            if (buffer->open(p.c_str(), std::ios_base::in | (isBinary ? std::ios_base::binary : 0)))
                return { std::move(buffer) };
            else
                return {};
        }
    }

    void setCwd(const Path &p)
    {
        auto result = chdir(p.c_str());
        assert(result == 0);
    }

    Path getCwd()
    {
        char buffer[256];
        auto result = getcwd(buffer, sizeof(buffer));
        assert(result);
        return buffer;
    }

}
}

#endif