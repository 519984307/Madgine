#include "../../metalib.h"
#include "serializestream.h"

#include "serializestreamdata.h"

//#include "../../keyvalue/valuetype.h"

#include "../formatter.h"

#include "Generic/cowstring.h"

#include "Generic/bytebuffer.h"

#include "base64/base64.h"

#include "../serializemanager.h"

namespace Engine {
namespace Serialize {

    SerializeStream::SerializeStream()
    {
    }

    SerializeStream::SerializeStream(std::unique_ptr<std::basic_streambuf<char>> buffer, std::unique_ptr<SerializeStreamData> data)
        : Stream(std::move(buffer))
        , mData(std::move(data))
    {
    }

    SerializeStream::SerializeStream(SerializeStream &&other)
        : Stream(std::move(other))
        , mData(std::move(other.mData))
    {
    }

    SerializeStream::SerializeStream(SerializeStream &&other, SerializeManager *mgr)
        : Stream(std::move(other))
        , mData(std::move(other.mData))
    {
        data().setManager(mgr);
    }

    SerializeStream::~SerializeStream() = default;

    SerializeStream& SerializeStream::operator=(SerializeStream&& other) {
        Stream::operator=(std::move(other));
        std::swap(mData, other.mData);
        return *this;
    }

    StreamResult SerializeStream::read(void *buffer, size_t size)
    {
        Stream::read(buffer, size);
        if (!*this)
            return STREAM_PARSE_ERROR(*this, true, "Unexpected EOF");
        return {};
    }

    StreamResult SerializeStream::readN(std::string &buffer, size_t n)
    {
        //assert(!format().mBinary);

        skipWs();

        if (n == 0)
            return {};

        buffer.resize(n, '\0');
        return read(&buffer[0], n);
    }

    StreamResult SerializeStream::readUntil(std::string &buffer, const char *delim)
    {
        //assert(!format().mBinary);

        skipWs();

        buffer.resize(255);
        size_t i = 0;
        do {
            STREAM_PROPAGATE_ERROR(read(&buffer[i], 1));
            ++i;
        } while (!strchr(delim, buffer[i - 1]));
        buffer.resize(i);

        return {};
    }

    StreamResult SerializeStream::peekN(std::string &buffer, size_t n)
    {
        //assert(!format().mBinary);

        pos_type pos = tell();
        STREAM_PROPAGATE_ERROR(readN(buffer, n));
        seek(pos);
        return {};
    }

    StreamResult SerializeStream::peekUntil(std::string &buffer, const char *c)
    {
        //assert(!format().mBinary);

        pos_type pos = tell();
        STREAM_PROPAGATE_ERROR(readUntil(buffer, c));
        seek(pos);
        return {};
    }

    SerializeManager *SerializeStream::manager() const
    {
        return data().manager();
    }

    void SerializeStream::setId(ParticipantId id)
    {
        data().setId(id);
    }

    ParticipantId SerializeStream::id() const
    {
        return data().id();
    }

    bool SerializeStream::isMaster(StreamMode mode)
    {
        return data().isMaster(mode);
    }

    SerializeStreamData &SerializeStream::data() const
    {
        return *mData;
    }

    SerializableUnitList &SerializeStream::serializableList()
    {
        return data().serializableList();
    }

    SerializableUnitMap &SerializeStream::serializableMap()
    {
        return data().serializableMap();
    }

    StreamResult SerializeStream::read(SyncableUnitBase *&p)
    {
        UnitId ptr;
        STREAM_PROPAGATE_ERROR(read(ptr));
        assert(ptr <= (std::numeric_limits<UnitId>::max() >> 2));
        if (ptr)
            ptr = (ptr << 2) | static_cast<UnitId>(UnitIdTag::SYNCABLE);
        p = reinterpret_cast<SyncableUnitBase *>(ptr);
        return {};
    }

    StreamResult SerializeStream::read(SerializableDataUnit *&p)
    {
        uint32_t ptr;
        STREAM_PROPAGATE_ERROR(read(ptr));
        assert(ptr <= (std::numeric_limits<uint32_t>::max() >> 2));
        if (ptr)
            ptr = (ptr << 2) | static_cast<uint32_t>(UnitIdTag::SERIALIZABLE);
        p = reinterpret_cast<SerializableDataUnit *>(ptr);
        return {};
    }

