#include "../../metalib.h"

#include "../streams/serializablemapholder.h"
#include "statetransmissionflags.h"

#include "serializetable.h"

#include "Generic/offsetptr.h"

#include "toplevelunit.h"

#include "../operations.h"

namespace Engine {
namespace Serialize {

    void SerializableDataConstPtr::writeState(FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy, StateTransmissionFlags flags) const
    {
        SerializableMapHolder holder { out };

        if (out.isMaster(StreamMode::WRITE) && out.data() && !(flags & StateTransmissionFlags_SkipId)) {
            out.beginExtendedWrite(name, 1);
            write(out, mUnit, "serId");
        }

        out.beginCompoundWrite(name);
        mType->writeState(mUnit, out, hierarchy);
        out.endCompoundWrite(name);
    }

    StreamResult SerializableDataPtr::readState(FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy, StateTransmissionFlags flags) const
    {
        SerializableListHolder holder { in };

        if (!in.isMaster(StreamMode::READ) && in.data() && !(flags & StateTransmissionFlags_SkipId)) {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
            SerializableUnitBase *idHelper;
            STREAM_PROPAGATE_ERROR(read(in, idHelper, "serId"));
            uint32_t id = reinterpret_cast<uintptr_t>(idHelper) >> 2;
            SerializableUnitList &list = in.serializableList();
            if (list.size() <= id)
                list.resize(id + 1);
            assert(!list[id]);
            list[id] = unit();
        }

        STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(mType->readState(unit(), in, flags, hierarchy));
        return in.endCompoundRead(name);
    }

    void SerializableDataPtr::setActive(bool active, bool existenceChanged) const
    {
        //assert(mSynced == active);
        mType->setActive(unit(), active, existenceChanged);
    }

    SerializableDataPtr::SerializableDataPtr(const SerializableUnitPtr &other)
        : SerializableDataPtr(other.unit(), other.mType)
    {
    }

    SerializableDataUnit *SerializableDataPtr::unit() const
    {
        return const_cast<SerializableDataUnit *>(mUnit);
    }

    const SerializableUnitBase *SerializableUnitConstPtr::unit() const
    {
        return static_cast<const SerializableUnitBase *>(mUnit);
    }

    SerializableUnitConstPtr::SerializableUnitConstPtr(const SerializableUnitBase *unit, const SerializeTable *type)
        : SerializableDataConstPtr { unit, type }
    {
    }

    bool SerializableUnitConstPtr::isActive(OffsetPtr offset) const
    {
        //TODO: Maybe save lookup -> enforce order of elements in memory
        return mType->getIndex(offset) < unit()->mActiveIndex;
    }

    StreamResult SerializableUnitPtr::readState(FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy, StateTransmissionFlags flags) const
    {
        SerializableListHolder holder { in };

        if (!in.isMaster(StreamMode::READ) && in.data() && !(flags & StateTransmissionFlags_SkipId)) {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
            SerializableUnitBase *idHelper;
            STREAM_PROPAGATE_ERROR(read(in, idHelper, "serId"));
            uint32_t id = reinterpret_cast<uintptr_t>(idHelper) >> 2;
            SerializableUnitList &list = in.serializableList();
            if (list.size() <= id)
                list.resize(id + 1);
            assert(!list[id]);
            list[id] = unit();
        }

        STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(mType->readState(unit(), in, flags, hierarchy));
        return in.endCompoundRead(name);
    }

    void SerializableUnitConstPtr::writeAction(uint8_t index, const std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> &outStreams, const void *data) const
    {
        mType->writeAction(unit(), index, outStreams, data);
    }

    StreamResult SerializableUnitPtr::readAction(FormattedBufferedStream &in, PendingRequest *request) const
    {
        return mType->readAction(unit(), in, request);
    }

    void SerializableUnitConstPtr::writeRequest(uint8_t index, FormattedBufferedStream &out, const void *data) const
    {
        mType->writeRequest(unit(), index, out, data);
    }

    StreamResult SerializableUnitPtr::readRequest(FormattedBufferedStream &in, TransactionId id) const
    {
        return mType->readRequest(unit(), in, id);
    }

    void SerializableUnitPtr::setParent(SerializableUnitBase *parent) const
    {
        if (unit()->mTopLevel != mUnit)
            unit()->mTopLevel = parent ? parent->mTopLevel : nullptr;
        mType->setParent(unit());
    }

    StreamResult SerializableDataPtr::applyMap(FormattedSerializeStream &in, bool success) const
    {
        return mType->applyMap(unit(), in, success);
    }

    void SerializableUnitPtr::setSynced(bool b) const
    {
        assert(unit()->mSynced != b);
        unit()->mSynced = b;
        mType->setSynced(unit(), b);
    }

    void SerializableUnitPtr::setActive(bool active, bool existenceChanged) const
    {
        //assert(mSynced == active);
        mType->setActive(unit(), active, existenceChanged);
    }

    SerializableUnitBase *SerializableUnitPtr::unit() const
    {
        return const_cast<SerializableUnitBase *>(SerializableUnitConstPtr::unit());
    }
}
}