#pragma once

#include "../serializable.h"
#include "../streams/bufferedstream.h"
#include "../streams/operations.h"
#include "../syncable.h"
#include "../unithelper.h"

#include "Generic/functor.h"
#include "Generic/memberoffsetptr.h"

//TODO rewrite to modern operations
namespace Engine {
namespace Serialize {

    DERIVE_FUNCTION2(PlusAssign, operator+=);
    DERIVE_FUNCTION2(MinusAssign, operator-=);

#define SYNCED(Name, ...) MEMBER_OFFSET_CONTAINER(Name, ::Engine::Serialize::Synced<__VA_ARGS__>)

    template <typename T, typename Observer = NoOpFunctor, typename OffsetPtr = TaggedPlaceholder<MemberOffsetPtrTag, 0>>
    struct Synced : Syncable<OffsetPtr>, Serializable<OffsetPtr>, private Observer {

        friend struct Operations<Synced<T, Observer, OffsetPtr>>;

        enum class Operation {
            SET,
            ADD,
            SUB
        };

        template <typename... _Ty>
        Synced(_Ty &&...args)
            : mData(std::forward<_Ty>(args)...)
        {
        }

        Observer &observer()
        {
            return *this;
        }

        template <typename Ty>
        void operator=(Ty &&v)
        {
            if (mData != v) {
                if (this->isMaster()) {
                    T old = mData;
                    mData = std::forward<Ty>(v);
                    notify(old);
                } else {
                    std::pair<Operation, T> data { SyncedOperation::SET, std::forward<Ty>(v) };
                    this->writeRequest(&data);
                }
            }
        }

        template <typename Ty>
        void operator+=(Ty &&v)
        {
            if (this->isMaster()) {
                T old = mData;
                mData += std::forward<Ty>(v);
                notify(old);
            } else {
                std::pair<Operation, T> data { SyncedOperation::ADD, std::forward<Ty>(v) };
                this->writeRequest(&data);
            }
        }

        template <typename Ty>
        void operator-=(Ty &&v)
        {
            if (this->isMaster()) {
                T old = mData;
                mData -= std::forward<Ty>(v);
                notify(old);
            } else {
                std::pair<Operation, T> data { SyncedOperation::SUB, std::forward<Ty>(v) };
                this->writeRequest(, &data);
            }
        }

        T *operator->()
        {
            return &mData;
        }

        T *ptr()
        {
            return &mData;
        }

        operator const T &() const
        {
            return mData;
        }

        void setDataSynced(bool b)
        {
            UnitHelper<T>::setItemDataSynced(mData, b);
        }

        void setActive(bool active, bool existenceChanged)
        {
            if (!active) {
                if (mData != T {})
                    Observer::operator()(T {}, mData);
            }
            UnitHelper<T>::setItemActive(mData, active, existenceChanged);
            if (active) {
                if (mData != T {})
                    Observer::operator()(mData, T {});
            }
        }

        void applySerializableMap(SerializeInStream &in, bool success)
        {
            UnitHelper<T>::applyMap(in, mData, success);
        }

        void setParent(SerializableUnitBase *parent)
        {
            UnitHelper<T>::setItemParent(mData, parent);
        }

    protected:
        void notify(const T &old, ParticipantId answerTarget = 0, TransactionId answerId = 0)
        {
            if (this->isSynced()) {
                std::pair<Operation, T> data { Operation::SET, mData };
                this->writeAction(&data, answerTarget, answerId);
            }
            if (this->isActive()) {
                Observer::operator()(mData, old);
            }
        }

    private:
        T mData;
    };

    template <typename T, typename Observer, typename OffsetPtr>
    struct Operations<Synced<T, Observer, OffsetPtr>> {
        template <typename... Args>
        static StreamResult readRequest(Synced<T, Observer, OffsetPtr> &synced, BufferedInOutStream &inout, TransactionId id, Args &&...args)
        {
            if (synced.isMaster()) {
                Synced::Operation op;
                STREAM_PROPAGATE_ERROR(Serialize::read(inout, op, nullptr));
                T old = synced.mData;
                T value;
                STREAM_PROPAGATE_ERROR(Serialize::read(inout, value, nullptr));
                switch (op) {
                case Synced::Operation::SET:
                    synced.mData = value;
                    break;
                case Synced::Operation::ADD:
                    if constexpr (has_function_PlusAssign_v<T>)
                        synced.mData += value;
                    else
                        throw 0;
                    break;
                case Synced::Operation::SUB:
                    if constexpr (has_function_MinusAssign_v<T>)
                        synced.mData -= value;
                    else
                        throw 0;
                    break;
                }
                synced.notify(old, inout.id(), id);
            } else {
                BufferedOutStream &out = synced.getSlaveActionMessageTarget(inout.id(), id);
                out.pipe(inout);
                out.endMessage();
            }
            return {};
        }

        template <typename... Args>
        static void writeRequest(const Synced<T, Observer, OffsetPtr> &synced, BufferedOutStream &out, const void *_data, Args &&...args)
        {
            const std::pair<Synced::Operation, T> &data = *static_cast<const std::pair<Synced::Operation, T> *>(_data);
            Serialize::write(out, data.first, nullptr);
            Serialize::write(out, data.second, nullptr);
            out.endMessage();
        }

        template <typename... Args>
        static StreamResult readAction(Synced<T, Observer, OffsetPtr> &synced, SerializeInStream &in, PendingRequest *request, Args &&...args)
        {
            Synced<T, Observer,OffsetPtr>::Operation op;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, op, nullptr));
            T old = synced.mData;
            T value;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, value, nullptr));
            switch (op) {
            case Synced::Operation::SET:
                synced.mData = value;
                break;
            case Synced::Operation::ADD:
                if constexpr (has_function_PlusAssign_v<T>)
                    synced.mData += value;
                else
                    throw 0;
                break;
            case Synced::Operation::SUB:
                if constexpr (has_function_MinusAssign_v<T>)
                    synced.mData -= value;
                else
                    throw 0;
                break;
            }
            synced.notify(old);
            return {};
        }

        template <typename... Args>
        static void writeAction(const Synced<T, Observer, OffsetPtr> &synced, const std::set<BufferedOutStream *, CompareStreamId> &outStreams, const void *_data, Args &&...args)
        {
            const std::pair<Synced::Operation, T> &data = *static_cast<const std::pair<Synced::Operation, T> *>(_data);
            for (BufferedOutStream *out : outStreams) {
                Serialize::write(*out, data.first, nullptr);
                Serialize::write(*out, data.second, nullptr);
                out->endMessage();
            }
        }

        template <typename... Args>
        static StreamResult read(SerializeInStream &in, Synced<T, Observer, OffsetPtr> &synced, const char *name, Args &&...args)
        {
            T old = synced.mData;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, synced.mData, name));
            synced.notify(old);
            return {};
        }

        template <typename... Args>
        static void write(SerializeOutStream &out, const Synced<T, Observer, OffsetPtr> &synced, const char *name, Args &&...args)
        {
            Serialize::write(out, synced.mData, name);
        }
    };
}
}
