
#include "FileL.hpp"
#include "WatchMain.h"
#include "WatchInterface.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include "mylogCtrl.h"
#include <fstream>
#include <sys/wait.h>
#include <sstream>
#include <thread>
#include "single_instance.h"
#include <sys/stat.h>
#include <sys/reboot.h>

int main(int argc, char *argv[])
{
    if(proc_is_exist("WatchService"))
    {
        return 0;
    }
    else
    {
        printf("\n11 [%s] build time: %s %s\n", argv[0], __DATE__, __TIME__);
    }
    CWatchMain app(argc, argv);
    #if 0
    //system("/usr/sbin/zhapp/midware/bin/UpgradeService &");
    #else
    //system("/usr/sbin/zhapp/midware/bin/UpgradeService >/dev/null 2>&1 &");
    #endif
    app.Start(true);
    return EXIT_SUCCESS;
}

CWatchMain::CWatchMain(int argc, char *argv[]):mIPC("WatchService"),mLogIPC("LogService")
{
    LOG_SDEBUG("%s\n", __func__);
    (void)argc,(void)argv;

	AppInfo logServiceInfo;
    logServiceInfo.AppName = "LogService";
    logServiceInfo.AppPath = "/usr/sbin/zhapp/midware/bin";
    logServiceInfo.RunParameter = "";
    logServiceInfo.LogLevel_0To6 = 3;
	logServiceInfo.ResumeFlag = true;
	mCfgFileContentList.push_back(logServiceInfo);

    AppInfo upgradServiceInfo;
    upgradServiceInfo.AppName = "UpgradeService";
    upgradServiceInfo.AppPath = "/usr/sbin/zhapp/midware/bin";
    
    upgradServiceInfo.LogLevel_0To6 = 3;
	upgradServiceInfo.ResumeFlag = true;
    //
	upgradServiceInfo.RunParameter = "";
    runApp(upgradServiceInfo);//先不带-r参数启动
	upgradServiceInfo.RunParameter = "-r";
    mCfgFileContentList.push_back(upgradServiceInfo);
    mCfgFileContentList.push_back({"MsgService", "/usr/sbin/zhapp/midware/bin", "-r", 3, true});
    mCfgFileContentList.push_back({"kanzi.sh", "/usr/sbin/zhapp", "", 3, true});
    
	runApp(logServiceInfo);
	
    // HUMsgService加入配置文件
    AppInfo huMsgServiceInfo;
    huMsgServiceInfo.AppName = "HUMsgService";
    huMsgServiceInfo.AppPath = "/usr/sbin/zhapp/midware/bin";
    huMsgServiceInfo.RunParameter = "";
    huMsgServiceInfo.LogLevel_0To6 = 3;
    huMsgServiceInfo.ResumeFlag = true;
    mCfgFileContentList.push_back(huMsgServiceInfo);
    runApp(huMsgServiceInfo);

	AppInfo chimeServiceInfo;
    chimeServiceInfo.AppName = "ChimeService";
    chimeServiceInfo.AppPath = "/usr/sbin/zhapp/midware/bin";
    chimeServiceInfo.LogLevel_0To6 = 3;
	chimeServiceInfo.ResumeFlag = true;
	chimeServiceInfo.RunParameter = "";
	mCfgFileContentList.push_back(chimeServiceInfo);
    runApp(chimeServiceInfo);

	mbHeartbeatSwtich = true;
}

CWatchMain::~CWatchMain()
{
}

