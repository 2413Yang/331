#include "logService.h"
#include <unistd.h>
#include "stdlib.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <list>
#include <algorithm>
#include "Application.h"
#include "DoShellCmd.hpp"
#include <time.h>
#include <stdio.h>
#include <sys/time.h>

using namespace ZH::logService;

int main(int argc, char* argv[])
{
	printf("start program name:%s\n", argv[0]);
	CLogService logService;
	while (1)
	{
		sleep(10);
	}
    return 0;
}

CLogService::CLogService():
mIPC("LogService"),
mThread(&CLogService::threaFunc, this)
{
	LogRecord("LogService", EnLogLevel::LOGLEVEL_INFO, "\n\n#####################start LogService################\n");
    mFp = nullptr;
    subscriber::LogRecord(mIPC, *this);
	subscriber::LogIVICtrl(mIPC, *this);
	publisher::LogIVICtrl_Resp(mIPC);
    mIPC.start();
	mbExitThread = false;
	mCurFileLinesCount = 0;

	mLogCtrlExportValue = 0;
	mLogCtrlSwitchValue = 0;
}
CLogService::~CLogService()
{
    mbExitThread = true;
	mThread.join();
}

void CLogService::initLogFile()
{
	if(access(cStrLogPath, F_OK) == -1)
    {
        mkdir(cStrLogPath, 0766);
    }
	else
	{
		DIR *directory_pointer;
		struct dirent *entry;
		directory_pointer=opendir(CLogService::cStrLogPath);
		while((entry=readdir(directory_pointer))!=NULL)
		{
			if(entry->d_type & DT_REG) //查找文件
			{
				if(memcmp(cStrLogFilePrefix, entry->d_name, strlen(cStrLogFilePrefix)) == 0)
				{
					mLogfileNameVec.push_back(std::string(entry->d_name));
					//printf("line:%d, name = %s\n", __LINE__, entry->d_name);
				}
			}
		}
	}
	if(mLogfileNameVec.empty())
	{
		std::string firstfile = CLogService::cStrLogFilePrefix;
		firstfile += "0.txt";
		mLogfileNameVec.push_back(firstfile);
		FILE* fp = fopen(std::string(cStrLogPath + firstfile).c_str(), "w");
		fclose(fp);
	}
	else
	{
		std::sort(mLogfileNameVec.begin(), mLogfileNameVec.end(), [](const std::string& a, const std::string& b)->bool{
			int startOffset = strlen(CLogService::cStrLogFilePrefix);
			int stopOffset = a.find(".");
			std::string aNum = a.substr(startOffset, (stopOffset - startOffset));
			int aNumber = atoi(aNum.c_str());
			//
			stopOffset = b.find(".");
			std::string bNum = b.substr(startOffset, (stopOffset - startOffset));
			int bNumber = atoi(bNum.c_str());
			return aNumber < bNumber;
		});
		#if 0	
		printf("line:%d, sort result:\n", __LINE__);
		for(auto iter : mLogfileNameVec)
		{
			printf(" name = %s\n", iter.c_str());
		}
		#endif
	}
	mCurFileLinesCount = 0;
	this->checkLogFile();
	//printf("%s,%d, currentFile = %s, mFp = %p\n", __func__, __LINE__, currentFile, mFp);
	char cfgName[128];
	snprintf(cfgName, sizeof(cfgName), "/opt/data/%s",cStrLogSwitchName);
	if(access(cfgName, F_OK) == -1)
	{
		FILE* fp = fopen(cfgName, "w");
		fprintf(fp, "1");
		fsync(fileno(fp));
		fclose(fp);
		mLogCtrlSwitchValue = 1;
	}
	else
	{
		char c = 0;
		FILE* fp = fopen(cfgName, "r");
		fwrite(&c, 1,1,fp);
		mLogCtrlSwitchValue = static_cast<int>(c);
		fclose(fp);
	}
}

void CLogService::checkLogFile()
{//检测日志文件的个数是否超过最大文件个数
	char currentFile[256];
	sprintf(currentFile, "%s%s", cStrLogPath, mLogfileNameVec.back().data());
	FILE* fp = fopen(currentFile, "r");
	long fLen = 0;
	if(fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		fLen = ftell(fp);
		fclose(fp);
	}
	if((fLen > long(cFileMaxLength / 2)) || 
		(mCurFileLinesCount >= cMaxFileLines))
	{
		std::string newfile = CLogService::cStrLogFilePrefix;
		newfile += std::to_string(mLogfileNameVec.size()) + cStrLogFileSuffix;
		sprintf(currentFile, "%s%s", cStrLogPath, newfile.c_str());
		FILE* fp = fopen(currentFile, "w");
		if(fp)
		{
			fclose(fp);
		}
		mLogfileNameVec.push_back(newfile);
	}
	uint32_t vecSize = mLogfileNameVec.size();
	if(vecSize >= cMaxFileCount)
	{
		int i = 0;
		for(auto iter = mLogfileNameVec.begin(); iter != mLogfileNameVec.end(); i++)
		{
			if(i < int(vecSize - cMaxFileCount))
			{
				std::string name(cStrLogPath + *iter);
				int ret = remove(name.c_str());
				//printf("%d, remove:%s, ret = %d\n", __LINE__, name.c_str(), ret);
				if(-1 == ret)
				{
					perror("remove");
				}
				iter = mLogfileNameVec.erase(iter);
			}
			else
			{
				break;
			}
		}
		i = 0;
		for(auto iter = mLogfileNameVec.begin(); iter != mLogfileNameVec.end(); iter++, ++i)
		{
			std::string newName = cStrLogFilePrefix;
			newName += std::to_string(i) + cStrLogFileSuffix;
			std::string oldName = std::string(cStrLogPath + *iter);
			int ret = rename(oldName.c_str(), std::string(cStrLogPath + newName).c_str());
			//printf("line:%d, oldName:%s, rename[%d]:%s, ret = %d\n", __LINE__, oldName.c_str(), i, std::string(cStrLogPath + newName).c_str(), ret);
			if(-1 == ret)
			{
				perror("rename");
			}
			*iter = newName;
		}
	}
	sprintf(currentFile, "%s%s", cStrLogPath, mLogfileNameVec.back().data());
	mFp = fopen(currentFile, "a");
}

