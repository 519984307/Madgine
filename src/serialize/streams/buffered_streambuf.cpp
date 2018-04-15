#include "../../interfaceslib.h"
#include "buffered_streambuf.h"


namespace Engine
{
	namespace Serialize
	{
		constexpr size_t buffered_streambuf::BUFFER_SIZE;

		buffered_streambuf::buffered_streambuf() :
			mIsClosed(false),
			mBytesToRead(sizeof(BufferedMessageHeader))
		{
			extend();
		}

		buffered_streambuf::buffered_streambuf(buffered_streambuf&& other) noexcept :
			//basic_streambuf<char>(other),
			mIsClosed(other.mIsClosed),
			mBytesToRead(other.mBytesToRead),
			mReceiveMessageHeader(other.mReceiveMessageHeader),
			mRecBuffer(std::forward<std::unique_ptr<char[]>>(other.mRecBuffer)),
			mSendBuffer(std::forward<std::list<std::array<char, BUFFER_SIZE>>>(other.mSendBuffer)),
			mStagedSendBuffer(std::forward<std::list<std::array<char, BUFFER_SIZE>>>(other.mStagedSendBuffer)),
			mBufferedSendMsgs(std::forward<std::list<BufferedSendMessage>>(other.mBufferedSendMsgs))
		{
			setg(other.eback(), other.gptr(), other.egptr());
			setp(other.pbase(), other.epptr());
			other.mIsClosed = true;

			//setp(mSendBuffer.back().data(), mSendBuffer.back().data() + (other.pptr() - other.pbase()), mSendBuffer.back().data() + BUFFER_SIZE);
		}

		buffered_streambuf::~buffered_streambuf()
		{
		}

		buffered_streambuf::pos_type buffered_streambuf::seekoff(off_type off, std::ios_base::seekdir dir,
		                                                         std::ios_base::openmode mode)
		{
			if (mode & std::ios_base::in)
			{
				switch (dir)
				{
				case std::ios_base::beg:
					if (0 <= off && eback() + off <= egptr())
					{
						setg(eback(), eback() + off, egptr());
						return pos_type(off);
					}
					break;
				case std::ios_base::cur:
					if (eback() <= gptr() + off && gptr() + off <= egptr())
					{
						setg(eback(), gptr() + off, egptr());
						return pos_type(gptr() - eback());
					}
					break;
				case std::ios_base::end:
					if (eback() <= egptr() + off && off <= 0)
					{
						setg(eback(), egptr() + off, egptr());
						return pos_type(gptr() - eback());
					}
				}
			}

			return pos_type(off_type(-1));
		}

		buffered_streambuf::pos_type buffered_streambuf::seekpos(pos_type pos, std::ios_base::openmode mode)
		{
			if (0 <= pos && eback() + pos <= egptr())
			{
				setg(eback(), eback() + pos, egptr());
				return pos;
			}
			return pos_type(off_type(-1));
		}


		bool buffered_streambuf::isClosed()
		{
			return mIsClosed;
		}

		void buffered_streambuf::close(StreamError cause)
		{
			if (!mIsClosed)
			{
				mIsClosed = true;
				mCloseCause = cause;
			}
		}

		void buffered_streambuf::extend()
		{
			mSendBuffer.emplace_back();
			setp(mSendBuffer.back().data(), mSendBuffer.back().data() + BUFFER_SIZE);
		}

		bool buffered_streambuf::isMessageAvailable(const std::string& context)
		{
			if (mIsClosed)
				return false;
			if (mRecBuffer && mBytesToRead == 0 && gptr() == eback())
				return true;
			receive(context);
			return mRecBuffer && mBytesToRead == 0 && gptr() == eback();
		}


		buffered_streambuf::int_type buffered_streambuf::overflow(int c)
		{
			if (c != EOF)
			{
				extend();
				*pptr() = c;
				pbump(1);
				return c;
			}
			return traits_type::eof();
		}

		int buffered_streambuf::sync()
		{
			return sendMessages() == 0 ? 0 : -1; // TODO return value
		}

