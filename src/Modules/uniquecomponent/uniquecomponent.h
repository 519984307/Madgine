#pragma once

#if ENABLE_PLUGINS

#    include "indexholder.h"

namespace Engine {

template <typename T, typename Base, typename _VBase = Base>
struct VirtualUniqueComponentImpl : Base {
    using VBase = _VBase;
    using Base::Base;

    struct Inner {
        Inner()
        {
            Base::_preg() = &reg;
        }

    private:
        typename Base::Collector::template ComponentRegistrator<T> reg;
    };
};

DLL_IMPORT_VARIABLE2(typename T::Inner, _vreg, typename T, typename Base);

#    define VIRTUALUNIQUECOMPONENT(Name) DLL_EXPORT_VARIABLE2(, Name::Inner, Engine::, _vreg, {}, Name, Name::VBase)

template <typename T, typename _Collector, typename Base>
struct VirtualUniqueComponentBase : public Base {
public:
    using Collector = _Collector;

    using Base::Base;

    static size_t component_index()
    {
        return _preg()->index();
    }

protected:
    static IndexHolder *&_preg()
    {
        static IndexHolder *dummy;
        return dummy;
    }
};

DLL_IMPORT_VARIABLE2(typename Collector::template ComponentRegistrator<T>, _reg, typename T, typename Collector);

#    define UNIQUECOMPONENT(Name) DLL_EXPORT_VARIABLE2(, Name::Collector::ComponentRegistrator<Name>, Engine::, _reg, {}, Name, Name::Collector)

template <typename T, typename _Collector, typename _Base>
struct UniqueComponent : _Base {
    using Collector = _Collector;

    using _Base::_Base;

    static size_t component_index()
    {
        return _reg<T, Collector>().index();
    }
};

template <typename T>
size_t component_index()
{
    return T::component_index();
}

struct IndexRef {
    IndexHolder *mHolder;

    operator size_t() const
    {
        return mHolder->index();
    }

    bool isValid() const
    {
        return mHolder->isValid();
    }
};

template <typename T>
IndexRef indexRef()
{
    return { &_reg<T, typename T::Collector>() };
}

template <typename T>
IndexRef virtualIndexRef()
{
    return { T::_preg() };
}

}

#else

namespace Engine {
template <typename T, typename Base, typename _VBase = Base>
struct VirtualUniqueComponentImpl : Base {
    using Base::Base;
};

template <typename T, typename Collector, typename Base>
struct VirtualUniqueComponentBase : Base {
    using Base::Base;
};

template <typename T, typename Collector, typename Base>
struct UniqueComponent : Base {
    using Base::Base;
};

template <typename T>
size_t component_index();

struct IndexRef {
    size_t mIndex;

    operator size_t() const
    {
        return mIndex;
    }

    bool isValid() const
    {
        return true;
    }
};

template <typename T>
IndexRef indexRef()
{
    return { component_index<T>() };
}

template <typename T>
IndexRef virtualIndexRef()
{
    return { component_index<T>() };
}

}


#    define UNIQUECOMPONENT(Name)
#    define VIRTUALUNIQUECOMPONENT(Name)

#endif