void CLogService::LogRecord(std::string moudleName, EnLogLevel level, std::string log)
{
	if(cEnLogLevel < level)
	{
		return;
	}
	if(mLogCtrlSwitchValue == 0x02)
	{
		return;
	}
	char timeStr[64];
	#if 0
	time_t now;
	struct tm *ptm;
	time (&now);
	//获取当地日期和时间
	ptm = localtime (&now);
	snprintf(timeStr, sizeof(timeStr), "[%02d:%02d:%02d][%s]", ptm->tm_hour, ptm->tm_min, ptm->tm_sec, moudleName.c_str());
	#else
	int timeMs = ZH::BaseLib::getWorldTimeMS();
	int hour = (timeMs / (60*60*1000)) % 24;
	int min = (timeMs / (60*1000)) % 60;
	int sec = (timeMs / 1000) % 60;
	snprintf(timeStr, sizeof(timeStr), "[%02d:%02d:%02d.%03d][%s]", hour, min, sec,(timeMs % 1000), moudleName.c_str());
	#endif
    mLogCache.push(std::string(timeStr + log).c_str());
}

void CLogService::threaFunc()
{
	sleep(2);
	initLogFile();
	while (!mbExitThread)
	{
		mLogCache.wait();
		std::string log;
		while (mLogCache.pull(log))
		{
			if(mFp)
			{
				if(log.c_str()[log.size() - 1] != '\n')
				{
					log +="\n";
				}
				fprintf(mFp, "%s", log.c_str());
				mCurFileLinesCount++;
			}
		}
		if(mFp)
		{
			fflush(mFp);
			fsync(fileno(mFp));
			if(mCurFileLinesCount > cMaxFileLines)
			{
				fclose(mFp);
				mFp = nullptr;
				this->checkLogFile();
				mCurFileLinesCount = 0;
			}
		}
		//
		#if 1
		CDoShellCmd docmd;	
		if(mLogCtrlExportOptValue == 1)
		{//日志导出请求
			LOGINF("%s,%d\n", __func__, __LINE__);
			time_t now;
			struct tm *ptm;
			time (&now);
			//获取当地日期和时间
			ptm = localtime (&now);
			char cmdStr[256];
			char mileageLogFile[] = "/opt/data/mileageLog.txt";
			if(access(mileageLogFile, F_OK) == -1)
			{
				mileageLogFile[0] = '\0';
			}
			snprintf(cmdStr, sizeof(cmdStr), "fsg off && rm -rf /run/ota/ZH-IC-Log* && tar -zcf /run/ota/ZH-IC-Log_%d-%d-%d_%d-%d.tar ZH-IC-Log %s -C /opt/data/ && fsg on", 
				(ptm->tm_year + 1900), (ptm->tm_mon + 1), ptm->tm_mday, ptm->tm_hour, ptm->tm_min, mileageLogFile);
			LOGINF("%s,%d\n", __func__, __LINE__);
			docmd(cmdStr);
			LogIVICtrl_Resp(2);//通知中控，日志导出完成
		}
		else if(mLogCtrlExportOptValue == 2)
		{//日志导出完成
			docmd("fsg off");
		}
		else if(mLogCtrlExportOptValue == 3)
		{//日志导出失败
			docmd("fsg off");
		}
		mLogCtrlExportOptValue = 0;
		#endif
		usleep(5*1000);
		sched_yield();
	}
	
}

void CLogService::LogIVICtrl(int ctrl)
{
	LOGINF("%s,%d, ctrl = 0x%02x\n", __func__, __LINE__, ctrl);
	int ctrlExport = ctrl & 0x0f;
	int ctrlSwitch = (ctrl >> 4) & 0x0f;
	if(ctrlSwitch && (ctrlSwitch != mLogCtrlSwitchValue))
	{
		mLogCtrlSwitchValue = ctrlSwitch;
		LogIVICtrl_Resp(ctrlSwitch);
	}
	if(ctrlExport && (ctrlExport != mLogCtrlExportValue))
	{
		LOGINF("%s,%d, ctrl = 0x%02x\n", __func__, __LINE__, ctrl);
		mLogCtrlExportValue = ctrlExport;
		mLogCtrlExportOptValue = ctrlExport;
		char log[128];
		snprintf(log, sizeof(log), "%s,ctrl = %d\n", __func__, ctrl);
		mLogCache.push(log);
	}
	
}

const char* CLogService::cStrLogPath = "/opt/data/ZH-IC-Log/";
const char* CLogService::cStrLogFilePrefix = "ZH-Log-";
const char* CLogService::cStrLogFileSuffix = ".txt";
const uint32_t CLogService::cMaxFileCount = 10;
const uint32_t CLogService::cFileMaxLength = 2*1024*1024;
const EnLogLevel CLogService::cEnLogLevel = EnLogLevel::LOGLEVEL_DEBUG;
const uint32_t	CLogService::cMaxFileLines = cFileMaxLength / 30;
const char* CLogService::cStrLogSwitchName = "LogFileSwitch.cfg";