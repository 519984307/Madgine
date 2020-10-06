#include "../moduleslib.h"

#include "serializableunitptr.h"

#include "streams/serializestream.h"

#include "formatter.h"

#include "serializableunit.h"

#include "streams/operations.h"

#include "serializemanager.h"

#include "toplevelunit.h"

#include "streams/serializablemapholder.h"

namespace Engine {
namespace Serialize {

    bool SerializableUnitConstPtr::isActive(size_t offset) const
    {
        //TODO: Maybe save lookup -> enforce order of elements in memory
        return mType->getIndex(offset) < mUnit->mActiveIndex;
    }

    void SerializableUnitConstPtr::writeState(SerializeOutStream &out, const char *name, StateTransmissionFlags flags) const
    {
        if (out.isMaster() && !(flags & StateTransmissionFlags_SkipId)) {
            out.format().beginExtended(out, name, 1);
            write(out, static_cast<uint64_t>(reinterpret_cast<uintptr_t>(mUnit)) >> 2, "id");
        }
        out.format().beginCompound(out, name);
        mType->writeState(mUnit, out);
        out.format().endCompound(out, name);
    }

    void SerializableUnitPtr::readState(SerializeInStream &in, const char *name, StateTransmissionFlags flags) const
    {
        SerializableMapHolder holder;
        in.startSerializableRead(&holder);

        if (!in.isMaster() && !(flags & StateTransmissionFlags_SkipId)) {
            in.format().beginExtended(in, name, 1);
            uint64_t id;
            read(in, id, "id");
            bool result = in.serializableMap().try_emplace(id, unit()).second;
            assert(result);
        }
        in.format().beginCompound(in, name);
        mType->readState(unit(), in, flags);
        in.format().endCompound(in, name);
    }

    void SerializableUnitConstPtr::writeAction(uint8_t index, int op, const void *data, ParticipantId answerTarget, TransactionId answerId) const
    {
        mType->writeAction(mUnit, index, op, data, answerTarget, answerId);
    }

    void SerializableUnitPtr::readAction(BufferedInOutStream &in, PendingRequest *request) const
    {
        mType->readAction(unit(), in, request);
    }

    void SerializableUnitConstPtr::writeRequest(uint8_t index, int op, const void *data, ParticipantId requester, TransactionId requesterTransactionId, std::function<void(void *)> callback) const
    {
        mType->writeRequest(mUnit, index, op, data, requester, requesterTransactionId, callback);
    }

    void SerializableUnitPtr::readRequest(BufferedInOutStream &in, TransactionId id) const
    {
        mType->readRequest(unit(), in, id);
    }

    void SerializableUnitPtr::setParent(SerializableUnitBase *parent) const
    {
        if (unit()->mTopLevel != mUnit)
            unit()->mTopLevel = parent ? parent->mTopLevel : nullptr;
        mType->setParent(unit());
    }

    void SerializableUnitPtr::applySerializableMap(SerializeInStream &in) const
    {
        mType->applySerializableMap(unit(), in);
    }

    void SerializableUnitPtr::setDataSynced(bool b) const
    {
        assert(mUnit->mSynced != b);
        unit()->mSynced = b;
        mType->setDataSynced(unit(), b);
    }

    void SerializableUnitPtr::setActive(bool active, bool existenceChanged) const
    {
        //assert(mSynced == active);
        mType->setActive(unit(), active, existenceChanged);
    }

    SerializableUnitBase *SerializableUnitPtr::unit() const
    {
        return const_cast<SerializableUnitBase *>(mUnit);
    }

}
}