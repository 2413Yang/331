#ifndef _LOGSERVICE_H_
#define _LOGSERVICE_H_
#include "LogServiceApi.h"
#include <vector>
#include <thread>
#include "TemplateCache.hpp"

namespace ZH
{
namespace logService
{
    class CLogService
    {
    private:
        CIPCConnector  	mIPC;
		std::vector<std::string>	mLogfileNameVec;
		std::thread		mThread;
		bool			mbExitThread;
		ZH::BaseLib::CTemplateCache<std::string>	mLogCache;
		FILE*			mFp;
		uint32_t		mCurFileLinesCount;
		int				mLogCtrlExportValue;
		int				mLogCtrlSwitchValue;
		int				mLogCtrlExportOptValue;

		const static uint32_t	cMaxFileCount;
		const static uint32_t	cFileMaxLength;
		const static EnLogLevel	cEnLogLevel;
		const static uint32_t	cMaxFileLines;
		const static char*		cStrLogSwitchName;
	private:
		void initLogFile();
		void threaFunc();
		void checkLogFile();
	public:
		const static char* cStrLogPath;
        const static char* cStrLogFilePrefix;
		const static char* cStrLogFileSuffix;
    public:
        CLogService();
        ~CLogService();
        void LogRecord(std::string moudleName, EnLogLevel level, std::string log);
		void LogIVICtrl(int ctrl);
    };

}//namespace logService
}//namespace ZH


#endif //!_LOGSERVICE_H_