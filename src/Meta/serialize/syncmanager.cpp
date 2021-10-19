#include "../metalib.h"

#include "syncmanager.h"

#include "toplevelunit.h"

#include "messageheader.h"

#include "streams/buffered_streambuf.h"

#include "serializeexception.h"

#include "streams/operations.h"

#include "serializableids.h"

namespace Engine {
namespace Serialize {

    SyncManager::SyncManager(const std::string &name)
        : SerializeManager(name)
        , mReceivingMasterState(false)
        , mSlaveStreamInvalid(false)
    {
    }

    SyncManager::SyncManager(SyncManager &&other) noexcept
        : SerializeManager(std::move(other))
        , mReceivingMasterState(other.mReceivingMasterState)
        , mTopLevelUnits(std::move(other.mTopLevelUnits))
        , mSlaveStreamInvalid(other.mSlaveStreamInvalid)
    {
        for (TopLevelUnitBase *unit : mTopLevelUnits) {
            unit->removeManager(&other);
            bool result = unit->addManager(this);
            assert(result);
        }
        other.mTopLevelUnits.clear();
        if (other.mSlaveStream) {
            mSlaveStream.emplace(std::move(*other.mSlaveStream), this);
            other.mSlaveStream.reset();
        }
        for (BufferedInOutStream &stream : other.mMasterStreams) {
            mMasterStreams.emplace(std::move(stream), this);
        }
    }

    SyncManager::~SyncManager() { clearTopLevelItems(); }

    void SyncManager::readMessage(BufferedInOutStream &stream)
    {
        MessageHeader header;
        stream.readHeader(header);

        if (header.mObject == SERIALIZE_MANAGER) {
            ParticipantId id;
            switch (header.mCmd) {
            case INITIAL_STATE_DONE:
                mReceivingMasterState = false;
                stream >> id;
                stream.setId(id);
                break;
            default:
                throw SerializeException(
                    "Unknown Builtin Command-Code for SerializeManager: " + std::to_string(header.mCmd));
            }
        } else {
            SyncableUnitBase *object = convertPtr(stream, header.mObject);
            switch (header.mType) {
            case ACTION: {
                PendingRequest *request = stream.fetchRequest(header.mTransaction);
                object->readAction(stream, request);
                if (request)
                    stream.popRequest(header.mTransaction);
                break;
            }
            case REQUEST:
                object->readRequest(stream, header.mTransaction);
                break;
            case STATE:
                object->readState(stream, nullptr, {}, StateTransmissionFlags_ApplyMap);
                break;
            default:
                throw SerializeException("Invalid Message-Type: " + std::to_string(header.mType));
            }
        }
    }

    std::set<BufferedOutStream *, CompareStreamId>
    SyncManager::getMasterMessageTargets()
    {
        std::set<BufferedOutStream *, CompareStreamId> result;

        for (BufferedInOutStream &stream : mMasterStreams) {
            //if (!stream.isClosed()) {
            result.insert(&stream);
            //}
        }
        return result;
    }

    void SyncManager::clearTopLevelItems()
    {
        while (!slavesMap().empty()) {
            slavesMap().begin()->second->clearSlaveId(this);
        }
        for (TopLevelUnitBase *unit : mTopLevelUnits) {
            unit->removeManager(this);
        }
        mTopLevelUnits.clear();
    }

    bool SyncManager::addTopLevelItem(TopLevelUnitBase *unit,
        bool sendStateFlag)
    {
        if (!unit->addManager(this))
            return false;
        mTopLevelUnits.insert(unit);

        if (mSlaveStream) {
            unit->initSlaveId(this);
        }

        if (sendStateFlag && unit->mSynced) {
            for (BufferedInOutStream &stream : mMasterStreams) {
                this->sendState(stream, unit);
            }
        }
        return true;
    }

    void SyncManager::removeTopLevelItem(TopLevelUnitBase *unit)
    {
        auto it2 = slavesMap().begin();
        while (it2 != slavesMap().end()) {
            if (it2->second->mTopLevel == unit) {
                it2++->second->clearSlaveId(this);
            } else {
                ++it2;
            }
        }
        unit->removeManager(this);

        mTopLevelUnits.erase(unit);
    }

    void SyncManager::moveTopLevelItem(TopLevelUnitBase *oldUnit,
        TopLevelUnitBase *newUnit)
    {
        removeTopLevelItem(oldUnit);
        addTopLevelItem(newUnit, false);
    }

    BufferedOutStream &SyncManager::getSlaveMessageTarget()
    {
        assert(mSlaveStream);
        return *mSlaveStream;
    }

