#pragma once



#include "boostipcconnection.h"

using namespace std::literals::chrono_literals;

namespace Maditor {
	namespace Shared {


		class MADITOR_SHARED_EXPORT BoostIPCServer
		{
		public:
			BoostIPCServer(boost::interprocess::managed_shared_memory::segment_manager *mgr);

			void enqueue(const SharedConnectionPtr &conn, std::chrono::milliseconds timeout);
			SharedConnectionPtr poll(std::chrono::milliseconds timeout = 0ms);

		private:
			SharedConnectionQueue mQueue;
			boost::interprocess::interprocess_mutex mMutex;
			
		};

		

	}
}

