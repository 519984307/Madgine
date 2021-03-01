#include "../interfaceslib.h"

#if UNIX

#    include "socketapi.h"

#    include <arpa/inet.h>
#    include <fcntl.h>
#    include <netinet/ip.h>
#    include <netinet/tcp.h>
#    include <sys/ioctl.h>
#    include <sys/socket.h>
#    include <unistd.h>

namespace Engine {

SocketAPIResult SocketAPI::init()
{
    return SocketAPIResult::SUCCESS;
}

void SocketAPI::finalize()
{
}

void Socket::close()
{
    assert(mSocket);
    int result = ::close(mSocket);
    assert(result == 0);
    mSocket = Invalid_Socket;
}

int Socket::send(const char *buf, size_t len) const
{
    return ::send(mSocket, buf, len, 0);
}

int Socket::recv(char *buf, size_t len) const
{
    return ::read(mSocket, buf, len);
}

int Socket::in_available() const
{
    int bytes_available = 0;
    if (ioctl(mSocket, FIONREAD, &bytes_available) < 0)
        return -1;
    return bytes_available;
}

SocketAPIResult SocketAPI::getError(const char *op)
{
    int error = errno;
    switch (error) {
    case EWOULDBLOCK:
        return SocketAPIResult::WOULD_BLOCK;
    case EADDRINUSE:
        return SocketAPIResult::ALREADY_IN_USE;
    case EBADF:
    case ENOTSOCK:
    case EOPNOTSUPP:
        throw 0;
    default:
        fprintf(stderr, "Unknown Linux Socket-Error-Code from %s: %d", op, error);
        fflush(stderr);
        return SocketAPIResult::UNKNOWN_ERROR;
    }
}

int SocketAPI::getOSError()
{
    return errno;
}

SocketAPIResult preInitSock(unsigned long long s)
{
#    if !EMSCRIPTEN
    int on = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        return SocketAPI::getError("setsockopt");
    }
#        if LINUX || ANDROID
    if (setsockopt(s, SOL_TCP, TCP_NODELAY, &on, sizeof(on)) < 0) {
        return SocketAPI::getError("setsockopt");
    }
#        endif
#    endif

    return SocketAPIResult::SUCCESS;
}

SocketAPIResult postInitSock(unsigned long long s)
{
    int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) {
        return SocketAPI::getError("fcntl");
    }
    flags |= O_NONBLOCK;
    if (fcntl(s, F_SETFL, flags) != 0) {
        return SocketAPI::getError("fcntl");
    }
    return SocketAPIResult::SUCCESS;
}

std::pair<Socket, SocketAPIResult> SocketAPI::socket(int port)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0) {
        return { Socket {}, getError("socket") };
    }

    Socket sock { static_cast<unsigned long long>(s) };

    if (SocketAPIResult result = preInitSock(sock); result != SocketAPIResult::SUCCESS) {
        sock.close();
        return { std::move(sock), result };
    }

    if (SocketAPIResult result = postInitSock(sock); result != SocketAPIResult::SUCCESS) {
        sock.close();
        return { std::move(sock), result };
    }

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        SocketAPIResult result = getError("bind");
        sock.close();
        return { std::move(sock), result };
    }

    int result = listen(sock, SOMAXCONN);
    if (result != 0) {
        SocketAPIResult result = getError("listen");
        sock.close();
        return { std::move(sock), result };
    }
    return { std::move(sock), SocketAPIResult::SUCCESS };
}

std::pair<Socket, SocketAPIResult> Socket::accept(TimeOut timeout) const
{
    struct timeval tv;
    fd_set readfds;

    if (timeout.isZero()) {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    } else if (timeout.isInfinite()) {
        tv.tv_sec = std::numeric_limits<long>::max();
        tv.tv_usec = 0;
    } else {
        std::chrono::milliseconds remainder = std::chrono::duration_cast<std::chrono::milliseconds>(timeout.remainder());
        tv.tv_sec = static_cast<long>(remainder.count()) / 1000;
        tv.tv_usec = static_cast<long>(remainder.count()) % 1000 * 1000;
    }

    FD_ZERO(&readfds);
    FD_SET(mSocket, &readfds);

    int retval = select(1, &readfds, NULL, NULL, &tv);
    if (retval > 0) {
#    if OSX || IOS
        int socket = ::accept(mSocket, NULL, NULL);
#    else
        int socket = accept4(mSocket, NULL, NULL, O_NONBLOCK);
#    endif
        if (socket >= 0)
            return { Socket { static_cast<unsigned long long>(socket) }, SocketAPIResult::SUCCESS };
        else
            return { Socket {}, SocketAPI::getError("accept") };
    } else {
        if (retval == 0)
            return { Socket {}, SocketAPIResult::TIMEOUT };
        else
            return { Socket {}, SocketAPI::getError("select") };
    }
}

std::pair<Socket, SocketAPIResult> SocketAPI::connect(const std::string &url, int portNr)
{
    //Fill out the information needed to initialize a socket…
    struct sockaddr_in target; //Socket address information

    target.sin_family = AF_INET; // address family Internet
    target.sin_port = htons(portNr); //Port to connect on

    if (inet_pton(AF_INET, url.c_str(), &target.sin_addr) <= 0) {
        return { Socket {}, getError("inet_pton") };
    }

    int s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
    if (s < 0) {
        return { Socket {}, getError("socket") };
    }

    Socket sock { static_cast<unsigned long long>(s) };

    if (SocketAPIResult result = preInitSock(sock); result != SocketAPIResult::SUCCESS) {
        sock.close();
        return { std::move(sock), result };
    }

    //Try connecting...

    if (::connect(sock, (struct sockaddr *)&target, sizeof(target)) < 0) {
        SocketAPIResult error = getError("connect");
        sock.close();
        return { std::move(sock), error };
    }

    if (SocketAPIResult result = postInitSock(sock); result != SocketAPIResult::SUCCESS) {
        sock.close();
        return { std::move(sock), result };
    }

    return { std::move(sock), SocketAPIResult::SUCCESS };
}

}

#endif
