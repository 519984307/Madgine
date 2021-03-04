#pragma once

#include "../configs/configselector.h"
#include "../configs/creationconfig.h"
#include "../configs/requestpolicy.h"
#include "../container/physical.h"
#include "../formatter.h"
#include "../primitivetypes.h"
#include "../serializableunitptr.h"
#include "Generic/callerhierarchy.h"
#include "Generic/container/atomiccontaineroperation.h"
#include "Generic/container/containerevent.h"
#include "bufferedstream.h"
#include "comparestreamid.h"
#include "pendingrequest.h"

namespace Engine {
namespace Serialize {

    DERIVE_TYPENAME(Config);
    DERIVE_FUNCTION(writeState, SerializeOutStream &, const char *)
    DERIVE_FUNCTION2(writeState2, writeState, SerializeOutStream &, const char *, CallerHierarchyBasePtr)
    DERIVE_FUNCTION(readState, SerializeInStream &, const char *)
    DERIVE_FUNCTION2(readState2, readState, SerializeInStream &, const char *, CallerHierarchyBasePtr)
    /*DERIVE_FUNCTION(readAction, SerializeInStream &, PendingRequest*)
    DERIVE_FUNCTION(readRequest, BufferedInOutStream &, TransactionId)
    DERIVE_FUNCTION(writeAction, int, const void *, ParticipantId, TransactionId)
    DERIVE_FUNCTION(writeRequest, int, const void *, ParticipantId, TransactionId, std::function<void(void*)>)*/

    template <typename T, typename... Configs, typename Hierarchy = std::monostate>
    void read(SerializeInStream &in, T &t, const char *name, const Hierarchy &hierarchy = {}, StateTransmissionFlags flags = 0);

    template <typename T, typename... Configs, typename Hierarchy = std::monostate>
    void write(SerializeOutStream &out, const T &t, const char *name, const Hierarchy &hierarchy = {}, StateTransmissionFlags flags = 0);

    template <typename C, typename... Configs>
    struct ContainerOperations {

        using Creator = ConfigSelectorDefault<CreatorCategory, DefaultCreator<typename C::value_type>, Configs...>;
        using RequestPolicy = ConfigSelectorDefault<RequestPolicyCategory, RequestPolicy::all_requests, Configs...>;

        template <typename Op, typename Hierarchy = std::monostate>
        static void readOp(SerializeInStream &in, Op &op, const char *name, const Hierarchy &hierarchy = {})
        {
            using T = typename C::value_type;

            in.format().beginExtended(in, name, 1);
            uint32_t size;
            Serialize::read(in, size, "size");
            in.format().beginCompound(in, name);

            if constexpr (!container_traits<C>::is_fixed_size) {
                TupleUnpacker::invoke(&Creator::template clear<Op>, op, size, hierarchy);

                while (size--) {
                    TupleUnpacker::invoke(&Creator::template readItem<Op>, in, op, physical(op).end(), hierarchy);
                }
            } else {
                assert(op.size() == size);
                for (T &t : physical(op)) {
                    Serialize::read(in, t, "Item");
                }
            }

            in.format().endCompound(in, name);
        }

