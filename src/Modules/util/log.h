#pragma once

#include "messagetype.h"

namespace Engine
{
	namespace Util
	{

		class MODULES_EXPORT Log
		{
		public:
			virtual ~Log() = default;
			virtual void log(const std::string& msg, MessageType lvl);
			virtual std::string getName() = 0;

			void addListener(LogListener* listener);
			void removeListener(LogListener* listener);

		private:
			std::list<LogListener*> mListeners;

		};
		
	}
}
