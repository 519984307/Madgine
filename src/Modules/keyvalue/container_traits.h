#pragma once

#include "../keyvalue/keyvalue.h"

#include "sortedcontainerapi.h"

namespace Engine {

template <typename T, typename = void>
struct is_iterable : std::false_type {
};

template <typename T>
struct is_iterable<T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> : std::true_type {
};

template <typename T>
constexpr const bool is_iterable_v = is_iterable<T>::value;

template <typename C>
struct container_traits : C::traits {
};

template <typename T>
struct container_traits<std::list<T>> {
    static constexpr const bool sorted = false;
    static constexpr const bool has_dependent_handle = false;
    static constexpr const bool remove_invalidates_handles = false;

    typedef std::list<T> container;
    typedef typename container::iterator iterator;
    typedef typename container::const_iterator const_iterator;
    typedef typename container::reverse_iterator reverse_iterator;
    typedef typename container::const_reverse_iterator const_reverse_iterator;
    typedef T *handle;
    typedef const T *const_handle;
    typedef iterator position_handle;
    typedef const_iterator const_position_handle;
    typedef typename container::value_type value_type;
    typedef void key_type;
    typedef T type;

    template <template <typename> typename M>
    using rebind = container_traits<std::list<M<T>>>;

    template <typename C>
    struct api : C {

        using C::C;

        void remove(const type &item)
        {
            for (iterator it = this->begin(); it != this->end();) {
                if (*it == item) {
                    it = this->erase(it);
                } else {
                    ++it;
                }
            }
        }

        void push_back(const type &item)
        {
            this->emplace(this->end(), item);
        }

        template <class... _Ty>
        std::pair<iterator, bool> emplace_back(_Ty &&... args)
        {
            return this->emplace(this->end(), std::forward<_Ty>(args)...);
        }

        const type &back() const
        {
            return C::back();
        }

        type &back()
        {
            return C::back();
        }
    };

    template <typename... _Ty>
    static std::pair<iterator, bool> emplace(container &c, const const_iterator &where, _Ty &&... args)
    {
        return std::make_pair(c.emplace(where, std::forward<_Ty>(args)...), true);
    }

    static position_handle toPositionHandle(container &c, const iterator &it)
    {
        return it;
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const container &c, const const_iterator &it)
    {
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const container &c, const const_iterator &it, size_t count = 1)
    {
    }

    static iterator toIterator(container &c, const position_handle &handle)
    {
        return handle;
    }

    static const_iterator toIterator(const container &c, const const_position_handle &handle)
    {
        return handle;
    }

    static position_handle next(const position_handle &handle)
    {
        return std::next(handle);
    }
};

template <typename T>
struct container_traits<std::vector<T>> {
    static constexpr const bool sorted = false;
    static constexpr const bool has_dependent_handle = true;
    static constexpr const bool remove_invalidates_handles = true;

    typedef std::vector<T> container;
    typedef typename container::iterator iterator;
    typedef typename container::const_iterator const_iterator;
    typedef typename container::reverse_iterator reverse_iterator;
    typedef typename container::const_reverse_iterator const_reverse_iterator;

    struct handle_t {
        handle_t(size_t index = std::numeric_limits<size_t>::max())
            : mIndex(index)
        {
        }

        operator size_t() const { return mIndex; }

        void operator++() { ++mIndex; }
        void operator--() { --mIndex; }
        handle_t &operator-=(size_t s)
        {
            mIndex -= s;
            return *this;
        }

        size_t mIndex;
    };

    typedef handle_t handle;
    typedef handle_t const_handle;
    typedef handle_t position_handle;
    typedef handle_t const_position_handle;
    typedef typename container::value_type value_type;
    typedef void key_type;
    typedef T type;

    template <template <typename> typename M>
    using rebind = container_traits<std::vector<M<T>>>;

    template <typename C>
    struct api : C {

        using C::C;

        using C::operator=;

        /*void resize(size_t size)
            {
                C::resize(size);
            }*/

        void remove(const type &item)
        {
            for (const_iterator it = this->begin(); it != this->end();) {
                if (*it == item) {
                    it = erase(it);
                } else {
                    ++it;
                }
            }
        }

        void push_back(const type &item)
        {
            emplace(this->end(), item);
        }

        template <typename... _Ty>
        type &emplace_back(_Ty &&... args)
        {
            return *this->emplace(this->end(), std::forward<_Ty>(args)...).first;
        }

