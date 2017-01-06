#include "madginelib.h"

#include "bufferedstream.h"
#include "buffered_streambuf.h"

namespace Engine {
namespace Serialize {



	BufferedInStream::BufferedInStream(buffered_streambuf & buffer, SerializeManager & mgr) :
		SerializeInStream(mIfs, mgr),
		mBuffer(buffer),
		mIfs(&buffer),
		Stream(mgr)
	{
	}

	bool BufferedInStream::isMessageAvailable()
	{
		return mBuffer.isMessageAvailable();
	}

	buffered_streambuf * BufferedInStream::rdbuf()
	{
		return &mBuffer;
	}

	BufferedOutStream::BufferedOutStream(buffered_streambuf & buffer, SerializeManager & mgr) :
		SerializeOutStream(mOfs, mgr),
		mBuffer(buffer),
		mOfs(&buffer),
		Stream(mgr)
	{
	}

	void BufferedOutStream::beginMessage()
	{
		mBuffer.beginMessage();
	}

	void BufferedOutStream::endMessage()
	{
		mBuffer.sendMessage();
	}

	BufferedOutStream & BufferedOutStream::operator<<(BufferedInStream & in)
	{
		mOfs << in.rdbuf();
		return *this;
	}



	BufferedInOutStream::BufferedInOutStream(buffered_streambuf & buffer, SerializeManager & mgr) :
		BufferedOutStream(buffer, mgr),
		BufferedInStream(buffer, mgr),
		Stream(mgr),
		mBuffer(buffer)
	{
	}

	bool BufferedInOutStream::isValid()
	{
		return bool(*this) && !mBuffer.isClosed();
	}
}
} // namespace Scripting

