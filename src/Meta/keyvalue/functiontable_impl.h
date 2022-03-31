#pragma once

#include "Generic/linestruct.h"
#include "valuetype_forward.h"

#include "functionargument.h"
#include "functiontable.h"

namespace Engine {

struct MetaFunctionTag;

template <typename R, typename... Args, size_t... Is>
static constexpr std::array<FunctionArgument, sizeof...(Args)> metafunctionArgsHelper(std::string_view args, std::index_sequence<Is...>)
{
    std::array<std::string_view, sizeof...(Args)> argumentNames = StringUtil::tokenize<sizeof...(Args)>(args, ',');
    return { { { toValueTypeDesc<std::remove_const_t<std::remove_reference_t<Args>>>(), argumentNames[Is] }... } };
}

template <typename R, typename T, typename... Args, size_t... Is>
static constexpr std::array<FunctionArgument, sizeof...(Args) + 1> metafunctionArgsMemberHelper(std::string_view args, std::index_sequence<Is...>)
{
    std::array<std::string_view, sizeof...(Args)> argumentNames = StringUtil::tokenize<sizeof...(Args)>(args, ',');
    return { { { toValueTypeDesc<T *>(), "this" }, { toValueTypeDesc<std::remove_const_t<std::remove_reference_t<Args>>>(), argumentNames[Is] }... } };
}

template <typename R, typename... Args>
static constexpr std::array<FunctionArgument, sizeof...(Args)> metafunctionArgs(R (*f)(Args...), std::string_view args)
{
    return metafunctionArgsHelper<R, Args...>(args, std::index_sequence_for<Args...>());
}

template <typename R, typename T, typename... Args>
static constexpr std::array<FunctionArgument, sizeof...(Args) + 1> metafunctionArgs(R (T::*f)(Args...), std::string_view args)
{
    return metafunctionArgsMemberHelper<R, T, Args...>(args, std::index_sequence_for<Args...>());
}

template <typename R, typename T, typename... Args>
static constexpr std::array<FunctionArgument, sizeof...(Args) + 1> metafunctionArgs(R (T::*f)(Args...) const, std::string_view args)
{
    return metafunctionArgsMemberHelper<R, T, Args...>(args, std::index_sequence_for<Args...>());
}

template <typename T, typename... Args>
static constexpr std::array<FunctionArgument, sizeof...(Args) + 1> metafunctionArgs(void (T::*f)(ValueType &, Args...), std::string_view args)
{
    return metafunctionArgsMemberHelper<ValueType, T, Args...>(args, std::index_sequence_for<Args...>());
}

template <auto F, typename R, typename T, typename... Args, size_t... I>
static void unpackMemberHelper(const FunctionTable *table, ValueType &retVal, const ArgumentList &args, std::index_sequence<I...>)
{
    T *t = ValueType_as<TypedScopePtr>(getArgument(args, 0)).safe_cast<T>();
    to_ValueType<true>(retVal, invoke_patch_void<std::monostate>(F, t, ValueType_as<std::remove_cv_t<std::remove_reference_t<Args>>>(getArgument(args, I + 1))...));
}

template <auto F, typename R, typename... Args, size_t... I>
static void unpackApiHelper(const FunctionTable *table, ValueType &retVal, const ArgumentList &args, std::index_sequence<I...>)
{
    to_ValueType<true>(retVal, invoke_patch_void<std::monostate>(F, ValueType_as<std::remove_cv_t<std::remove_reference_t<Args>>>(getArgument(args, I))...));
}

template <auto F, typename T, typename... Args, size_t... I>
static void unpackReturnHelper(const FunctionTable *table, ValueType &retVal, const ArgumentList &args, std::index_sequence<I...>)
{
    T *t = ValueType_as<TypedScopePtr>(getArgument(args, 0)).safe_cast<T>();
    (t->*F)(retVal, ValueType_as<std::remove_cv_t<std::remove_reference_t<Args>>>(getArgument(args, I + 1))...);
}

template <auto F, typename R, typename T, typename... Args>
static void unpackMemberApiMethod(const FunctionTable *table, ValueType &retVal, const ArgumentList &args)
{
    unpackMemberHelper<F, R, T, Args...>(table, retVal, args, std::index_sequence_for<Args...>());
}

template <auto F, typename R, typename... Args>
static void unpackApiMethod(const FunctionTable *table, ValueType &retVal, const ArgumentList &args)
{
    unpackApiHelper<F, R, Args...>(table, retVal, args, std::index_sequence_for<Args...>());
}

template <auto F, typename T, typename... Args>
static void unpackReturnMethod(const FunctionTable *table, ValueType &retVal, const ArgumentList &args)
{
    unpackReturnHelper<F, T, Args...>(table, retVal, args, std::index_sequence_for<Args...>());
}

template <auto F, typename R, typename... Args>
static constexpr typename FunctionTable::FPtr wrapHelper(R (*f)(Args...))
{
    return &unpackApiMethod<F, R, Args...>;
}

template <auto F, typename R, typename T, typename... Args>
static constexpr typename FunctionTable::FPtr wrapHelper(R (T::*f)(Args...))
{
    return &unpackMemberApiMethod<F, R, T, Args...>;
}

template <auto F, typename R, typename T, typename... Args>
static constexpr typename FunctionTable::FPtr wrapHelper(R (T::*f)(Args...) const)
{
    return &unpackMemberApiMethod<F, R, T, Args...>;
}

template <auto F, typename T, typename... Args>
static constexpr typename FunctionTable::FPtr wrapHelper(void (T::*f)(ValueType &, Args...))
{
    return &unpackReturnMethod<F, T, Args...>;
}
}

