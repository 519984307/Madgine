#pragma once

#include "../comparator_traits.h"

namespace Engine {

    template <typename C, typename>
struct container_traits;

template <typename C>
struct SortedContainerApi : C {

    using C::C;
    using C::operator=;

    using traits = container_traits<C>;    

    /*iterator find(const key_type& key)
			{
				return kvFind(this->mData, key);
			}*/

    template <typename T>
    bool contains(const T &key)
    {
        return this->find(key) != this->end();
    }

    template <typename... _Ty>
    auto emplace(_Ty &&... args)
    {
        return this->C::emplace(this->end(), std::forward<_Ty>(args)...);
    }

    /*template <class Ty, class _ = decltype(std::declval<typename C::NativeContainerType>().find(std::declval<Ty>()))>
			iterator find(const Ty& v)
			{
				return this->mData.find(v);
			}*/
};

template <typename C>
struct container_traits<SortedContainerApi<C>> : container_traits<C> {
};
}
