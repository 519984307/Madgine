#include "networklib.h"

#include "networkbuffer.h"

namespace Engine {
namespace Network {
    NetworkBuffer::NetworkBuffer(Socket socket)
        : mSocket(std::move(socket))
    {
    }

    /*NetworkBuffer::NetworkBuffer(NetworkBuffer &&other) noexcept
        : buffered_streambuf(std::move(other))
        , mSocket(std::exchange(other.mSocket, Invalid_Socket))
    {
    }*/

    NetworkBuffer::~NetworkBuffer()
    {
    }

    std::streamsize NetworkBuffer::xsgetn(char *buf, std::streamsize len)
    {
        int result = mSocket.recv(buf, len);
        if (result == -1) {
            switch (SocketAPI::getError("recv")) {
            default:
                return 0;
            }
        }
        return result;
    }

    std::streamsize NetworkBuffer::xsputn(const char *buf, std::streamsize len)
    {
        int result = mSocket.send(buf, len);
        if (result == -1) {
            switch (SocketAPI::getError("recv")) {
            default:
                return 0;
            }
        }
        return result;
    }

    std::streamsize NetworkBuffer::showmanyc()
    {
        return mSocket.in_available();
    }

    /*void NetworkBuffer::handleError()
    {
        switch (SocketAPI::getError("unknown")) {
        case SocketAPIResult::WOULD_BLOCK:
            break;
        default:
            close(Serialize::StreamState::UNKNOWN_ERROR);
        }
    }*/
}
}
