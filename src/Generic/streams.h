#pragma once

namespace Engine {

typedef std::istream::pos_type pos_type;
typedef std::istream::off_type off_type;

struct GENERIC_EXPORT InStream {
    InStream();
    InStream(std::unique_ptr<std::streambuf> &&buffer);
    InStream(InStream &&other);
    ~InStream();

    template <typename T>
    InStream &operator>>(T &t)
    {
        mStream >> t;
        return *this;
    }

    std::istreambuf_iterator<char> iterator();
    std::istreambuf_iterator<char> end();

    size_t readRaw(void *buffer, size_t size);

    explicit operator bool() const;

    pos_type tell();
    bool seek(pos_type p);
    bool seek(off_type p, std::ios::seekdir dir);

    void skipWs();

	std::unique_ptr<std::streambuf> release();

protected:
    InStream(std::streambuf *buffer);

    std::streambuf &buffer() const;

    std::istream mStream;

    friend struct OutStream;

private:
    bool mOwning = true;
};

struct GENERIC_EXPORT OutStream {
    OutStream();
    OutStream(std::unique_ptr<std::streambuf> &&buffer);
    OutStream(OutStream &&other);
    ~OutStream();

    template <typename T>
    OutStream &operator<<(const T &t)
    {
        mStream << t;
        return *this;
    }

    void writeRaw(const void *data, size_t count);

    explicit operator bool() const;

    std::unique_ptr<std::streambuf> release();

    void pipe(InStream &in);

protected:
    std::streambuf &buffer() const;

    std::ostream mStream;
};

struct GENERIC_EXPORT Stream : InStream, OutStream {
    Stream(std::unique_ptr<std::streambuf> &&buffer);

    using InStream::buffer;
};

}