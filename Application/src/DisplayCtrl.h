#ifndef DISPCTRL_H_
#define DISPCTRL_H_


#include <vector>
#include <string>
#include <map>
#include <atomic>
#include <list>

#include <kanzi/kanzi.hpp>
#include "DataDefine.h"
#include "HUMsgDefine.h"
#include "HmiMain.h"
#include "WarnIdDefine.h"
#include "HmiLock.h"
#include "IPC/IPCCore.h"
#include <mutex>
#include "LogServiceApi.h"
#include <sys/syscall.h>

#define SpeedSectionNum 6
#define PowerSectionNum 6

using namespace kanzi;

class CMenuDisplay;
class CLampStartegy;
class CDispCtrlScreenLayer
{
	friend class Hmi;


public:
	CDispCtrlScreenLayer();
	~CDispCtrlScreenLayer() {}


	void init(ScreenSharedPtr, Domain *, Hmi *_app);
	void UpdateDispVehicleSpeed(const int &fVehicleSpeed, bool validity);
	void UpdateDispPowerValue(const int &fPowerValue, bool validity);
	void UpdateDispGear(const EmGearsValue &gear);
	void UpdateDispTripA(const float &iTripA);
	void UpdateDispTripB(const float &iTripB);
	void UpdateDispOdograph(const int &iOdograph);
	void UpdateDispRechargeMileage(float, bool);
	void UpdateDispLamp(const std::vector<StuTelltableLampState> lamp);
	void UpdateDispMenu(std::string majorPage, std::string minorPage);
	void UpdateDispPopWarn(EmPopWarnID, int);
	void UpdateHistoryWarnList(std::vector<EmPopWarnID>);
	void UpdateDispCarTire(StuCarTire carTire);
	void UpdateSysVersion(std::string, std::string);
	void UpdateDispKey(EmKey key,EmKeyState keyState);
	void SystemUpdateMessaage(EmSWState, std::string, std::string, int);
	void UpdatePowerStatus(EmPowerStatus status);
	void UpdateTime(int hour, int minute,int format);
	void UpdateDispSOCValue(const int socValue, const bool status, bool validity);
	void UpdateDispEnergyFlow(const int Value);
	void UpdateDispEnergyRecycle(const int Value);
	void UpdateInstEnergyConsum(const float Value);
	void UpdateAverEnergyConsum(const float Value);
	void UpdateDispDrivingTime(const int hour, const int minute, bool validity);
	void UpdateDispMotorSpeed(const float motorSpeed, bool validity);
	void UpdateThemeColor(int, int);
	void UpdateVoicePlay(int, int);
	void UpdateTCInfoIndex(int);
	void CarDoorStatus(int);
	
	void SelfCheck();
	void UpdateTurnLampSts(int leftTurn, int rightTurn, int warnLamp);
	void UpdateIVISourceStatus(int);
	void UpdateIVINaviInfo(bool, int, int, int, int, std::string, std::string, std::string);
	void UpdateIVIMusicPlayInfo(bool, int, int, std::string, std::string, std::string, std::string, int, std::string, std::string, int);
	void UpdateIVIRadioPlayInfo(bool, int, std::string, std::string);
	void UpdateIVIPhoneInfo(int, std::string, std::string, int, int, std::string);
	void UpdateIVIBtInfo(int, std::string);
	void UpdateChargeSubscribe(int, int, int, int, int, int, StuChargeSubscribe, StuChargeSubscribe);
	void UpdateCharging(int, int, int, int, int, int, int, int, int, int, StuChargeSubscribe, StuChargeSubscribe);
	void UpdateIVIMusicPlayList(int, int, int, std::vector<std::string>);
	void UpdateIVIRadioList(int, int, std::vector<std::string>);
	void UpdateIVIContactList(bool, int, std::vector<std::string>);
	void UpdateIVICallRecordList(bool, int, std::vector<StuCallRecord>);
	void UpdateIVIThemeColor(int, int);
	void UpdateIVIMusicDevice(int, bool);
	void UpdateIVIKeyMode(int);
	void UpdateIVIBrightness(int);
public:

