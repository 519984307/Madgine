#pragma once

#include "../serialize/syncmanager.h"
#include "../threading/signal.h"
#include "../threading/slot.h"
#include "networkbuffer.h"

namespace Engine {
namespace Network {

    ENUM_BASE(NetworkManagerResult, Serialize::SyncManagerResult,
        ALREADY_CONNECTED,
        NO_SERVER,
        SOCKET_ERROR);

    struct MODULES_EXPORT NetworkManager : Serialize::SyncManager {
        NetworkManager(const std::string &name);
        NetworkManager(const NetworkManager &) = delete;
        NetworkManager(NetworkManager &&) noexcept;
        virtual ~NetworkManager();

        void operator=(const NetworkManager &) = delete;

        NetworkManagerResult startServer(int port);
        NetworkManagerResult connect(const std::string &url, int portNr, TimeOut timeout = {});

        void close();

        NetworkManagerResult acceptConnection(TimeOut timeout);
        int acceptConnections(int limit = -1);

        bool isConnected() const;

        NetworkManagerResult moveMasterStream(Serialize::ParticipantId streamId,
            NetworkManager *target);

        Threading::SignalStub<NetworkManagerResult> &connectionResult();

        SocketAPIResult getSocketAPIError() const;

    protected:
        NetworkManagerResult recordSocketError(SocketAPIResult error);

    private:
        SocketId mSocket;

        bool mIsServer;

        SocketAPIResult mSocketAPIError = SocketAPIResult::SUCCESS;

        //std::map<Serialize::ParticipantId, NetworkStream> mStreams;
        //std::unique_ptr<NetworkStream> mSlaveStream;

        //static constexpr UINT sMessageSignature = 1048;

        Threading::Signal<NetworkManagerResult> mConnectionResult;
    };
}
}
