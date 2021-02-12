#pragma once

#include "sortedcontainerapi.h"
#include "underlying_container.h"
#include "../then.h"

namespace Engine {

	
template <typename C, typename Base>
struct container_api_impl : container_api_impl<C, typename underlying_container<Base>::type> {
    using container_api_impl<C, typename underlying_container<Base>::type>::operator=;
};


template <typename C>
using container_api = container_api_impl<C, C>;

template <typename C, typename Base>
struct underlying_container<container_api_impl<C, Base>> {
    typedef C type;
};

template <typename C, typename... Ty>
struct container_api_impl<C, std::list<Ty...>> : C {

    using C::C;

    void remove(const typename C::value_type &item)
    {
        for (typename C::iterator it = this->begin(); it != this->end();) {
            if (*it == item) {
                it = this->erase(it);
            } else {
                ++it;
            }
        }
    }

    decltype(auto) push_back(const typename C::value_type &item)
    {
        return emplace_back(item);
    }

    template <typename... _Ty>
    decltype(auto) emplace_back(_Ty &&... args)
    {
        return then(this->emplace(this->end(), std::forward<_Ty>(args)...), [](auto &&it) -> decltype(auto) {
            return *it;
        });
    }

    const typename C::value_type &back() const
    {
        return C::back();
    }

    typename C::value_type &back()
    {
        return C::back();
    }
};


template <typename C, typename... Ty>
struct container_api_impl<C, std::vector<Ty...>> : C {

    using C::C;

    using C::operator=;

    using value_type = typename C::value_type;

    /*void resize(size_t size)
            {
                C::resize(size);
            }*/

    void remove(const value_type &item)
    {
        for (typename C::const_iterator it = this->begin(); it != this->end();) {
            if (*it == item) {
                it = erase(it);
            } else {
                ++it;
            }
        }
    }

    decltype(auto) push_back(const value_type &item)
    {
        return then(emplace_back(item), [](auto &&it) -> decltype(auto) {
            return *it;
        });
    }

    decltype(auto) push_back(value_type &&item)
    {
        return then(emplace_back(std::move(item)), [](auto &&it) -> decltype(auto) {
            return *it;
        });
    }

    template <typename... _Ty>
    decltype(auto) emplace_back(_Ty &&... args)
    {
        return this->emplace(this->end(), std::forward<_Ty>(args)...);
    }

    value_type &at(size_t i)
    {
        return C::at(i);
    }

    const value_type &at(size_t i) const
    {
        return C::at(i);
    }

    value_type &operator[](size_t i)
    {
        return C::operator[](i);
    }

    const value_type &operator[](size_t i) const
    {
        return C::operator[](i);
    }
};


template <typename C, typename... Ty>
struct container_api_impl<C, std::set<Ty...>> : SortedContainerApi<C> {
};

template <typename C, typename K, typename T, typename... Ty>
struct container_api_impl<C, std::map<K, T, Ty...>> : SortedContainerApi<C> {
    using SortedContainerApi<C>::SortedContainerApi;

    T &operator[](const K &key)
    {
        typename C::iterator it = C::lower_bound(key);
        if (it == this->end() || it->first != key) {
            auto pib = try_emplace(key);
            assert(pib.second);
            it = pib.first;
        }
        return it->second;
    }

    template <typename... _Ty>
    auto try_emplace(const K &key, _Ty &&... args) -> decltype(C::emplace(C::lower_bound(key), std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<_Ty>(args)...)))
    {
        auto it = C::lower_bound(key);
        return C::emplace(it, std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<_Ty>(args)...));
    }
};


}