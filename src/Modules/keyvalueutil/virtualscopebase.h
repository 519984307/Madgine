#pragma once

#include "../keyvalue/typedscopeptr.h"

#include "../reflection/decay.h"

#include "../keyvalue/scopebase.h"

namespace Engine {

struct MODULES_EXPORT VirtualScopeBase : ScopeBase {
    virtual ~VirtualScopeBase() = default;
    virtual TypedScopePtr customScopePtr() = 0;
};

template <typename T, typename _Base = VirtualScopeBase>
struct VirtualScope : _Base {
    using _Base::_Base;
    virtual TypedScopePtr customScopePtr() override {
        return { this, table<decayed_t<T>> };
    }
};

}