void CWatchMain::Init()
{
	while (1)
	{
		CDoShellCmd doshell;
		std::string retStr = doshell("df | grep /opt/data");
        if(std::string::npos != retStr.find("/opt/data"))
		{
			break;
		}
		usleep(300*1000);
	}
	
    mCheckAppAliveTimer = new ZH::BaseLib::CSimpleTimer<CWatchMain>(2*1000, &CWatchMain::CheckAlive_OnTime, this, true);
    mTimerManager.addNode(mCheckAppAliveTimer);
    mAppSts.clear();

    auto funcCreateCfgFile = [this]()->void{
        FILE* fp = fopen(CWatchMain::cfgFileName, "w");
        if(fp)
        {
            mJsonValue["watch_Version"] = CWatchMain::cfgVersion;
            for(auto str : mCfgFileContentList)
            {
                Json::Value appValue;
                LOG_SDEBUG("AppName = %s,AppPath = %s, RunParameter = %s, ResumeFlag = %d\n", str.AppName.c_str(), str.AppPath.c_str(), str.RunParameter.c_str(),str.ResumeFlag);
                appValue["AppName"] = str.AppName;
                appValue["AppPath"] = str.AppPath;
                appValue["RunParameter"]  = str.RunParameter;
                appValue["LogLevel_0To6"] = str.LogLevel_0To6;
				appValue["ResumeFlag"] = str.ResumeFlag;
                this->mJsonValue[str.AppName] = appValue;
                appSts app;
                app.appInfo = str;
                app.isAlive = false;
                app.isStartHeartbeat = false;
				app.heartbeatCount = 0;
                this->mAppSts[str.AppName] = app;
            }
            std::string code(this->mJsonValue.toStyledString());
            fwrite(code.c_str(), code.size(), 1, fp);
            fflush(fp);
            int fd = fileno(fp);
            if(fd != -1)
            {
                fsync(fd);
            }
            fclose(fp);
        }
    };
    if(access(CWatchMain::cfgFileName, F_OK) == -1)
    {
        LOG_SDEBUG("%s, line = %d\n", __func__, __LINE__);
        funcCreateCfgFile();
    }
    else
    {
        LOG_SDEBUG("%s, line = %d\n", __func__, __LINE__);
        bool bNeedModifyCfgFile = false;
        std::ifstream ifs;
        ifs.open(CWatchMain::cfgFileName, std::ios::binary);
        if(mJsonReader.parse(ifs, mJsonValue, false))
        {
            LOG_SDEBUG("%s, %d, mJsonValue.size = %d, mJsonValue = %s\n",__func__, __LINE__, mJsonValue.size(), mJsonValue.toStyledString().c_str());
            Json::Value::Members mem = mJsonValue.getMemberNames();
            for(auto iter = mem.begin(); iter != mem.end(); iter++)
            {
                LOG_SDEBUG("%s, line = %d, (*iter) = %s\n", __func__, __LINE__, (*iter).c_str());
                if((*iter) == "watch_Version")
                {
                    if(mJsonValue[(*iter)].asString() != CWatchMain::cfgVersion)
                    {
                        LOG_SINFO("%s,%d, (fileVersion(%s) != codeVersion(%s)),need modify cfg file\n",__func__, __LINE__, mJsonValue[(*iter)].asString().c_str(), CWatchMain::cfgVersion.c_str());
                        bNeedModifyCfgFile = true;
                        mAppSts.clear();
                        this->mJsonValue.clear();
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    if(mJsonValue[(*iter)].isMember("AppName") &&
                        mJsonValue[(*iter)].isMember("AppPath") &&
                        mJsonValue[(*iter)].isMember("RunParameter") &&
                        mJsonValue[(*iter)].isMember("LogLevel_0To6") &&
						mJsonValue[(*iter)].isMember("ResumeFlag") )
                    {
                        appSts app;
                        app.appInfo.AppName = mJsonValue[(*iter)]["AppName"].asString();
                        app.appInfo.AppPath = mJsonValue[(*iter)]["AppPath"].asString();
                        app.appInfo.RunParameter = mJsonValue[(*iter)]["RunParameter"].asString();
                        app.appInfo.LogLevel_0To6 = mJsonValue[(*iter)]["LogLevel_0To6"].asUInt();
						app.appInfo.ResumeFlag = mJsonValue[(*iter)]["ResumeFlag"].asBool();
                        app.isAlive = false;
                        app.isStartHeartbeat = false;
						app.heartbeatCount = 0;
                        mAppSts[app.appInfo.AppName] = app;
                        LOG_SDEBUG("appName = %s,AppPath = %s, RunParameter = %s logLevel = %d, ResumeFlag = %d\n", 
                        app.appInfo.AppName.c_str(), app.appInfo.AppPath.c_str(), app.appInfo.RunParameter.c_str(), app.appInfo.LogLevel_0To6, app.appInfo.ResumeFlag);
                    }
                }
                
            }
        }
        ifs.close();
        if(bNeedModifyCfgFile)
        {
            funcCreateCfgFile();
        }
    }
    //
    do
    {
        if(access(CWatchMain::cLogSwitchFileName, F_OK) == -1)
        {
            int fd = open(CWatchMain::cLogSwitchFileName, O_CREAT | O_RDWR);
            if(fd > 0)
            {
                char ch = '0';
                write(fd, &ch, 1);
                close(fd);
            }
            mLogSwitchFlag = false;
        }
        else
        {
            int fd = open(CWatchMain::cLogSwitchFileName, O_RDONLY);
            if(fd > 0)
            {
                char ch = '0';
                read(fd, &ch, 1);
                if(ch == '1')
                {
                    mLogSwitchFlag = true;
                }
                else
                {
                    mLogSwitchFlag = false;
                }
                close(fd);
            }
            else
            {
                mLogSwitchFlag = false;
            }
        }
        
    } while (0);
    
	#if 0
	struct stat tempStat;
    int retStat = -1;
	retStat = stat(CWatchMain::cfgFileName, &tempStat);
    if(retStat == 0)
    {
        mLogCfgFileModifyTime = tempStat.st_mtim.tv_sec;
    }
    else
    {
        mLogCfgFileModifyTime = 0;
    }
	
    retStat = stat(CWatchMain::cLogSwitchFileName, &tempStat);
    if(retStat == 0)
    {
        mLogSwitchFileModifyTime = tempStat.st_mtim.tv_sec;
    }
    else
    {
        mLogSwitchFileModifyTime = 0;
    }
	#else
	mLogCfgFileModifyTime = 0;
	mLogSwitchFileModifyTime = 0;
	#endif
    mCheckAppLogLevelTimer = new ZH::BaseLib::CSimpleTimer<CWatchMain>(2*1000, &CWatchMain::CheckLogLevel_OnTime, this, true);
    mTimerManager.addNode(mCheckAppLogLevelTimer);
    for(auto iter = mAppSts.begin(); iter != mAppSts.end();)
    {
        std::string fullPath = iter->second.appInfo.AppPath + "/" + iter->second.appInfo.AppName;
        if((access(fullPath.c_str(), F_OK) == -1) ||
            (access(fullPath.c_str(), X_OK) == -1))
        {//若配置的文件不存在或没有执行权限，则从守护列表删除
            LOG_SINFO("%s,%d, mApplist erase:%s\n", __func__, __LINE__, iter->first.c_str());
            iter = mAppSts.erase(iter);
        }
        else
        {
            iter++;
        }
    }
    WatchIpc::subscriber::HeartbeatStart(mIPC, *this);
    WatchIpc::subscriber::HeartbeatSignal_Resp(mIPC, *this);
    WatchIpc::subscriber::HeartbeatStop(mIPC, *this);
    WatchIpc::publisher::LogChannelControl(mIPC);
	WatchIpc::publisher::LogLevelControl(mIPC);
	WatchIpc::publisher::HeartbeatSignal(mIPC);
	WatchIpc::publisher::NotifyMcuReboot(mIPC);
    mIPC.start();
    if(!mAppSts.empty())
    {
        mCheckAppAliveTimer->start();
        mCheckAppLogLevelTimer->start();
    }
	ZH::logService::publisher::LogRecord(mLogIPC);
	mLogIPC.start();
}

void CWatchMain::DoWork()
{
    int huMsgServiceLaunchDelay = (40 * 1000) * 1000;
    int loopDelay = 100 * 1000;

    while (true)
    {
        mTimerManager.MainCheckTimerOut();
        uint32_t nowTick = ZH::BaseLib::getWorldTimeMS();
        for (auto iter = mAppSts.begin(); iter != mAppSts.end(); iter++)
        {
			if(false == mbHeartbeatSwtich)
			{
				iter->second.timeoutCount = 0;
				break;
			}
            if(iter->second.isStartHeartbeat)
            {
                uint32_t deltaTick = nowTick - iter->second.lastHeatbeatTick;
                //LOG_SDEBUG("%d, appName = %s, deltaTick = %u\n", __LINE__, iter->first.c_str(), deltaTick);
                if(deltaTick >= 1*1000)
                {
					iter->second.lastHeatbeatTick = nowTick;
					if((iter->second.heartbeatCount + 1) != iter->second.heartbeatCount_resp)
                    {//app 心跳超时
						LOG_SDEBUG("%s,%d, timeoutCount = %d\n", __func__, __LINE__, iter->second.timeoutCount);
						iter->second.timeoutCount++;
						if(iter->second.timeoutCount > 10)
						{
							CDoShellCmd doshell;
							if(iter->second.appInfo.AppName == "kanzi.sh")
							{
								doshell("cp  /tmp/hmi.log /opt/data/");
							}
							iter->second.isStartHeartbeat = false;
							if(iter->second.appInfo.ResumeFlag)
							{
								//重启app
								std::string killAppCmd = "killall -9 " + iter->second.appInfo.AppName;
								std::string retShell =  doshell(killAppCmd);
								LOG_SDEBUG("%d, retShell = %s\n", __LINE__, retShell.c_str());
								runApp(iter->second.appInfo);
							}
							LOG_SDEBUG("%d, RunParameter = %s\n", __LINE__, iter->second.appInfo.RunParameter.c_str());
							time_t now;
							struct tm *ptm;
							time (&now);
							//获取当地日期和时间
							ptm = localtime (&now);
							char log[256];
							snprintf(log, sizeof(log), "HeartbeatTimeout--run_App:%s,ResumeFlag:%d, curTime:%d-%d-%d_%d:%d:%d", iter->second.appInfo.AppName.c_str(),iter->second.appInfo.ResumeFlag,
								(ptm->tm_year + 1900), (ptm->tm_mon + 1), ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
							ZH::logService::LogRecord("Watch", ZH::logService::EnLogLevel::LOGLEVEL_INFO, log);
							break;
						}
                    }
					else
					{
						iter->second.timeoutCount = 0;
					}
					WatchIpc::HeartbeatSignal(iter->second.appInfo.AppName, iter->second.heartbeatCount++);
					LOG_SDEBUG("%s,%d, count = %d, resp = %d\n", __func__, __LINE__, iter->second.heartbeatCount, iter->second.heartbeatCount_resp);
                }
            }
        }
        usleep(loopDelay);
#if 0
        if (huMsgServiceLaunchDelay > 0)
        {
            huMsgServiceLaunchDelay -= loopDelay;
            if (huMsgServiceLaunchDelay <= 0)
            {
                // 延时启动HUMsgService
                // TODO: 测试发现：HU需要启动完成后，IP才能向HU建立socket客户端连接。
                // 否则即使连接成功，IP也会收不到HU发送的数据。
                // 实测HU启动到默认界面要36秒。所以这里等待40秒才开始启动HUMsgService。 
                AppInfo huMsgServiceInfo;
                huMsgServiceInfo.AppName = "HUMsgService";
                huMsgServiceInfo.AppPath = "/usr/sbin/zhapp/midware/bin";
                huMsgServiceInfo.RunParameter = "";
                huMsgServiceInfo.LogLevel_0To6 = 3;
                huMsgServiceInfo.ResumeFlag = true;
                runApp(huMsgServiceInfo);
            }
        }
#endif
    }
   
}

void CWatchMain::runApp(CWatchMain::AppInfo& appInfo)
{
	do
	{
		std::string cmdStr = "ps -e | grep ' " + appInfo.AppName + "'";
        CDoShellCmd doshell;
        std::string retStr = doshell(cmdStr);
		if((retStr != "") && (std::string::npos != retStr.find(appInfo.AppName)))
		{
			return;
		}
	} while (0);
	
    LOG_SDEBUG("%s %d, AppName = %s\n", __func__, __LINE__, appInfo.AppName.c_str());
    if(mAppSts.find(appInfo.AppName) != mAppSts.end())
    {
        mAppSts[appInfo.AppName].isStartHeartbeat = false;
    }
    auto func = [](AppInfo& appInfo)->void{
        pid_t fpid;
        fpid = fork();
        if(fpid < 0)
        {
            LOG_SERROR("(%s,%d)error in fork", __func__, __LINE__);
        }
        else if(fpid == 0)
        {//子程序
            setsid();
            char cmdStr[512];
            snprintf(cmdStr, sizeof(cmdStr), "LD_LIBRARY_PATH=/usr/sbin/zhapp/midware/lib/ cd %s && ./%s %s &", 
                appInfo.AppPath.c_str(), appInfo.AppName.c_str(), appInfo.RunParameter.c_str());
            LOG_SDEBUG("(%s,%d), cmd = %s\n", __func__, __LINE__, cmdStr);
            system(cmdStr);
        }
        else if(fpid > 0)
        {//当前程序
            int sta;
            waitpid(fpid, &sta, 0);
        }
    };
    std::thread t1(func, std::ref(appInfo));
    t1.join();
}

void CWatchMain::CheckAlive_OnTime()
{
	static int hmiCount = 0;
    for (auto iter = mAppSts.begin(); iter != mAppSts.end(); iter++)
    {
        std::string cmdStr = "ps -e | grep ' " + iter->first + "'";
        CDoShellCmd doshell;
        std::string retStr = doshell(cmdStr);
        LOG_SDEBUG(" cmdStr = %s,retStr = %s\n",cmdStr.c_str(), retStr.c_str());
        if((retStr == "") || (std::string::npos == retStr.find(iter->first)))
        {
            LOG_SDEBUG("(%s,%d), appName = %s is not alive\n", __func__, __LINE__, iter->first.c_str());
			if(iter->second.appInfo.AppName == "kanzi.sh")
			{
				if(hmiCount == 0)
				{
					doshell("cp /tmp/hmi.log /opt/data/");
				}
				printf("hmiCount = %d\n", hmiCount);
				if(++hmiCount > 4)
				{
					//重启arm
					WatchIpc::NotifyMcuReboot(0);
					sleep(3);
					reboot(RB_AUTOBOOT);
				}
			}
			//doshell("cp /var/log/messages /opt/data/");
            if(iter->second.appInfo.ResumeFlag)
            {
				runApp(iter->second.appInfo);
			}
			else
			{
				return;
			}
			time_t now;
			struct tm *ptm;
			time (&now);
			//获取当地日期和时间
			ptm = localtime (&now);
			char log[256];
			snprintf(log, sizeof(log), "CheckAlive run_App:%s, ResumeFlag:%d, curTime:%d-%d-%d_%d:%d:%d", iter->second.appInfo.AppName.c_str(), iter->second.appInfo.ResumeFlag,
				(ptm->tm_year + 1900), (ptm->tm_mon + 1), ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
			ZH::logService::LogRecord("Watch", ZH::logService::EnLogLevel::LOGLEVEL_INFO, log);
        }
		else
		{
			if(iter->second.appInfo.AppName == "kanzi.sh")
			{
				hmiCount = 0;
			}
		}
    }
}

void CWatchMain::CheckLogLevel_OnTime()
{
    LOG_SDEBUG("%s\n", __func__);
    struct stat tempStat;
    int retStat = stat(CWatchMain::cLogSwitchFileName, &tempStat);
    if(retStat == 0)
    {
        if(mLogSwitchFileModifyTime != tempStat.st_mtim.tv_sec)
        {
            mLogSwitchFileModifyTime = tempStat.st_mtim.tv_sec;
            int fd = open(CWatchMain::cLogSwitchFileName, O_RDONLY);
            if(fd  > 0)
            {
                char ch = 0;
                read(fd, &ch, 1);
                close(fd);
                if(ch == '1')
                {
                    mLogSwitchFlag = true;
                }
                else
                {
                    mLogSwitchFlag = false;
                }
            }
            else
            {
                mLogSwitchFlag = false;
            }
            for(auto iter : mAppSts)
            {
                uint32_t logLevel = (mLogSwitchFlag == false) ? 0 : iter.second.appInfo.LogLevel_0To6;
                WatchIpc::LogLevelControl(iter.second.appInfo.AppName, static_cast<LOG_LEV_EN>(logLevel));
				
            }
        }
    }
    retStat = stat(CWatchMain::cfgFileName, &tempStat);
    if(retStat == 0)
    {
        if(mLogCfgFileModifyTime != tempStat.st_mtim.tv_sec)
        {
            mLogCfgFileModifyTime = tempStat.st_mtim.tv_sec;
            //
            std::ifstream ifs;
            Json::Value jsonValue;
            ifs.open(CWatchMain::cfgFileName, std::ios::binary);
            if(mJsonReader.parse(ifs, jsonValue, false))
            {
                Json::Value::Members mem = jsonValue.getMemberNames();
                for(auto iter = mem.begin(); iter != mem.end(); iter++)
                {
                    if((*iter) == "watch_Version")
                    {
                        continue;
                    }
                    else
                    {
                        if( jsonValue[(*iter)].isMember("AppName") &&
                            jsonValue[(*iter)].isMember("LogLevel_0To6") )
                        {
                            std::string appName = jsonValue[(*iter)]["AppName"].asString();;
                            uint32_t logLevel = jsonValue[(*iter)]["LogLevel_0To6"].asUInt();
                            if(logLevel != mAppSts[appName].appInfo.LogLevel_0To6)
                            {
                                if(mLogSwitchFlag)
                                {
                                    WatchIpc::LogLevelControl(appName, static_cast<LOG_LEV_EN>(logLevel));
                                }
                                LOG_SINFO("logLevel = %d != %d\n", logLevel, mAppSts[appName].appInfo.LogLevel_0To6);
                                mAppSts[appName].appInfo.LogLevel_0To6 = logLevel;
                            }
                        }
                    }
                    
                }
            }
            ifs.close();
        }
    }
    else
    {
        mLogCfgFileModifyTime = 0;
    }
}

void CWatchMain::HeartbeatStart(std::string appName)
{
    LOG_SDEBUG("%s, appName = %s\n", __func__, appName.c_str());
	if(appName == "All")
	{
		mbHeartbeatSwtich = true;
		ZH::logService::LogRecord("Watch", ZH::logService::EnLogLevel::LOGLEVEL_INFO, "HeartbeatStart All");
	}
    if(mAppSts.find(appName) == mAppSts.end())
    {
        return;
    }
    mAppSts[appName].isStartHeartbeat = true;
    mAppSts[appName].isAlive = true;
	mAppSts[appName].heartbeatCount = 0;
	mAppSts[appName].heartbeatCount_resp = 0;
	mAppSts[appName].lastHeatbeatTick = ZH::BaseLib::getWorldTimeMS();
	WatchIpc::HeartbeatSignal(appName, mAppSts[appName].heartbeatCount++);
}
void CWatchMain::HeartbeatSignal_Resp(std::string appName, int count)
{
    //LOG_SDEBUG("%s, appName = %s\n", __func__, appName.c_str());
    if(mAppSts.find(appName) == mAppSts.end())
    {
        return;
    }
    mAppSts[appName].isAlive = true;
	mAppSts[appName].timeoutCount = 0;
	mAppSts[appName].heartbeatCount_resp = count;
}
void CWatchMain::HeartbeatStop(std::string appName)
{
    LOG_SDEBUG("%s, appName = %s\n", __func__, appName.c_str());
	if(appName == "All")
	{
		mbHeartbeatSwtich = false;
		ZH::logService::LogRecord("Watch", ZH::logService::EnLogLevel::LOGLEVEL_INFO, "HeartbeatStop All");
	}
    if(mAppSts.find(appName) == mAppSts.end())
    {
        return;
    }
    mAppSts[appName].isStartHeartbeat = false;
    mAppSts[appName].isAlive = true;
}

void CWatchMain::UInit()
{
    mTimerManager.removeNode(mCheckAppAliveTimer);
    delete mCheckAppAliveTimer;
}

const char* CWatchMain::cfgFileName = "/opt/data/WatchService.cfg";
const std::string CWatchMain::cfgVersion = "V1.2.7";
const char* CWatchMain::cLogSwitchFileName = "/opt/data/logSwitch";