	void setLampStatus(std::string,EmLampState);
	void startUpdateDispSpeed();
	void dispSpeedDone();
	void SwitchBg(int index);
	void SwitchTheme(int style);
	void SetMusicPlay(bool state){ m_nIsMusicPlay = state; };
	void SetRadioPlay(bool state){ m_nIsRadioPlay = state; };
	int getCurMusicType(){ return m_nCurMusicType; };
	int getCurRadioType(){ return m_nCurRadioType; };
	int getCurMediaSourceType(){ return m_nMediaSourceType; };
	void setKanizTid(int tid) { mKanzi_tid = tid; }
private:
	void setLampSelfCheck(EmLampState state);
	void InitNode();
	void InitMap();
	float PointerRotation(float x, float arrayX[], float arrayY[]);
	void WarnImagePage(WarnPage warnId);
	void DispWarnImage(EmPopWarnID, int);
	void SwitchSportColor(int value);
	void SwitchThemeSyncInfo(int theme);
	void RemoveWarn();
	void RestartMomoryData();

public:
	Hmi *m_pApp;
	CHmiMutex mLock;
	std::mutex m_Mutex;
	CHmiMutex* ptrLock;

	Node2DSharedPtr mMainView;
	Node2DSharedPtr mBgNode;
	Node2DSharedPtr mLampNode;
	Node2DSharedPtr mPopNode;
	Node2DSharedPtr mUpgradeNode;
	Node2DSharedPtr mBgClassic;
	Node2DSharedPtr mBgSport;
	Node2DSharedPtr mBgTechnology;
	Node2DSharedPtr mStyleClassic;
	Node2DSharedPtr mStyleSport;
	Node2DSharedPtr mStyleTechnology;
	Node2DSharedPtr mLamp;
	Node2DSharedPtr mPop; 
	Node2DSharedPtr mWarnNode;
	Node2DSharedPtr mUpgrade;
	Node2DSharedPtr mNaviNode;
	Node2DSharedPtr mNavi;
	Node2DSharedPtr mOffNode;
	Node2DSharedPtr mOff;
	Node2DSharedPtr mSysVersionNode;
	BindingSharedPtr binding;

	PrefabTemplateSharedPtr mBgPrefabClassic;
	PrefabTemplateSharedPtr mBgPrefabSport;
	PrefabTemplateSharedPtr mBgPrefabTechnology;

	PrefabTemplateSharedPtr mStylePrefabClassic;
	PrefabTemplateSharedPtr mStylePrefabSport;
	PrefabTemplateSharedPtr mStylePrefabTechnology;
protected:
	void IsReady(void);
	void PushUpdateKzAttrTaskToMain(std::string, std::function<bool(void)>);
	void RunTask();

private:
	CIPCConnector opt;
	CIPCConnector opt_update;
	CIPCConnector opt_ivi;
	CIPCConnector opt_log;
	ScreenSharedPtr m_oRoot;
	CMenuDisplay *pMenuCtrl;
	CLampStartegy *pLampStartegy;

	map<string, EmLampState> mIndicatorStatus;
	std::map<std::string, std::function<bool(void)>> mUpdateKzAttrList;
	std::atomic_bool isSelfChecking;
	
	std::map<EmGearsValue, std::string> mGearsMap;
	std::map<EmPopWarnID, std::string> mWarnTxtMap;
	std::map < WarnPage, std::string> mWarnPageMap;
	map<string, EmLampState> mOffPageLampStatus;

	bool m_isUpgradeNow;
	int lineRun;

	kanzi::TimerSubscriptionToken m_CarLineToken;
	kanzi::TimerSubscriptionToken m_SysVersionTimer;
	kanzi::TimerSubscriptionToken DoorCloseTimer;
	
private:
	EmPowerStatus PowerStatus;
	int mSyncMemeryFlag;
	int m_nCurSportColor;
	bool m_nIsMusicPlay;
	bool m_nIsRadioPlay;
	int m_nCurMusicType;
	int m_nCurRadioType;
	int m_nMediaSourceType;//0:音乐 1：电台
	bool m_bIsRestart;
	std::mutex mTaskMutex;
	int mKanzi_tid;

	std::atomic_int mMemoryVoiceState, mMemoryVoiceSize, mMemoryColor, mMemoryTheme, mMemoryTcInfo;
};

#include <unistd.h>
#include <fcntl.h>
class testLogManager
{
private:
	static uint32_t	sLogCount;
	std::string	mStrLogContent;
private:
	void setTestLog(bool isEnter = true);
public:
	testLogManager(const char* log);
	~testLogManager();
};

#endif //DISPCTRL_H_
