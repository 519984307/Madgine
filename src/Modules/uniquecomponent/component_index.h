#pragma once

namespace Engine{
    namespace UniqueComponent{

#if ENABLE_PLUGINS

    template <typename T>
    size_t component_index()
    {
        return T::component_index();
    }

#else

    template <typename T>
    size_t component_index();

#endif

    }
}