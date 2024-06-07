#include <kanzi/kanzi.hpp>
#include <string>
#include "MsgInterface.h"
#include "HmiMain.h"
#include "DisplayCtrl.h"
#include "MenuDisplay.h"
#include "Mutex.h"
#include "mlog.h"
#include "WatchInterface.h"
#include "Application.h"
#include <unistd.h>
//#include <fcntl.h>
#include <signal.h>
#include <errno.h>
void SigExit(int sig)
{
	printf("===== ERROR SIG_TYPE: %d =====\n", sig);
#ifndef WIN32
	printf("===== ERROR NO: %d, %s =====\n", errno, strerror(errno));
	signal(SIGINT, SIG_IGN);
	kill(0, SIGINT);
#endif
	printf("===== ERROR SIG_TYPE: %d \n", sig);
	exit(-1);
}

Hmi::~Hmi() {}

KzsThreadLock *Hmi::pMsgLock = NULL;

//auto func = [](const char* funcName, int line)->void{
//		{
//			FILE* fp = fopen("/tmp/MsgService.txt", "a+");
//			if (fp)
//			{
//				fprintf(fp, "(%s,%d): time:%d\n", funcName, line, ZH::BaseLib::getWorldTimeMS());
//				fflush(fp);
//				fclose(fp);
//			}
//		} while (0);
//};
Hmi::Hmi()
	:opt("vpuplayer"), opt_watch("WatchService"), m_bSelfCheckState(false), isAmiLastframe(false), isFirstTimeAni(true), mSaveSysTime(0)
{
	msgLayer = new CDispCtrlScreenLayer();
	HmiIPC::SelfCheckState(0xFF); //开始接收同步消息
	//func(__func__, __LINE__);
}


void Hmi::onConfigure(ApplicationProperties &configuration)
{
	configuration.binaryName = "a301.kzb.cfg";
	configuration.defaultWindowProperties.x = 0;
	configuration.defaultWindowProperties.y = 0;
	configuration.defaultWindowProperties.width = 1920;
	configuration.defaultWindowProperties.height = 720;
	configuration.defaultWindowProperties.order = 10;
	configuration.defaultSurfaceProperties.bitsDepthBuffer = 24;
	configuration.defaultSurfaceProperties.bitsColorR = 8;
	configuration.defaultSurfaceProperties.bitsColorG = 8;
	configuration.defaultSurfaceProperties.bitsColorB = 8;
	configuration.defaultSurfaceProperties.bitsAlpha = 8;
	configuration.useMemoryMappedLoading = 1;
	configuration.loadingThreadCount = 3;
	configuration.defaultWindowProperties.groupName = "a301";

	kzsThreadLockCreate(&Hmi::pMsgLock);

#ifdef SIGQUIT
	signal(SIGQUIT, SigExit);
#endif
	signal(SIGINT, SigExit);
	signal(SIGTERM, SigExit);
#ifdef SIGUP
	signal(SIGHUP, SigExit);
#endif
	signal(SIGSEGV, SigExit);
#ifdef SIGBUS
	signal(SIGBUS, SigExit);
#endif
#ifdef SIGKILL
	signal(SIGKILL, SigExit);
#endif
}

void Hmi::onProjectLoaded()
{
	m_oRoot = getScreen();
	m_Domain = getDomain();
	msgLayer->init(m_oRoot, m_Domain, this);
	publisher::StartupAnimation_Stop(opt);
	publisher::LaneAnimation_SetColor(opt);
	subscriber::StartupAnimation_PlaySts(opt, *this);

	WatchIpc::publisher::HeartbeatStart(opt_watch);
	WatchIpc::publisher::HeartbeatStop(opt_watch);


	WatchIpc::subscriber::HeartbeatSignal(opt_watch, *this);
	WatchIpc::publisher::HeartbeatSignal_Resp(opt_watch);
	
	WatchIpc::subscriber::LogLevelControl(opt_watch, *this);

	opt.start();
	opt_watch.start();
}

#define DiagnosesPtrIsNullptr(ptr)              \
	if (ptr == nullptr)                         \
	{                                           \
		LOGERR("ptr [%s] is nullptr.\n", #ptr); \
		return;                                 \
	}

//static unsigned long long frameNum = 0;
void Hmi::onUpdate(chrono::milliseconds deltaTime)
{
	static bool flag = false;
	if (!flag)
	{
		int tid = static_cast<pid_t>(::syscall(SYS_gettid));
		msgLayer->setKanizTid(tid);
		flag = true;
	}
	//ZH::BaseLib::CAutoLock lock(*(msgLayer->ptrLock));
	if (isAmiLastframe )
	{
		isAmiLastframe = false;
		StartupAnimation_Stop(1);
		WatchIpc::HeartbeatStart("kanzi.sh");
		msgLayer->SelfCheck();
	}
	msgLayer->RunTask();
	/*frameNum++;
	if (frameNum == 600)
	{
		HmiIPC::TestResetCommand(1);
	}*/
}

void Hmi::registerMetadataOverride(ObjectFactory & /*factory*/)
{
	KanziComponentsModule::registerModule(getDomain());
}


void Hmi::StartupAnimation_PlaySts(tstAnimationSts status)
{
	auto UpdateVersionTask = [this, status]() -> bool
	{
		if (status == tstAnimationSts::PLAYING_FIRSTFRAME)
		{

		}
		else if (status == tstAnimationSts::PLAYING_LASTFRAME)
		{
			this->isAmiLastframe = true;
			if (isFirstTimeAni)
			{
				isFirstTimeAni = false;
			}
		}
		return true;
	};
	msgLayer->PushUpdateKzAttrTaskToMain("StartupAnimation", UpdateVersionTask);
}


void Hmi::LogLevelControl(std::string name, LOG_LEV_EN level)
{
	//LOGINF("LogLevelControl name = %s,flag = %d", name.data(), level);
	if (name == "kanzi.sh")
	{
		//LOGINF("\n\nLogLevelControl name = %s,flag = %d", name.data(), level);
		setPrintLevel(level);
	}
}

void Hmi::HeartbeatSignal(std::string name, int flag)
{
	if (name == "kanzi.sh")
	{
		WatchIpc::HeartbeatSignal_Resp("kanzi.sh", flag + 1);
	}
}

Application *createApplication()
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	select(0, NULL, NULL, NULL, &tv);


	return new Hmi;
}
