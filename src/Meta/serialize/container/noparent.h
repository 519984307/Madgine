#pragma once

namespace Engine {
namespace Serialize {

    template <typename T>
    struct NoParentUnit : T {
        using decay_t = T;

        template <typename... Args>
        NoParentUnit(Args &&...args)
            : T(std::forward<Args>(args)...)
        {
            if constexpr (std::derived_from<T, TopLevelUnitBase>)
                this->sync();
            else
                setActive(*this, true, true);
        }

        ~NoParentUnit()
        {
            if constexpr (std::derived_from<T, TopLevelUnitBase>)
                this->unsync();
            else
                setActive(*this, false, true);
        }
    };
}
}
