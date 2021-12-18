#pragma once

#include "../toolscollector.h"
#include "../toolbase.h"

#include "Interfaces/util/loglistener.h"


namespace Engine {
namespace Tools {

    struct LogViewer : Tool<LogViewer>, Util::LogListener {
        SERIALIZABLEUNIT(LogViewer);

        LogViewer(ImRoot &root);
        ~LogViewer();

        virtual void render() override;
        virtual void renderStatus() override;

        virtual void messageLogged(std::string_view message, Util::MessageType lml, const Debug::StackTrace<32> &stackTrace, Util::Log *log) override;

        std::string_view key() const override;

    private:
        struct LogEntry {
            std::string mMsg;
            Util::MessageType mType;
        };
        std::deque<LogEntry> mEntries;
        std::array<size_t, Util::MessageType::COUNT> mMsgCounts;
        std::array<bool, Util::MessageType::COUNT> mMsgFilters;
        std::mutex mMutex;
    };

}
}

RegisterType(Engine::Tools::LogViewer);