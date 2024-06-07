#include "mlog.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int ModuleID = -1;
static std::string ModuleName = "";

#ifdef _QNX_TARGET_
#include <sys/slog.h>
#include <sys/slogcodes.h>

int xLog(LOG_LEV_EN emLevel, const char *pFileName, const char *pFuncName,
		 int iLineNumber, const char *fmt, ...)
{
	if (ModuleID == -1)
		ModuleID = getpid();
	if (ModuleName.empty())
		ModuleName = getModuleID(ModuleID);

	if (NULL == pFileName || NULL == pFuncName || NULL == fmt)
	{
		return 0;
	}

	unsigned int i = strlen(pFileName);
	for (; i > 0; i--)
	{
		if ('/' == pFileName[i] && strlen(pFileName) != i)
		{
			i = i + 1;
			break;
		}
	}
	if (strlen(&pFileName[i]) > 256)
		return 0;

	const int BUFF_SIZE = 5120;
	char caLogBuf[BUFF_SIZE];
	int iPrefixLen = sprintf(caLogBuf,
							 " [%s] %s %s() <%d>: ",
							 ModuleName.data(), &pFileName[i], pFuncName, iLineNumber);
	if (iPrefixLen > BUFF_SIZE - 1)
		return 0;

	va_list vap;
	va_start(vap, fmt);
	vsnprintf(caLogBuf + iPrefixLen, BUFF_SIZE - iPrefixLen, fmt, vap);
	va_end(vap);

	unsigned int uiMinor = 104;
	int ret = slogf(_SLOG_SETCODE(ModuleID, uiMinor), emLevel, "%s", caLogBuf);
	if (ret == -1)
		return 0;

	return 1;
}

std::string getModuleID(int pid)
{
	char buf[512] = {0};
	static std::string ModuleName;
	int iModuleID = (pid == 0 ? getpid() : pid);
	std::string file = "/proc/" + std::to_string(iModuleID) + "/exefile";

	int fd = 0, ret = 0;
	fd = open(file.data(), O_RDONLY);
	ret = read(fd, buf, sizeof(buf));
	close(fd);
	if (ret > 0)
	{
		ModuleName = buf;
		ModuleName = ModuleName.substr(ModuleName.find_last_of("/") + 1);
	}
	return ModuleName;
}

#endif
#ifdef _LINUX_TARGET_
#include <syslog.h>
#include <fcntl.h>
#include <mutex>
class CLogClient
{
public:
	CLogClient()
	{
		if (ModuleID == -1)
			ModuleID = getpid();
		if (ModuleName.empty())
			ModuleName = getModuleID(ModuleID);
		openlog(ModuleName.data(), LOG_PID | LOG_NDELAY, LOG_USER);
	}

	~CLogClient()
	{
		closelog();
	}
};

int emptytest()
{
	return 1;
}


static CLogClient logClient;

static LOG_LEV_EN	s_LogLevel = LOG_LEV_EN::LOG_WAR;
int xLog(LOG_LEV_EN emLevel, const char *pFileName, const char *pFuncName,
		 int iLineNumber, const char *fmt, ...)
{
	if(s_LogLevel < emLevel)
	{
		return 0;
	}
	if (NULL == pFileName || NULL == pFuncName || NULL == fmt)
	{
		return 0;
	}
    
	const int BUFF_SIZE = 1024;
	char caLogBuf[BUFF_SIZE];

	va_list vap;
	va_start(vap, fmt);
	vsnprintf(caLogBuf, BUFF_SIZE, fmt, vap);
	va_end(vap);
    syslog(emLevel + 1, "%s, %s, %d, %s", pFileName, pFuncName, iLineNumber, caLogBuf);
	return 1;
}

static std::mutex mMutex;
void setPrintLevel(LOG_LEV_EN enlevel)
{
	std::lock_guard<std::mutex> lock(mMutex);
	s_LogLevel = enlevel;
}

std::string getModuleID(int pid)
{
	char buf[512] = {0};
	static std::string ModuleName;
	int iModuleID = (pid == 0 ? getpid() : pid);
	std::string file = "/proc/" + std::to_string(iModuleID) + "/cmdline";

	int fd = 0, ret = 0;
	fd = open(file.data(), O_RDONLY);
	ret = read(fd, buf, sizeof(buf));
	close(fd);
	if (ret > 0)
	{
		ModuleName = buf;
		ModuleName = ModuleName.substr(ModuleName.find_last_of("/") + 1);
	}
	return ModuleName;
}
#endif