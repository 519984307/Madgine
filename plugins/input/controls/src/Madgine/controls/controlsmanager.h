#pragma once

#include "Madgine/app/globalapibase.h"
#include "Madgine/app/globalapicollector.h"

namespace Engine {
namespace Input {

    typedef void ActionCallback(const ActionDescriptor *, void *);

    struct ActionListener {
        ActionCallback *mF;
        void *mUserData;
    };

    struct ActionDescriptor {
        std::string_view mName;
        std::vector<ActionListener> mListeners;
    };

    struct MADGINE_CONTROLS_EXPORT ControlsManager : App::GlobalAPI<ControlsManager> {

        ControlsManager(App::Application &app);

        const ActionDescriptor *action(std::string_view name);

        template <auto f>
        void addListener(std::string_view action, typename CallableTraits<decltype(f)>::class_type *item)
        {
            addListener(
                action,
                [](ActionDescriptor *desc, void *self) {
                    TupleUnpacker::invoke(f, reinterpret_cast<typename CallableTraits<decltype(f)>::class_type *>(self), desc);
                },
                item);
        }
        void addListener(std::string_view action, ActionCallback callback, void *userData = nullptr);

        void trigger(const ActionDescriptor *action);

        void addAxisEventListener(AxisEventListener *listener);
        void removeAxisEventListener(AxisEventListener *listener);

        bool injectAxisEvent(const Input::AxisEventArgs &arg);

    private:
        std::map<std::string, ActionDescriptor> mControlsMap;
        std::vector<AxisEventListener *> mAxisEventListeners;
    };

}
}

RegisterType(Engine::Input::ControlsManager)