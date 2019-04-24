#pragma once

#include "../serialize/streams/buffered_streambuf.h"

#include "socketapi.h"

namespace Engine
{
	namespace Network
	{
		class INTERFACES_EXPORT NetworkBuffer :
			public Serialize::buffered_streambuf
		{
		public:
			NetworkBuffer(SocketId socket, Serialize::SerializeManager &mgr, Serialize::ParticipantId id = 0);
			NetworkBuffer(const NetworkBuffer&) = delete;
			NetworkBuffer(NetworkBuffer&& other) noexcept;
			virtual ~NetworkBuffer();

		protected:
			// Geerbt �ber buffered_streambuf
			Serialize::StreamError getError() override;

			int recv(char*, size_t) override;

			int send(char*, size_t) override;

		private:
			SocketId mSocket; // = SOCKET
		};
	}
}