		void buffered_streambuf::beginMessage()
		{
			if (mIsClosed)
				return;
			if (mSendBuffer.size() != 1 || pptr() != pbase())
				throw 0;
		}

		void buffered_streambuf::endMessage()
		{
			if (mIsClosed)
				return;

			BufferedSendMessage msg;
			msg.mHeaderSent = false;
			msg.mHeader.mMsgSize = (mSendBuffer.size() - 1) * BUFFER_SIZE + (pptr() - pbase());
			mBufferedSendMsgs.emplace_back(msg);

			mStagedSendBuffer.splice(mStagedSendBuffer.end(), mSendBuffer);

			extend();
		}

		int buffered_streambuf::sendMessages()
		{
			if (mIsClosed)
				return -1;
			for (auto it = mBufferedSendMsgs.begin(); it != mBufferedSendMsgs.end(); it = mBufferedSendMsgs.erase(it))
			{
				if (!it->mHeaderSent)
				{
					int num = send(reinterpret_cast<char*>(&it->mHeader), sizeof it->mHeader);
					if (num == -1)
					{
						handleError();
						return mIsClosed ? -1 : mBufferedSendMsgs.size();
					}
					if (num != sizeof it->mHeader)
					{
						throw 0;
					}
					it->mHeaderSent = true;
				}
				auto it2 = mStagedSendBuffer.begin();
				while (it->mHeader.mMsgSize > 0)
				{
					if (it2 == mStagedSendBuffer.end())
						throw 0;
					size_t count = std::min(it->mHeader.mMsgSize, BUFFER_SIZE);
					int num = send(it2->data(), count);
					if (num == 0)
					{
						close();
						return -1;
					}
					if (num == -1)
					{
						handleError();
						return mIsClosed ? -1 : mBufferedSendMsgs.size();
					}
					if (static_cast<size_t>(num) != count)
					{
						throw 0;
					}
					it->mHeader.mMsgSize -= count;
					it2 = mStagedSendBuffer.erase(it2);
				}
			}
			return 0;
		}

		StreamError buffered_streambuf::closeCause() const
		{
			return mCloseCause;
		}

		buffered_streambuf::int_type buffered_streambuf::underflow()
		{
			return traits_type::eof();
		}

		void buffered_streambuf::receive(const std::string& context)
		{
			if (mRecBuffer && mBytesToRead == 0)
			{
				if (gptr() != egptr())
				{
					LOG_WARNING(Exceptions::messageNotFullyRead(context));
				}
				mRecBuffer.reset();
				mBytesToRead = sizeof mReceiveMessageHeader;
			}
			if (!mRecBuffer)
			{
				assert(mBytesToRead > 0);
				int num = rec(reinterpret_cast<char*>(&mReceiveMessageHeader + 1) - mBytesToRead, mBytesToRead);
				if (num == 0)
				{
					close();
					return;
				}
				if (num == -1)
				{
					handleError();
					return;
				}
				mBytesToRead -= num;
				if (mBytesToRead == 0)
				{
					mBytesToRead = mReceiveMessageHeader.mMsgSize;
					mRecBuffer = std::make_unique<char[]>(mBytesToRead);
				}
			}

			if (mRecBuffer && mBytesToRead > 0)
			{
				int num = rec(mRecBuffer.get() + mReceiveMessageHeader.mMsgSize - mBytesToRead, mBytesToRead);
				if (num == 0)
				{
					close();
					return;
				}
				if (num == -1)
				{
					handleError();
					return;
				}
				mBytesToRead -= num;
				if (mBytesToRead == 0)
				{
					setg(mRecBuffer.get(), mRecBuffer.get(), mRecBuffer.get() + mReceiveMessageHeader.mMsgSize);
				}
			}
		}

		void buffered_streambuf::handleError()
		{
			int error = getError();
			switch (error)
			{
			case WOULD_BLOCK:
				break;
			default:
				close(UNKNOWN_ERROR);
			}
		}
	} // namespace Serialize
} // namespace Core