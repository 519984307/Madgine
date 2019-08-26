#pragma once

#include "Modules/plugins/pluginlocal.h"

#include "Modules/keyvalue/objectptr.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct PluginEntityComponents {
            std::map<std::string, std::function<std::unique_ptr<EntityComponentBase>(Entity &, const ObjectPtr &)>> mComponents;
        };

#if ENABLE_PLUGINS
        extern "C" DLL_EXPORT PluginEntityComponents *PLUGIN_LOCAL(pluginEntityComponents)();
#endif

        class MADGINE_BASE_EXPORT EntityComponentCollector {
        public:
            static bool existsComponent(const std::string &name);

            static std::set<std::string> registeredComponentNames();

            static std::unique_ptr<EntityComponentBase> createComponent(Entity &e, const std::string &name,
                const ObjectPtr &table = {});

        private:
            typedef std::function<std::unique_ptr<EntityComponentBase>(Entity &, const ObjectPtr &)> ComponentBuilder;
#if ENABLE_PLUGINS
            struct LocalComponentStore {
                static PluginEntityComponents *sRegisteredComponentsByName()
                {
                    static PluginEntityComponents dummy;
                    return &dummy;
                }
            };

            friend PluginEntityComponents *PLUGIN_LOCAL(pluginEntityComponents)()
            {
                return LocalComponentStore::sRegisteredComponentsByName();
            }

#define PLUGABLE_COMPONENT LocalComponentStore::

#else

#define PLUGABLE_COMPONENT

#endif

            template <class T>
            static std::unique_ptr<EntityComponentBase> createComponent_t(Entity &e, const ObjectPtr &table = {})
            {
                return std::make_unique<T>(e, table);
            }

        public:
            template <class T>
            class ComponentRegistrator {
            public:
                ComponentRegistrator()
                {
                    const std::string name = T::componentName();
                    assert(PLUGABLE_COMPONENT sRegisteredComponentsByName()->mComponents.find(name) == PLUGABLE_COMPONENT sRegisteredComponentsByName()->mComponents.end());
                    PLUGABLE_COMPONENT sRegisteredComponentsByName()->mComponents[name] = &createComponent_t<T>;
                }

                ~ComponentRegistrator()
                {
                    const std::string name = T::componentName();
                    assert(PLUGABLE_COMPONENT sRegisteredComponentsByName()->mComponents.find(name) != PLUGABLE_COMPONENT sRegisteredComponentsByName()->mComponents.end());
                    PLUGABLE_COMPONENT sRegisteredComponentsByName()->mComponents.erase(name);
                }
            };

        private:
            static PluginEntityComponents *sRegisteredComponentsByName();
        };

    }
}
}