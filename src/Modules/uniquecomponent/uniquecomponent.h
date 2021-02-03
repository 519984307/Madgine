#pragma once

#if ENABLE_PLUGINS

#    include "indexholder.h"

namespace Engine {

template <typename T, typename Base, typename _VBase>
struct VirtualUniqueComponentImpl : Base {
    using VBase = _VBase;
    using Base::Base;

    struct Inner {
        Inner()
        {
            assert(!Base::_preg());
            Base::_preg() = &reg;
        }
        ~Inner() 
        {
            assert(Base::_preg() == &reg);
            Base::_preg() = nullptr;
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
        static IndexHolder *dummy = nullptr;
        //LOG("Accessing registry for " << typeid(T).name() << " at: " << &dummy << "(= " << dummy << ")");
        return dummy;
    }
};

DLL_IMPORT_VARIABLE2(typename T::Collector::template ComponentRegistrator<T>, _reg, typename T);

#    define UNIQUECOMPONENT(Name) DLL_EXPORT_VARIABLE2(, Name::Collector::ComponentRegistrator<Name>, Engine::, _reg, {}, Name)
#    define UNIQUECOMPONENT2(Name, ext) DLL_EXPORT_VARIABLE3(, Name::Collector::ComponentRegistrator<Name>, Engine::, _reg, ext, {}, Name)

template <typename T, typename _Collector, typename _Base>
struct UniqueComponent : _Base {
    using Collector = _Collector;

    using _Base::_Base;

    static size_t component_index()
    {
        return _reg<T>().index();
    }
};

template <typename T>
size_t component_index()
{
    return T::component_index();
}

}

#else

namespace Engine {
template <typename T, typename Base, typename _VBase>
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

}

#    define UNIQUECOMPONENT(Name)
#    define UNIQUECOMPONENT2(Name, ext)
#    define VIRTUALUNIQUECOMPONENT(Name)

#endif

namespace Engine {

DLL_IMPORT_VARIABLE2(const std::string_view, _componentName, typename T);

template <typename T, typename Base>
struct NamedComponent : Base {
    using Base::Base;

    static const std::string_view &componentName()
    {
        return _componentName<T>();
    }
};

}

#define COMPONENT_NAME(Name, FullType) \
    DLL_EXPORT_VARIABLE2(constexpr, const std::string_view, Engine::, _componentName, #Name, FullType);

#define NAMED_UNIQUECOMPONENT(Name, FullType) \
    COMPONENT_NAME(Name, FullType)            \
    UNIQUECOMPONENT(FullType)
