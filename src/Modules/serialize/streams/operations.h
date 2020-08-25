#pragma once

#include "../../generic/container/atomiccontaineroperation.h"
#include "../../generic/container/containerevent.h"
#include "../container/physical.h"
#include "../creationhelper.h"
#include "../formatter.h"
#include "../primitivetypes.h"
#include "../statetransmissionflags.h"
#include "bufferedstream.h"
#include "pendingrequest.h"

namespace Engine {
namespace Serialize {

    DERIVE_TYPENAME(Config);
    DERIVE_FUNCTION(writeState, SerializeOutStream &, const char *)
    //DERIVE_FUNCTION(writeState, writeState2, SerializeOutStream &, const char *, StateTransmissionFlags)
    DERIVE_FUNCTION(readState, SerializeInStream &, const char *)
    //DERIVE_FUNCTION(readState, readState2, SerializeInStream &, const char *, StateTransmissionFlags)
    /*DERIVE_FUNCTION(readAction, SerializeInStream &, PendingRequest*)
    DERIVE_FUNCTION(readRequest, BufferedInOutStream &, TransactionId)
    DERIVE_FUNCTION(writeAction, int, const void *, ParticipantId, TransactionId)
    DERIVE_FUNCTION(writeRequest, int, const void *, ParticipantId, TransactionId, std::function<void(void*)>)*/

    template <typename T, typename... Configs, typename... Args>
    void read(SerializeInStream &in, T &t, const char *name, Args &&... args);

    template <typename T, typename... Configs, typename... Args>
    void read(SerializeInStream &in, T &t, Args &&... args);

    template <typename T, typename... Configs, typename... Args>
    void write(SerializeOutStream &out, const T &t, const char *name, Args &&... args);

    template <typename T, typename... Configs, typename... Args>
    void write(SerializeOutStream &out, const T &t, Args &&... args);

    template <typename C, typename Config = DefaultCreator<typename C::value_type>>
    struct ContainerOperations {

