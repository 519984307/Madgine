#pragma once

#include "serializestream.h"

namespace Engine
{
	namespace Serialize
	{
		struct INTERFACES_EXPORT BufferedInStream : SerializeInStream
		{
			friend struct BufferedOutStream;

			BufferedInStream(std::unique_ptr<buffered_streambuf> &&buffer);
			BufferedInStream(BufferedInStream &&other);

			bool isMessageAvailable() const;

			void readHeader(MessageHeader& header);

		protected:
			BufferedInStream(buffered_streambuf *buffer);

			buffered_streambuf &buffer() const;
		};

		struct INTERFACES_EXPORT BufferedOutStream : SerializeOutStream
		{
		public:
			BufferedOutStream(std::unique_ptr<buffered_streambuf> &&buffer);
			BufferedOutStream(BufferedOutStream &&other);

			void beginMessage(SerializableUnitBase* unit, MessageType type);
			void beginMessage(Command cmd);
			void endMessage();

			int sendMessages();

			BufferedOutStream& operator<<(BufferedInStream& in);
			using SerializeOutStream::operator<<;

			template <class... _Ty>
			void writeCommand(Command cmd, const _Ty&... args)
			{
				beginMessage(cmd);

				(void)(*this << ... << args);

				endMessage();
			}

		protected:
			buffered_streambuf &buffer() const;
		};

		struct INTERFACES_EXPORT BufferedInOutStream :
			BufferedInStream,
			BufferedOutStream
		{
			friend struct SerializeManager;

			BufferedInOutStream(std::unique_ptr<buffered_streambuf> &&buffer);
			BufferedInOutStream(BufferedInOutStream &&other);

			StreamError error() const;
			bool isClosed() const;
			void close();

			explicit operator bool() const;

			using BufferedInStream::id;

		protected:
			using BufferedInStream::buffer;
		};
	}
} // namespace Scripting