        type &at(size_t i)
        {
            return C::at(i);
        }

        const type &at(size_t i) const
        {
            return C::at(i);
        }

        type &operator[](size_t i)
        {
            return C::operator[](i);
        }

        const type &operator[](size_t i) const
        {
            return C::operator[](i);
        }
    };

    template <class... _Ty>
    static std::pair<iterator, bool> emplace(container &c, const const_iterator &where, _Ty &&... args)
    {
        return std::make_pair(c.emplace(where, std::forward<_Ty>(args)...), true);
    }

    static position_handle toPositionHandle(container &c, const iterator &it)
    {
        return std::distance(c.begin(), it);
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const container &c, const const_iterator &it)
    {
        size_t item = std::distance(c.begin(), it);
        if (item <= handle)
            ++handle;
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const container &c, const const_iterator &it, size_t count = 1)
    {
        size_t pivot = std::distance(c.begin(), it);
        assert(handle < pivot || handle >= pivot + count);
        if (handle > pivot)
            handle -= count;
    }

    static iterator toIterator(container &c, const position_handle &handle)
    {
        return c.begin() + handle;
    }

    static const_iterator toIterator(const container &c, const const_position_handle &handle)
    {
        return c.begin() + handle;
    }

    static position_handle next(const position_handle &handle)
    {
        return handle + 1;
    }
};

template <typename T, typename Cmp, typename It>
class SetConstIterator;

template <typename T, typename Cmp, typename It>
class SetIterator {
public:
    using iterator_category = typename It::iterator_category;
    using value_type = typename It::value_type;
    using difference_type = typename It::difference_type;
    using pointer = std::remove_const_t<typename It::value_type> *;
    using reference = std::remove_const_t<typename It::value_type> &;

    SetIterator()
    {
    }

    SetIterator(It &&it)
        : mIterator(std::forward<It>(it))
    {
    }

    SetIterator(const It &it)
        : mIterator(it)
    {
    }

    template <typename It2>
    SetIterator(const SetIterator<T, Cmp, It2> &other)
        : mIterator(static_cast<const It2 &>(other))
    {
    }

    T &operator*() const
    {
        return const_cast<T &>(*mIterator);
    }

    T *operator->() const
    {
        return &const_cast<T &>(*mIterator);
    }

    bool operator!=(const SetIterator<T, Cmp, It> &other) const
    {
        return mIterator != other.mIterator;
    }

    bool operator==(const SetIterator<T, Cmp, It> &other) const
    {
        return mIterator == other.mIterator;
    }

    SetIterator<T, Cmp, It> &
    operator++()
    {
        ++mIterator;
        return *this;
    }

    SetIterator<T, Cmp, It> &operator--()
    {
        --mIterator;
        return *this;
    }

    operator const It &() const
    {
        return mIterator;
    }

private:
    friend class SetConstIterator<T, Cmp, It>;

    It mIterator;
};

template <class T, typename Cmp, typename It>
class SetConstIterator {
public:
    using iterator_category = typename It::iterator_category;
    using value_type = typename It::value_type;
    using difference_type = typename It::difference_type;
    using pointer = typename It::pointer;
    using reference = typename It::reference;

    SetConstIterator(It &&it)
        : mIterator(std::forward<It>(it))
    {
    }

    SetConstIterator(const It &it)
        : mIterator(it)
    {
    }

    SetConstIterator(const SetIterator<T, Cmp, It> &it)
        : mIterator(it.mIterator)
    {
    }

    const T &operator*() const
    {
        return *mIterator;
    }

    const T *operator->() const
    {
        return &*mIterator;
    }

    bool operator!=(const SetConstIterator<T, Cmp, It> &other) const
    {
        return mIterator != other.mIterator;
    }

    bool operator==(const SetConstIterator<T, Cmp, It> &other) const
    {
        return mIterator == other.mIterator;
    }

    void operator++()
    {
        ++mIterator;
    }

    void operator--()
    {
        --mIterator;
    }

    operator const It &() const
    {
        return mIterator;
    }

private:
    friend class SetIterator<T, Cmp, It>;

    It mIterator;
};

template <typename T, typename Cmp>
struct cmp_type {
    using type = typename Cmp::cmp_type;
};

template <typename T, typename V>
struct cmp_type<T, std::less<V>> {
    using type = T;
};

template <typename T, typename Cmp>
using cmp_type_t = typename cmp_type<T, Cmp>::type;

template <typename T, typename Cmp>
struct container_traits<std::set<T, Cmp>> {
    static constexpr const bool sorted = true;
    static constexpr const bool has_dependent_handle = false;
    static constexpr const bool remove_invalidates_handles = false;

