#include "../maditorsharedlib.h"

#include "boostipcserver.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Maditor {
	namespace Shared {


		
		BoostIPCServer::BoostIPCServer(boost::interprocess::managed_shared_memory::segment_manager * mgr) :
			mQueue(mgr)
		{
		}

		void BoostIPCServer::enqueue(const SharedConnectionPtr &conn, std::chrono::milliseconds timeout)
		{
			mQueue.emplace_back(conn);
		}

		SharedConnectionPtr BoostIPCServer::poll(std::chrono::milliseconds timeout)
		{

			std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

			boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock = (timeout == 0ms ?
				boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>(mMutex, boost::posix_time::second_clock::local_time() + boost::posix_time::milliseconds(timeout.count())) :
				boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>(mMutex));

			if (lock) {
				while (mQueue.empty()) {
					if (std::chrono::duration_cast<std::chrono::milliseconds>
						(std::chrono::steady_clock::now() - start) > timeout) {
						return SharedConnectionPtr();
					}
				}
				SharedConnectionPtr conn = std::move(mQueue.front());
				mQueue.pop_front();
				return conn;
			}
			else {
				return SharedConnectionPtr();
			}
		}

	}
}
