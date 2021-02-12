#pragma once

#include "Generic/builder.h"

namespace Engine {
namespace Serialize {

    template <typename Builder>
    struct RequestBuilder : Builder{        

        using Builder::Builder;

        template <typename C>
        auto onResult(C &&c) &&
        {
            return std::move(*this).template append<0>(std::forward<C>(c));
        }
        
        template <typename C>
        auto onSuccess(C &&c) &&
        {
            return std::move(*this).template append<1>(std::forward<C>(c));
        }

        template <typename C>
        auto onFailure(C &&c) &&
        {
            return std::move(*this).template append<2>(std::forward<C>(c));
        }
    };

    template <typename F>
    RequestBuilder(F &&f)->RequestBuilder<Builder<F, RequestBuilder, 3>>;

}
}