    bool SyncManager::isMessageAvailable()
    {
        if (mSlaveStream && mSlaveStream->isMessageAvailable())
            return true;
        for (BufferedInOutStream &stream : mMasterStreams) {
            if (stream.isMessageAvailable())
                return true;
        }
        return false;
    }

    void SyncManager::removeAllStreams()
    {
        removeSlaveStream();
        mMasterStreams.clear();
    }

    SyncManagerResult SyncManager::setSlaveStream(BufferedInOutStream &&stream,
        bool receiveState,
        TimeOut timeout)
    {
        if (mSlaveStream)
            return SyncManagerResult::UNKNOWN_ERROR;

        SyncManagerResult state = SyncManagerResult::SUCCESS;

        auto it = mTopLevelUnits.begin();

        for (; it != mTopLevelUnits.end(); ++it) {
            if (!(*it)->updateManagerType(this, false)) {
                state = SyncManagerResult::UNKNOWN_ERROR;
                break;
            }
        }

        mSlaveStream.emplace(std::move(stream));
        setSlaveStreamData(&mSlaveStream->data());

        if (receiveState) {
            if (state == SyncManagerResult::SUCCESS) {
                for (TopLevelUnitBase *unit : mTopLevelUnits) {
                    unit->initSlaveId(this);
                }
                mReceivingMasterState = true;
                while (mReceivingMasterState) {
                    int msgCount = -1;
                    if (!receiveMessages(*mSlaveStream, msgCount)) {
                        state = recordStreamError(StreamState::UNKNOWN_ERROR);
                        mReceivingMasterState = false;
                    }
                    if (mReceivingMasterState && timeout.expired()) {
                        state = SyncManagerResult::TIMEOUT;
                        mReceivingMasterState = false;
                    }
                }
                if (state != SyncManagerResult::SUCCESS) {
                    while (!slavesMap().empty()) {
                        slavesMap().begin()->second->clearSlaveId(this);
                    }
                }
            }
        }

        if (state != SyncManagerResult::SUCCESS) {
            for (auto it2 = mTopLevelUnits.begin(); it2 != it; ++it2) {
                bool result = (*it2)->updateManagerType(this, true);
                assert(result);
            }
            setSlaveStreamData(nullptr);
            mSlaveStream.reset();
        }

        return state;
    }

    void SyncManager::removeSlaveStream()
    {
        if (mSlaveStream) {
            while (!slavesMap().empty()) {
                size_t s = slavesMap().size();
                slavesMap().begin()->second->clearSlaveId(this);
                assert(s > slavesMap().size());
            }
            for (TopLevelUnitBase *topLevel : mTopLevelUnits) {
                topLevel->updateManagerType(this, true);
            }
            mSlaveStream->setId(0);
            mSlaveStream.reset();
        }
    }

    SyncManagerResult SyncManager::addMasterStream(BufferedInOutStream &&stream,
        bool sendStateFlag)
    {
        if (sendStateFlag && stream) {
            for (TopLevelUnitBase *unit : mTopLevelUnits) {
                sendState(stream, unit);
            }
            stream.writeCommand(INITIAL_STATE_DONE, stream.id());
        }

        if (!stream)
            return recordStreamError(StreamState::UNKNOWN_ERROR);

        mMasterStreams.emplace(std::move(stream));
        return SyncManagerResult::SUCCESS;
    }

    SyncManagerResult SyncManager::moveMasterStream(ParticipantId streamId,
        SyncManager *target)
    {
        auto it = mMasterStreams.find(streamId);
        assert(it != mMasterStreams.end());            
        if (SyncManagerResult result = target->addMasterStream(
                std::move(*it), false);
            result != SyncManagerResult::SUCCESS)
            return result;
        BufferedInOutStream &stream = *target->mMasterStreams.find(streamId);
        std::vector<TopLevelUnitBase *> newTopLevels;
        newTopLevels.reserve(16);
        set_difference(target->getTopLevelUnits().begin(),
            target->getTopLevelUnits().end(), getTopLevelUnits().begin(),
            getTopLevelUnits().end(), back_inserter(newTopLevels));
        for (TopLevelUnitBase *newTopLevel : newTopLevels) {
            sendState(stream, newTopLevel);
        }
        return SyncManagerResult::SUCCESS;
    }

    ParticipantId SyncManager::getParticipantId(SyncManager *manager)
    {
        if (manager && manager->mSlaveStream) {
            return manager->mSlaveStream->id();
        } else {
            return sLocalMasterParticipantId;
        }
    }

    StreamState SyncManager::getStreamError() const
    {
        return mStreamError;
    }

