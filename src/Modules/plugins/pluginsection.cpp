#include "../moduleslib.h"

#if ENABLE_PLUGINS

#include "pluginsection.h"

#include "plugin.h"

#include "pluginmanager.h"

#include "Interfaces/stringutil.h"

#include "Interfaces/exception.h"

#include "../threading/defaulttaskqueue.h"

#include "pluginlistener.h"

#include "Interfaces/filesystem/runtime.h"

#include "../keyvalue/keyvalue.h"

namespace Engine {
namespace Plugins {

    PluginSection::PluginSection(PluginManager &mgr, const std::string &name)
        : mName(name)
        , mMgr(mgr)
    {
        const std::regex e { SHARED_LIB_PREFIX "Plugin_([a-zA-Z]*)_" + mName + "_([a-zA-Z]*)\\" SHARED_LIB_SUFFIX };
        std::smatch match;
        for (auto path : Filesystem::listSharedLibraries()) {
            if (std::regex_match(path.str(), match, e)) {
                std::string project = match[1];
                std::string name = match[2];
                auto pib = mPlugins.try_emplace(name, name, this, project, path);
                assert(pib.second);
            }
        }
    }

    void PluginSection::setAtleastOne(bool atleastOne)
    {
        mAtleastOne = atleastOne;
        if (atleastOne) {
            if (mPlugins.empty())
                throw exception("No plugin available in Section tagged as atleastOne: "s + mName);
            for (std::pair<const std::string, Plugin> &p : mPlugins)
                if (p.second.isLoaded())
                    return;
            if (!loadPlugin(&mPlugins.begin()->second))
                throw exception("Failed to load default Plugin for atleastOne Section: "s + mPlugins.begin()->first);
            mMgr.saveCurrentSelectionFile();
        }
    }

    bool PluginSection::isAtleastOne() const
    {
        return mAtleastOne;
    }

    void PluginSection::setExclusive(bool exclusive)
    {
        mExclusive = exclusive;
        if (exclusive) {
            bool foundOne = false;
            for (std::pair<const std::string, Plugin> &p : mPlugins) {
                if (p.second.isLoaded()) {
                    if (!foundOne)
                        foundOne = true;
                    else if (!unloadPlugin(&p.second))
                        throw exception("Failed to unload Plugin for exclusive Section: "s + p.first);
                }
            }
            mMgr.saveCurrentSelectionFile();
        }
    }

    bool PluginSection::isExclusive() const
    {
        return mExclusive;
    }

    bool PluginSection::isLoaded(const std::string &name) const
    {
        auto it = mPlugins.find(name);
        if (it != mPlugins.end())
            return it->second.isLoaded();
        return false;
    }

    Plugin::LoadState PluginSection::loadPlugin(const std::string &name)
    {
        Plugin *plugin = getPlugin(name);
        if (!plugin)
            return Plugin::UNLOADED;
        return loadPlugin(plugin);
    }

    Plugin::LoadState PluginSection::unloadPlugin(const std::string &name)
    {
        Plugin *plugin = getPlugin(name);
        if (!plugin)
            return Plugin::UNLOADED;
        return unloadPlugin(plugin);
    }

    bool PluginSection::loadPluginByFilename(const std::string &name)
    {
        auto pib = mPlugins.try_emplace(name, name);
        assert(pib.second);
        return loadPlugin(&pib.first->second);
    }

    void PluginSection::addListener(PluginListener *listener)
    {
        std::unique_lock lock(mMgr.mListenersMutex);
        mListeners.push_back(listener);
        for (const std::pair<const std::string, Plugins::Plugin> &p : *this) {
            if (p.second.isLoaded())
                listener->onPluginLoad(&p.second);
        }
    }

