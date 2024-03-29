#pragma once

#include "messagetype.h"

#define ENABLE_DEBUG_LOGGING 0

namespace Engine {
namespace Util {

    INTERFACES_EXPORT void setLog(Log *log = nullptr);
    INTERFACES_EXPORT void log(std::string_view msg, MessageType lvl, const char *file = nullptr);

    struct INTERFACES_EXPORT LogDummy {
        LogDummy(MessageType lvl, const char *file = nullptr);
        ~LogDummy();

        template <typename T>
        LogDummy &operator<<(T &&t)
        {
            mStream << std::forward<T>(t);
            return *this;
        }

    private:
        std::ostringstream mStream;
        MessageType mLvl;
        const char *mFile;
    };

#if ENABLE_DEBUG_LOGGING
#    define LOG_DEBUG(s) Engine::Util::LogDummy { Engine::Util::MessageType::DEBUG_TYPE } << s
#else
#    define LOG_DEBUG(s)
#endif
#define LOG(s) Engine::Util::LogDummy { Engine::Util::MessageType::INFO_TYPE, __FILE__ } << s
#define LOG_WARNING(s) Engine::Util::LogDummy { Engine::Util::MessageType::WARNING_TYPE, __FILE__ } << s
#define LOG_ERROR(s) Engine::Util::LogDummy { Engine::Util::MessageType::ERROR_TYPE, __FILE__ } << s

#define ONCE(f) std::call_once([]() -> std::once_flag & {static std::once_flag dummy; return dummy; }(), [&]() { f; })

#define LOG_ONCE(s) ONCE(LOG(s))
#define LOG_WARNING_ONCE(s) ONCE(LOG_WARNING(s))
#define LOG_ERROR_ONCE(s) ONCE(LOG_ERROR(s))

}
}
