#pragma once

#include "../generic/linestruct.h"
#include "../generic/virtualiterator.h"
#include "accessor.h"
#include "apifunction.h"
#include "functiontable_impl.h"
#include "keyvalueiterator.h"
#include "metatable.h"
#include "typedscopeptr.h"
#include "valuetype_forward.h"

namespace Engine {

struct MetaTableTag;

template <size_t Line>
using MetaTableLineStruct = LineStruct<MetaTableTag, Line>;

template <typename Scope, auto Getter, auto Setter>
constexpr Accessor property()
{
    using getter_traits = CallableTraits<decltype(Getter)>;
    using GetterScope = typename getter_traits::class_type;
    using T = typename getter_traits::return_type;

    void (*setter)(TypedScopePtr, const ValueType &) = nullptr;

    if constexpr (Setter != nullptr) {
        using setter_traits = CallableTraits<decltype(Setter)>;
        using SetterScope = typename setter_traits::class_type;

        //TODO remove const in tuple types
        //static_assert(std::is_same_v<typename setter_traits::argument_types, std::tuple<T>>);

        setter = [](TypedScopePtr scope, const ValueType &v) {
            if constexpr (std::is_same_v<SetterScope, void>) {
                using SetterScope = std::remove_pointer_t<std::tuple_element_t<0, typename setter_traits::argument_types>>;
                if constexpr (std::is_convertible_v<Scope &, SetterScope &>) {
                    TupleUnpacker::invoke(Setter, scope.safe_cast<Scope>(), ValueType_as<std::decay_t<T>>(v));
                } else {
                    TupleUnpacker::invoke(Setter, scope, ValueType_as<std::decay_t<T>>(v));
                }
            } else {
                static_assert(std::is_convertible_v<Scope &, SetterScope &>);
                TupleUnpacker::invoke(Setter, scope.safe_cast<Scope>(), ValueType_as<std::decay_t<T>>(v));
            }
        };
    }

    return {
        [](ValueType &retVal, TypedScopePtr scope) {
            T value = [=]() -> T {
                if constexpr (std::is_same_v<GetterScope, void>) {
                    using GetterScope = std::remove_pointer_t<std::tuple_element_t<0, typename getter_traits::argument_types>>;
                    if constexpr (std::is_convertible_v<Scope &, GetterScope &>) {
                        return TupleUnpacker::invoke(Getter, scope.safe_cast<Scope>());
                    } else {
                        return TupleUnpacker::invoke(Getter, scope);
                    }
                } else {
                    static_assert(std::is_convertible_v<Scope &, GetterScope &>);
                    return TupleUnpacker::invoke(Getter, scope.safe_cast<Scope>());
                }
            }();

            to_ValueType(retVal, std::forward<T>(value));
        },
        setter
    };
}

template <auto P, typename Scope, typename T>
void setField(Scope *s, const T &t)
{
    s->*P = t;
}

template <auto P, typename Scope, typename T>
T *getFieldPtr(Scope *s)
{
    return &(s->*P);
}

template <typename Scope, auto P>
constexpr Accessor member()
{
    using traits = CallableTraits<decltype(P)>;
    using DerivedScope = typename traits::class_type;
    using T = typename traits::return_type;

    if constexpr (std::is_reference_v<T> && std::is_convertible_v<T, ScopeBase &>) {
        return property<Scope, &getFieldPtr<P, DerivedScope, std::remove_reference_t<T>>, nullptr>();
    } else if constexpr (std::is_const_v<DerivedScope> || !std::is_assignable_v<T &, const T &> || (is_iterable_v<T> && !std::is_same_v<std::string, T>))
        return property<Scope, P, nullptr>();
    else
        return property<Scope, P, &setField<P, DerivedScope, std::remove_reference_t<T>>>();
}

template <auto F>
static constexpr ApiFunction method(TypedScopePtr scope)
{
    return { &function<F>() };
}

/*template <typename T, size_t... Is>
static constexpr std::array<std::pair<const char *, ::Engine::Accessor>, std::tuple_size_v<T> + 1> structMembers(std::index_sequence<Is...>)
{
    return {

        { nullptr, nullptr }
    };
}*/
}

#define METATABLE_BEGIN(T) _METATABLE_BEGIN_IMPL(T, nullptr)

#define METATABLE_BEGIN_BASE(T, Base) _METATABLE_BEGIN_IMPL(T, &table<Base>)

#define _METATABLE_BEGIN_IMPL(T, BasePtr)                                    \
    namespace Engine {                                                       \
        template <>                                                          \
        struct LineStruct<MetaTableTag, __LINE__> {                          \
            using Ty = T;                                                    \
            static constexpr const MetaTable **baseClass = BasePtr;          \
            static constexpr const bool base = true;                         \
            constexpr const std::pair<const char *, Accessor> *data() const; \
        };                                                                   \
    }

#define METATABLE_ENTRY(Name, ...)                                                      \
    namespace Engine {                                                                  \
        template <>                                                                     \
        struct LineStruct<MetaTableTag, __LINE__> : MetaTableLineStruct<__LINE__ - 1> { \
            constexpr const std::pair<const char *, Accessor> *data() const             \
            {                                                                           \
                if constexpr (MetaTableLineStruct<__LINE__ - 1>::base)                  \
                    return &mData;                                                      \
                else                                                                    \
                    return MetaTableLineStruct<__LINE__ - 1>::data();                   \
            }                                                                           \
            static constexpr const bool base = false;                                   \
            std::pair<const char *, Accessor> mData = { Name, __VA_ARGS__ };            \
        };                                                                              \
    }

#define METATABLE_END(T)                                                        \
    METATABLE_ENTRY(nullptr, { nullptr, nullptr })                              \
    namespace Meta_##T                                                          \
    {                                                                           \
        static constexpr ::Engine::MetaTableLineStruct<__LINE__> sMembers = {}; \
    }                                                                           \
    DLL_EXPORT_VARIABLE(constexpr, const ::Engine::MetaTable, , table, SINGLE_ARG({ &::table<T>, #T, ::Engine::MetaTableLineStruct<__LINE__>::baseClass, Meta_##T::sMembers.data() }), T);

/*#define STRUCT_METATABLE(T)                                                                                              \
    namespace {                                                                                                          \
        namespace Meta_##T                                                                                               \
        {                                                                                                                \
            constexpr const auto members = ::Engine::structMembers<T>(std::make_index_sequence<std::tuple_size_v<T>>()); \
        }                                                                                                                \
    }                                                                                                                    \
    DLL_EXPORT_VARIABLE2(constexpr, const ::Engine::MetaTable, ::, table, SINGLE_ARG3({ #T, nullptr, Meta_##T::members.data() }), T);
*/

#define MEMBER(M) \
    METATABLE_ENTRY(#M, ::Engine::member<Ty, &Ty::M>())

#define READONLY_PROPERTY(Name, Getter) \
    METATABLE_ENTRY(#Name, ::Engine::property<Ty, &Ty::Getter, nullptr>())

#define PROPERTY(Name, Getter, Setter) \
    METATABLE_ENTRY(#Name, ::Engine::property<Ty, &Ty::Getter, &Ty::Setter>())

#define FUNCTION(/*F, */...)                                                    \
    FUNCTIONTABLE(::Engine::MetaTableLineStruct<__LINE__ - 1>::Ty::__VA_ARGS__) \
    METATABLE_ENTRY(STRINGIFY2(FIRST(__VA_ARGS__)), ::Engine::property<Ty, &::Engine::method<&Ty::FIRST(__VA_ARGS__)>, nullptr>())
