#pragma once

#include "Meta/serialize/syncmanager.h"

#include "Meta/serialize/formatter/safebinaryformatter.h"

#include "serialize/xml/xmllib.h"

#include "serialize/xml/xmlformatter.h"

using namespace Engine::Serialize;

struct Buffer {
    std::array<char, 4096> mBuffer[2];
    uint64_t mWrittenCount[2] = { 0, 0 };
};

using TestFormatter = Engine::Serialize::SafeBinaryFormatter;

struct BufferedTestBuf : std::basic_streambuf<char> {
    BufferedTestBuf(Buffer &buffer, bool isMaster)
        : mBuffer(buffer)
        , mIsMaster(isMaster)
    {
    }

    /*virtual void handleError() override
    {        
    }*/

    virtual std::streamsize xsgetn(char *buffer, std::streamsize count) override
    {
        size_t index = mIsMaster ? 0 : 1;

        std::streamsize avail = mBuffer.mWrittenCount[index] - mReadOffset;
        std::streamsize readCount = std::min(count, avail);
        if (readCount == 0)
            return 0;
        std::memcpy(buffer, mBuffer.mBuffer[index].data() + mReadOffset, readCount);
        mReadOffset += readCount;
        return static_cast<int>(readCount);
    }

    virtual std::streamsize xsputn(const char *buffer, std::streamsize count) override
    {
        size_t index = mIsMaster ? 1 : 0;

        assert(mBuffer.mWrittenCount[index] + count <= mBuffer.mBuffer[index].size());
        std::memcpy(mBuffer.mBuffer[index].data() + mBuffer.mWrittenCount[index], buffer, count);
        mBuffer.mWrittenCount[index] += count;
        return static_cast<int>(count);
    }

    virtual std::streamsize showmanyc() override
    {
        size_t index = mIsMaster ? 0 : 1;
        return mBuffer.mWrittenCount[index] - mReadOffset;
    }

    Buffer &mBuffer;
    uint64_t mReadOffset = 0;
    bool mIsMaster;
};

struct TestBuf : std::basic_streambuf<char> {
    TestBuf(Buffer &buffer, bool isMaster = true)
        : mBuffer(buffer)
    {
        size_t index = isMaster ? 1 : 0;
        setp(mBuffer.mBuffer[index].data(), mBuffer.mBuffer[index].data());

        setg(mBuffer.mBuffer[1 - index].data(), mBuffer.mBuffer[1 - index].data(), mBuffer.mBuffer[1 - index].data() + mBuffer.mWrittenCount[1 - index]);
    }

    std::basic_streambuf<char>::int_type overflow(int c) override
    {
        if (c != EOF) {
            size_t index = 1;

            setp(pptr(), pptr() + 1);
            ++mBuffer.mWrittenCount[index];

            *pptr() = c;
            pbump(1);
            return c;
        }
        return traits_type::eof();
    }

    std::basic_streambuf<char>::int_type underflow() override
    {
        size_t index = 1;
        if (mBuffer.mWrittenCount[index] > (egptr() - eback())) {
            setg(eback(), gptr(), eback() + mBuffer.mWrittenCount[index]);
            std::basic_streambuf<char>::int_type c = *gptr();
            //gbump(1);
            return c;
        }
        return traits_type::eof();
    }

    Buffer &mBuffer;
};

struct TestManager : Engine::Serialize::SyncManager {
    TestManager(const char *id)
        : SyncManager("Test-Manager_"s + id)
    {
    }

    BufferedInOutStream &setBuffer(Buffer &buffer, bool slave, bool shareState = true)
    {
        std::unique_ptr<BufferedTestBuf> buf = std::make_unique<BufferedTestBuf>(buffer, !slave);
        if (slave) {
            setSlaveStream(Engine::Serialize::BufferedInOutStream { std::move(buf), std::make_unique<Engine::Serialize::SafeBinaryFormatter>(), *this, 0 }, shareState, std::chrono::milliseconds { 1000 });
            return *getSlaveStream();
        } else {
            addMasterStream(Engine::Serialize::BufferedInOutStream { std::move(buf), std::make_unique<Engine::Serialize::SafeBinaryFormatter>(), *this, 1 }, shareState);
            return const_cast<BufferedInOutStream &>(*getMasterStreams().find(1));
        }
    }
};