#pragma once

#include "Interfaces/socket/socketapi.h"

namespace Engine {
namespace Network {
    struct MADGINE_NETWORK_EXPORT NetworkBuffer : std::basic_streambuf<char> {
        NetworkBuffer(SocketId socket);
        NetworkBuffer(const NetworkBuffer &) = delete;
        NetworkBuffer(NetworkBuffer &&other) noexcept = delete;
        virtual ~NetworkBuffer();

    protected:        
        //void handleError() override;

        std::streamsize xsgetn(char *, std::streamsize) override;

        std::streamsize xsputn(const char *, std::streamsize) override;

        std::streamsize showmanyc() override;

    private:
        SocketId mSocket; // = SOCKET
    };
}
}