        template <typename... Args>
        static void read(SerializeInStream &in, C &container, const char *name, Args &&... args)
        {
            decltype(auto) op = resetOperation(container);
            readOp(in, op, name, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void write(SerializeOutStream &out, const C &container, const char *name, Args &&... args)
        {
            out.format().beginExtended(out, name, 1);
            Serialize::write<uint32_t>(out, container.size(), "size");
            out.format().beginCompound(out, name);
            for (const auto &t : container) {
                TupleUnpacker::invoke(&Creator::writeItem, out, t, std::forward<Args>(args)...);
            }
            out.format().endCompound(out, name);
        }

        static typename C::iterator readIterator(SerializeInStream &in, C &c)
        {
            int32_t dist;
            Serialize::read(in, dist, "it");
            return std::next(c.begin(), dist);
        }

        static void writeIterator(SerializeOutStream &out, const C &c, const typename C::const_iterator &it)
        {
            Serialize::write<int32_t>(out, std::distance(c.begin(), it), "it");
        }

        template <typename... Args>
        static typename container_traits<C>::emplace_return performOperation(C &c, ContainerEvent op, SerializeInStream &in, ParticipantId answerTarget, TransactionId answerId, Args &&... args)
        {
            typename container_traits<C>::emplace_return it = c.end();
            switch (op) {
            case EMPLACE: {
                if constexpr (!container_traits<C>::sorted) {
                    it = readIterator(in, c);
                }
                decltype(auto) op = insertOperation(c, it, answerTarget, answerId);
                typename container_traits<C>::const_iterator cit = static_cast<const typename container_traits<C>::iterator &>(it);
                it = TupleUnpacker::invoke(&Creator::template readItem<InsertOperation_t<C>>, in, op, cit, std::forward<Args>(args)...);
                break;
            }
            case ERASE: {
                it = readIterator(in, c);
                it = removeOperation(c, it, answerTarget, answerId).erase(it);
                break;
            }
                /*case REMOVE_RANGE: {
                iterator from = this->read_iterator(in);
                iterator to = this->read_iterator(in);
                it = erase_impl(from, to, answerTarget, answerId);
                break;
            }
            case RESET:
                //readState_impl(in, nullptr, answerTarget, answerId);
                throw "TODO";
                break;*/
            default:
                throw 0;
            }
            return it;
        }

        //TODO: Maybe move loop out of this function
        template <typename... Args>
        static void writeAction(const C &c, const std::set<BufferedOutStream *, CompareStreamId> &outStreams, const void *_data, Args &&... args)
        {
            const std::tuple<ContainerEvent, typename C::const_iterator> &data = *static_cast<const std::tuple<ContainerEvent, typename C::const_iterator> *>(_data);

            ContainerEvent op = std::get<0>(data);
            const typename C::const_iterator &it = std::get<1>(data);

            for (BufferedOutStream *out : outStreams) {
                Serialize::write(*out, op, "op");
                switch (op) {
                case EMPLACE: {
                    if constexpr (!container_traits<C>::sorted) {
                        writeIterator(*out, c, it);
                    }
                    TupleUnpacker::invoke(&Creator::writeItem, *out, *it, std::forward<Args>(args)...);
                    break;
                }
                case ERASE: {
                    writeIterator(*out, c, it);
                    break;
                }
                default:
                    throw 0;
                }
                out->endMessage();
            }
        }

        template <typename... Args>
        static void readAction(C &c, SerializeInStream &in, PendingRequest *request, Args &&... args)
        {
            ContainerEvent op;
            in >> op;

            bool accepted = (op & ~MASK) != ABORTED;

            if (accepted) {
                typename container_traits<C>::emplace_return it = performOperation(c, op, in, request ? request->mRequester : 0, request ? request->mRequesterTransactionId : 0, std::forward<Args>(args)...);
                if (request) {
                    (*request)(&it);
                }
            } else {
                assert(request);
                if (request->mRequesterTransactionId) {
                    BufferedOutStream &out = c.getActionResponseTarget(request->mRequester, request->mRequesterTransactionId);
                    out << op;
                    out.endMessage();
                }
                (*request)(nullptr);
            }
        }

        template <typename... Args>
        static void writeRequest(const C &c, BufferedOutStream &out, const void *_data, Args &&... args)
        {
            if (RequestPolicy::sCallByMasterOnly)
                throw 0;

            const std::tuple<ContainerEvent, typename C::const_iterator, typename C::value_type &> &data = *static_cast<const std::tuple<ContainerEvent, typename C::const_iterator, typename C::value_type &> *>(_data);
            ContainerEvent op = std::get<0>(data);
            out << op;
            switch (op) {
            case EMPLACE: {
                if constexpr (!container_traits<C>::sorted) {
                    writeIterator(out, c, std::get<1>(data));
                }
                TupleUnpacker::invoke(&Creator::writeItem, out, std::get<2>(data), std::forward<Args>(args)...);
                break;
            }
            default:
                throw 0;
            }
            out.endMessage();
        }

        template <typename... Args>
        static void readRequest(C &c, BufferedInOutStream &inout, TransactionId id, Args &&... args)
        {
            bool accepted = !RequestPolicy::sCallByMasterOnly;

            ContainerEvent op;
            inout >> op;

            if (!accepted) {
                if (id) {
                    c.beginActionResponseMessage(inout, id);
                    Serialize::write(inout, op | ABORTED, "op");
                    inout.endMessage();
                }
            } else {
                if (c.isMaster()) {
                    performOperation(c, op, inout, inout.id(), id, std::forward<Args>(args)...);
                } else {
                    BufferedOutStream &out = c.getSlaveActionMessageTarget(inout.id(), id);
                    Serialize::write(out, op, "op");
                    out.pipe(inout);
                    out.endMessage();
                }
            }
        }
    };

    template <typename T, typename... Configs>
    struct Operations;

    template <typename T, typename... Configs>
    struct BaseOperations {
        template <typename Hierarchy = std::monostate>
        static void read(SerializeInStream &in, T &t, const char *name, const Hierarchy &hierarchy = {}, StateTransmissionFlags flags = 0)
        {
            if constexpr (has_function_readState2_v<T>) {
                t.readState(in, name, CallerHierarchyPtr { hierarchy });
            } else if constexpr (has_function_readState_v<T>) {
                t.readState(in, name);
            } else if constexpr (isPrimitiveType_v<T>) {
                in.format().beginPrimitive(in, name, PrimitiveTypeIndex_v<T>);
                in.readUnformatted(t);
                in.format().endPrimitive(in, name, PrimitiveTypeIndex_v<T>);
                //mLog.log(t);
            } else if constexpr (PrimitiveTypesContain_v<std::remove_const_t<T>> || std::is_enum_v<std::remove_const_t<T>>) {
                //Don't do anything here
            } else if constexpr (std::is_base_of_v<SerializableUnitBase, T>) {
                SerializableUnitPtr { &t }.readState(in, name, CallerHierarchyPtr { hierarchy }, flags);
            } else if constexpr (std::is_base_of_v<SerializableDataUnit, T>) {
                SerializableDataPtr { &t }.readState(in, name, CallerHierarchyPtr { hierarchy }, flags);
            } else if constexpr (is_string_like_v<T>) {
                std::string s;
                Serialize::read<std::string>(in, s, name, hierarchy);
                t = s;
            } else if constexpr (is_iterable_v<T>) {
                ContainerOperations<T, Configs...>::read(in, t, name, hierarchy);
            } else if constexpr (TupleUnpacker::is_tuplefyable_v<T>) {
                Operations<decltype(TupleUnpacker::toTuple(std::declval<T &>())), Configs...>::read(in, TupleUnpacker::toTuple(t), name, hierarchy);
            } else {
                static_assert(dependent_bool<T, false>::value, "Invalid Type");
            }
        }

        template <typename Hierarchy = std::monostate>
        static void write(SerializeOutStream &out, const T &t, const char *name, const Hierarchy &hierarchy = {}, StateTransmissionFlags flags = 0)
        {
            if constexpr (has_function_writeState2_v<T>) {
                t.writeState(out, name, CallerHierarchyPtr { hierarchy });
            } else if constexpr (has_function_writeState_v<T>) {
                t.writeState(out, name);
            } else if constexpr (isPrimitiveType_v<T>) {
                out.format().beginPrimitive(out, name, PrimitiveTypeIndex_v<T>);
                out.writeUnformatted(t);
                out.format().endPrimitive(out, name, PrimitiveTypeIndex_v<T>);
                //mLog.log(t);
            } else if constexpr (PrimitiveTypesContain_v<std::remove_const_t<T>> || std::is_enum_v<std::remove_const_t<T>>) {
                //Don't do anything here
            } else if constexpr (std::is_base_of_v<SerializableUnitBase, T>) {
                SerializableUnitConstPtr { &t }.writeState(out, name, CallerHierarchyPtr { hierarchy }, flags);
            } else if constexpr (std::is_base_of_v<SerializableDataUnit, T>) {
                SerializableDataConstPtr { &t }.writeState(out, name, CallerHierarchyPtr { hierarchy }, flags);
            } else if constexpr (is_string_like_v<T>) {
                Serialize::write<std::string>(out, t, name, hierarchy);
            } else if constexpr (is_iterable_v<T>) {
                ContainerOperations<std::remove_const_t<T>, Configs...>::write(out, t, name, hierarchy);
            } else if constexpr (TupleUnpacker::is_tuplefyable_v<T>) {
                Operations<decltype(TupleUnpacker::toTuple(std::declval<T &>())), Configs...>::write(out, TupleUnpacker::toTuple(t), name, hierarchy);
            } else {
                static_assert(dependent_bool<T, false>::value, "Invalid Type");
            }
        }

        template <typename... Args>
        static void writeAction(const T &t, const std::set<BufferedOutStream *, CompareStreamId> &outStreams, const void *data, Args &&... args)
        {
            /*if constexpr (PrimitiveTypesContain_v<T> || std::is_enum_v<T>) {
            out.format().beginPrimitive(out, name, PrimitiveTypeIndex_v<T>);
            out.writeUnformatted(t);
            out.format().endPrimitive(out, name, PrimitiveTypeIndex_v<T>);
            //mLog.log(t);
        } else if constexpr (has_function_writeAction_v<T>) {
            t.writeAction(op, data, answerTarget, answerId);
        } else */
            if constexpr (is_iterable_v<T>) {
                ContainerOperations<T, Configs...>::writeAction(t, outStreams, data, std::forward<Args>(args)...);
            } /*else if constexpr (TupleUnpacker::is_tuplefyable_v<T>) {
            write(out, TupleUnpacker::toTuple(t), name);
        } */
            else {
                static_assert(dependent_bool<T, false>::value, "Invalid Type");
            }
        }

        template <typename... Args>
        static void readAction(T &t, SerializeInStream &in, PendingRequest *request, Args &&... args)
        {
            /*if constexpr (PrimitiveTypesContain_v<T> || std::is_enum_v<T>) {
            out.format().beginPrimitive(out, name, PrimitiveTypeIndex_v<T>);
            out.writeUnformatted(t);
            out.format().endPrimitive(out, name, PrimitiveTypeIndex_v<T>);
            //mLog.log(t);
        } else if constexpr (has_function_readAction_v<T>) {
             t.readAction(in, request);
        } else */
            if constexpr (is_iterable_v<T>) {
                ContainerOperations<T, Configs...>::readAction(t, in, request, std::forward<Args>(args)...);
            } /*else if constexpr (TupleUnpacker::is_tuplefyable_v<T>) {
            write(out, TupleUnpacker::toTuple(t), name);
        } */
            else {
                static_assert(dependent_bool<T, false>::value, "Invalid Type");
            }
        }

        template <typename... Args>
        static void writeRequest(const T &t, BufferedOutStream &out, const void *data, Args &&... args)
        {
            /*if constexpr (PrimitiveTypesContain_v<T> || std::is_enum_v<T>) {
            out.format().beginPrimitive(out, name, PrimitiveTypeIndex_v<T>);
            out.writeUnformatted(t);
            out.format().endPrimitive(out, name, PrimitiveTypeIndex_v<T>);
            //mLog.log(t);
        } else  if constexpr (has_function_writeRequest_v<T>) {
            t.writeRequest(op, data, requester ,requesterTransactionId, callback);
        } else */
            if constexpr (is_iterable_v<T>) {
                ContainerOperations<T, Configs...>::writeRequest(t, out, data, std::forward<Args>(args)...);
            } /*else if constexpr (TupleUnpacker::is_tuplefyable_v<T>) {
            write(out, TupleUnpacker::toTuple(t), name);
        } */
            else {
                static_assert(dependent_bool<T, false>::value, "Invalid Type");
            }
        }

        template <typename... Args>
        static void readRequest(T &t, BufferedInOutStream &inout, TransactionId id, Args &&... args)
        {
            /*if constexpr (PrimitiveTypesContain_v<T> || std::is_enum_v<T>) {
            out.format().beginPrimitive(out, name, PrimitiveTypeIndex_v<T>);
            out.writeUnformatted(t);
            out.format().endPrimitive(out, name, PrimitiveTypeIndex_v<T>);
            //mLog.log(t);
        } else if constexpr (has_function_readRequest_v<T>) {
            t.readRequest(inout, id);
        } else */
            if constexpr (is_iterable_v<T>) {
                ContainerOperations<T, Configs...>::readRequest(t, inout, id, std::forward<Args>(args)...);
            } /*else if constexpr (TupleUnpacker::is_tuplefyable_v<T>) {
            write(out, TupleUnpacker::toTuple(t), name);
        } */
            else {
                static_assert(dependent_bool<T, false>::value, "Invalid Type");
            }
        }
    };

    template <typename T, typename... Configs>
    struct Operations : BaseOperations<T, Configs...> {
    };

    template <typename T, typename... Configs>
    struct Operations<std::unique_ptr<T>, Configs...> {

        template <typename... Args>
        static void read(SerializeInStream &in, const std::unique_ptr<T> &p, const char *name, Args &&... args)
        {
            Operations<T, Configs...>::read(in, *p, name, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void write(SerializeOutStream &out, const std::unique_ptr<T> &p, const char *name, Args &&... args)
        {
            Operations<T, Configs...>::write(out, *p, name, std::forward<Args>(args)...);
        }
    };

    template <typename T, typename... Configs>
    struct Operations<const std::unique_ptr<T>, Configs...> {

        template <typename... Args>
        static void read(SerializeInStream &in, const std::unique_ptr<T> &p, const char *name, Args &&... args)
        {
            Operations<T, Configs...>::read(in, *p, name, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void write(SerializeOutStream &out, const std::unique_ptr<T> &p, const char *name, Args &&... args)
        {
            Operations<T, Configs...>::write(out, *p, name, std::forward<Args>(args)...);
        }
    };

    template <typename... Ty, typename... Configs>
    struct Operations<std::tuple<Ty...>, Configs...> {

        template <typename... Args, size_t... Is>
        static void read(SerializeInStream &in, std::tuple<Ty...> &t, const char *name, std::index_sequence<Is...>, Args &&... args)
        {
            in.format().beginCompound(in, name);
            (Serialize::read<Ty>(in, std::get<Is>(t), nullptr, args...), ...);
            in.format().endCompound(in, name);
        }

        template <typename... Args>
        static void read(SerializeInStream &in, std::tuple<Ty...> &t, const char *name, Args &&... args)
        {
            read(in, t, name, std::make_index_sequence<sizeof...(Ty)> {}, std::forward<Args>(args)...);
        }

        template <typename... Args, size_t... Is>
        static void write(SerializeOutStream &out, const std::tuple<Ty...> &t, const char *name, std::index_sequence<Is...>, Args &&... args)
        {
            out.format().beginCompound(out, name);
            (Serialize::write<Ty>(out, std::get<Is>(t), "Element", args...), ...);
            out.format().endCompound(out, name);
        }

        template <typename... Args>
        static void write(SerializeOutStream &out, const std::tuple<Ty...> &t, const char *name, Args &&... args)
        {
            write(out, t, name, std::make_index_sequence<sizeof...(Ty)> {}, std::forward<Args>(args)...);
        }
    };

    template <typename... Ty, typename... Configs>
    struct Operations<std::tuple<Ty &...>, Configs...> {

        template <typename... Args, size_t... Is>
        static void read(SerializeInStream &in, std::tuple<Ty &...> t, const char *name, std::index_sequence<Is...>, Args &&... args)
        {
            in.format().beginCompound(in, name);
            (Serialize::read<Ty>(in, std::get<Is>(t), nullptr, args...), ...);
            in.format().endCompound(in, name);
        }

        template <typename... Args>
        static void read(SerializeInStream &in, std::tuple<Ty &...> t, const char *name, Args &&... args)
        {
            read(in, t, name, std::make_index_sequence<sizeof...(Ty)> {}, std::forward<Args>(args)...);
        }

        template <typename... Args, size_t... Is>
        static void write(SerializeOutStream &out, const std::tuple<const Ty &...> t, const char *name, std::index_sequence<Is...>, Args &&... args)
        {
            out.format().beginCompound(out, name);
            (Serialize::write<Ty>(out, std::get<Is>(t), "Element", args...), ...);
            out.format().endCompound(out, name);
        }

        template <typename... Args>
        static void write(SerializeOutStream &out, const std::tuple<const Ty &...> t, const char *name, Args &&... args)
        {
            write(out, t, name, std::make_index_sequence<sizeof...(Ty)> {}, std::forward<Args>(args)...);
        }
    };

    template <typename U, typename V, typename... Configs>
    struct Operations<std::pair<U, V>, Configs...> {

        static void read(SerializeInStream &in, std::pair<U, V> &t, const char *name = nullptr)
        {
            in.format().beginCompound(in, name);
            Serialize::read<U>(in, t.first);
            Serialize::read<V>(in, t.second);
            in.format().endCompound(in, name);
        }

        static void write(SerializeOutStream &out, const std::pair<U, V> &t, const char *name = nullptr)
        {
            out.format().beginCompound(out, name);
            Serialize::write<U>(out, t.first, "Element");
            Serialize::write<V>(out, t.second, "Element");
            out.format().endCompound(out, name);
        }
    };

    template <typename T, typename... Configs, typename Hierarchy>
    void read(SerializeInStream &in, T &t, const char *name, const Hierarchy &hierarchy, StateTransmissionFlags flags)
    {
        Operations<T, Configs...>::read(in, t, name, hierarchy, flags);
    }

    template <typename T, typename... Configs, typename Hierarchy>
    void write(SerializeOutStream &out, const T &t, const char *name, const Hierarchy &hierarchy, StateTransmissionFlags flags)
    {
        Operations<T, Configs...>::write(out, t, name, hierarchy, flags);
    }

    template <typename T>
    inline SerializeInStream &SerializeInStream::operator>>(T &t)
    {
        read(*this, t, nullptr);
        return *this;
    }

    template <typename T>
    inline SerializeOutStream &SerializeOutStream::operator<<(const T &t)
    {
        write(*this, t, nullptr);
        return *this;
    }

    inline SerializeOutStream &SerializeOutStream::operator<<(const char *s)
    {
        write(*this, std::string_view { s }, nullptr);
        return *this;
    }

}
}