    void SyncManager::receiveMessages(int msgCount, TimeOut timeout)
    {
        if (mSlaveStream && !mSlaveStreamInvalid) {
            if (!receiveMessages(*mSlaveStream, msgCount, timeout)) {
                removeSlaveStream();
            }
        }
        for (auto it = mMasterStreams.begin(); it != mMasterStreams.end();) {
            if (!receiveMessages(*it, msgCount,
                    timeout)) {
                it = mMasterStreams.erase(it);
            } else {
                ++it;
            }
        }
    }

    void SyncManager::sendMessages()
    {
        if (mSlaveStream && !mSlaveStreamInvalid) {
            if (!mSlaveStream->sendMessages()) {
                removeSlaveStream();
            }
        }
        for (auto it = mMasterStreams.begin(); it != mMasterStreams.end();) {
            if (!it->sendMessages()) {
                it = mMasterStreams.erase(it);
            } else {
                ++it;
            }
        }
    }

    void SyncManager::sendAndReceiveMessages()
    {
        receiveMessages();
        sendMessages();
    }

    SyncableUnitBase *SyncManager::convertPtr(SerializeInStream &in,
        UnitId unit)
    {
        if (unit == NULL_UNIT_ID)
            return nullptr;
        try {
            if (mSlaveStream && (&in == &*mSlaveStream)) {
                return slavesMap().at(unit);
            } else {
                if (unit < RESERVED_ID_COUNT) {
                    assert(unit >= BEGIN_STATIC_ID_SPACE);
                    auto it = std::find_if(
                        mTopLevelUnits.begin(), mTopLevelUnits.end(),
                        [unit](TopLevelUnitBase *topLevel) {
                            return topLevel->masterId() == unit;
                        });
                    if (it == mTopLevelUnits.end()) {
                        throw SerializeException(
                            "Illegal TopLevel-Id used! Possible configuration "
                            "mismatch!");
                    }
                    return *it;
                } else {
                    SyncableUnitBase *u = getByMasterId(unit);
                    if (std::find(mTopLevelUnits.begin(), mTopLevelUnits.end(), u->mTopLevel) == mTopLevelUnits.end()) {
                        throw SerializeException(
                            "Illegal Toplevel-Id used! Possible configuration "
                            "mismatch!");
                    }
                    return u;
                }
            }
        } catch (const std::out_of_range &) {
            std::stringstream ss;
            ss << "Unknown Unit-Id (" << unit
               << ") used! Possible binary mismatch!";
            throw SerializeException(ss.str());
        }
    }

    std::vector<ParticipantId> SyncManager::getMasterParticipantIds()
    {
        std::vector<ParticipantId> result;
        result.reserve(mMasterStreams.size());
        for (const BufferedInOutStream &stream : mMasterStreams) {
            result.push_back(stream.id());
        }
        return result;
    }

    bool SyncManager::receiveMessages(BufferedInOutStream &stream,
        int &msgCount, TimeOut timeout)
    {

        while (stream && ((stream.isMessageAvailable() && msgCount == -1) || msgCount > 0)) {
            while (stream.isMessageAvailable() && msgCount != 0) {
                //try {
                readMessage(stream);
                /*} catch (const SerializeException &e) {
                    LOG_ERROR(e.what());
                }*/
                if (msgCount > 0)
                    --msgCount;
                if (!timeout.isZero() && timeout.expired())
                    break;
            }
            if (timeout.expired())
                break;
        }

        return bool(stream);
    }

    bool SyncManager::sendAllMessages(BufferedInOutStream &stream,
        TimeOut timeout)
    {
        //TODO: Use Timeout (possibly pass down)
        return bool(stream.sendMessages());
    }

    BufferedInOutStream *SyncManager::getSlaveStream()
    {
        return mSlaveStream ? &*mSlaveStream : nullptr;
    }

    BufferedInOutStream &SyncManager::getMasterStream(ParticipantId id)
    {
        return *mMasterStreams.find(id);
    }

    const std::set<TopLevelUnitBase *> &
    SyncManager::getTopLevelUnits() const
    {
        return mTopLevelUnits;
    }

    size_t SyncManager::clientCount() const
    {
        return mMasterStreams.size();
    }

    void SyncManager::sendState(BufferedInOutStream &stream,
        SyncableUnitBase *unit)
    {
        stream.beginMessage(unit, STATE, 0);
        unit->writeState(stream);
        stream.endMessage();
    }

    SyncManagerResult SyncManager::recordStreamError(StreamState error)
    {
        mStreamError = error;
        return SyncManagerResult::STREAM_ERROR;
    }

} // namespace Serialize
} // namespace Engine
