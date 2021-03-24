#pragma once

#include "signalstub.h"

namespace Engine {
namespace Threading {

    template <typename... _Ty>
    struct Signal : SignalStub<_Ty...> {
        Signal() = default;

        Signal(const Signal<_Ty...> &other) = default;

        Signal(Signal<_Ty...> &&other) noexcept = default;

        ~Signal()
        {
            this->disconnectAll();
        }

        void emit(_Ty... args)
        {
            std::vector<std::shared_ptr<Connection<_Ty...>>> connections;

            this->mConnectedSlots.erase(
                std::remove_if(this->mConnectedSlots.begin(), this->mConnectedSlots.end(),
                    [&](const std::weak_ptr<Connection<_Ty...>> &weak) {
                        if (std::shared_ptr<Connection<_Ty...>> ptr = weak.lock()) {
                            connections.emplace_back(std::move(ptr));
                            return false;
                        } else {
                            return true;
                        }
                    }),
                this->mConnectedSlots.end());

            for (std::shared_ptr<Connection<_Ty...>> &con : connections) {
                (*con)(args...);
            }
        }
    };
}
}
