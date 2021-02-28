#include <gtest/gtest.h>

#include "serialize/network/networklib.h"

#include "serialize/network/networkmanager.h"

#include <future>

TEST(NetworkManager, Connect)
{
    Engine::Network::NetworkManager server("testNetworkServer");

    bool done = false;

    ASSERT_EQ(server.startServer(1234), Engine::Network::NetworkManagerResult::SUCCESS) << "SocketAPI: " << server.getSocketAPIError();

    #if !EMSCRIPTEN
    auto future = std::async([&]() {
        Engine::Network::NetworkManagerResult result = server.acceptConnection(4000ms);
        while (!done) {
            server.sendMessages();
        }
        return result;
    });
    Engine::Network::NetworkManager client("testNetworkClient");

    EXPECT_EQ(client.connect("127.0.0.1", 1234, 2000ms), Engine::Network::NetworkManagerResult::SUCCESS) << "Stream-Error: " << server.getStreamError();    

    done = true;

    EXPECT_EQ(future.get(), Engine::Network::NetworkManagerResult::SUCCESS);
    #endif
}
