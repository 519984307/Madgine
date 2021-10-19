#pragma once

#include "type_pack.h"

namespace Engine {

template <typename F, typename Pack, template <typename Builder> typename Facade>
struct BuilderImpl {

    using Tuple = type_pack_as_tuple_t<type_pack_apply_t<type_pack_as_tuple_t, Pack>>;
    using return_type = decltype(TupleUnpacker::invokeFromTuple(std::declval<F &&>(), std::declval<Tuple &&>()));

    BuilderImpl(F &&f)
        : mF(std::move(f))
    {
    }

    BuilderImpl(F f, Tuple data)
        : mF(std::move(f))
        , mData(std::move(data))
    {
    }

    BuilderImpl(const BuilderImpl<F, Pack, Facade> &) = delete;
    BuilderImpl(BuilderImpl<F, Pack, Facade> &&other) = default;

    ~BuilderImpl()
    {
        if (mF) {
            execute();
        }
    }

    decltype(auto) execute()
    {
        F f = std::move(*mF);
        mF.reset();
        return TupleUnpacker::invokeFromTuple(std::move(f), std::move(mData));
    }

    operator return_type()
    {
        return execute();
    }

    template <typename T>
    requires std::convertible_to<return_type, T>
    operator T()
    {
        return execute();
    }

    template <typename G>
    auto then(G &&g) &&
    {
        auto modified_f = [f { std::move(mF) }, g { std::forward<G>(g) }](auto &&...args) mutable -> decltype(auto) {
            return Engine::then(std::move(f)(std::forward<decltype(args)>(args)...), std::move(g));
        };
        return Facade<BuilderImpl<decltype(modified_f), Pack, Facade>> {
            std::move(modified_f),
            std::move(mData)
        };
    }

protected:
    template <size_t Dim, typename T>
    auto append(T &&t) &&
    {
        static_assert(Dim < type_pack_size_v<Pack>);
        return append_impl<Dim, T>(std::forward<T>(t), type_pack_indices_t<Pack> {});
    }

private:
    template <typename T>
    struct appender {
        template <typename Pack2>
        using type = type_pack_append_t<Pack2, T>;
    };

    template <size_t Dim, typename T, size_t... Is>
    Facade<BuilderImpl<F, type_pack_apply_to_nth_t<appender<T>::template type, Pack, Dim>, Facade>> append_impl(T &&t, std::index_sequence<Is...>)
    {
        F f = std::move(*mF);
        mF.reset();
        return { std::move(f),
            std::forward_as_tuple(
                [&]() {
                    if constexpr (Is == Dim)
                        return std::tuple_cat(std::get<Is>(std::move(mData)), std::forward_as_tuple(std::forward<T>(t)));
                    else
                        return std::get<Is>(std::move(mData));
                }()...) };
    }

private:
    std::optional<F> mF;
    Tuple mData;
};

template <typename F, template <typename Builder> typename Facade, size_t Dim = 1>
using Builder = BuilderImpl<F, type_pack_repeat_n_times<type_pack<>, Dim>, Facade>;

}