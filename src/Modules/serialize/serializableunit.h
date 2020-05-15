#pragma once

#include "streams/comparestreamid.h"

#include "serializetable.h"

#include "statetransmissionflags.h"

namespace Engine {
namespace Serialize {

#define SERIALIZABLEUNIT_MEMBERS()        \
    READONLY_PROPERTY(Synced, isSynced)   \
    READONLY_PROPERTY(MasterId, masterId) \
    READONLY_PROPERTY(SlaveId, slaveId)

    struct MODULES_EXPORT SerializableUnitBase {
    protected:
        SerializableUnitBase(size_t masterId = 0);
        SerializableUnitBase(const SerializableUnitBase &other);
        SerializableUnitBase(SerializableUnitBase &&other) noexcept;
        ~SerializableUnitBase();

    public:
        const TopLevelSerializableUnitBase *topLevel() const;

        void writeState(SerializeOutStream &out, const char *name = nullptr, StateTransmissionFlags flags = 0) const;
        void readState(SerializeInStream &in, const char *name = nullptr, StateTransmissionFlags flags = 0);

        void readAction(BufferedInOutStream &in, PendingRequest *request);
        void readRequest(BufferedInOutStream &in, TransactionId id);

        void applySerializableMap(SerializeInStream &in);

        void writeCreationData(SerializeOutStream &out) const;

        size_t slaveId() const;
        size_t masterId() const;

        bool isSynced() const;
        bool isMaster() const;

    protected:
        void sync();
        void unsync();

        void setSynced(bool b);

        void setSlaveId(size_t id, SerializeManager *mgr);

        const SerializeTable *serializeType() const;

        size_t moveMasterId(size_t newId = 0);

    private:
        std::set<BufferedOutStream *, CompareStreamId> getMasterMessageTargets() const;
        BufferedOutStream *getSlaveMessageTarget() const;

        void clearSlaveId(SerializeManager *mgr);

        void setDataSynced(bool b);
        void setActive(bool active, bool existenceChanged);

        bool isActive(size_t index) const;

        friend struct SyncManager;
        friend struct SerializeUnitHelper;
        friend struct SyncableBase;
        friend struct SerializeTable;
        template <typename T, bool b>
        friend struct UnitHelper;
        template <typename T, typename Base>
        friend struct TableInitializer;
        template <typename T>
        friend struct Syncable;
        template <typename T>
        friend struct Serializable;

    protected:
        void setParent(SerializableUnitBase *parent);

    private:
        SerializableUnitBase *mParent = nullptr;

        size_t mSlaveId = 0;
        size_t mMasterId;
        size_t mActiveIndex = 0;

        bool mSynced = false; // Maybe move only into TopLevelUnit?

        const SerializeTable *mType = nullptr;
    };

    template <typename T, typename Base>
    struct SerializableUnit;

    template <typename T, typename Base>
    struct TableInitializer {
        template <typename... Args>
        TableInitializer(Args &&...)
        {
            static_cast<SerializableUnit<T, Base> *>(this)->mType = &serializeTable<T>();
        }
        TableInitializer(const TableInitializer &)
        {
            static_cast<SerializableUnit<T, Base> *>(this)->mType = &serializeTable<T>();
        };
        TableInitializer(TableInitializer &&)
        {
            static_cast<SerializableUnit<T, Base> *>(this)->mType = &serializeTable<T>();
        }
    };

    template <typename T, typename _Base = SerializableUnitBase>
    struct SerializableUnit : _Base, private TableInitializer<T, _Base> {
    protected:
        friend TableInitializer<T, _Base>;

        typedef T Self;

        using _Base::_Base;
    };

    template <typename>
    struct __SerializeInstance;

#define SERIALIZABLEUNIT friend struct ::Engine::Serialize::__SerializeInstance<Self>

} // namespace Serialize
} // namespace Core
