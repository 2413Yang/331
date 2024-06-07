
#ifndef WATCHMAIN__H__
#define WATCHMAIN__H__

#include "Application.h"
#include "IPC/IPCCore.h"
#include <vector>
#include <string>
#include "json/json.h"
#include <map>
#include "DoShellCmd.hpp"
#include "simpleTimer.h"
#include "LogServiceApi.h"

class CWatchMain
    : public ZH::BaseLib::CApplication
{
public:
    struct AppInfo
    {
        std::string AppName; //待守护的app名称
        std::string AppPath; //待守护的app路径
        std::string RunParameter; //运行app的命令
        uint32_t LogLevel_0To6; //app日志输出级别
		bool ResumeFlag;
    };
    struct appSts
    {
        AppInfo appInfo;
        bool    isAlive;
        bool    isStartHeartbeat;
        uint32_t    lastHeatbeatTick;//最后一次接收到心跳的时间
        uint32_t    timeoutCount;
		uint32_t	heartbeatCount;
		uint32_t	heartbeatCount_resp;
    };
public:
    CWatchMain(int argc, char *argv[]);
    ~CWatchMain();

    void HeartbeatStart(std::string appName);
    void HeartbeatSignal_Resp(std::string appName, int count);
    void HeartbeatStop(std::string appName);
private:
    virtual void Init();
    virtual void DoWork();
    virtual void UInit();

    void CheckAlive_OnTime();
    void runApp(AppInfo& appInfo);

    void CheckLogLevel_OnTime();
private:
    CIPCConnector mIPC;
    const static char* cfgFileName;
    const static char* cLogSwitchFileName;
    const static std::string cfgVersion;
    std::vector<AppInfo> mCfgFileContentList;
    //std::vector<AppInfo>    mAppList;
    std::map<std::string, appSts>   mAppSts;
    Json::Reader mJsonReader;
    Json::Value  mJsonValue;
    ZH::BaseLib::CTimerManager<CWatchMain>  mTimerManager;
    ZH::BaseLib::CSimpleTimer<CWatchMain>   *mCheckAppAliveTimer;
    ZH::BaseLib::CSimpleTimer<CWatchMain>   *mCheckAppLogLevelTimer;
    time_t      mLogCfgFileModifyTime;
    time_t      mLogSwitchFileModifyTime;
    bool        mLogSwitchFlag;
	CIPCConnector	mLogIPC;
	bool		mbHeartbeatSwtich;
};

#endif /*WATCHMAIN__H__*/