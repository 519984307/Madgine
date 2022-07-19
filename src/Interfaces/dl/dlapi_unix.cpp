#include "../interfaceslib.h"

#if UNIX

#include "dlapi.h"

#include <dlfcn.h>
#include <unistd.h>

namespace Engine {
namespace Dl {

    static DlAPIResult toResult(const char *op)
    {        
        const char *error = dlerror();
        if (error){
            fprintf(stderr, "Unix Dl-Error from %s: %s\n", op, error);
            fflush(stderr);
        }
        return DlAPIResult::UNKNOWN_ERROR;
    }

    DlHandle::~DlHandle(){
        close();
    }

    DlAPIResult DlHandle::open(std::string_view name)
    {
        close();

        if (name.empty())
            mHandle = dlopen(nullptr, RTLD_LAZY);
        else
            mHandle = dlopen(name.data(), RTLD_NOW);
        
        if (!mHandle)
            return toResult("DlHandle::open");

        return DlAPIResult::SUCCESS;
    }

    DlAPIResult DlHandle::close()
    {
        if (mHandle){
            int result = dlclose(mHandle);
            
            if (result != 0)
                return toResult("DlHandle::close");
            
            mHandle = nullptr;
        }
        return DlAPIResult::SUCCESS;
    }

    const void *DlHandle::getSymbol(std::string_view name) const
    {
        return dlsym(mHandle, name.data());
    }

    Filesystem::Path DlHandle::fullPath(std::string_view symbolName) const
    {
        Dl_info info;
        auto result = dladdr(getSymbol(symbolName), &info);
        assert(result);
        return info.dli_fname;
    }

}
}

#endif