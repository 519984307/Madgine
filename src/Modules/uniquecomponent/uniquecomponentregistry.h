#pragma once

#include "../reflection/classname.h"

#include "../threading/signal.h"

#include "../plugins/binaryinfo.h"

namespace Engine {

template <typename T, typename Base, typename... _Ty>
std::unique_ptr<Base> createComponent(_Ty... arg)
{
    return std::make_unique<T>(std::forward<_Ty>(arg)...);
}

template <typename Base, typename... _Ty>
using Collector_F = std::unique_ptr<Base> (*)(_Ty...);

/*template <typename Container, typename _Base, typename... _Ty>
struct UniqueComponentContainer;*/

template <typename _Base, typename... _Ty>
struct UniqueComponentSelector;

}

#if ENABLE_PLUGINS

namespace Engine {

struct CollectorInfo {
#    ifdef _MSC_VER
    typedef void *ComponentType;
#    else
    typedef Collector_F<void> ComponentType;
#    endif

    std::vector<ComponentType> mComponents;
    const TypeInfo *mRegistryInfo;
    const TypeInfo *mBaseInfo;
    const Plugins::BinaryInfo *mBinary;
    std::vector<std::vector<const TypeInfo *>> mElementInfos;
    size_t mBaseIndex = 0;
};

MODULES_EXPORT void skipUniqueComponentOnExport(const TypeInfo *t);
MODULES_EXPORT void exportStaticComponentHeader(const Filesystem::Path &outFile, bool hasTools);

MODULES_EXPORT std::map<std::string, ComponentRegistryBase *> &registryRegistry();

struct MODULES_EXPORT ComponentRegistryBase {
    ComponentRegistryBase(const TypeInfo *ti, const Plugins::BinaryInfo *binary)
        : mBinary(binary)
        , mTi(ti)
    {
        registryRegistry()[ti->mFullName] = this;
    }

    ~ComponentRegistryBase()
    {
        registryRegistry().erase(mTi->mFullName);
    }

    virtual void onPluginLoad(const Plugins::BinaryInfo *) = 0;
    virtual void onPluginUnload(const Plugins::BinaryInfo *) = 0;

    const TypeInfo *type_info()
    {
        return mTi;
    }

    std::vector<CollectorInfo *>::iterator begin()
    {
        return mLoadedCollectors.begin();
    }

    std::vector<CollectorInfo *>::iterator end()
    {
        return mLoadedCollectors.end();
    }

    const Plugins::BinaryInfo *mBinary;

protected:
    std::vector<CollectorInfo *> mLoadedCollectors;

private:
    const TypeInfo *mTi;
};

template <typename _Base, typename... _Ty>
struct UniqueComponentRegistry;

DLL_IMPORT_VARIABLE2(Registry, uniqueComponentRegistry, typename Registry);

template <typename _Base, typename... _Ty>
struct UniqueComponentRegistry : ComponentRegistryBase {

    typedef _Base Base;
    typedef std::tuple<_Ty...> Ty;
    typedef Collector_F<Base, _Ty...> F;

    UniqueComponentRegistry()
        : ComponentRegistryBase(&typeInfo<UniqueComponentRegistry>(), &Plugins::PLUGIN_LOCAL(binaryInfo))
    {
    }

    static UniqueComponentRegistry &sInstance()
    {
        return uniqueComponentRegistry<UniqueComponentRegistry<_Base, _Ty...>>();
    }

    static std::vector<F> &sComponents()
    {
        return sInstance().mComponents;
    }
    static const std::vector<const MetaTable *> &sTables()
    {
        return sInstance().mTables;
    }

    void addCollector(CollectorInfo *info)
    {
        mUnloadedCollectors.push_back(info);
    }

    void onPluginLoad(const Plugins::BinaryInfo *bin)
    {
        for (auto it = mUnloadedCollectors.begin(); it != mUnloadedCollectors.end();) {
            CollectorInfo *info = *it;
            if (info->mBinary == bin) {
                mLoadedCollectors.push_back(info);
                info->mBaseIndex = mComponents.size();
                const std::vector<F> &comps = reinterpret_cast<const std::vector<F> &>(info->mComponents);
                for (F f : comps) {
                    mComponents.push_back(f);
                }
                mUpdate.emit(info, true, comps);
                it = mUnloadedCollectors.erase(it);
            } else {
                ++it;
            }
        }
    }

    void removeCollector(CollectorInfo *info)
    {
        //assert(std::find(mLoadedCollectors.begin(), mLoadedCollectors.end(), info) == mLoadedCollectors.end());
        mUnloadedCollectors.erase(std::remove(mUnloadedCollectors.begin(), mUnloadedCollectors.end(), info), mUnloadedCollectors.end());
    }

    void onPluginUnload(const Plugins::BinaryInfo *bin)
    {
        for (auto it = mLoadedCollectors.begin(); it != mLoadedCollectors.end();) {
            CollectorInfo *info = *it;
            if (info->mBinary == bin) {
                mUnloadedCollectors.push_back(info);
                it = mLoadedCollectors.erase(it);
                mComponents.erase(mComponents.begin() + info->mBaseIndex, mComponents.begin() + info->mBaseIndex + info->mComponents.size());

                for (CollectorInfo *i : mLoadedCollectors) {
                    if (i->mBaseIndex >= info->mBaseIndex)
                        i->mBaseIndex -= info->mComponents.size();
                }

                std::vector<F> clearV {};
                mUpdate.emit(info, false, clearV);

                info->mBaseIndex = -1;
            } else {
                ++it;
            }
        }
    }

    static Threading::SignalStub<CollectorInfo *, bool, const std::vector<F> &> &update()
    {
        return sInstance().mUpdate;
    }

private:
    static inline UniqueComponentRegistry *sSelf = &sInstance(); //Keep to ensure instantiation of registry, even with no component/collector in it

    std::vector<F> mComponents;
    Threading::Signal<CollectorInfo *, bool, const std::vector<F> &> mUpdate;

    std::vector<CollectorInfo *> mUnloadedCollectors;
};

}

#else

namespace Engine {

template <typename _Base, typename... _Ty>
struct UniqueComponentRegistry {

    typedef _Base Base;
    typedef std::tuple<_Ty...> Ty;
    typedef Collector_F<Base, _Ty...> F;

    static std::vector<F> sComponents();
    static const std::vector<const MetaTable *> &sTables();
};

}
#endif