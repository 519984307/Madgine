#pragma once

namespace Engine {

typedef std::istream::pos_type pos_type;
typedef std::istream::off_type off_type;

struct InStream {
    InStream()
        : mStream(nullptr)
    {
    }
    InStream(std::unique_ptr<std::streambuf> &&buffer)
        : mStream(buffer.release())
        , mOwning(true)
    {
    }
    InStream(InStream &&other)
        : mStream(other.mStream.rdbuf(nullptr))
        , mOwning(std::exchange(other.mOwning, false))
    {
    }
    ~InStream()
    {
        if (mOwning) {
            assert(mStream.rdbuf());
            delete mStream.rdbuf();
        }
        mStream.rdbuf(nullptr);
    }

    template <typename T>
    InStream &operator>>(T &t)
    {
        if constexpr (std::is_enum_v<T>) {
            std::underlying_type_t<T> val;
            mStream >> val;
            t = static_cast<T>(val);
        } else {
            mStream >> t;
        }
        return *this;
    }

    std::istreambuf_iterator<char> iterator()
    {
        return std::istreambuf_iterator<char>(mStream);
    }
    std::istreambuf_iterator<char> end()
    {
        return std::istreambuf_iterator<char>();
    }

    size_t readRaw(void *buffer, size_t size)
    {
        mStream.read(static_cast<char *>(buffer), size);
        return mStream.gcount();
    }

    explicit operator bool() const
    {
        return mStream.rdbuf() != nullptr && static_cast<bool>(mStream);
    }

    pos_type tell()
    {
        return mStream.tellg();
    }
    bool seek(pos_type p)
    {
        return static_cast<bool>(mStream.seekg(p));
    }
    bool seek(off_type p, std::ios::seekdir dir)
    {
        return static_cast<bool>(mStream.seekg(p, dir));
    }

    std::ios_base::iostate state() const
    {
        return mStream.rdstate();
    }

    void clear()
    {
        mStream.clear();
    }

    void setState(std::ios_base::iostate state)
    {
        mStream.setstate(state);
    }

    void skipWs()
    {
        if (mStream.flags() & std::ios_base::skipws) {
            mStream >> std::ws;
        }
    }

    std::unique_ptr<std::streambuf> release()
    {
        assert(mOwning);
        mOwning = false;
        return std::unique_ptr<std::streambuf>(mStream.rdbuf(nullptr));
    }

protected:
    InStream(std::streambuf *buffer)
        : mStream(buffer)
    {
    }

    std::streambuf &buffer() const
    {
        return *mStream.rdbuf();
    }

    std::istream mStream;

    friend struct OutStream;

    bool mOwning = false;
};

struct OutStream {
    OutStream()
        : mStream(nullptr)
    {
    }
    OutStream(std::unique_ptr<std::streambuf> &&buffer)
        : mStream(buffer.release())
    {
    }
    OutStream(OutStream &&other)
        : mStream(other.mStream.rdbuf(nullptr))
    {
    }
    ~OutStream()
    {
        if (mStream.rdbuf())
            delete mStream.rdbuf(nullptr);
    }

    template <typename T>
    OutStream &operator<<(const T &t)
    {
        if constexpr (std::is_enum_v<T>) {
            mStream << static_cast<std::underlying_type_t<T>>(t);
        } else {
            mStream << t;
        }
        return *this;
    }

    void writeRaw(const void *data, size_t count)
    {
        mStream.write(static_cast<const char *>(data), count);
    }

    explicit operator bool() const
    {
        return mStream.rdbuf() != nullptr && static_cast<bool>(mStream);
    }

    std::unique_ptr<std::streambuf> release()
    {
        return std::unique_ptr<std::streambuf>(mStream.rdbuf(nullptr));
    }

    void pipe(InStream &in)
    {
        mStream << &in.buffer();
    }

protected:
    std::streambuf &buffer() const
    {
        return *mStream.rdbuf();
    }

    std::ostream mStream;
};

/*struct Stream : InStream, OutStream {
    Stream(std::unique_ptr<std::streambuf> &&buffer)
        : InStream(buffer.get())
        , OutStream(std::move(buffer))
    {
    }

    using InStream::buffer;
};*/
}