        template <typename Op, typename... Args>
        static void readOp(SerializeInStream &in, Op &op, const char *name, Args &&... args)
        {
            using T = typename C::value_type;

            in.format().beginExtended(in, name, 1);
            uint32_t size;
            Serialize::read(in, size, "size");
            in.format().beginCompound(in, name);

            if constexpr (!container_traits<C>::is_fixed_size) {
                TupleUnpacker::invoke(&Config::template clear<Op>, op, size);

                while (size--) {
                    TupleUnpacker::invoke(&Config::template readItem<Op>, in, op, physical(op).end(), std::forward<Args>(args)...);
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
                TupleUnpacker::invoke(&Config::writeItem, out, t, std::forward<Args>(args)...);
            }
            out.format().endCompound(out, name);
        }

        static typename C::iterator readIterator(SerializeInStream &in, C &c)
        {
            int32_t dist;
            Serialize::read(in, dist);
            return std::next(c.begin(), dist);
        }

        static void writeIterator(SerializeOutStream &out, const C &c, const typename C::const_iterator &it)
        {
            Serialize::write<int32_t>(out, std::distance(c.begin(), it));
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
                it = TupleUnpacker::invoke(&Config::template readItem<InsertOperation_t<C>>, in, op, cit, std::forward<Args>(args)...);
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

        template <typename... Args>
        static void writeAction(const C &c, int op, const void *data, ParticipantId answerTarget, TransactionId answerId, Args &&... args)
        {

            for (BufferedOutStream *out : c.getMasterActionMessageTargets(answerTarget, answerId)) {
                Serialize::write(*out, op);
                switch (op) {
                case EMPLACE: {
                    const typename C::iterator &it = *static_cast<const typename C::iterator *>(data);
                    if constexpr (!container_traits<C>::sorted) {
                        writeIterator(*out, c, it);
                    }
                    TupleUnpacker::invoke(&Config::writeItem, *out, *it, std::forward<Args>(args)...);
                    break;
                }
                case ERASE: {
                    const typename C::iterator &it = *static_cast<const typename C::iterator *>(data);
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
                if (request) {
                    if (request->mRequesterTransactionId) {
                        BufferedOutStream *out = c.beginActionResponseMessage(request->mRequester, request->mRequesterTransactionId);
                        *out << op;
                        out->endMessage();
                    }
                    (*request)(nullptr);
                }
            }
        }

        template <typename... Args>
        static void writeRequest(const C &c, int op, const void *data, ParticipantId requester, TransactionId requesterTransactionId, std::function<void(void *)> callback, Args &&... args)
        {
            BufferedOutStream *out = c.getSlaveActionMessageTarget(requester, requesterTransactionId, std::move(callback));
            *out << op;
            switch (op) {
            case EMPLACE: {
                const std::pair<typename C::const_iterator, typename C::value_type &> &it = *static_cast<const std::pair<typename C::const_iterator, typename C::value_type &> *>(data);
                if constexpr (!container_traits<C>::sorted) {
                    writeIterator(*out, c, it.first);
                }
                TupleUnpacker::invoke(&Config::writeItem, *out, it.second, std::forward<Args>(args)...);
                break;
            }
            default:
                throw 0;
            }
            out->endMessage();
        }

        template <typename... Args>
        static void readRequest(C &c, BufferedInOutStream &inout, TransactionId id, Args &&... args)
        {
            bool accepted = true; //(Config::requestMode == __syncablecontainer__impl__::ALL_REQUESTS); //Check TODO

            ContainerEvent op;
            inout >> reinterpret_cast<int &>(op);

            if (!accepted) {
                if (id) {
                    c.beginActionResponseMessage(&inout, id);
                    Serialize::write(inout, op | ABORTED);
                    inout.endMessage();
                }
            } else {
                if (c.isMaster()) {
                    performOperation(c, op, inout, inout.id(), id, std::forward<Args>(args)...);
                } else {
                    BufferedOutStream *out = c.getSlaveActionMessageTarget(inout.id(), id);
                    Serialize::write(*out, op);
                    out->pipe(inout);
                    out->endMessage();
                }
            }
        }
    };

    template <typename T, typename... Configs>
    struct Operations;

    template <typename T, typename... Configs>
    struct BaseOperations {
        template <typename... Args>
        static void read(SerializeInStream &in, T &t, const char *name, Args &&... args)
        {
            if constexpr (PrimitiveTypesContain_v<T> || std::is_enum_v<T>) {
                in.format().beginPrimitive(in, name, PrimitiveTypeIndex_v<T>);
                in.readUnformatted(t);
                in.format().endPrimitive(in, name, PrimitiveTypeIndex_v<T>);
                //mLog.log(t);
            } else if constexpr (PrimitiveTypesContain_v<std::remove_const_t<T>> || std::is_enum_v<std::remove_const_t<T>>) {
                //Don't do anything here
            } else if constexpr (has_function_readState_v<T>) {
                TupleUnpacker::invoke(&T::readState, &t, in, name, StateTransmissionFlags_DontApplyMap);
            } else if constexpr (is_iterable_v<T>) {
                ContainerOperations<T, Configs...>::read(in, t, name, std::forward<Args>(args)...);
            } else if constexpr (TupleUnpacker::is_tuplefyable_v<T>) {
                Operations<decltype(TupleUnpacker::toTuple(t)), Configs...>::read(in, TupleUnpacker::toTuple(t), name);
            } else {
                static_assert(dependent_bool<T, false>::value, "Invalid Type");
            }
        }

        template <typename... Args>
        static void write(SerializeOutStream &out, const T &t, const char *name, Args &&... args)
        {
            if constexpr (PrimitiveTypesContain_v<T> || std::is_enum_v<T>) {
                out.format().beginPrimitive(out, name, PrimitiveTypeIndex_v<T>);
                out.writeUnformatted(t);
                out.format().endPrimitive(out, name, PrimitiveTypeIndex_v<T>);
                //mLog.log(t);
            } else if constexpr (PrimitiveTypesContain_v<std::remove_const_t<T>> || std::is_enum_v<std::remove_const_t<T>>) {
                //Don't do anything here
            } else if constexpr (has_function_writeState_v<T>) {
                t.writeState(out, name);
            } else if constexpr (is_iterable_v<T>) {
                ContainerOperations<T, Configs...>::write(out, t, name, std::forward<Args>(args)...);
            } else if constexpr (TupleUnpacker::is_tuplefyable_v<T>) {
                Operations<decltype(TupleUnpacker::toTuple(t)), Configs...>::write(out, TupleUnpacker::toTuple(t), name);
            } else {
                static_assert(dependent_bool<T, false>::value, "Invalid Type");
            }
        }

        template <typename... Args>
        static void writeAction(const T &t, int op, const void *data, ParticipantId answerTarget, TransactionId answerId, Args &&... args)
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
                ContainerOperations<T, Configs...>::writeAction(t, op, data, answerTarget, answerId, std::forward<Args>(args)...);
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
        static void writeRequest(const T &t, int op, const void *data, ParticipantId requester, TransactionId requesterTransactionId, std::function<void(void *)> callback, Args &&... args)
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
                ContainerOperations<T, Configs...>::writeRequest(t, op, data, requester, requesterTransactionId, std::move(callback), std::forward<Args>(args)...);
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

    template <typename... Ty, typename... Configs>
    struct Operations<std::tuple<Ty...>, Configs...> {

        template <typename... Args>
        static void read(SerializeInStream &in, std::tuple<Ty...> &t, const char *name, Args &&... args)
        {
            in.format().beginCompound(in, name);
            TupleUnpacker::forEach(t, [&](auto &e) { Serialize::read(in, e, nullptr, args...); });
            in.format().endCompound(in, name);
        }

        template <typename... Args>
        static void write(SerializeOutStream &out, const std::tuple<Ty...> &t, const char *name, Args &&... args)
        {
            out.format().beginCompound(out, name);
            TupleUnpacker::forEach(t, [&](const auto &e) { Serialize::write(out, e, "Element", args...); });
            out.format().endCompound(out, name);
        }
    };

    template <typename... Ty, typename... Configs>
    struct Operations<std::tuple<Ty &...>, Configs...> {

        template <typename... Args>
        static void read(SerializeInStream &in, std::tuple<Ty &...> t, const char *name, Args &&... args)
        {
            in.format().beginCompound(in, name);
            TupleUnpacker::forEach(t, [&](auto &e) { Serialize::read(in, e, nullptr, args...); });
            in.format().endCompound(in, name);
        }

        template <typename... Args>
        static void write(SerializeOutStream &out, const std::tuple<Ty &...> &t, const char *name, Args &&... args)
        {
            out.format().beginCompound(out, name);
            TupleUnpacker::forEach(t, [&](const auto &e) { Serialize::write(out, e, "Element", args...); });
            out.format().endCompound(out, name);
        }
    };

    template <typename U, typename V, typename... Configs>
    struct Operations<std::pair<U, V>, Configs...> {

        static void read(SerializeInStream &in, std::pair<U, V> &t, const char *name = nullptr)
        {
            in.format().beginCompound(in, name);
            Serialize::read(in, t.first);
            Serialize::read(in, t.second);
            in.format().endCompound(in, name);
        }

        static void write(SerializeOutStream &out, const std::pair<U, V> &t, const char *name = nullptr)
        {
            out.format().beginCompound(out, name);
            Serialize::write(out, t.first, "Element");
            Serialize::write(out, t.second, "Element");
            out.format().endCompound(out, name);
        }
    };

    template <typename T, typename... Configs, typename... Args>
    void read(SerializeInStream &in, T &t, const char *name, Args &&... args)
    {
        Operations<T, Configs...>::read(in, t, name, std::forward<Args>(args)...);
    }

    template <typename T, typename... Configs, typename... Args>
    void read(SerializeInStream &in, T &t, Args &&... args)
    {
        read<T, Configs...>(in, t, static_cast<const char *>(nullptr), std::forward<Args>(args)...);
    }

    template <typename T, typename... Configs, typename... Args>
    void write(SerializeOutStream &out, const T &t, const char *name, Args &&... args)
    {
        Operations<T, Configs...>::write(out, t, name, std::forward<Args>(args)...);
    }

    template <typename T, typename... Configs, typename... Args>
    void write(SerializeOutStream &out, const T &t, Args &&... args)
    {
        write<T, Configs...>(out, t, static_cast<const char *>(nullptr), std::forward<Args>(args)...);
    }

    template <typename T>
    inline SerializeInStream &SerializeInStream::operator>>(T &t)
    {
        read(*this, t);
        return *this;
    }

    template <typename T>
    inline SerializeOutStream &SerializeOutStream::operator<<(const T &t)
    {
        write(*this, t);
        return *this;
    }

    inline SerializeOutStream &SerializeOutStream::operator<<(const char *s)
    {
        write(*this, std::string { s });
        return *this;
    }

}
}