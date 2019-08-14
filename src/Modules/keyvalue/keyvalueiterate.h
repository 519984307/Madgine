#pragma once

#include "keyvalue.h"
#include "../generic/virtualiterator.h"

namespace Engine {

using KeyValueVirtualIterator = VirtualIterator<std::pair<ValueType, ValueType>>;

template <typename It>
struct KeyValueIterator {

    KeyValueIterator(It &&it)
        : mIt(std::move(it))
    {
    }

    bool operator==(const KeyValueIterator<It> &other) const
    {
        return mIt == other.mIt;
    }

    void operator++()
    {
        ++mIt;
    }

    auto operator*() const
    {
        decltype(auto) k = kvKey(*mIt);
        decltype(auto) v = kvValue(*mIt);
        return std::pair<decltype(k), decltype(v)>(k,v);
    }

private:
    It mIt;
};

}