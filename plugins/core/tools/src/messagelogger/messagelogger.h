#pragma once

#if ENABLE_MESSAGE_LOGGING

#include "../toolbase.h"
#include "../toolscollector.h"

namespace Engine {
namespace Serialize {
    struct MessageLogger;
}
namespace Tools {

    struct MessageLogger : Tool<MessageLogger> {
        SERIALIZABLEUNIT(MessageLogger);

        MessageLogger(ImRoot &root);
        ~MessageLogger();

        virtual void render() override;

        std::string_view key() const override;

    private:
        Serialize::MessageLogger &mLogger;

        std::map<Serialize::buffered_streambuf *, bool> mVisibleStreams;
    };

}
}

RegisterType(Engine::Tools::MessageLogger);

#endif