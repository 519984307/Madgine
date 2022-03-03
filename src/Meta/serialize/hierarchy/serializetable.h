#pragma once

#include "serializetable_forward.h"

#include "Generic/callerhierarchy.h"

namespace Engine {
namespace Serialize {

    struct SerializeTableCallbacks {
        template <typename T>
        constexpr SerializeTableCallbacks(type_holder_t<T>)
            : onActivate([](SerializableDataUnit *unit, bool active, bool existenceChanged) {
                if constexpr (has_function_onActivate_upTo_v<T, 2>)
                    TupleUnpacker::invoke(&T::onActivate, static_cast<T*>(unit), active, existenceChanged);
            })
        {

        }

        void (*onActivate)(SerializableDataUnit *, bool, bool);
    };

    struct META_EXPORT SerializeTable {
        const char *mTypeName;
        SerializeTableCallbacks mCallbacks;
        const SerializeTable &(*mBaseType)();
        const Serializer *mFields;
        bool mIsTopLevelUnit;

        void writeState(const SerializableDataUnit *unit, FormattedSerializeStream &out, CallerHierarchyBasePtr hierarchy = {}) const;
        StreamResult readState(SerializableDataUnit *unit, FormattedSerializeStream &in, StateTransmissionFlags flags = 0, CallerHierarchyBasePtr hierarchy = {}) const;
        StreamResult readState(SerializableUnitBase *unit, FormattedSerializeStream &in, StateTransmissionFlags flags = 0, CallerHierarchyBasePtr hierarchy = {}) const;

        StreamResult readAction(SerializableUnitBase *unit, FormattedBufferedStream &in, PendingRequest *request) const;
        StreamResult readRequest(SerializableUnitBase *unit, FormattedBufferedStream &in, TransactionId id) const;

        StreamResult applyMap(SerializableDataUnit *unit, FormattedSerializeStream &in, bool success) const;
        void setSynced(SerializableUnitBase *unit, bool b) const;
        void setActive(SerializableDataUnit *unit, bool active, bool existenceChanged) const;
        void setActive(SerializableUnitBase *unit, bool active, bool existenceChanged) const;
        void setParent(SerializableUnitBase *unit) const;

        void writeAction(const SerializableUnitBase *parent, uint8_t index, const std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> &outStreams, const void *data) const;
        void writeRequest(const SerializableUnitBase *parent, uint8_t index, FormattedBufferedStream &out, const void *data) const;

        uint8_t getIndex(OffsetPtr offset) const;
        const Serializer &get(uint8_t index) const;
    };

}
}