    typedef std::set<T, Cmp> container;
    typedef SetIterator<T, Cmp, typename std::set<T, Cmp>::iterator> iterator;
    typedef SetConstIterator<T, Cmp, typename std::set<T, Cmp>::const_iterator> const_iterator;
    typedef SetIterator<T, Cmp, typename std::set<T, Cmp>::reverse_iterator> reverse_iterator;
    typedef SetConstIterator<T, Cmp, typename std::set<T, Cmp>::const_reverse_iterator> const_reverse_iterator;
    typedef T *handle;
    typedef const T *const_handle;
    typedef iterator position_handle;
    typedef const_iterator const_position_handle;
    typedef cmp_type_t<T, Cmp> key_type;
    typedef typename container::value_type value_type;
    typedef T type;

    template <template <typename> typename M>
    using rebind = container_traits<std::set<M<T>, Cmp>>;

    template <typename C>
    struct api : SortedContainerApi<C> {
        using Base = SortedContainerApi<C>;

        using Base::Base;

        template <class... _Ty>
        std::pair<iterator, bool> emplace(_Ty &&... args)
        {
            return this->Base::emplace(this->end(), std::forward<_Ty>(args)...);
        }
    };

    template <class... _Ty>
    static std::pair<iterator, bool> emplace(container &c, const const_iterator &where, _Ty &&... args)
    {
        return c.emplace(std::forward<_Ty>(args)...);
    }

    static position_handle toPositionHandle(container &c, const iterator &it)
    {
        return it;
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const container &c, const const_iterator &it)
    {
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const container &c, const const_iterator &it, size_t count = 1)
    {
    }

    static iterator toIterator(container &c, const position_handle &handle)
    {
        return handle;
    }

    static const_iterator toIterator(const container &c, const const_position_handle &handle)
    {
        return handle;
    }

    static position_handle next(const position_handle &handle)
    {
        return std::next(handle);
    }
};

template <typename K, typename T>
struct container_traits<std::map<K, T>> {
    static constexpr const bool sorted = true;
    static constexpr const bool has_dependent_handle = false;
    static constexpr const bool remove_invalidates_handles = false;

    typedef std::map<K, T> container;
    typedef typename container::iterator iterator;
    typedef typename container::const_iterator const_iterator;
    typedef typename container::reverse_iterator reverse_iterator;
    typedef typename container::const_reverse_iterator const_reverse_iterator;
    typedef std::pair<const K, T> *handle;
    typedef const std::pair<const K, T> *const_handle;
    typedef iterator position_handle;
    typedef const_iterator const_position_handle;
    typedef K key_type;
    typedef T value_type;
    typedef std::pair<const K, T> type;

    template <template <typename> typename M>
    using rebind = container_traits<std::map<M<K>, M<T>>>;

    template <typename C>
    struct api : C {
        using C::C;

        T &operator[](const K &key)
        {
            iterator it = C::lower_bound(key);
            if (it == this->end() || it->first != key) {
                auto pib = try_emplace(key);
                assert(pib.second);
                it = pib.first;
            }
            return it->second;
        }

        const T &at(const std::string &key) const
        {
            return C::at(key);
        }

        template <class... _Ty>
        std::pair<iterator, bool> try_emplace(const K &key, _Ty &&... args)
        {
            auto it = C::lower_bound(key);
            if (it != this->end() && it->first == key) {
                return { it, false };
            }
            return C::emplace(it, std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...));
        }
    };

    template <class... _Ty>
    static std::pair<iterator, bool> emplace(container &c, const const_iterator &where, _Ty &&... args)
    {
        return c.emplace(std::forward<_Ty>(args)...);
    }

    static position_handle toPositionHandle(container &c, const iterator &it)
    {
        return it;
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const container &c, const const_iterator &it)
    {
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const container &c, const const_iterator &it, size_t count = 1)
    {
    }

    static iterator toIterator(container &c, const position_handle &handle)
    {
        return handle;
    }

    static const_iterator toIterator(const container &c, const const_position_handle &handle)
    {
        return handle;
    }

    static position_handle next(const position_handle &handle)
    {
        return std::next(handle);
    }
};

}
