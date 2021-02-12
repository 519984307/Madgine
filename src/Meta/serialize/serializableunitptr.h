#pragma once

#include "serializetable.h"
#include "Generic/callerhierarchy.h"

namespace Engine {
namespace Serialize {

    DERIVE_FUNCTION(customUnitPtr);

    struct META_EXPORT SerializableDataConstPtr {
    private:
        template <typename T>
        SerializableDataConstPtr(const T *t, std::true_type)
            : SerializableDataConstPtr(t ? t->customUnitPtr() : SerializableDataConstPtr { nullptr, &serializeTable<decayed_t<T>>() })
        {
        }

        template <typename T>
        SerializableDataConstPtr(const T *t, std::false_type)
            : mUnit(t)
            , mType(&serializeTable<decayed_t<T>>())
        {
        }

    public:
        constexpr SerializableDataConstPtr() = default;

        template <typename T, typename = std::enable_if_t<!std::is_same_v<SerializableDataUnit, T> && std::is_base_of_v<SerializableDataUnit, T>>>
        SerializableDataConstPtr(const T *t)
            : SerializableDataConstPtr(t, has_function_customUnitPtr<T> {})
        {
        }

        constexpr SerializableDataConstPtr(const SerializableDataConstPtr &other)
            : mUnit(other.mUnit)
            , mType(other.mType)
        {
        }

        SerializableDataConstPtr(const SerializableDataUnit *unit, const SerializeTable *type)
            : mUnit(unit)
            , mType(type)
        {
        }

        bool operator==(const SerializableDataConstPtr &other) const
        {
            return mUnit == other.mUnit && mType == other.mType;
        }

        bool operator<(const SerializableDataConstPtr &other) const
        {
            if (mUnit < other.mUnit)
                return true;
            if (mUnit > other.mUnit)
                return false;
            return mType < other.mType;
        }

        /*template <typename T>
        T *safe_cast() const
        {
            if (!mType->isDerivedFrom<std::remove_const_t<T>>())
                std::terminate();
            return static_cast<T *>(mScope);
        }*/

        explicit operator bool() const
        {
            return mUnit != nullptr;
        }

        void writeState(SerializeOutStream &out, const char *name = nullptr, CallerHierarchyBasePtr hierarchy = {}, StateTransmissionFlags flags = 0) const;

        const SerializableDataUnit *mUnit = nullptr;
        const SerializeTable *mType = nullptr;
    };

    struct META_EXPORT SerializableDataPtr : SerializableDataConstPtr {
    private:
        template <typename T>
        SerializableDataPtr(T *t, std::true_type)
            : SerializableDataPtr(t ? SerializableDataPtr { t->customUnitPtr() } : SerializableDataPtr { nullptr, &serializeTable<decayed_t<T>>() })
        {
        }

        template <typename T>
        SerializableDataPtr(T *t, std::false_type)
            : SerializableDataConstPtr(t)
        {
        }

    public:
        constexpr SerializableDataPtr() = default;

        template <typename T, typename = std::enable_if_t<!std::is_same_v<SerializableDataUnit, T> && std::is_base_of_v<SerializableDataUnit, T> && !std::is_const_v<T>>>
        SerializableDataPtr(T *t)
            : SerializableDataPtr(t, has_function_customUnitPtr<T> {})
        {
        }

        constexpr SerializableDataPtr(const SerializableDataPtr &other) = default;

        SerializableDataPtr(SerializableDataUnit *unit, const SerializeTable *type)
            : SerializableDataConstPtr(unit, type)
        {
        }

        SerializableDataPtr(const SerializableUnitPtr &other);

        /*template <typename T>
        T *safe_cast() const
        {
            if (!mType->isDerivedFrom<std::remove_const_t<T>>())
                std::terminate();
            return static_cast<T *>(mScope);
        }*/

        explicit operator bool() const
        {
            return mUnit != nullptr;
        }

        void readState(SerializeInStream &in, const char *name = nullptr, CallerHierarchyBasePtr hierarchy = {}, StateTransmissionFlags flags = 0) const;

        void applySerializableMap(SerializeInStream &in) const;

        SerializableDataUnit *unit() const;
    };

