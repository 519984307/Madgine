#include "libinclude.h"

#include "UtilMethods.h"

namespace Engine {
	namespace Util {
		std::map<const std::exception*, std::list<TraceBack>> UtilMethods::sExceptions;
		std::list<TraceBack> UtilMethods::sCurrentTraceBack;
		std::list<TraceBack> UtilMethods::sExceptionTraceBack;
		std::list<TraceBack> UtilMethods::sTraceBack;
		const std::string UtilMethods::sLogFileName = "Madgine.log";
		Ogre::Log *UtilMethods::sLog = 0;

		void UtilMethods::setup() {
			sLog = Ogre::LogManager::getSingleton().createLog(sLogFileName);
		}

		void UtilMethods::log(const std::string & msg, Ogre::LogMessageLevel level, const std::list<TraceBack> &traceBack)
		{
			sCurrentTraceBack = traceBack.empty() ? sTraceBack : traceBack;
			sLog->logMessage(msg.c_str(), level);
		}

		void UtilMethods::pushTraceBack(const TraceBack &t)
		{
			sTraceBack.push_back(t);
		}

		void UtilMethods::logException(const std::exception & e)
		{
			log(e.what(), Ogre::LML_CRITICAL, sExceptionTraceBack);
			sExceptionTraceBack.clear();
		}

		void UtilMethods::popTraceBack()
		{
			sTraceBack.pop_back();
		}

		void UtilMethods::registerException(const TraceBack & t)
		{
			sExceptionTraceBack = sTraceBack;
			if (sExceptionTraceBack.empty() 
				|| sExceptionTraceBack.back().mFile != t.mFile
				|| sExceptionTraceBack.back().mFunction != t.mFunction
				|| sExceptionTraceBack.back().mLineNr != -1)
				sExceptionTraceBack.push_back(t);
			else
				sExceptionTraceBack.back().mLineNr = t.mLineNr;
		}

		void UtilMethods::abort()
		{
			throw AbortException();
		}

		const std::list<TraceBack> &UtilMethods::traceBack() {
			return sCurrentTraceBack;
		}

	}
}