#define FUNCTIONTABLE_IMPL(F, NameInit, Name, Tag, ...)                                                                                                                                                                                                                                                                                                           \
    namespace Engine {                                                                                                                                                                                                                                                                                                                                            \
        template <>                                                                                                                                                                                                                                                                                                                                               \
        struct LineStruct<Tag, __LINE__> {                                                                                                                                                                                                                                                                                                                        \
            static constexpr const auto args = metafunctionArgs(&F, #__VA_ARGS__);                                                                                                                                                                                                                                                                                \
            using return_type = typename ::Engine::CallableTraits<decltype(&F)>::return_type;                                                                                                                                                                                                                                                                     \
            static constexpr const ExtendedValueTypeDesc returnType = toValueTypeDesc<patch_void_t<return_type, std::monostate>>();                                                                                                                                                                                      \
            NameInit                                                                                                                                                                                                                                                                                                                                              \
        };                                                                                                                                                                                                                                                                                                                                                        \
    }                                                                                                                                                                                                                                                                                                                                                             \
    DLL_EXPORT_VARIABLE2(constexpr, const ::Engine::FunctionTable, ::, function, SINGLE_ARG({ ::Engine::wrapHelper<&F>(&F), Name, ::Engine::CallableTraits<decltype(&F)>::argument_count, ::Engine::CallableTraits<decltype(&F)>::is_member_function, ::Engine::LineStruct<Tag, __LINE__>::args.data(), &::Engine::LineStruct<Tag, __LINE__>::returnType }), &F); \
    namespace Engine {                                                                                                                                                                                                                                                                                                                                            \
        static ::Engine::FunctionTableRegistrator<&F> CONCAT2(__reg_, __LINE__);                                                                                                                                                                                                                                                                                  \
    }

#define FUNCTIONTABLE(F, ...) FUNCTIONTABLE_IMPL(F, , #F, ::Engine::MetaFunctionTag, __VA_ARGS__)
#define FUNCTIONTABLE_EX(Name, Tag, F, ...) FUNCTIONTABLE_IMPL(F, static constexpr fixed_string name = Name;, SINGLE_ARG(::Engine::LineStruct<Tag, __LINE__>::name), Tag, __VA_ARGS__)