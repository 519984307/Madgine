#pragma once

#include "statetransmissionflags.h"

namespace Engine {
namespace Serialize {

    DERIVE_FUNCTION(onActivate);    

    struct SerializeTableCallbacks {
        template <typename T>
        constexpr SerializeTableCallbacks(type_holder_t<T>)
            : onActivate([](SerializableUnitBase *unit, int active, int existenceChanged) {
                if constexpr (has_function_onActivate_upTo_v<T, 2>)
                    TupleUnpacker::invoke(&T::onActivate, static_cast<T*>(unit), active, existenceChanged);
            })
        {

        }

        void (*onActivate)(SerializableUnitBase *, int, int);
    };

    struct MODULES_EXPORT SerializeTable {
        const char *mTypeName;
        SerializeTableCallbacks mCallbacks;
        const SerializeTable &(*mBaseType)();
        const std::pair<const char *, Serializer> *mFields;
        bool mIsTopLevelUnit;

        void writeState(const SerializableUnitBase *unit, SerializeOutStream &out) const;
        void readState(SerializableUnitBase *unit, SerializeInStream &in, StateTransmissionFlags flags = 0) const;

        void readAction(SerializableUnitBase *unit, SerializeInStream &in, PendingRequest *request) const;
        void readRequest(SerializableUnitBase *unit, BufferedInOutStream &in, TransactionId id) const;

        void applySerializableMap(SerializableUnitBase *unit, SerializeInStream &in) const;
        void setDataSynced(SerializableUnitBase *unit, bool b) const;
        void setActive(SerializableUnitBase *unit, bool active, bool existenceChanged) const;

        void writeAction(const SerializableUnitBase *parent, uint8_t index, int op, const void *data, ParticipantId answerTarget, TransactionId answerId) const;
        void writeRequest(const SerializableUnitBase *parent, uint8_t index, int op, const void *data, ParticipantId requester, TransactionId requesterTransactionId, std::function<void(void *)> callback) const;

        bool isInstance(SerializableUnitBase *unit) const;

        uint8_t getIndex(size_t offset) const;
        const Serializer &get(uint8_t index) const;
    };

}
}

DLL_IMPORT_VARIABLE2(const Engine::Serialize::SerializeTable, serializeTable, typename T);