    StreamResult SerializeStream::operator>>(SyncableUnitBase *&p)
    {
        UnitId ptr;
        STREAM_PROPAGATE_ERROR(operator>>(ptr));
        assert(ptr <= (std::numeric_limits<UnitId>::max() >> 2));
        if (ptr)
            ptr = (ptr << 2) | static_cast<UnitId>(UnitIdTag::SYNCABLE);
        p = reinterpret_cast<SyncableUnitBase *>(ptr);
        return {};
    }

    StreamResult SerializeStream::operator>>(SerializableDataUnit *&p)
    {
        uint32_t ptr;
        STREAM_PROPAGATE_ERROR(operator>>(ptr));
        assert(ptr <= (std::numeric_limits<uint32_t>::max() >> 2));
        if (ptr)
            ptr = (ptr << 2) | static_cast<uint32_t>(UnitIdTag::SERIALIZABLE);
        p = reinterpret_cast<SerializableDataUnit *>(ptr);
        return {};
    }

    StreamResult SerializeStream::read(std::string &s)
    {
        uint32_t size;
        STREAM_PROPAGATE_ERROR(read(size));
        s.resize(size);
        STREAM_PROPAGATE_ERROR(read(&s[0], size));
        return {};
    }

    StreamResult SerializeStream::operator>>(std::string &s)
    {
        Stream::operator>>(s);
        if (!*this)
            return STREAM_PARSE_ERROR(*this, false, "Expected <string>");
        return {};
    }

    StreamResult SerializeStream::read(ByteBuffer &b)
    {
        uint32_t size;
        STREAM_PROPAGATE_ERROR(read(size));
        std::unique_ptr<std::byte[]> buffer = std::make_unique<std::byte[]>(size);
        STREAM_PROPAGATE_ERROR(read(buffer.get(), size));
        b = ByteBuffer { std::move(buffer), size };
        return {};
    }

    StreamResult SerializeStream::operator>>(ByteBuffer &b)
    {
        std::string base64Encoded;
        STREAM_PROPAGATE_ERROR(operator>>(base64Encoded));
        if (!Base64::decode(b, base64Encoded))
            return STREAM_PARSE_ERROR(*this, false, "Invalid Base64 String '" << base64Encoded << "'");
        return {};
    }

    StreamResult SerializeStream::read(std::monostate &)
    {
        return {};
    }

    StreamResult SerializeStream::operator>>(std::monostate &)
    {
        return {};
    }

    void SerializeStream::write(const SyncableUnitBase *p)
    {
        Stream::write(SerializeManager::convertPtr(*this, p));
    }

    void SerializeStream::write(const SerializableDataUnit *p)
    {
        uint32_t id = 0;
        if (p) {
            SerializableUnitMap &map = serializableMap();
            auto it = map.try_emplace(p, map.size() + 1).first;
            id = it->second;
        }
        Stream::write(id);
    }

    SerializeStream &SerializeStream::operator<<(const SyncableUnitBase *p)
    {
        Stream::operator<<(SerializeManager::convertPtr(*this, p));
        return *this;
    }
    SerializeStream &SerializeStream::operator<<(const SerializableDataUnit *p)
    {
        uint32_t id = 0;
        if (p) {
            SerializableUnitMap &map = serializableMap();
            auto it = map.try_emplace(p, map.size() + 1).first;
            id = it->second;
        }
        Stream::operator<<(id);
        return *this;
    }

    void SerializeStream::write(const std::string_view &s)
    {
        Stream::write<uint32_t>(s.size());
        Stream::write(s.data(), s.size());
    }

    SerializeStream &SerializeStream::operator<<(const std::string_view &s)
    {
        Stream::operator<<(s);
        return *this;
    }

    void SerializeStream::write(const ByteBuffer &b)
    {
        Stream::write<uint32_t>(b.mSize);
        Stream::write(b.mData, b.mSize);
    }

    SerializeStream &SerializeStream::operator<<(const ByteBuffer &b)
    {
        Stream::operator<<(Base64::encode(b));
        return *this;
    }

    void SerializeStream::write(const std::monostate &)
    {
    }

    SerializeStream &SerializeStream::operator<<(const std::monostate &)
    {
        return *this;
    }

}
} // namespace Scripting
