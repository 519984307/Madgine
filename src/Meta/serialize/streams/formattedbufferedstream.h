#pragma once

#include "formattedserializestream.h"

#include "Generic/lambda.h"

#include "Generic/nulledptr.h"

#include "pendingrequest.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT FormattedBufferedStream : FormattedSerializeStream {

        FormattedBufferedStream(std::unique_ptr<Formatter> format, std::unique_ptr<message_streambuf> buffer, std::unique_ptr<SyncStreamData> data);
        FormattedBufferedStream(FormattedBufferedStream &&other) = default;
        FormattedBufferedStream(FormattedBufferedStream &&other, SyncManager *mgr);

        FormattedBufferedStream &operator=(FormattedBufferedStream &&) = default;

        void beginMessageWrite();
        void beginMessageWrite(ParticipantId requester, MessageId requestId, GenericMessagePromise promise);
        void endMessageWrite();

        struct META_EXPORT MessageReadMarker {
            MessageReadMarker(MessageId id = 0, Formatter *formatter = nullptr)
                : mId(id)
                , mFormatter(formatter)
            {
            }
            MessageReadMarker(MessageReadMarker &&other) = default;
            ~MessageReadMarker();

            MessageReadMarker &operator=(MessageReadMarker &&other);

            StreamResult end();

            explicit operator bool() const;

            MessageId mId;
            Formatter *mFormatter;
        };

        StreamResult beginMessageRead(MessageReadMarker &msg);

        PendingRequest getRequest(MessageId id);

        FormattedBufferedStream &sendMessages();

        StreamResult beginHeaderRead();
        StreamResult endHeaderRead();

        void beginHeaderWrite();
        void endHeaderWrite();

    };

}
}