    struct META_EXPORT SerializableUnitConstPtr : SerializableDataConstPtr {
    private:
        template <typename T>
        SerializableUnitConstPtr(const T *t, std::true_type)
            : SerializableUnitConstPtr(t ? t->customUnitPtr() : SerializableUnitConstPtr { nullptr, &serializeTable<decayed_t<T>>() })
        {
        }

        template <typename T>
        SerializableUnitConstPtr(const T *t, std::false_type)
            : SerializableUnitConstPtr(t, &serializeTable<decayed_t<T>>())
        {
        }

    public:
        constexpr SerializableUnitConstPtr() = default;

        template <typename T, typename = std::enable_if_t<!std::is_same_v<SerializableUnitBase, T> && std::is_base_of_v<SerializableUnitBase, T>>>
        SerializableUnitConstPtr(const T *t)
            : SerializableUnitConstPtr(t, has_function_customUnitPtr<T> {})
        {
        }

        constexpr SerializableUnitConstPtr(const SerializableUnitConstPtr &other) = default;

        SerializableUnitConstPtr(const SerializableUnitBase *unit, const SerializeTable *type);

        bool operator==(const SerializableUnitConstPtr &other) const
        {
            return mUnit == other.mUnit && mType == other.mType;
        }

        bool operator<(const SerializableUnitConstPtr &other) const
        {
            if (mUnit < other.mUnit)
                return true;
            if (mUnit > other.mUnit)
                return false;
            return mType < other.mType;
        }

        /*template <typename T>
        T *safe_cast() const
        {
            if (!mType->isDerivedFrom<std::remove_const_t<T>>())
                std::terminate();
            return static_cast<T *>(mScope);
        }*/

        explicit operator bool() const
        {
            return mUnit != nullptr;
        }

        bool isActive(size_t offset) const;

        void writeState(SerializeOutStream &out, const char *name = nullptr, CallerHierarchyBasePtr hierarchy = {}, StateTransmissionFlags flags = 0) const;

        void writeAction(uint8_t index, int op, const void *data, const std::set<BufferedOutStream *, CompareStreamId> &outStreams) const;
        void writeRequest(uint8_t index, int op, const void *data, BufferedOutStream *out) const;

        const SerializableUnitBase *unit() const;
    };

    struct META_EXPORT SerializableUnitPtr : SerializableUnitConstPtr {
    private:
        template <typename T>
        SerializableUnitPtr(T *t, std::true_type)
            : SerializableUnitPtr(t ? t->customUnitPtr() : SerializableUnitPtr { nullptr, &serializeTable<decayed_t<T>>() })
        {
        }

        template <typename T>
        SerializableUnitPtr(T *t, std::false_type)
            : SerializableUnitConstPtr(t)
        {
        }

    public:
        constexpr SerializableUnitPtr() = default;

        template <typename T, typename = std::enable_if_t<!std::is_same_v<SerializableUnitBase, T> && std::is_base_of_v<SerializableUnitBase, T> && !std::is_const_v<T>>>
        SerializableUnitPtr(T *t)
            : SerializableUnitPtr(t, has_function_customUnitPtr<T> {})
        {
        }

        constexpr SerializableUnitPtr(const SerializableUnitPtr &other) = default;

        SerializableUnitPtr(SerializableUnitBase *unit, const SerializeTable *type)
            : SerializableUnitConstPtr(unit, type)
        {
        }

        /*template <typename T>
        T *safe_cast() const
        {
            if (!mType->isDerivedFrom<std::remove_const_t<T>>())
                std::terminate();
            return static_cast<T *>(mScope);
        }*/

        explicit operator bool() const
        {
            return mUnit != nullptr;
        }

        void readState(SerializeInStream &in, const char *name = nullptr, CallerHierarchyBasePtr hierarchy = {}, StateTransmissionFlags flags = 0) const;

        void readAction(BufferedInOutStream &in, PendingRequest *request) const;
        void readRequest(BufferedInOutStream &in, TransactionId id) const;

        void setDataSynced(bool b) const;
        void setActive(bool active, bool existenceChanged) const;
        void setParent(SerializableUnitBase *parent) const;

        SerializableUnitBase *unit() const;        
    };

}
}