    void PluginSection::removeListener(PluginListener *listener)
    {
        std::unique_lock lock(mMgr.mListenersMutex);
        for (const std::pair<const std::string, Plugins::Plugin> &p : *this) {
            if (p.second.isLoaded())
                listener->aboutToUnloadPlugin(&p.second);
        }
        mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), listener), mListeners.end());
    }

    Plugin *PluginSection::getPlugin(const std::string &name)
    {
        auto it = mPlugins.find(name);
        if (it == mPlugins.end())
            return nullptr;
        return &it->second;
    }

    Plugin::LoadState PluginSection::loadPlugin(Plugin *p)
    {
        if (p->isLoaded() != Plugin::UNLOADED)
            return p->isLoaded();

        bool ok = true;
        {
            std::unique_lock lock(mMgr.mListenersMutex);
            for (PluginListener *listener : mListeners)
                ok &= listener->aboutToLoadPlugin(p);
        }

        auto task = [=]() {
            Plugin::LoadState result = p->load();
            auto task = [=]() {
                mMgr.saveCurrentSelectionFile();
                Plugin::LoadState result = Plugin::LOADED;
                Plugin *unloadExclusive
                    = nullptr;
                if (mExclusive) {
                    for (Plugin &p2 : kvValues(mPlugins)) {
                        if (&p2 != p && p2.isLoaded()) {
                            assert(!unloadExclusive);
                            unloadExclusive = &p2;
                        }
                    }
                }

                if (unloadExclusive)
                    result = unloadPlugin(unloadExclusive);
                auto task = [=]() {
                    if (unloadExclusive && unloadExclusive->isLoaded())
                        return p->unload();
                    else {
                        std::unique_lock lock(mMgr.mListenersMutex);
                        for (PluginListener *listener : mListeners)
                            listener->onPluginLoad(p);
                    }
                    return Plugin::LOADED;
                };
                if (result == Plugin::DELAYED) {
                    Threading::DefaultTaskQueue::getSingleton().queue(std::move(task));
                    return Plugin::DELAYED;
                } else {
                    return task();
                }
            };
            if (result == Plugin::DELAYED) {
                Threading::DefaultTaskQueue::getSingleton().queue(std::move(task));
                return Plugin::DELAYED;
            } else if (result == Plugin::LOADED) {
                return task();
            } else {
                return Plugin::UNLOADED;
            }
        };

        if (!ok) {
            Threading::DefaultTaskQueue::getSingleton().queue(std::move(task));
            return Plugin::DELAYED;
        } else {
            return task();
        }
    }

    Plugin::LoadState PluginSection::unloadPlugin(Plugin *p)
    {
        if (p->isLoaded() != Plugin::LOADED)
            return p->isLoaded();

        //assert(!mAtleastOne);
        bool ok = true;
        {
            std::unique_lock lock(mMgr.mListenersMutex);
            for (PluginListener *listener : mListeners)
                ok &= listener->aboutToUnloadPlugin(p);
        }

        auto task = [=]() {
            Plugin::LoadState result = p->unload();
            auto task = [=]() {
                mMgr.saveCurrentSelectionFile();
                if (!p->isLoaded()) {
                    std::unique_lock lock(mMgr.mListenersMutex);
                    for (PluginListener *listener : mListeners)
                        listener->onPluginUnload(p);
                    return Plugin::LOADED;
                }
                return Plugin::UNLOADED;
            };
            if (result == Plugin::DELAYED) {
                Threading::DefaultTaskQueue::getSingleton().queue(std::move(task));
                return Plugin::DELAYED;
            } else if (result == Plugin::LOADED) {
                return task();
            } else {
                return Plugin::UNLOADED;
            }
        };

        if (ok) {
            return task();
        } else {
            Threading::DefaultTaskQueue::getSingleton().queue(std::move(task));
            return Plugin::DELAYED;
        }
    }

    std::map<std::string, Plugin>::const_iterator PluginSection::begin() const
    {
        return mPlugins.begin();
    }

    std::map<std::string, Plugin>::const_iterator PluginSection::end() const
    {
        return mPlugins.end();
    }

    std::map<std::string, Plugin>::iterator PluginSection::begin()
    {
        return mPlugins.begin();
    }

    std::map<std::string, Plugin>::iterator PluginSection::end()
    {
        return mPlugins.end();
    }

    void PluginSection::loadFromIni(Ini::IniSection &sec)
    {
        for (const std::pair<const std::string, std::string> &p : sec) {
            auto it = mPlugins.find(p.first);
            if (it == mPlugins.end()) {
                LOG("Could not find Plugin \"" << p.first << "\"!");
                continue;
            }
            Plugin &plugin = it->second;
            bool result = p.second.empty() ? (unloadPlugin(&plugin) == Plugin::UNLOADED) : (loadPlugin(&plugin) == Plugin::LOADED);
            if (!result) {
                LOG("Could not load Plugin \"" << p.first << "\"!");
            }
        }
    }

    PluginManager &PluginSection::manager()
    {
        return mMgr;
    }

    const void *PluginSection::getUniqueSymbol(const std::string &name) const
    {
        const void *symbol = nullptr;
        for (const std::pair<const std::string, Plugin> &p : mPlugins) {
            const void *s = p.second.getSymbol(name);
            if (s) {
                if (symbol)
                    throw 0;
                symbol = s;
            }
        }
        return symbol;
    }

}
}

#endif