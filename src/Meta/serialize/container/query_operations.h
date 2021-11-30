#pragma once

#include "query.h"
#include "../streams/pendingrequest.h"
#include "../streams/bufferedstream.h"
#include "../unithelper.h"

namespace Engine {
namespace Serialize {

   template <auto f, typename OffsetPtr, typename R, typename T, typename... _Ty, typename... Configs>
    struct Operations<__query__impl__::QueryImpl<f, OffsetPtr, R, T, _Ty...>, Configs...> {

        template <typename... Args>
        static void writeAction(const Query<f, OffsetPtr> &query, const std::set<BufferedOutStream *, CompareStreamId> &outStreams, const void *data, Args &&... args)
        {
            assert(outStreams.size() == 1);
            for (BufferedOutStream *out : outStreams) {
                write(*out, *static_cast<const R *>(data), nullptr, args...);
                out->endMessage();
            }
        }

        template <typename... Args>
        static StreamResult readAction(Query<f, OffsetPtr> &query, SerializeInStream &in, PendingRequest *request, Args &&... args)
        {
            R returnValue;
            STREAM_PROPAGATE_ERROR(read(in, returnValue, nullptr, args...));
            STREAM_PROPAGATE_ERROR(UnitHelper<R>::applyMap(in, returnValue, true));
            assert(request);
            query.writeRequestResponse(&returnValue, request->mRequester, request->mRequesterTransactionId);
            (*request)(&returnValue);
            return {};
        }

        template <typename... Args>
        static void writeRequest(const Query<f, OffsetPtr> &query, BufferedOutStream &out, const void *data, Args &&... args)
        {
            TupleUnpacker::forEach(*static_cast<const std::tuple<_Ty...> *>(data), [&](auto &field) { write(out, field, nullptr, args...); });
            out.endMessage();
        }

        template <typename... Args>
        static StreamResult readRequest(Query<f, OffsetPtr> &query, BufferedInOutStream &in, TransactionId id, Args &&... args)
        {
            std::tuple<std::remove_const_t<std::remove_reference_t<_Ty>>...> data;
            STREAM_PROPAGATE_ERROR(TupleUnpacker::accumulate(
                data, [&](auto &field, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return read(in, field, nullptr, args...);
                },
                StreamResult {}));
            STREAM_PROPAGATE_ERROR(UnitHelper<decltype(data)>::applyMap(in, data, true));
            query.tryCall(data, in.id(), id);
            return {};
        }
    };


}
}