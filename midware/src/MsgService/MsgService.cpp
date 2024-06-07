
#include <iostream>
#include <functional>
#include "MsgService.h"
#include "IdDefine.h"
#include <unistd.h>
#include "hmi/WarnIdDefine.h"
#include "hmi/MsgInterface.h"
#include "WatchInterface.h"
#include "LocalInterface.h"
#include "UpgradeInterface.h"
#include "FileL.hpp"
#include "FaultCodeTable.h"
#include "videoPlay/videoPlayer_api.h"
#include "mylogCtrl.h"
#include <fcntl.h>

#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <mutex>
#include "DataDefine.h"

std::mutex mLogMutex;

CMsgService* s_MsgS = nullptr;
void SigExit(int sig)
{
	printf("===== ERROR SIG_TYPE: %d =====\n", sig);
#ifndef WIN32
	printf("===== ERROR NO: %d, %s =====\n", errno, strerror(errno));
	signal(SIGINT, SIG_IGN);
	kill(0, SIGINT);
#endif
	if(s_MsgS)
	{
		LOG_RECORD("==== ERROR SIG_TYPE:%d,NO:%d,%s ====",sig, errno, strerror(errno));
		delete s_MsgS;
	}
	exit(-1);
}

int main(int argc, char *argv[])
{
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
	signal(SIGFPE, SigExit);

    printf("[%s] build time: %s %s\n", argv[0], __DATE__, __TIME__);
    bool autoSimulateMode = false;
    if (argc >= 2 && ((std::string("-T") == argv[1]) || (std::string("-t") == argv[1])))
        autoSimulateMode = true;
#if 1
	s_MsgS = new CMsgService(argc, argv, autoSimulateMode);
	s_MsgS->Start(true);
#else
    CMsgService MsgS(argc, argv, autoSimulateMode);
    MsgS.Start(true);
#endif
    return EXIT_SUCCESS;
}

#define BIND(func) std::bind(&CMsgService::func, this)
//ZH::BaseLib::CMultitaskTemplateTimer CMsgTimer::mMsgTimer;

CMsgService::CMsgService(int argc, char *argv[], bool mode)
    : bAutoSimulateMode(mode), opt("vpuplayer"),upgradeIPC("UpgradeService"),m_pTimerSelfInspet(nullptr)
{
    mSaveDispUnitPress = EmUnitPress::PRESS_KPA;
    //
    o_stgCtx.oWarnIO.registerChangeWarn(
			std::bind(&CMsgService::NotifyHMICurrentPopWarn, this, std::placeholders::_1));
	o_stgCtx.oWarnIO.registeNotifyHistoryWarnList(std::bind(&CMsgService::NotifyHMIHistoryPopWarnList,this, std::placeholders::_1));
    mOTACtrlValue = 0xff;

    mHeartbeatTimer = new ZH::BaseLib::STimer((5*1000), std::bind(&CMsgService::heartbeat_OnTimer, this), 0, true);
    ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mHeartbeatTimer);

    if (argc >= 2 && ((std::string("-r") == argv[1]) || (std::string("-R") == argv[1])))
    {
        mIsReStartMsgService = true;
    }
    else
    {
        mIsReStartMsgService = false;
    }
    mXKeyValue = 16;
    mPreformanceMode = 1;
    mLimitSpdValue = 0;

	// mUpdateHMIClock = new ZH::BaseLib::CSimpleTimer<CMsgService>(1000, &CMsgService::UpdateCurTime, this, true);
	// mSimpleTimerManager.addNode(mUpdateHMIClock);
	// mUpdateHMIClock->start();

    m_nChargeOrderStart.Year = m_nChargeOrderStart.Month = m_nChargeOrderStart.Day = m_nChargeOrderStart.Hour = m_nChargeOrderStart.Minute = 0;
    m_nChargeOrderEnd.Year = m_nChargeOrderEnd.Month = m_nChargeOrderEnd.Day = m_nChargeOrderEnd.Hour = m_nChargeOrderEnd.Minute = 0;
    m_eChargeOrderMode = CHARGE_ORDER_MODE_NONE;
    m_eChargeOrderSts = CHARGE_ORDER_RESERVED;
    ChargStateVilid = -1;
    ChargTimeVilid = -1;

    m_eChargeDC = (m_eChargeAC = CHARGE_DEVICE_RESERVED);
    m_eChargingSts = CHARGING_RESERVED;
    m_nElectricType = -1;
    m_nCurTheme = 0;
    m_nCurColor = 0;
    m_nCurVoiceSwitch = 0;//默认开启
    m_nCurVoiceSize = 0;//默认音量高
	mSaveCurMenuItem = 0;
	mChimeIconBitwiseData = 0;
    m_nGear = 0;
	mRepeatSend0x810000Timer = new ZH::BaseLib::STimer(100, std::bind(&CMsgService::repeatSend0x81_Ontime, this));
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mRepeatSend0x810000Timer);
	mRepeatSend0x81Count = 0;
	mChimeSpdOverFlag = false;
	o_stgCtx.oTTProcess.setCallBack(std::bind(&CMsgService::ttSelfCheck_OnChangeSts, this));

	mLampSigDelayProcessor 	= new CSigDelayProcessor(std::bind(&CMsgService::GetVehicleLightStatus_Process, this, std::placeholders::_1, std::placeholders::_2), 300, "GetVehicleLightStatus");
	mTurnLampSigDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetTurnLightStatus_Process, this, std::placeholders::_1, std::placeholders::_2), 300, "GetTurnLightStatus");
	mVehSpdSigDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetVehicleSpeed_Process, this, std::placeholders::_1, std::placeholders::_2), 200, "GetVehicleSpeed");
	mEngineSpdDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetEngineSpeed_Process, this, std::placeholders::_1, std::placeholders::_2), 200, "GetEngineSpeed");
	mTempDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetTemp_Process, this, std::placeholders::_1, std::placeholders::_2), 300, "GetTemp");
	mTPMSDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetTireStatus_Process, this, std::placeholders::_1, std::placeholders::_2), 300, "GetTireStatus");
	mSafeBeltDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetsafetyBeltOrAirbag_Process, this, std::placeholders::_1, std::placeholders::_2), 300, "GetsafetyBeltOrAirbag");
	mEnergyDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetEnergyValue_Process, this, std::placeholders::_1, std::placeholders::_2), 300, "GetEnergyValue");
	mRechargeDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetRechargeMileage_Process, this, std::placeholders::_1, std::placeholders::_2), 200, "GetRechargeMileage");
	mBatteryVoltageDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetBatteryVoltage_Process, this, std::placeholders::_1, std::placeholders::_2), 500, "GetBatteryVoltage");
	mPopUpAlarmDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetPopUpAlarm_Process, this, std::placeholders::_1, std::placeholders::_2), 500, "GetPopUpAlarm");
	mChargeOrderDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetChargeOrder_Process, this, std::placeholders::_1, std::placeholders::_2), 0, "GetChargeOrder");
	mChargeStsDelayProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetChargeStatus_Process, this, std::placeholders::_1, std::placeholders::_2), 0, "GetChargeStatus");
	mPowerModeProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetPowerMode_Process, this, std::placeholders::_1, std::placeholders::_2), 500, "GetPowerMode");
    mPowerGearProcessor = new CSigDelayProcessor(std::bind(&CMsgService::GetPowerGear_Process, this, std::placeholders::_1, std::placeholders::_2), 100, "GetPowerGear");
    mCurPowerMode = 0xff;
	mCurPopWarnID = 0;
	mInstEnergyConsumTimer = new ZH::BaseLib::STimer(1700, std::bind(&CMsgService::InstEnergyConsum_OnTimer, this), 0, true);
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mInstEnergyConsumTimer);
	mInstEnergyConsumValue = 0;
	mDisplaySpeed.first = 0.f;
	mDisplaySpeed.second = false;
	mDisplayPowerValue.first = 0;
	mDisplayPowerValue.second = false;

	mIgnDelayUpdateHmiTimer = new ZH::BaseLib::STimer(200,
	[this](int)->void{
		std::lock_guard<std::mutex> lock(CSigDelayProcessor::sMutex);
		HmiIPC::UpdateDispVehicleSpeed(mDisplaySpeed.first, mDisplaySpeed.second);
		LaneAnimation_Speed(int(mDisplaySpeed.first));
		HmiIPC::UpdateDispPowerValue(mDisplayPowerValue.first, mDisplayPowerValue.second);
	});
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mIgnDelayUpdateHmiTimer);

	memset(&mStUpdateCharging, 0, sizeof(mStUpdateCharging));
	mStUpdateCharging.delayProcessTimer = new ZH::BaseLib::STimer(200, std::bind(&CMsgService::togetherProcess, this));
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mStUpdateCharging.delayProcessTimer);

	mBrightnessValue = 0;
	mBrightnessRepeatSendCount = 0;
	mBrightnessSendFilterTimer = new ZH::BaseLib::STimer(100, [this](int)->void{
		std::lock_guard<std::mutex> lock(mSendMCUDataMutex);

		if(mBrightnessRepeatSendCount < 5)
		{
            sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_IVI_CTRL_BRIGHTNESS, {mBrightnessValue}), false);
			mBrightnessSendFilterTimer->restart();
			++mBrightnessRepeatSendCount;
		}
	});
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mBrightnessSendFilterTimer);

	mRepeatSendIgnOffWarnIdTimer = new ZH::BaseLib::STimer(500, [this](int)->void{
		std::lock_guard<std::mutex> lock(mSendMCUDataMutex);
		sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_IGN_OFF_POPSID, {mCurPopWarnID}), false);
		if(mCurPowerMode == 0)
		{
			mRepeatSendIgnOffWarnIdTimer->restart();
		}
	});
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mRepeatSendIgnOffWarnIdTimer);
	//
	do
	{
		char var[125];
		memset(var, 0, sizeof(var));
		std::map<std::string, int> month{
			{"Jan", 1},
			{"Feb", 2},
			{"Mar", 3},
			{"Apr", 4},
			{"May", 5},
			{"Jun", 6},
			{"Jul", 7},
			{"Aug", 8},
			{"Sep", 9},
			{"Oct", 10},
			{"Nov", 11},
			{"Dec", 12},
		};
		std::string date(__DATE__);
		size_t firstNu = date.find(" "), lastNu = date.find_last_of(" ");
		int iMt = month[date.substr(0, firstNu)], day = atoi(date.substr(firstNu + 1, lastNu - firstNu - 1).c_str());
		sprintf(var, "%s%02d%02d%s", date.substr(lastNu + 1).c_str(), iMt, day, "A301v0.1.0");
		mSocVersion = var;
	} while (0);
	mbSetVoiceFlag = false;
    mbHmiReady = false;


    mStMemoryDataSync.isInitReCoverSts = true;
    mStMemoryDataSync.repeatSendMemoryDataTimer = new ZH::BaseLib::STimer(200, [this](int)->void{
        std::lock_guard<std::mutex> lock(mSendMCUDataMutex);
        sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_MEMORY_INFO, {m_nCurTheme, m_nCurColor, m_nCurVoiceSwitch, m_nCurVoiceSize, 0, mSaveCurMenuItem}), false);
        if(++mStMemoryDataSync.repeatSendCount < 5)
        {
            mStMemoryDataSync.repeatSendMemoryDataTimer->restart();
        }
    });
    ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mStMemoryDataSync.repeatSendMemoryDataTimer);
    mMemDataManager = new CMemoryDataManager();

    mOdoValue = 0;
    mOdoFirstUpdate = true;
    mOdoDelayCount = 0;
}

CMsgService::~CMsgService() {
    if(mHeartbeatTimer)
    {
        ZH::BaseLib::STimerManager::getInstance()->removeTimerNode(mHeartbeatTimer);
        delete mHeartbeatTimer;
    }
    if (m_pTimerSelfInspet)
    {
        m_pTimerSelfInspet->stop();
        ZH::BaseLib::STimerManager::getInstance()->removeTimerNode(m_pTimerSelfInspet);
        delete m_pTimerSelfInspet;
    }
	if(mRepeatSend0x810000Timer)
	{
		ZH::BaseLib::STimerManager::getInstance()->removeTimerNode(mRepeatSend0x810000Timer);
		delete mRepeatSend0x810000Timer;
	}
    if(mMemDataManager)
    {
        delete mMemDataManager;
    }
}

void CMsgService::Init()
{
	mMsgDev.start();
    if(mIsReStartMsgService == false)
    {
        sendDataToMCU(encode.getFrame(DOWN_MAJOR_INIT, {}), false);
    }
	else
	{
		mCurPowerMode = 1;
	}
    MsgLocal::subscriber::NotifyMcuDriveMode(*this);
    MsgLocal::subscriber::NotifyMcuBrightness(*this);
    //MsgLocal::subscriber::NotifyMcuBacklightState(*this);
    MsgLocal::subscriber::NotifyMcuCurrentPopWarn(*this);

    HmiIPC::publisher::UpdateDispVehicleSpeed(o_stgCtx.oMsg);

    HmiIPC::publisher::UpdateDispDrivingTime(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispMotorSpeed(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispPowerValue(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispEnergyFlow(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispEnergyRecycle(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateInstEnergyConsum(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateAverEnergyConsum(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispSOCValue(o_stgCtx.oMsg);
    
    HmiIPC::publisher::UpdateDispGear(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispCurTime(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispOutsideTemp(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispTripA(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateChargeSubscribe(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateCharging(o_stgCtx.oMsg);
    
    HmiIPC::publisher::UpdateDispTripB(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispOdograph(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispLamp(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispPopWarn(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispCarTire(o_stgCtx.oMsg);
//    HmiIPC::publisher::UpdateDispMotorTire(o_stgCtx.oMsg);

    HmiIPC::publisher::UpdateDispRechargeMileage(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateSysVersion(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispTheme(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateDispKey(o_stgCtx.oMsg);
    HmiIPC::publisher::SystemUpdateMessaage(o_stgCtx.oMsg);
    HmiIPC::publisher::SyncCurrentClock(o_stgCtx.oMsg);
    HmiIPC::publisher::SyncMemoryItem(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdatePowerStatus(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateHistoryWarnList(o_stgCtx.oMsg);
    HmiIPC::subscriber::SetCurrentClock(o_stgCtx.oMsg, *this);
//    HmiIPC::subscriber::SetMemoryItem(o_stgCtx.oMsg, *this);
    HmiIPC::subscriber::SelfCheckState(o_stgCtx.oMsg, *this);
    HmiIPC::subscriber::ResetFunctionItem(o_stgCtx.oUpgrade, *this);
    HmiIPC::subscriber::SetThemeColor(o_stgCtx.oMsg, *this);
    HmiIPC::subscriber::SetVoicePlay(o_stgCtx.oMsg, *this);
    HmiIPC::subscriber::SetClearMileageInfo(o_stgCtx.oMsg, *this);
    HmiIPC::publisher::UpdateThemeColor(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateVoicePlay(o_stgCtx.oMsg);
    //upgrade::subscriber::NotifyMcuIntoUpgradeMode(o_stgCtx.oUpgrade, *this);
    HmiIPC::publisher::UpdateTurnLampSts(o_stgCtx.oMsg);
    HmiIPC::publisher::UpdateTime(o_stgCtx.oMsg);
    HmiIPC::subscriber::SetPopWarnStatus(o_stgCtx.oMsg, *this);
    HmiIPC::publisher::UpdateScreenStatus(o_stgCtx.oMsg);

	HmiIPC::subscriber::SetTCInfoIndex(o_stgCtx.oMsg, *this);
    HmiIPC::publisher::UpdateTCInfoIndex(o_stgCtx.oMsg);
	HmiIPC::subscriber::TransitIVIBrightness(o_stgCtx.oMsg, *this);
    HmiIPC::publisher::CarDoorStatus(o_stgCtx.oMsg);

    publisher::StartupAnimation_Start (opt); // 初始化
    publisher::StartupAnimation_Stop (opt); // 初始化
	publisher::LaneAnimation_Speed(opt);
    opt.start();

    //
    upgrade::subscriber::NotifyMcuIntoUpgradeMode(upgradeIPC, *this);
    upgrade::publisher::upgrade_IVICtrl(upgradeIPC);
    upgrade::publisher::Upgrade_clusterVersion(upgradeIPC);
    
    upgradeIPC.start();

	WatchIpc::subscriber::NotifyMcuHeartbeatPacket(o_stgCtx.oWatch, *this);
    WatchIpc::subscriber::LogLevelControl(o_stgCtx.oWatch, *this);
	WatchIpc::subscriber::HeartbeatSignal(o_stgCtx.oWatch, *this);

	WatchIpc::publisher::HeartbeatStart(o_stgCtx.oWatch);
	WatchIpc::publisher::HeartbeatSignal_Resp(o_stgCtx.oWatch);
	WatchIpc::publisher::HeartbeatStop(o_stgCtx.oWatch);
	WatchIpc::subscriber::NotifyMcuReboot(o_stgCtx.oWatch, *this);

	ZH::logService::publisher::LogRecord(o_stgCtx.oLog);
	ZH::logService::publisher::LogIVICtrl(o_stgCtx.oLog);

    chime::subscriber::chimeOutput_ChimeReady(o_stgCtx.oChime, *this);

    multifuncion.Connect(UP_MINOR_TRIP_ODO_MILEAGE, BIND(GetOdograph));
    multifuncion.Connect(UP_MINOR_VEHICLE_LIGHT_STATUS, BIND(GetVehicleLightStatus));
    multifuncion.Connect(UP_MINOR_TURN_LIGHT_STATUS, BIND(GetTurnLightStatus));
    multifuncion.Connect(UP_MINOR_VEHICLE_SPEED, BIND(GetVehicleSpeed));
    multifuncion.Connect(UP_MINOR_ENGINE_SPEED, BIND(GetEngineSpeed));
    multifuncion.Connect(UP_MINOR_POWER_GEAR_STATUS, BIND(GetPowerGearStatus));
    //multifuncion.Connect(UP_MAJOR_RTC_TIME, BIND(GetCurTime));
    multifuncion.Connect(UP_MINOR_CUR_TIME, BIND(GetCurTime));
    multifuncion.Connect(UP_MINOR_TEMP, BIND(GetTemp));
    multifuncion.Connect(UP_MINOR_RADAR_INFOR, BIND(GetRadarInfor));
    multifuncion.Connect(UP_MINOR_TIRE_STATUS, BIND(GetTireStatus));
    multifuncion.Connect(UP_MAJOR_KEY, BIND(GetKey));
    multifuncion.Connect(UP_MAJOR_SET_CONFIG, BIND(GetSetConfig));
    multifuncion.Connect(UP_MINOR_SEATBELT_AIRBAG, BIND(GetsafetyBeltOrAirbag));
//    multifuncion.Connect(UP_MINOR_FAULT_CODE, BIND(GetFaultCode));
//    multifuncion.Connect(UP_MINOR_WATER_TEMP_AND_FUEL, BIND(GetwaterTempAndFuel));
//    multifuncion.Connect(UP_MINOR_FUEL_CONSUMPTION, BIND(GetTxtWarn));
    multifuncion.Connect(UP_MINOR_ENERGY_VALUE, BIND(GetEnergyValue));
    multifuncion.Connect(UP_MAJOR_POWER_MODE, BIND(GetPowerMode));
    multifuncion.Connect(UP_MINOR_ENERGY_CONSUMPTION, BIND(GetRechargeMileage));
    multifuncion.Connect(MAJOR_ENTER_UPGRADE_MODE2, BIND(GetUpgrade));
    //multifuncion.Connect(UP_MINOR_FUEL, BIND(GetFuel));
    multifuncion.Connect(UP_MAJOR_MCU_VERSION, BIND(GetVersion));
    multifuncion.Connect(UP_MINOR_ELECTRIC_INFOR, BIND(GetBatteryVoltage));
    //multifuncion.Connect(UP_MINOR_IVI_INTERACTIVE, BIND(GetIVIDisplayMsg));
	multifuncion.Connect(UP_MAJOR_SCREEN_MODE, BIND(GetScreenMode));
    multifuncion.Connect(UP_MINOR_POPUPS_WARNING, BIND(GetPopUpAlarm));
    multifuncion.Connect(UP_MINOR_CHARGE_ORDER, BIND(GetChargeOrder));
    multifuncion.Connect(UP_MINOR_CHARGE_STATUS, BIND(GetChargeStatus));
//    multifuncion.Connect(UP_MINOR_VOICE_WARNING, BIND(GetVoiceWarning));
    multifuncion.Connect(UP_MINOR_MEMORY_INFO, BIND(GetMemoryInfo));
    multifuncion.Connect(UP_MINOR_RESET_ACK, BIND(GetResetAck));
	multifuncion.Connect(UP_MAJOR_DATA_SYNC_COMPLETE, BIND(GetDataSyncComplete));
    multifuncion.Connect(UP_MINOR_MEMORY_INFO_ACK, BIND(GetMemoryInfo_ACK));

    o_stgCtx.init();
    mMsgDev.registerCB(std::bind(&CMsgService::recv, this, std::placeholders::_1));
    //mMsgTimer.SetTimer(25); //25ms

    mLastBrightnessValue = 0xff;
    mLastViewMode = 0xff;

    memset(mMCUPopUpWarnData, 0 ,sizeof(mMCUPopUpWarnData));
    mCurrentTimeMode = 0;

	WatchIpc::HeartbeatStart("MsgService");
#if OVER_WRITE_TT_SELFCHECK
#else
    m_vecSelfInspet = {
        {LAMP_BRAKE_FLUID_LEVEL, EmLampState::EXTINGUISH},
        {LAMP_ABS_FAULT, EmLampState::EXTINGUISH},
        {LAMP_EPS_FAULT, EmLampState::EXTINGUISH},
        {LAMP_TMPS_FAULT, EmLampState::EXTINGUISH},
        {LAMP_LOWCHARGE, EmLampState::EXTINGUISH},

        {LAMP_POWER_BATTERY_FAULT, EmLampState::EXTINGUISH},
        {LAMP_POWER_BATTERY_HIGHTEMP, EmLampState::EXTINGUISH},

        {LAMP_DRIVE_MOTOR_FAULT, EmLampState::EXTINGUISH},
        {LAMP_SYSTEM_FAULT_YELLOW, EmLampState::EXTINGUISH},
        {LAMP_SYSTEM_FAULT_RED, EmLampState::EXTINGUISH},
    };
    m_pTimerSelfInspet = new ZH::BaseLib::STimer(3000,
        std::bind(&CMsgService::TimerSelfInspet,this));
    ZH::BaseLib::STimerManager::getInstance()->addTimerNode(m_pTimerSelfInspet);
#endif
    
}

void CMsgService::DoWork()
{
    mHeartbeatTimer->start();
// 请求状态同步
#ifdef _QNX_TARGET_
    encode(DOWN_MAJOR_SYNC_DATA, {});
#endif
#ifdef _LINUX_TARGET_
    // encode(DOWN_MAJOR_SYNC_DATA, {0, 0});
    //mMsgTimer.addTask(1000, std::bind(&CUpgradeMcuCore::MainLoop, &o_mcuUpdate), (unsigned long)&o_mcuUpdate);
    //mMsgTimer.addTask(1000, std::bind(&CMsgService::UpdateCurTime, this), (unsigned long)this + 10); //UpdateCurTime

#endif
    // mMsgDev.send(encode.getFrame());
    // mMsgDev.send(encode.getFrame());

    // encode(DOWN_MAJOR_BRIGHTNESS_CTRL, {0, 1, 1, 1, 1});
    // mMsgDev.send(encode.getFrame());
    // mMsgTimer.addTask(3000, std::bind(&CMsgService::SelfCheckState, this, 1), 123);
    // mMsgTimer.addTask(6000, std::bind(&CMsgService::SelfCheckState, this, 0), 123);

    // UpdateDispBrightness(EmBrightness::LEVEL_TWO);

    while (!bAutoSimulateMode)
    {
        usleep(20*1000);
    }
    simulatedData();
}

void CMsgService::UInit() {}

void CMsgService::recv(std::string frame)
{
	#if 0
    LOGHEX(frame.data(), frame.length(), "frame");
	#else
	//LOGRECORDEHEX(frame.data(), frame.length(), "MsgService", "frame");
	#endif
    decode(frame, MAJOR_ID_OFFSET);
    unsigned int majorID = decode.getMajorID();
    unsigned int minorID = decode.getMinorID();
    //LOG_SDEBUG("ID: %04X,%04X \n", majorID, minorID);
    if (!multifuncion(majorID))
    {
        decode(frame);
        //LOG_SDEBUG("recv:: majorID = 0x%x, minorID = 0x%04x, decode[0] = 0x%02x\n", majorID, minorID, decode[0]);  
        multifuncion(minorID);
        McuAck(minorID);
    }
}

void CMsgService::McuAck(unsigned int ID)
{
    //暂时先屏蔽
    #if 0
    ZH::ProtocolCodec::StuAckMcu ack(ID);
    encode(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_ACK,
           {ack.id0x600, ack.id0x601, ack.id0x602, ack.id0x603,
            ack.id0x604, ack.id0x605, ack.id0x606, ack.id0x607,
            ack.id0x608, ack.id0x609, ack.id0x60A, ack.id0x60B,
            ack.id0x60C, ack.id0x60D, ack.id0x60E, ack.id0x60F,
            ack.id0x610, ack.id0x620, ack.id0x611, ack.id0x612,
            ack.id0x613, ack.id0x614, ack.id0x615, ack.id0x616,
			ack.id0x617, ack.id0x618, 0});
	
    mMsgDev.send(encode.getFrame());
    #endif
}

void CMsgService::NotifyMcuDriveMode(EmDriveMode mode)
{
    // encode(MPU_MINOR_BRIGHTNESS_LEVEL, {(int)level});
    // mMsgDev.send(encode.getFrame(MPU_MAJOR_BRIGHTNESS_LEVEL));
}

void CMsgService::NotifyMcuBrightness(EmScreenType sType, int level)
{
    sendDataToMCU(encode.getFrame(DOWN_MAJOR_SET, {SET_IC_BRIGHTNESS_LEVEL, level}));
}

void CMsgService::NotifyMcuHeartbeatPacket(int timeout)
{
    sendDataToMCU(encode.getFrame(DOWN_MAJOR_HEARTBEAT, {timeout}), false);
}

void CMsgService::NotifyMcuBacklightState(std::vector<int> stateList)
{
    //
}

void CMsgService::NotifyMcuCurrentPopWarn(int id)
{
    
}

void CMsgService::SetPopWarnStatus(int status)
{
   
}

void CMsgService::NotifyHMICurrentPopWarn(EmPopWarnID id)
{
    //LOG_SDEBUG("\n\n %s id = %d\n", __func__, id);
	LOG_RECORD("%s, warnId:%d\n", __func__, id);
    HmiIPC::UpdateDispPopWarn(id, o_stgCtx.oWarnIO.getDoorSts());
	chime::chimeInput_SyncDisplayWarning(id);
	mCurPopWarnID = uint8_t(id);
	std::lock_guard<std::mutex> lock(mSendMCUDataMutex);
	if(0 == mCurPowerMode)
	{
		sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_IGN_OFF_POPSID, {mCurPopWarnID}));
	}
}
void CMsgService::NotifyHMIHistoryPopWarnList(std::vector<EmPopWarnID>& vecHistoryWarn)
{
	HmiIPC::UpdateHistoryWarnList(vecHistoryWarn);
	std::vector<StuTelltableLampState> lamp;
	StuTelltableLampState warnLamp;
	warnLamp.strID = LAMP_MAIN_ALARM;
	if(vecHistoryWarn.empty())
	{
		warnLamp.state = EmLampState::EXTINGUISH;
	}
	else
	{
		warnLamp.state = EmLampState::BRIGHT;
	}
	lamp.push_back(warnLamp);
	HmiIPC::UpdateDispLamp(lamp);
#if 0	
	std::string historyWarnStr = "";
	for(auto iter : vecHistoryWarn)
	{
		historyWarnStr += "," + std::to_string(int(iter));
	}
	LOG_RECORD("notify HistoryWarn id:{%s} warnLamp.state = %d\n",historyWarnStr.c_str(), warnLamp.state);
#endif
}

void CMsgService::LogLevelControl(std::string name, LOG_LEV_EN logLevel)
{
    if(name == "MsgService")
    {
        setPrintLevel(logLevel);
    }
}

void CMsgService::TransitIVIBrightness(int brightness)
{
	if(brightness > 100)
	{
		brightness = 0;
	}
	if(mBrightnessValue != brightness)
	{
		std::lock_guard<std::mutex> lock(mSendMCUDataMutex);
		mBrightnessValue = brightness;
		mBrightnessSendFilterTimer->start();
        mBrightnessRepeatSendCount = 0;
        sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_IVI_CTRL_BRIGHTNESS, {mBrightnessValue}));
	}
	
}
void CMsgService::SetTCInfoIndex(int idx)
{
    std::lock_guard<std::mutex> lock(mSendMCUDataMutex);
    mStMemoryDataSync.repeatSendMemoryDataTimer->start();
    mStMemoryDataSync.repeatSendCount = 0;
	mSaveCurMenuItem = idx;
    sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_MEMORY_INFO, {m_nCurTheme, m_nCurColor, m_nCurVoiceSwitch, m_nCurVoiceSize, 0, mSaveCurMenuItem}));
    mMemDataManager->DataManager_SetTCIndex(mSaveCurMenuItem);
}

void CMsgService::NotifyMcuIntoUpgradeMode(int value)
{
    if(value == 0xA0)
    {//升级MCU
        mMsgDev.stop();
    }
    else
	{
		sendDataToMCU(encode.getFrame(MAJOR_ENTER_UPGRADE_MODE, {value}));
	}
}

void CMsgService::UpdateDispBrightness(EmBrightness level)
{
    //
}

void CMsgService::SetCurrentClock(int, int, int, int hour, int minute, int second, int)
{
    char cmd[125] = {0};
    sprintf(cmd, "date -s %02d:%02d:%02d", hour, minute, second);
    system(cmd);

    sendDataToMCU(encode.getFrame(DOWN_MAJOR_RTC_SET, {0X02, 30, 1, 1, hour, minute, second}), false);

    this->UpdateCurTime();
}

void CMsgService::SetMemoryItem(EmFunctionItem item, int value)
{
    //弃用
}

void CMsgService::ResetFunctionItem(EmFunctionItem item)
{
    //
}

void CMsgService::SetThemeColor(int theme, int color)
{
    std::lock_guard<std::mutex> lock(mSendMCUDataMutex);
    mStMemoryDataSync.repeatSendMemoryDataTimer->start();
    mStMemoryDataSync.repeatSendCount = 0;
    m_nCurTheme = theme;
    m_nCurColor = color;
    sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_MEMORY_INFO, {m_nCurTheme, m_nCurColor, m_nCurVoiceSwitch, m_nCurVoiceSize, 0, mSaveCurMenuItem}));
    mMemDataManager->DataManager_SetTheme(m_nCurTheme, m_nCurColor);
    HmiIPC::UpdateThemeColor(m_nCurTheme,m_nCurColor);
}

void CMsgService::SetVoicePlay(int VoiceSwitch, int VoiceSize)
{
    LOGDBG("------VoiceSwitch = %d,VoiceSize = %d",VoiceSwitch,VoiceSize);
    std::lock_guard<std::mutex> lock(mSendMCUDataMutex);
    mStMemoryDataSync.repeatSendMemoryDataTimer->start();
    mStMemoryDataSync.repeatSendCount = 0;
    m_nCurVoiceSwitch = VoiceSwitch;
    m_nCurVoiceSize = VoiceSize;
    sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_MEMORY_INFO, {m_nCurTheme, m_nCurColor, m_nCurVoiceSwitch, m_nCurVoiceSize, 0, mSaveCurMenuItem}));
	chime::chimeSettings_VoiceVolumeCtrl(VoiceSwitch, static_cast<chime::EnVolume>(VoiceSize));
    mMemDataManager->DataManager_SetVoiceSwtich(m_nCurVoiceSwitch, m_nCurVoiceSize);
    HmiIPC::UpdateVoicePlay(m_nCurVoiceSwitch,m_nCurVoiceSize);
}

void CMsgService::SetClearMileageInfo(int id, int state)
{
    uint32_t minorId = 0xffff;
    uint8_t sendData[3]= {0, 0, 0};
    if(id == 0)
    {
        minorId = DOWN_MINOR_DATA_RESET;
        sendData[1] = state;
    }
    else if (id == 1)
    {
        minorId = DOWN_MINOR_DATA_RESET;
        sendData[0] = state;
    }
    else
        return;
    if(0xffff != minorId)
    {
        sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, minorId, {sendData[0], sendData[1], sendData[2]}));
    }
}
void CMsgService::repeatSend0x81_Ontime()
{
    sendDataToMCU(encode.getFrame(DOWN_MAJOR_SYNC_DATA, {0, 0}));
	if(++mRepeatSend0x81Count < 10)
	{
		mRepeatSend0x810000Timer->restart();
	}
}
void CMsgService::SelfCheckState(int val)
{

	LOG_RECORD("%s, value = %d", __func__, val);
	if(mRepeatSend0x81Count == 0)
	{
		mRepeatSend0x81Count = 1;
        sendDataToMCU(encode.getFrame(DOWN_MAJOR_SYNC_DATA, {0, 0}));
		mRepeatSend0x810000Timer->start();
        LOG_RECORD("soc version:%s", mSocVersion.c_str());
	}
    if (0xFF == val) //hmi初始化完成
    {
		//
    }
    else
    {
        mbHmiReady = true;
		chime::chimeInput_HmiReady(1);
		o_stgCtx.oWarnIO.setHmiReady();
        LOGINF("hmi SELF_CHECK");
		#if OVER_WRITE_TT_SELFCHECK
		o_stgCtx.oTTProcess.setHMIPageReady();
		#else
		#endif
    }
}
           
void CMsgService::GetFuel(void)
{
    // LOGDBG();
    //float fuelValue = decode[0] == 0x1f ? 10.f : decode[0]*0.39;
    //float fuelValue = decode[0]*0.39;
    // HmiIPC::UpdateDispFuel(fuelValue, true, EmWarningState::NORMAL);
}

void CMsgService::chimeOutput_ChimeReady(int)
{
    if(mbHmiReady)
    {
        chime::chimeInput_HmiReady(1);
    }
    if(mbSetVoiceFlag)
    {
        chime::chimeSettings_VoiceVolumeCtrl(m_nCurVoiceSwitch, static_cast<chime::EnVolume>(m_nCurVoiceSize));
    }
}
void CMsgService::GetMemoryInfo_ACK(void)
{
    if( (m_nCurTheme == int(decode[0])) &&
        (m_nCurColor == int(decode[1])) &&
        (m_nCurVoiceSwitch == int(decode[2])) &&
        (m_nCurVoiceSize == int(decode[3])) &&
        (mSaveCurMenuItem == int(decode[5])))
    {
        LOG_RECORD("GetMemoryInfo_ACK");
        std::lock_guard<std::mutex> lock(mSendMCUDataMutex);
        mStMemoryDataSync.repeatSendMemoryDataTimer->stop();
    }
}
void CMsgService::GetMemoryInfo(void)
{
    m_nCurTheme = decode[0];
    m_nCurColor = decode[1];
    m_nCurVoiceSwitch = decode[2];
    m_nCurVoiceSize = decode[3];
	mSaveCurMenuItem = decode[5];
    LOG_RECORD("(%s,%d), d0:0x%02x,d1:0x%02x,d2:0x%02x,d3:0x%02x, d5:0x%02x",__func__, __LINE__, decode[0],decode[1],decode[2],decode[3], mSaveCurMenuItem);
    if(mStMemoryDataSync.isInitReCoverSts)
    {
        mStMemoryDataSync.isInitReCoverSts = false;
        HmiIPC::UpdateThemeColor(m_nCurTheme,m_nCurColor);
        HmiIPC::UpdateVoicePlay(m_nCurVoiceSwitch,m_nCurVoiceSize);
        chime::chimeSettings_VoiceVolumeCtrl(m_nCurVoiceSwitch, static_cast<chime::EnVolume>(m_nCurVoiceSize));
        mbSetVoiceFlag = true;
        //
        HmiIPC::UpdateTCInfoIndex(mSaveCurMenuItem);
    }
    sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_MEMORY_INFO_ACK, {int(decode[0]),int(decode[1]),int(decode[2]),int(decode[3]),int(decode[4]),int(decode[5])}));
    mMemDataManager->DataManager_SetTheme(m_nCurTheme, m_nCurColor);
    mMemDataManager->DataManager_SetVoiceSwtich(m_nCurVoiceSwitch, m_nCurVoiceSize);
    mMemDataManager->DataManager_SetTCIndex(mSaveCurMenuItem);
}

void CMsgService::GetVersion(void)
{
	//MCU版本号
	char mcuVersion[48];
	for(int i = 0; i < 40; i++)
	{
		if(decode[i])
		{
			mcuVersion[i] = char(decode[i] & 0xff);
		}
		else
		{
			mcuVersion[i] = 0;
			break;
		}
	}
	mcuVersion[40] = 0;//最多40个字符
	const char* saveVersionfile = "/tmp/A301-IC-Version.txt";
    if(access(saveVersionfile, F_OK) == -1)
    {
        FILE* fp = fopen(saveVersionfile, "w+");
        fprintf(fp,"ARM:%s\n", mSocVersion.c_str());
		fprintf(fp, "MCU:%s\n",mcuVersion);
        fflush(fp);
        fclose(fp);
    }
	LOG_RECORD("%s,ARM:%s\nMCU:%s", __func__, mSocVersion.c_str(),mcuVersion);
    HmiIPC::UpdateSysVersion(mSocVersion.c_str(), mcuVersion);
	upgrade::Upgrade_clusterVersion(mSocVersion.c_str(), mcuVersion);
}
void CMsgService::GetVehicleSpeed_Process(uint32_t* pData, uint32_t)
{
	static int s_speedvalue = -1;
    std::vector<std::pair<std::string, int>> lamp;
    bool validitySpeed = true, validityTime = true;
	(void)validitySpeed;
    int dirveTime = pData[2];
    LOGDBG("------dirveTime = %d",dirveTime);
    int time = dirveTime/60, minute = dirveTime % 60;

	if(s_speedvalue != int(pData[0]))
	{
		int preSpd = s_speedvalue;
		s_speedvalue = pData[0];
		float spdDisp = 0.f;
		bool vaildFlag = false;
		if(pData[0] >= 0x1FFF)
		{
			vaildFlag = false;
			spdDisp = 0.f;
		}
		else
		{
			float spdReal = s_speedvalue * 0.05625;
//			if(spdReal <= 5.f)
			if(spdReal < 6.f)
			{
				spdDisp = spdReal + 0.5f;
				vaildFlag = true;
			}
			else if(spdReal <=160.f)
			{
				spdDisp = 2 + (spdReal / 0.98f) + 0.5f;
				vaildFlag = true;
			}
			else if(spdReal <= 360.f)
			{
				spdDisp = 160.f;
				vaildFlag = true;
			}
			else
			{
				spdDisp = 0.f;
				vaildFlag = false;
			}
			if(spdDisp > 160.f)
			{
				spdDisp = 160.f;
			}
		}
		bool SpdOverFlag = mChimeSpdOverFlag;
		if(spdDisp > 100.f)
		{
			if(!mChimeSpdOverFlag)
			{
				SpdOverFlag = true;
			}
		}
		else
		{
			if(mChimeSpdOverFlag)
			{
				SpdOverFlag = false;
			}
		}
		if(SpdOverFlag != mChimeSpdOverFlag)
		{
			mChimeSpdOverFlag = SpdOverFlag;
			chime::chimeInput_OverSpdSts(mChimeSpdOverFlag);
		}
		mDisplaySpeed.first = spdDisp;
		mDisplaySpeed.second = vaildFlag;
		if(mCurPowerMode == 1)
		{
			HmiIPC::UpdateDispVehicleSpeed(spdDisp, vaildFlag);
			LaneAnimation_Speed(int(spdDisp));
		}
        mMemDataManager->DataManager_SetVehicleSpeed(uint16_t(spdDisp));
		if((preSpd == 0) || 
			(s_speedvalue == 0))
		{
			LOG_RECORD("s_speedvalue:%d, spdDisp:%f, vaildFlag = %d, mCurPowerMode:%d",s_speedvalue, spdDisp, vaildFlag, mCurPowerMode);
		}
	}

    if (0xFFFF == dirveTime)
        validityTime = false;
    HmiIPC::UpdateDispDrivingTime(time, minute, validityTime);
}
void CMsgService::GetVehicleSpeed(void)
{
    mVehSpdSigDelayProcessor->updateSignal(decode.getVecData());
}
void CMsgService::GetOdograph(void)
{
    uint32_t deltaODO = 0;
    if(mOdoFirstUpdate)
    {
        mOdoFirstUpdate = false;
        LOG_RECORD("odo:%d, tripA:%d, tripB:%d", decode[1], decode[0], decode[2]);
    }
    else
    {
        deltaODO = (mOdoValue > decode[1]) ? (mOdoValue - decode[1]) : (decode[1] - mOdoValue);
    }
    mOdoValue = decode[1];
    HmiIPC::UpdateDispTripA(decode[0]*0.1f);
    if(deltaODO > 1000)
    {
        mOdoDelayCount++;
        if(mOdoDelayCount > 3)
        {
            HmiIPC::UpdateDispOdograph(decode[1]);
        }
        LOG_RECORD("odo:%d, tripA:%d, tripB:%d count:%d", decode[1], decode[0], decode[2], mOdoDelayCount);
    }
    else
    {
        mOdoDelayCount = 0;
        HmiIPC::UpdateDispOdograph(decode[1]);
    }
    HmiIPC::UpdateDispTripB(decode[2]*0.1f);
    
    mMemDataManager->DataManager_SetTripA(decode[0]*0.1f);
    mMemDataManager->DataManager_SetTripB(decode[2]*0.1f);
    mMemDataManager->DataManager_SetODO(decode[1]);
}

void CMsgService::GetBatteryVoltage_Process(uint32_t* pData, uint32_t)
{
    bool validity = true;
    static int sSOCValue = 0;
    static bool sSOCStatus = false;
    //static uint32_t nSOCAle = 0;
    int SOCValue = pData[0];
    bool SOCStatus = pData[1];
    if(sSOCValue != SOCValue || sSOCStatus != SOCStatus)
    {
        sSOCValue = SOCValue;
        sSOCStatus = SOCStatus;
        mMemDataManager->DataManager_SetSocElectricity(validity ? SOCValue : 0xff, sSOCStatus ? 1:0);
        HmiIPC::UpdateDispSOCValue(SOCValue, SOCStatus, validity);
    }
	LOG_RECORD_DEBUG("1234%s, SOCValue:%d, SOCStatus:%d", __func__, SOCValue, SOCStatus);
}
void CMsgService::GetBatteryVoltage(void)
{
	mBatteryVoltageDelayProcessor->updateSignal(decode.getVecData());
}

void CMsgService::HeartbeatSignal(std::string moudleName, int count)
{
	if(moudleName == "MsgService")
	{
		int countResp = count + 1;
		WatchIpc::HeartbeatSignal_Resp("MsgService", countResp);
	}
}
void CMsgService::NotifyMcuReboot(int)
{
    sendDataToMCU(encode.getFrame(MAJOR_ENTER_UPGRADE_MODE, {0x06}));
}
void CMsgService::GetEngineSpeed_Process(uint32_t* pData, uint32_t)
{
    static float s_MotorSpeed = 0.0;
    bool validity = true;
    if(pData[0] == 0xffff)
        validity = false;
    float MotorSpeed = pData[0] * 0.001;
    if(s_MotorSpeed != MotorSpeed)
    {
        s_MotorSpeed = MotorSpeed;
        HmiIPC::UpdateDispMotorSpeed(MotorSpeed, validity);
    }
    
}
void CMsgService::GetEngineSpeed(void)
{
    mEngineSpdDelayProcessor->updateSignal(decode.getVecData());
}

void CMsgService::GetEnergyValue_Process(uint32_t* pData, uint32_t)
{
    static int s_PowerValue = 0xfe;
    bool validity = true;
    if(pData[0] == 255)
        validity = false;
    int value = pData[0] - 100;
    if(validity == true)
    {
        if(value > 100)
            value = 100;
        if (value < -100)
            value = -100;
    }
    if(s_PowerValue != value)
    {
		if(!s_PowerValue || !value)
		{
			LOG_RECORD("%s value:%d, validity:%d, mCurPowerMode:%d", __func__, value, validity, mCurPowerMode);
		}
        s_PowerValue = value;
		mDisplayPowerValue.first = value;
		mDisplayPowerValue.second = validity;
        mMemDataManager->DataManager_SetPowerValue(validity ? value : 0xffff);
		if(mCurPowerMode == 1)
		{
			HmiIPC::UpdateDispPowerValue(value, validity);
		}
    }
    static int s_EnergyFlowValue = 0;
    int EnergyFlowValue = pData[1];
    if(s_EnergyFlowValue != EnergyFlowValue)
    {
        s_EnergyFlowValue = EnergyFlowValue;
        HmiIPC::UpdateDispEnergyFlow(EnergyFlowValue);
    }
    //static int s_EnergyRecycleValue = 0;
    int EnergyRecycleValue = pData[3];
    HmiIPC::UpdateDispEnergyRecycle(EnergyRecycleValue);
    mMemDataManager->DataManager_SetEgyRgnStyle(uint8_t(EnergyRecycleValue));
   /* if(s_EnergyRecycleValue != EnergyRecycleValue)
    {
        s_EnergyRecycleValue = EnergyRecycleValue;
        HmiIPC::UpdateDispEnergyRecycle(EnergyRecycleValue);
    }*/
	
}
void CMsgService::GetEnergyValue(void)
{
	mEnergyDelayProcessor->updateSignal(decode.getVecData());
}

#define SWValue(value) ((value) == 1 ? EmSWState::open : EmSWState::close)

void CMsgService::ttSelfCheck_OnChangeSts()
{
	//
}

void CMsgService::GetVehicleLightStatus_Process(uint32_t* pData, uint32_t len)
{
	o_stgCtx.oTTProcess.setTTData(pData, len);
	LOG_RECORD_DEBUG("%s() len = %d", __func__, len);
	if(pData[13] == 1)
	{
		SET_BITWISE_STATAS(mChimeIconBitwiseData, chime::EnChimeLampBitwise::ICON_BITWISE_READY);
	}
	else
	{
		CLEAR_BITWISE_STATAS(mChimeIconBitwiseData, chime::EnChimeLampBitwise::ICON_BITWISE_READY);
	}
	if(pData[0] == 1)
	{
		SET_BITWISE_STATAS(mChimeIconBitwiseData, chime::EnChimeLampBitwise::ICON_BITWISE_LOWBREAKFLUID);
	}
	else
	{
		CLEAR_BITWISE_STATAS(mChimeIconBitwiseData, chime::EnChimeLampBitwise::ICON_BITWISE_LOWBREAKFLUID);
	}
    mMemDataManager->DataManager_SetLampData0(uint8_t(pData[0]));
    mMemDataManager->DataManager_SetLampData1(uint8_t(pData[1]));
    mMemDataManager->DataManager_SetLampData2(uint8_t(pData[2]));
    mMemDataManager->DataManager_SetLampData3(uint8_t(pData[3]));
    mMemDataManager->DataManager_SetLampData4(uint8_t(pData[4]));
	chime::chimeInput_Icon(mChimeIconBitwiseData);
}
void CMsgService::GetVehicleLightStatus(void)
{
	mLampSigDelayProcessor->updateSignal(decode.getVecData());
}


void CMsgService::TimerSelfInspet()
{
#if OVER_WRITE_TT_SELFCHECK
#else
    LOGINF("m_pTimerSelfInspet timeout");
    // 自检结束后恢复自检警告灯正常显示
    HmiIPC::UpdateDispLamp(m_vecSelfInspet);

    // 同步不自检(灭状态的指示警告灯不发送)
    for (auto itor=m_vecNotSelfInspet.begin(); itor != m_vecNotSelfInspet.end(); itor++) {
        if ((*itor).state == EmLampState::EXTINGUISH || (*itor).state == EmLampState::NONE)
        {
            LOGINF("m_vecNotSelfInspet %s-%d", (*itor).strID.data(), (*itor).state);
            m_vecNotSelfInspet.erase(itor);
        }
    }
    HmiIPC::UpdateDispLamp(m_vecNotSelfInspet);

    GetSelfInspet(EmPowerStatus::EM_POWER_NONE,false,true/*,-1,false,-1,false*/,"001");
#endif
}

bool CMsgService::GetSelfInspet(EmPowerStatus ePowerSts,bool bHmiReady,bool bSelfInspet/*,int nSpd,bool bSpdValid,int nPower,bool bPowerValid*/,std::string sMask)
{
#if OVER_WRITE_TT_SELFCHECK
	return true;
#else
    std::lock_guard<std::mutex> guard(m_lockSelfInspet);

    static struct d { bool bSelfInspet = false; EmPowerStatus ePowerSts = EmPowerStatus::EM_POWER_OFF; bool bHmiReady = false;/* int nSpd = 0; bool bSpdValid = false; int nPower = 0; bool bPowerValid = false;*/ bool bTimerRuning = false; } dd;
    const int nLen = sMask.length();

    // 近获取返回值状态,不参与逻辑处理
    if (nLen == 0 || (nLen < 3 && dd.bSelfInspet)) return dd.bSelfInspet;

    for (int i=0;i<nLen;i++)
    {
        if (sMask[i] == '0') continue;

        switch (i)
        {
            case 0:
            {
                if ((ePowerSts != dd.ePowerSts) && (ePowerSts == EmPowerStatus::EM_POWER_OFF || ePowerSts == EmPowerStatus::EM_POWER_ON))
                {
                    dd.ePowerSts = ePowerSts;
                    if (dd.ePowerSts == EmPowerStatus::EM_POWER_ON && dd.bHmiReady)
                    {
                        std::vector<StuTelltableLampState> bri;
                        for (auto i=m_vecSelfInspet.begin(); i<m_vecSelfInspet.end(); i++)
                        {
                            StuTelltableLampState t;
                            t.strID = (*i).strID;
                            t.state = EmLampState::BRIGHT;
                            bri.push_back(t);
                        }
                        HmiIPC::UpdateDispLamp(bri);
                        m_pTimerSelfInspet->start();
                        dd.bTimerRuning = true;
                        dd.bSelfInspet = false;
                        LOGERR("m_pTimerSelfInspet running. m_vecSelfInspet:%d",m_vecSelfInspet.size());
                    }
                    else if (dd.bTimerRuning)
                    {
                        HmiIPC::UpdateDispLamp(m_vecSelfInspet);
                        m_pTimerSelfInspet->stop();
                    }
                }
            }
                break;
            case 1:
            {
                // if (bHmiReady != dd.bHmiReady)
                {
                    dd.bHmiReady = bHmiReady;
                    LOGINF("bHmiReady:%d", dd.bHmiReady);
                    if (/*dd.ePowerSts == EmPowerStatus::EM_POWER_ON && */dd.bHmiReady)
                    {
                        std::vector<StuTelltableLampState> bri;
                        for (auto i=m_vecSelfInspet.begin(); i<m_vecSelfInspet.end(); i++)
                        {
                            StuTelltableLampState t;
                            t.strID = (*i).strID;
                            t.state = EmLampState::BRIGHT;
                            bri.push_back(t);
                        }
                        HmiIPC::UpdateDispLamp(bri);
                        m_pTimerSelfInspet->start();
                        dd.bSelfInspet = false;
                        LOGERR("1 m_pTimerSelfInspet running. m_vecSelfInspet:%d",m_vecSelfInspet.size());
                    }
                }
            }
                break;
            case 2:
            {
                if (bSelfInspet != dd.bSelfInspet)
                {
                    dd.bSelfInspet = bSelfInspet;
                    dd.bTimerRuning = false;

                    // if (dd.bSelfInspet)
                    // {
                    //     HmiIPC::UpdateDispVehicleSpeed(dd.nSpd, dd.bSpdValid);
                    //     HmiIPC::UpdateDispPowerValue(dd.nPower, dd.bPowerValid);
                    // }
                }
            }
                break;
            // case 3:
            // {
            //     if (nSpd != dd.nSpd)
            //     {
            //         dd.nSpd = nSpd;
            //     }
            // }
            //     break;
            // case 4:
            // {
            //     if (bSpdValid != dd.bSpdValid)
            //     {
            //         dd.bSpdValid = bSpdValid;
            //     }
            // }
            //     break;
            // case 5:
            // {
            //     if (nPower != dd.nPower)
            //     {
            //         dd.nPower = nPower;
            //     }
            // }
            //     break;
            // case 6:
            // {
            //     if (bPowerValid != dd.bPowerValid)
            //     {
            //         dd.bPowerValid = bPowerValid;
            //     }
            // }
            //     break;
            default:
                break;
        }
    }

    return dd.bSelfInspet;
#endif
}

void CMsgService::UpdNotSelfInspet(std::string sId, EmLampState e)
{
    if (e == EmLampState::EXTINGUISH || e == EmLampState::NONE) return;
    //std::lock_guard<std::mutex> guard(m_lockSelfInspet);

    const uint32_t size = m_vecNotSelfInspet.size();
    uint32_t i = 0;

    for (; i<size; i++)
    {
        if (m_vecNotSelfInspet[i].strID == sId && m_vecNotSelfInspet[i].state != e)
        {
            m_vecNotSelfInspet[i].state = e;
        }
    }

    if (i >= size)
    {
        StuTelltableLampState t;
        t.strID = sId;
        t.state = e;
        m_vecNotSelfInspet.push_back(t);
    }
}

void CMsgService::sendDataToMCU(const std::string& frameData, bool isLogRecord)
{
	mMsgDev.send(frameData);
    std::string ret(encode.getErrorStr());
    if(ret != "")
    {
        LOG_RECORD("ecode:%s\n", ret.c_str());
    }
	if(isLogRecord)
	{
		LOGRECORDEHEX(frameData.data(), frameData.length(), "Msg", "Send2MCU");
	}
}

void CMsgService::heartbeat_OnTimer()
{
    NotifyMcuHeartbeatPacket(45*100);//四十五秒,精度10ms
}

void CMsgService::GetTurnLightStatus_Process(uint32_t* pData, uint32_t)
{
	//LOG_RECORD_DEBUG("%s, d0 = 0x%02d, d1=0x%02x", __func__, pData[0], pData[1]);
    HmiIPC::UpdateTurnLampSts(pData[0],pData[1],0); 
	//
	if(pData[0])
	{
		SET_BITWISE_STATAS(mChimeIconBitwiseData, chime::EnChimeLampBitwise::ICON_BITWISE_TURN_LEFT);
	}
	else
	{
		CLEAR_BITWISE_STATAS(mChimeIconBitwiseData, chime::EnChimeLampBitwise::ICON_BITWISE_TURN_LEFT);
	}
	if(pData[1])
	{
		SET_BITWISE_STATAS(mChimeIconBitwiseData, chime::EnChimeLampBitwise::ICON_BITWISE_TURN_RIGHT);
	}
	else
	{
		CLEAR_BITWISE_STATAS(mChimeIconBitwiseData, chime::EnChimeLampBitwise::ICON_BITWISE_TURN_RIGHT);
	}
	//LOG_RECORD_DEBUG("(%s,%d), mChimeIconBitwiseData:%0x\n",__func__, __LINE__, mChimeIconBitwiseData);
	chime::chimeInput_Icon(mChimeIconBitwiseData);
}
void CMsgService::GetTurnLightStatus(void)
{
	mTurnLampSigDelayProcessor->updateSignal(decode.getVecData());
}

void CMsgService::GetsafetyBeltOrAirbag_Process(uint32_t* pData, uint32_t)
{
    auto StateTransition = [](int value) -> EmLampState
    {
        switch (value)
        {
        case 0:
            return EmLampState::EXTINGUISH;
        case 1:
            return EmLampState::BRIGHT;
        case 2:
            return EmLampState::TWINKLE_0_5HZ;
        case 3:
            return EmLampState::TWINKLE_1HZ;
        case 4:
            return EmLampState::TWINKLE_2HZ;
        case 5:
            return EmLampState::TWINKLE_4HZ;
        default:
            return EmLampState::NONE;
        }
    };
    std::vector<StuTelltableLampState>
        lamp = {
            {LAMP_MAIN_SEATBELT_INDICATOR, StateTransition(pData[0])},
            // {LAMP_FRONT_SEATBELT_INDICATOR_2, StateTransition(pData[1])},
            {LAMP_AIR_BAG_FAULT, StateTransition(pData[2])},
        };
    HmiIPC::UpdateDispLamp(lamp);
}
void CMsgService::GetsafetyBeltOrAirbag(void)
{
	mSafeBeltDelayProcessor->updateSignal(decode.getVecData());
}

void CMsgService::GetFaultCode(void)
{
    std::vector<std::string> FCTable;
    for (int i = 0; i < 64; i++)
    {
        if (1 == decode[i])
            FCTable.push_back(FaultCodeTable[i]);
    }

    //HmiIPC::PushFaultCodeList(FCTable);
}

void CMsgService::GetPopUpAlarm_Process(uint32_t* pData, uint32_t)
{
	char inputWarnData[8];
	for(int i = 0; i < 8; i++)
	{
		inputWarnData[i] = pData[i];
	}
	LOG_RECORD_DEBUG("_warn_%s d0:0x%02x,d1:0x%02x,d2:0x%02x,d3:0x%02x,d4:0x%02x,d5:0x%02x,d6:0x%02x,d7:0x%02x,",__func__,pData[0],pData[1],pData[2],pData[3],pData[4],pData[5],pData[6],pData[7]);
	static char sTpms = 0;
	if(sTpms != inputWarnData[0])
	{
		sTpms = inputWarnData[0];
		//胎压bit位特殊处理，调换bit位,bit低的优先级高
		uint8_t tpms = uint8_t(inputWarnData[0]);
		uint8_t tmps_AirLeak = tpms & 0x08;
		uint8_t tmps_PressLH = tpms & 0x06;
		uint8_t tmpsNewValue = (tmps_PressLH << 1) | (tmps_AirLeak >> 2);
		inputWarnData[0] = (tpms & 0xF1) | tmpsNewValue;
		LOG_RECORD_DEBUG("_warn_%s,tmpsNewValue:0x%02x", __func__, tmpsNewValue);
	}
    char doorSts = inputWarnData[7] >> 1 & 0x3f;
    if(o_stgCtx.oWarnIO.getDoorSts() != doorSts)
    {
        HmiIPC::CarDoorStatus(doorSts);
    }
	o_stgCtx.oWarnIO.warningInputData(inputWarnData);
    
	static int sbcmBuzWarnMod = 0, s_chimeData6 = 0, s_chimeData8 = 0;
	if(sbcmBuzWarnMod != int(pData[4]))
	{
		sbcmBuzWarnMod = pData[4];
		chime::chimeInput_BcmBuzWarnMod(sbcmBuzWarnMod);
	}
	//
	int chime6 = pData[6] & 0xf0;
	if(pData[7] &  0x80)
	{//预约空调
		chime6 |= 0x01;
	}
	if((s_chimeData6 ^ chime6) || 
		(s_chimeData8 ^ pData[8]) )
	{
		s_chimeData6 = pData[6];
		s_chimeData8 = pData[8];
		chime::chimeInput_Data6_8(s_chimeData6, s_chimeData8);
	}
}
void CMsgService::GetPopUpAlarm(void)
{
	mPopUpAlarmDelayProcessor->updateSignal(decode.getVecData());
}

void CMsgService::togetherProcess()
{
	std::lock_guard<std::mutex> lock(CSigDelayProcessor::sMutex);
	bool bSend = false;

    // 预约充电开始时间
    if (m_nChargeOrderStart.Year!=mStUpdateCharging.ChargeSubscribeData[0] || m_nChargeOrderStart.Month!=mStUpdateCharging.ChargeSubscribeData[1] || m_nChargeOrderStart.Day!=mStUpdateCharging.ChargeSubscribeData[2] || m_nChargeOrderStart.Hour!=mStUpdateCharging.ChargeSubscribeData[3] || m_nChargeOrderStart.Minute!=mStUpdateCharging.ChargeSubscribeData[4])
    {
        if (mStUpdateCharging.ChargeSubscribeData[0]<0 || mStUpdateCharging.ChargeSubscribeData[0]>255) LOG_SERROR("m_nChargeOrderSYear:%d",mStUpdateCharging.ChargeSubscribeData[0]);
        else if (m_nChargeOrderStart.Year != mStUpdateCharging.ChargeSubscribeData[0]) m_nChargeOrderStart.Year = mStUpdateCharging.ChargeSubscribeData[0];

        if (mStUpdateCharging.ChargeSubscribeData[1]<0 || mStUpdateCharging.ChargeSubscribeData[1]>12) LOG_SERROR("m_nChargeOrderSMonth:%d",mStUpdateCharging.ChargeSubscribeData[1]);
        else if (m_nChargeOrderStart.Month != mStUpdateCharging.ChargeSubscribeData[1]) m_nChargeOrderStart.Month = mStUpdateCharging.ChargeSubscribeData[1];

        if (mStUpdateCharging.ChargeSubscribeData[2]<0 || mStUpdateCharging.ChargeSubscribeData[2]>31) LOG_SERROR("m_nChargeOrderSDay:%d",mStUpdateCharging.ChargeSubscribeData[2]);
        else if (m_nChargeOrderStart.Day != mStUpdateCharging.ChargeSubscribeData[2]) m_nChargeOrderStart.Day = mStUpdateCharging.ChargeSubscribeData[2];

        if (mStUpdateCharging.ChargeSubscribeData[3]<0 || mStUpdateCharging.ChargeSubscribeData[3]>23) LOG_SERROR("m_nChargeOrderSHour:%d",mStUpdateCharging.ChargeSubscribeData[3]);
        else if (m_nChargeOrderStart.Hour != mStUpdateCharging.ChargeSubscribeData[3]) m_nChargeOrderStart.Hour = mStUpdateCharging.ChargeSubscribeData[3];

        if (mStUpdateCharging.ChargeSubscribeData[4]<0 || mStUpdateCharging.ChargeSubscribeData[4]>59) LOG_SERROR("m_nChargeOrderSMinute:%d",mStUpdateCharging.ChargeSubscribeData[4]);
        else if (m_nChargeOrderStart.Minute != mStUpdateCharging.ChargeSubscribeData[4]) m_nChargeOrderStart.Minute = mStUpdateCharging.ChargeSubscribeData[4];

        //HmiIPC::UpdateChargeOrderTimeStart(m_nChargeOrderSYear,m_nChargeOrderSMonth,m_nChargeOrderSDay,m_nChargeOrderSHour,m_nChargeOrderSMinute);
        bSend = true;
    }
    //LOGERR("m_eChargeOrderSts = %d,mStUpdateCharging.ChargeSubscribeData[5] = %d",m_eChargeOrderSts,mStUpdateCharging.ChargeSubscribeData[5]);
    if (mStUpdateCharging.ChargeSubscribeData[5] != int32_t(m_eChargeOrderSts))
    {
        //HmiIPC::UpdateChargeOrder(m_eChargeOrderSts = (CHARGE_ORDER)mStUpdateCharging.ChargeSubscribeData[5]);
        m_eChargeOrderSts = (CHARGE_ORDER)mStUpdateCharging.ChargeSubscribeData[5];
        bSend = true;
    }
    //LOGERR("m_eChargeOrderMode = %d,mStUpdateCharging.ChargeSubscribeData[6] = %d",m_eChargeOrderMode,mStUpdateCharging.ChargeSubscribeData[6]);
    if (mStUpdateCharging.ChargeSubscribeData[6] != m_eChargeOrderMode)
    {
        //HmiIPC::UpdateChargeOrderMode(m_eChargeOrderMode = (CHARGE_ORDER_MODE)mStUpdateCharging.ChargeSubscribeData[6]);
        m_eChargeOrderMode = (CHARGE_ORDER_MODE)mStUpdateCharging.ChargeSubscribeData[6];
        bSend = true;
    }

    if (int(mStUpdateCharging.ChargeSubscribeData[7]) != ChargStateVilid)
    {
        ChargStateVilid = mStUpdateCharging.ChargeSubscribeData[7];
        bSend = true;
    }

     if (int(mStUpdateCharging.ChargeSubscribeData[8]) != ChargTimeVilid)
    {
        ChargTimeVilid = mStUpdateCharging.ChargeSubscribeData[8];
        bSend = true;
    }

    // 预约充电结束时间
    if (m_nChargeOrderEnd.Year!=mStUpdateCharging.ChargeSubscribeData[10] || m_nChargeOrderEnd.Month!=mStUpdateCharging.ChargeSubscribeData[11] || m_nChargeOrderEnd.Day!=mStUpdateCharging.ChargeSubscribeData[12] || m_nChargeOrderEnd.Hour!=mStUpdateCharging.ChargeSubscribeData[13] || m_nChargeOrderEnd.Minute!=mStUpdateCharging.ChargeSubscribeData[14])
    {
        if (mStUpdateCharging.ChargeSubscribeData[10]<0 || mStUpdateCharging.ChargeSubscribeData[10]>255) LOG_SERROR("m_nChargeOrderEYear:%d",mStUpdateCharging.ChargeSubscribeData[10]);
        else if (m_nChargeOrderEnd.Year != mStUpdateCharging.ChargeSubscribeData[10]) m_nChargeOrderEnd.Year = mStUpdateCharging.ChargeSubscribeData[10];

        if (mStUpdateCharging.ChargeSubscribeData[11]<0 || mStUpdateCharging.ChargeSubscribeData[11]>12) LOG_SERROR("m_nChargeOrderEMonth:%d",mStUpdateCharging.ChargeSubscribeData[11]);
        else if (m_nChargeOrderEnd.Month != mStUpdateCharging.ChargeSubscribeData[11]) m_nChargeOrderEnd.Month = mStUpdateCharging.ChargeSubscribeData[11];

        if (mStUpdateCharging.ChargeSubscribeData[12]<0 || mStUpdateCharging.ChargeSubscribeData[12]>31) LOG_SERROR("m_nChargeOrderEDay:%d",mStUpdateCharging.ChargeSubscribeData[12]);
        else if (m_nChargeOrderEnd.Day != mStUpdateCharging.ChargeSubscribeData[12]) m_nChargeOrderEnd.Day = mStUpdateCharging.ChargeSubscribeData[12];

        if (mStUpdateCharging.ChargeSubscribeData[13]<0 || mStUpdateCharging.ChargeSubscribeData[13]>23) LOG_SERROR("m_nChargeOrderEHour:%d",mStUpdateCharging.ChargeSubscribeData[13]);
        else if (m_nChargeOrderEnd.Hour != mStUpdateCharging.ChargeSubscribeData[13]) m_nChargeOrderEnd.Hour = mStUpdateCharging.ChargeSubscribeData[13];

        if (mStUpdateCharging.ChargeSubscribeData[14]<0 || mStUpdateCharging.ChargeSubscribeData[14]>59) LOG_SERROR("m_nChargeOrderEMinute:%d",mStUpdateCharging.ChargeSubscribeData[14]);
        else if (m_nChargeOrderEnd.Minute != mStUpdateCharging.ChargeSubscribeData[14]) m_nChargeOrderEnd.Minute = mStUpdateCharging.ChargeSubscribeData[14];

        //HmiIPC::UpdateChargeOrderTimeEnd(m_nChargeOrderEYear,m_nChargeOrderEMonth,m_nChargeOrderEDay,m_nChargeOrderEHour,m_nChargeOrderEMinute);
        bSend = true;
    }
    //LOGERR("m_eChargeAC = %d,m_eChargingSts = %d, ChargStateVilid = %d,ChargTimeVilid = %d\n",m_eChargeAC,m_eChargingSts,ChargStateVilid,ChargTimeVilid);
    //LOGERR("bSend = %d, m_nElectricType = %d",bSend,m_nElectricType);
    // 交流枪充电状态为连接充电状态（0x1）,以及充电状态为0x0（没有充电）或0x3（充电结束）——HMI显示预约充电状态和预约充电时间 SRD 3.18.2
    /*if (bSend && (m_nElectricType == 0))
    {
        HmiIPC::UpdateChargeSubscribe(m_nElectricType, m_eChargingSts, (int)m_eChargeOrderSts,(int)m_eChargeOrderMode,ChargStateVilid, ChargTimeVilid,\
            m_nChargeOrderStart,m_nChargeOrderEnd);
    }*/
    LOG_RECORD_DEBUG("%s mStUpdateCharging.ChargeSubscribeData[5]:0x%02x, mStUpdateCharging.ChargeSubscribeData[6]:0x%02x",__func__,mStUpdateCharging.ChargeSubscribeData[5], mStUpdateCharging.ChargeSubscribeData[6]);
	///////////////////////////////////////////////
	static uint32_t nChargingTime = -1;
    static uint32_t nChargingVol = -1;
    static uint32_t nChargingEle = -1;
   // LOGERR("m_eChargeAC = %d,mStUpdateCharging.ChargingData[1] = %d",m_eChargeAC,mStUpdateCharging[1]);
    if (mStUpdateCharging.ChargingData[1] != m_eChargeAC)
    {
        m_eChargeAC = (CHARGE_DEVICE)mStUpdateCharging.ChargingData[1] ;
        bSend = true;
        // 交流枪充电状态为连接充电状态（0x1）,以及充电状态为0x0（没有充电）或0x3（充电结束）——HMI显示预约充电状态和预约充电时间 SRD 3.18.2
        /*if (m_eChargeAC == CHARGE_DEVICE_CONNECTED && (mStUpdateCharging.ChargingData[2] == (int)CHARGING_NONE || mStUpdateCharging.ChargingData[2] == (int)CHARGING_END) && \
           (m_eChargeOrderSts != CHARGE_ORDER_RESERVED) && (m_nChargeOrderSYear > 0) && (m_nChargeOrderEYear > 0))
        {
            HmiIPC::UpdateChargeSubscribe((int)m_eChargeOrderSts,(int)m_eChargeOrderMode,\
                m_nChargeOrderSYear,m_nChargeOrderSMonth,m_nChargeOrderSDay,m_nChargeOrderSHour,m_nChargeOrderSMinute,\
                m_nChargeOrderEYear,m_nChargeOrderEMonth,m_nChargeOrderEDay,m_nChargeOrderEHour,m_nChargeOrderEMinute);
        }*/
    }
    //LOGERR("m_eChargeDC = %d,mStUpdateCharging.ChargingData[0] = %d",m_eChargeDC,mStUpdateCharging.ChargingData[0]);
    if (mStUpdateCharging.ChargingData[0] != (int32_t)m_eChargeDC)
    {
        m_eChargeDC = (CHARGE_DEVICE)mStUpdateCharging.ChargingData[0] ;
        bSend = true;
    }
	LOG_RECORD_DEBUG("m_eChargingSts(%d) = %d\n",m_eChargingSts,mStUpdateCharging.ChargingData[2]);
    // 充电状态
    if ((int32_t)m_eChargingSts != mStUpdateCharging.ChargingData[2])
    {
        switch (mStUpdateCharging.ChargingData[2]) {
            case 0x0:
                m_eChargingSts = CHARGING_NONE;
                break;
            case 0x1:
                m_eChargingSts = CHARGING_DC;
                break;
            case 0x2:
                m_eChargingSts = CHARGING_AC;
                break;
            case 0x3:
                m_eChargingSts = CHARGING_END;
                break;
            case 0x4:
                m_eChargingSts = CHARGING_FAULT;
                break;
            default:
                m_eChargingSts = CHARGING_RESERVED;
                break;
        }
        //HmiIPC::UpdateCharging(m_eChargingSts);
        bSend = true;
    }

    // 预估充电时间
    if (int32_t(nChargingTime) != mStUpdateCharging.ChargingData[3])
    {
        nChargingTime = mStUpdateCharging.ChargingData[3];
        //HmiIPC::UpdateChargingTime(nChargingTime);
        bSend = true;
    }

    // 充电电流
    if (int32_t(nChargingEle) != mStUpdateCharging.ChargingData[4])
    {
        nChargingEle = mStUpdateCharging.ChargingData[4];
        //HmiIPC::UpdateChargingVol(nChargingEle);
        bSend = true;
    }

    // 充电电压
    if (int32_t(nChargingVol) != mStUpdateCharging.ChargingData[5])
    {
        nChargingVol = mStUpdateCharging.ChargingData[5];
        //HmiIPC::UpdateChargingEle(nChargingVol);
        bSend = true;
    }

    if (bSend) {
        if(m_eChargeAC == 1 && m_eChargeDC == 0)
        {
            m_nElectricType = 0;
        }
        else if(m_eChargeAC == 1 && m_eChargeDC == 2)
        {
            m_nElectricType = 0;
        }
        else if(m_eChargeDC == 1 && m_eChargeAC == 1)
        {
            m_nElectricType = 2;
        }
        else if(m_eChargeDC == 1 && m_eChargeAC != 1 && m_eChargeAC <= 7)
        {
            m_nElectricType = 1;
        }
        else
        {
            m_nElectricType = 3;
        }
        HmiIPC::UpdateCharging(m_nElectricType,(int)m_eChargingSts,nChargingTime/60,nChargingTime%60,nChargingVol,nChargingEle,\
        m_eChargeOrderSts,m_eChargeOrderMode,ChargStateVilid,ChargTimeVilid, m_nChargeOrderStart,m_nChargeOrderEnd);
    }
}
void CMsgService::GetChargeOrder(void)
{
	mChargeOrderDelayProcessor->updateSignal(decode.getVecData());
}
void CMsgService::GetChargeOrder_Process(uint32_t* pData, uint32_t len)
{
    memcpy(mStUpdateCharging.ChargeSubscribeData, pData, len*(sizeof(uint32_t)));
	if(mStUpdateCharging.delayProcessTimer->getStatus() != ZH::BaseLib::simpleTimerStatus::timer_running)
	{//收到数据后延迟处理
		mStUpdateCharging.delayProcessTimer->start();
	}
}

void CMsgService::GetChargeStatus(void)
{
	mChargeStsDelayProcessor->updateSignal(decode.getVecData());
}
void CMsgService::GetChargeStatus_Process(uint32_t* pData, uint32_t len)
{
    memcpy(mStUpdateCharging.ChargingData, pData, len*(sizeof(uint32_t)));
	LOG_RECORD_DEBUG("%s Data[0]:%d, Data[1]:%d, Data[2]:%d, data3;%d, len;%d\n", __func__,
		mStUpdateCharging.ChargingData[0], mStUpdateCharging.ChargingData[1], mStUpdateCharging.ChargingData[2], mStUpdateCharging.ChargingData[3], len);
	if(mStUpdateCharging.delayProcessTimer->getStatus() != ZH::BaseLib::simpleTimerStatus::timer_running)
	{//收到数据后延迟处理
		mStUpdateCharging.delayProcessTimer->start();
	}
}

void CMsgService::GetVoiceWarning(void)
{
    //
}


void CMsgService::GetResetAck(void)
{
    static MACK ackTripA = MACK_RESERVED;
    static MACK ackAvrPower = MACK_RESERVED;
    static MACK ackTravelMile = MACK_RESERVED;
    static MACK ackTravelTime = MACK_RESERVED;

    if (ackTripA != decode[0] && (decode[0] == 0x0 || decode[0] == 0x1)) {
        switch (decode[0]) {
            case 0x0:
                ackTripA = MACK_OK;
                break;
            case 0x1:
                ackTripA = MACK_INVALID;
                break;
            default:
                break;
        }
        HmiIPC::UpdateResetAck(RESET_MODE_TRIP_A,ackTripA);
    }

    if (ackAvrPower != decode[1] && (decode[1] == 0x0 || decode[1] == 0x1)) {
        switch (decode[1]) {
            case 0x0:
                ackAvrPower = MACK_OK;
                break;
            case 0x1:
                ackAvrPower = MACK_INVALID;
                break;
            default:
                break;
        }
        HmiIPC::UpdateResetAck(RESET_MODE_AVR_POWER,ackAvrPower);
    }

    if (ackTravelMile != decode[2] && (decode[2] == 0x0 || decode[2] == 0x1)) {
        switch (decode[2]) {
            case 0x0:
                ackTravelMile = MACK_OK;
                break;
            case 0x1:
                ackTravelMile = MACK_INVALID;
                break;
            default:
                break;
        }
        HmiIPC::UpdateResetAck(RESET_MODE_TRAVEL_MILE,ackTravelMile);
    }

    if (ackTravelTime != decode[3] && (decode[3] == 0x0 || decode[3] == 0x1)) {
        switch (decode[2]) {
            case 0x0:
                ackTravelTime = MACK_OK;
                break;
            case 0x1:
                ackTravelTime = MACK_INVALID;
                break;
            default:
                break;
        }
        HmiIPC::UpdateResetAck(RESET_MODE_TRAVEL_TIME,ackTravelTime);
    }
}

void CMsgService::InstEnergyConsum_OnTimer()
{//当瞬时能耗值没变化时，每隔1.5秒更新一次瞬时能耗
	std::lock_guard<std::mutex> lock(mInstEnergyConsumMutex);
	HmiIPC::UpdateInstEnergyConsum(mInstEnergyConsumValue * 0.1f);
	LOG_RECORD_DEBUG("%s,%d mInstEnergyConsumValue = %d", __func__, __LINE__, mInstEnergyConsumValue);
}
void CMsgService::GetRechargeMileage_Process(uint32_t* pData, uint32_t)
{
    //static int s_fuelValue = 0;
    static float preMilevalue = -1.0;
    static float preAverEnConsum = -1.0;

    bool validty = true;
    if(pData[0] >= 0x7FF)
        validty = false;
    if(preMilevalue != pData[0])
    {
        mMemDataManager->DataManager_SetDTE(pData[0]);
        preMilevalue = pData[0];
        HmiIPC::UpdateDispRechargeMileage(pData[0], validty);
    }

	do
	{
		std::lock_guard<std::mutex> lock(mInstEnergyConsumMutex);
		if(mInstEnergyConsumValue != pData[1])
		{
			mInstEnergyConsumValue = pData[1];
			mInstEnergyConsumTimer->start();
			if(pData[1] >= 0x1FF)
			{
				mInstEnergyConsumTimer->stop();
			}
		}
        mMemDataManager->DataManager_SetVehMomEgyCnse(mInstEnergyConsumValue);
		LOG_RECORD_DEBUG("%s,%d mInstEnergyConsumValue = %d", __func__, __LINE__, mInstEnergyConsumValue);
		HmiIPC::UpdateInstEnergyConsum(mInstEnergyConsumValue * 0.1f);
	} while (0);
	
    if(preAverEnConsum != pData[2])
    {
        preAverEnConsum = pData[2];
        mMemDataManager->DataManager_SetAVPC(pData[2]);
        HmiIPC::UpdateAverEnergyConsum(pData[2] * 0.1f);
    }
    chime::chimeInput_DTEValue(pData[0]);
}
void CMsgService::GetRechargeMileage(void)
{
	mRechargeDelayProcessor->updateSignal(decode.getVecData());
}

void CMsgService::GetTxtWarn(void)
{
    //
}

void CMsgService::GetwaterTempAndFuel(void)
{
    //
}

void CMsgService::GetPowerGear_Process(uint32_t* pData, uint32_t)
{
    LOGDBG(" GetGearStatus = %d",pData[3]);
    m_nGear = pData[3];
    
    const static EmGearsValue gearList[6] = {
            EmGearsValue::R_4HZ,
            EmGearsValue::P,
            EmGearsValue::R,
            EmGearsValue::N,
            EmGearsValue::D,
            EmGearsValue::S
    }; 
    EmGearsValue gear = m_nGear > 5 ? EmGearsValue::NONE : gearList[m_nGear];
    HmiIPC::UpdateDispGear(gear);
    mMemDataManager->DataManager_SetGear(uint8_t(gear));
}
void CMsgService::GetPowerGearStatus(void)
{
    mPowerGearProcessor->updateSignal(decode.getVecData());
}

void CMsgService::GetPowerMode_Process(uint32_t* pData, uint32_t)
{
    int powerState = pData[0];
    switch(powerState)
    {
        case 0x0:
            HmiIPC::UpdatePowerStatus(EmPowerStatus::EM_POWER_OFF);
			#if OVER_WRITE_TT_SELFCHECK
			o_stgCtx.oTTProcess.setPowerMode(0);
			#else
            GetSelfInspet(EmPowerStatus::EM_POWER_OFF,false,false/*,-1,false,-1,false*/,"1");
			#endif
            if(m_nGear == 0)
            {
                HmiIPC::UpdateDispGear(EmGearsValue::NONE);
            }
			HmiIPC::UpdateDispVehicleSpeed(0, false);
			LaneAnimation_Speed(0);
			HmiIPC::UpdateDispPowerValue(0, false);
			mRepeatSendIgnOffWarnIdTimer->start();
        break;
        case 0x1:
            HmiIPC::UpdatePowerStatus(EmPowerStatus::EM_POWER_ON);
			#if OVER_WRITE_TT_SELFCHECK
			o_stgCtx.oTTProcess.setPowerMode(1);
			#else
            GetSelfInspet(EmPowerStatus::EM_POWER_ON,false,false/*,-1,false,-1,false*/,"1");
			#endif
			mIgnDelayUpdateHmiTimer->start();
        break;
        default:
        break;
    }
	o_stgCtx.oWarnIO.setCurrentPowerMode(powerState);
	chime::chime_CurrentPowerMode(powerState);
}
void CMsgService::GetPowerMode(void)
{
    int powerState = decode[0];
	if(powerState > 1)
	{
		powerState = 1;
	}
	static int sendCount = 0;
	if((sendCount < 2) || (mCurPowerMode != powerState))
	{
		sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_IGN_STATUS_RESPONSE, {powerState}));
		sendCount++;
	}
	if(mCurPowerMode == powerState)
	{
		return;
	}
    mMemDataManager->DataManager_SetPowerMode(uint8_t(powerState));
	sendCount = 0;
	mCurPowerMode = powerState;
    LOGDBG("D7 GetPowerMode powerState = %d",powerState);
	LOG_RECORD("D7 GetPowerMode powerState = %d",powerState);
    
    mPowerModeProcessor->updateSignal(decode.getVecData());
	if(0 == powerState)
	{
		sendDataToMCU(encode.getFrame(DOWN_MAJOR_UNIVERSAL, DOWN_MINOR_IGN_OFF_POPSID, {mCurPopWarnID}));
	}
}

void CMsgService::GetScreenMode(void)
{
    int screenState = decode[0];
    LOGDBG("D8 GetScreenMode = %d",screenState);
	LOG_RECORD("D8 GetScreenMode = %d",screenState);
    //0x1 屏幕点亮
    if (screenState == 1)
    {
        StartupAnimation_Start("");
        HmiIPC::UpdateScreenStatus(EmScreenStatus::EM_SCREEN_ON);
    }
    else if(screenState == 0)
    {
        StartupAnimation_Stop(0);
        HmiIPC::UpdateScreenStatus(EmScreenStatus::EM_SCREEN_OFF);
    }
}
void CMsgService::GetDataSyncComplete()
{
	LOG_RECORD("%s\n", __func__);
	mRepeatSend0x810000Timer->stop();
}

void CMsgService::GetCurTime(void)
{
    LOGDBG("hour = %d, minute = %d\n",decode[3], decode[4]);
	#if 0
    char cmd[125] = {0};
    sprintf(cmd, "date -s %02d:%02d:%02d", decode[4], decode[5], decode[6]);
    system(cmd);
	system(cmd);
	#else
	struct tm *tmTime = NULL;
    time_t tTime;
    time(&tTime);
    tmTime = localtime(&tTime);
	struct timeval tv;
	tmTime->tm_year = decode[0] + 100;
	tmTime->tm_mon = decode[1] - 1;
	tmTime->tm_mday = decode[2];
	tmTime->tm_hour = decode[3];
	tmTime->tm_min = decode[4];
	tmTime->tm_sec = decode[5];
	tv.tv_sec = mktime(tmTime);
	tv.tv_usec = 0;
	settimeofday(&tv, nullptr);
	#endif

    // std::string strTime(buf, 5);
    // HmiIPC::UpdateDispCurTime(strTime);
    mCurrentTimeMode = decode[7];
    HmiIPC::UpdateTime(decode[3], decode[4], mCurrentTimeMode);
    mMemDataManager->DataManager_SetTime(uint8_t(decode[3]), uint8_t(decode[4]), uint8_t(mCurrentTimeMode));
    static uint32_t s_min = 0;
	if(s_min != decode[4])
	{
        s_min = decode[4];
		LOG_RECORD("%s, h(%02d):m(%02d):s:(%02d)", __func__, decode[3], decode[4], decode[5]);
	}
}
void CMsgService::UpdateCurTime()
{
    struct tm *tmTime = NULL;
    time_t tTime;
    time(&tTime);
    tmTime = localtime(&tTime);
    if (tmTime && (tmTime->tm_sec < 2))
    {
        HmiIPC::UpdateTime(tmTime->tm_hour, tmTime->tm_min, mCurrentTimeMode);
		LOG_SDEBUG("##### size = %u\n", o_stgCtx.oMsg.getMsgCount());
		if((tmTime->tm_min % 30) == 0)
		{
			LOG_RECORD("%s, %d-%02d-%02d %02d:%02d:%02d\n", __func__, int(tmTime->tm_year + 1900), (tmTime->tm_mon + 1), tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
		}
    }
}

void CMsgService::GetTemp_Process(uint32_t* pData, uint32_t)
{
    // static int s_waterTemp = 0;
    bool validity = true;
    int outTemp = pData[0] - 40;
    if ((outTemp > 120) || (outTemp < -40))
        validity = false;
    HmiIPC::UpdateDispOutsideTemp(outTemp, validity);

}
void CMsgService::GetTemp(void)
{
    mTempDelayProcessor->updateSignal(decode.getVecData());
}

std::string CMsgService::convertTirePressValue(uint16_t valueKpa, EmUnitPress unitPress)
{
    char pressValue[64];
    if(EmUnitPress::PRESS_BAR == unitPress)
    {
        float barValue = valueKpa * 0.01f;
        sprintf(pressValue, "%3.1f", barValue);
    }
    else if(EmUnitPress::PRESS_PSI == unitPress)
    {
        float psiValue = valueKpa *0.145f;
        sprintf(pressValue, "%3.1f", psiValue);
    }
    else /*if(EmUnitPress::PRESS_KPA == unitPress)*/
    {
        sprintf(pressValue, "%3d", valueKpa);
    }
    return pressValue;
}
void CMsgService::updateHmiTire()
{
    //迈亚项目功能
}

void CMsgService::GetRadarInfor(void)
{
    static uint32_t rLeft = 0;         static RADAR_DET_DIST eRLeft = RADAR_DET_DIST_VALID;
    static uint32_t rRight = 0;        static RADAR_DET_DIST eRRight = RADAR_DET_DIST_VALID;
    static uint32_t voiceWarn = 0;
    static uint32_t sysFault = 0;
    static uint32_t reverseSig = 0;

    RADAR_DET_DIST tmp = RADAR_DET_DIST_NONE;

    // 后右雷达设备监测距离
    if (rRight!=(tmp = (RADAR_DET_DIST)decode[0]) && (uint32_t)tmp>=0 && (uint32_t)tmp<=255) {
        rRight = (uint32_t)tmp;
        eRRight = ( (RADAR_DET_DIST_NONE != tmp && RADAR_DET_DIST_ABN != tmp) ? RADAR_DET_DIST_VALID : tmp );
        HmiIPC::UpdateRadarDetDist(RADAR_DEVICE_REAR_RIGHT, eRRight, rRight);
    }
    
    // 后左雷达设备监测距离
    if (rLeft!=(tmp = (RADAR_DET_DIST)decode[1]) && (uint32_t)tmp>=0 && (uint32_t)tmp<=255) {
        rLeft = (uint32_t)tmp;
        eRLeft = ( (RADAR_DET_DIST_NONE != tmp && RADAR_DET_DIST_ABN != tmp) ? RADAR_DET_DIST_VALID : tmp );
        HmiIPC::UpdateRadarDetDist(RADAR_DEVICE_REAR_LEFT, eRLeft, rLeft);
    }

    // 雷達聲音報警
    if (decode[2] != voiceWarn)
    {
        voiceWarn = decode[2];
    }

    // 雷達系統故障
    if (decode[3] != sysFault)
    {
        sysFault = decode[3];
    }

    // 倒擋信號
    if (decode[4] != reverseSig)
    {
        reverseSig = decode[4];
    }
}

void CMsgService::GetTireStatus_Process(uint32_t* pData, uint32_t)
{
    int LFTireVaild = pData[0];
    int LFTireStatus = pData [1];
    float LFTirePressValue = pData[3]/10.0;
    char LFtmp[10] = {0};
    sprintf(LFtmp, "%.1f", LFTirePressValue);

    int RFTireVaild = pData[4];
    int RFTireStatus = pData [5];
    float RFTirePressValue = pData[7]/10.0;
    char RFtmp[10] = {0};
    sprintf(RFtmp, "%.1f", RFTirePressValue);

    int LRTireVaild = pData[8];
    int LRTireStatus = pData [9];
    float LRTirePressValue = pData[11]/10.0;
    char LRtmp[10] = {0};
    sprintf(LRtmp, "%.1f", LRTirePressValue);

    int RRTireVaild = pData[12];
    int RRTireStatus = pData [13];
    float RRTirePressValue = pData[15]/10.0;
    char RRtmp[10] = {0};
    sprintf(RRtmp, "%.1f", RRTirePressValue);

    LOGDBG("------GetTireStatus PressValue = %d ,%d,%d,%d",LFTirePressValue,RFTirePressValue,LRTirePressValue,RRTirePressValue);
    LOGDBG("------GetTireStatus status = %d ,%d,%d,%d",LFTireStatus,RFTireStatus,LRTireStatus,RRTireStatus);
    StuCarTire mTire;
    mTire.LFTire.validFlag = LFTireVaild;
    //mTire.LFTire.tireStatus = LFTireVaild == 0 ? LFTireStatus:0;
    mTire.LFTire.tireStatus = LFTireStatus;
    mTire.LFTire.presValue = LFTireVaild == 0 ? LFtmp:"-.-";

    mTire.RFTire.validFlag = RFTireVaild;
    //mTire.RFTire.tireStatus = RFTireVaild == 0 ? RFTireStatus:0;
    mTire.RFTire.tireStatus = RFTireStatus;
    mTire.RFTire.presValue = RFTireVaild == 0 ? RFtmp:"-.-";

    mTire.LRTire.validFlag = LRTireVaild;
    //mTire.LRTire.tireStatus = LRTireVaild == 0 ?LRTireStatus:0;
    mTire.LRTire.tireStatus = LRTireStatus;
    mTire.LRTire.presValue = LRTireVaild == 0 ? LRtmp:"-.-";

    mTire.RRTire.validFlag = RRTireVaild;
    //mTire.RRTire.tireStatus = RRTireVaild == 0 ?RRTireStatus:0;
    mTire.RRTire.tireStatus = RRTireStatus;
    mTire.RRTire.presValue = RRTireVaild == 0 ? RRtmp:"-.-";

    HmiIPC::UpdateDispCarTire(mTire);
}
void CMsgService::GetTireStatus(void)
{
	mTPMSDelayProcessor->updateSignal(decode.getVecData());
}

void CMsgService::GetKey(void)
{
    uint8_t keyValue = uint8_t(decode[0]);
    LOGWAR("GetKey = %d",keyValue);
	LOG_RECORD("GetKey = 0x%02x",keyValue);
    const std::vector<uint8_t> exportLogKeys = {
        0x2C,//EmKey::UP,
        0x2D,//EmKey::DOWN,
        0x2C,//EmKey::UP,
        0x2D,//EmKey::DOWN,
        0x25,//EmKey::OK,
        0x33,//EmKey::RETURN,
        0x33,//EmKey::RETURN,
    };
    static uint8_t exportLogIdx = 0;
    if(keyValue == exportLogKeys[exportLogIdx])
    {
        if(exportLogIdx >= (exportLogKeys.size() - 1))
        {
            exportLogIdx = 0;
            //exprot log
            if(access("/mnt/sda2/sda1/", F_OK) == 0)
            {
                system("cd  /opt/data/ && tar -cf /mnt/sda2/sda1/zh-ic-log.tar ZH-IC-Log && sync");
            }
            LOG_RECORD("Getkey Export Log");
        }
        else
        {
            exportLogIdx++;
        }
    }
    else
    {
        exportLogIdx = 0;
    }
    switch (keyValue)
    {
    case 0x25:
        HmiIPC::UpdateDispKey(EmKey::OK, EmKeyState::S_PRESS);
		LOG_RECORD("UpdateDispKey: press_Ok");
        break;
    case 0x2C:
        HmiIPC::UpdateDispKey(EmKey::UP, EmKeyState::S_PRESS);
        break;
    case 0x2D:
        HmiIPC::UpdateDispKey(EmKey::DOWN, EmKeyState::S_PRESS);
        break;
    case 0x2E:
        HmiIPC::UpdateDispKey(EmKey::LEFT, EmKeyState::S_PRESS);
        break;
    case 0x2F:
        HmiIPC::UpdateDispKey(EmKey::RIGHT, EmKeyState::S_PRESS);
        break;
    case 0x33:
        HmiIPC::UpdateDispKey(EmKey::RETURN, EmKeyState::S_PRESS);
        break;
    case 0x42:
        HmiIPC::UpdateDispKey(EmKey::UP, EmKeyState::L_PRESS);
        break;
	case 0x43:
        HmiIPC::UpdateDispKey(EmKey::OK, EmKeyState::L_PRESS);
        break;
    default:
        break;
    }
}

void CMsgService::GetSetConfig(void)
{
    LOG_SDEBUG("%s\n", __func__);
	static bool flag = false;
	if(!flag)
	{
		flag = true;
		upgrade::Upgrade_clusterVersion("","");
	}
	LOG_RECORD("%s,item:%d,value:%d", __func__, decode[0], decode[1]);
    static int s_brightnessValue = 0xff;
    switch (decode[0])
    {
    case SET_IC_BRIGHTNESS_LEVEL: // 仪表亮度等级设置
        if(s_brightnessValue != 6)
        {
            s_brightnessValue = decode[1];
            HmiIPC::SyncMemoryItem(EmFunctionItem::BRIGHTNESS, decode[1]);
            LOG_SDEBUG("%s case SET_IC_BRIGHTNESS_LEVEL: decode[1] = %d\n", __func__, decode[1]);
        }
        
        break;
    case SET_IC_MILE_FORMAT: // 仪表单位制式设置
        HmiIPC::SyncMemoryItem(EmFunctionItem::UNIT_SYSTEM, decode[1]);
        break;
    case SET_IC_LANGUAGE_MODE: // 仪表语言模式设置
        HmiIPC::SyncMemoryItem(EmFunctionItem::LANGUAGE_SYSTEM, decode[1]);
        break;
    case SET_IC_DAYLIGHT_MODE: // 仪表白天黑夜模式设置
        HmiIPC::SyncMemoryItem(EmFunctionItem::VIEW_MODE, decode[1]);
        break;
    case SET_IC_THEME_MODE: // 仪表主题模式设置
        HmiIPC::SyncMemoryItem(EmFunctionItem::THEME_MODE, decode[1]);
        break;
    case SET_IC_BLUETOOTH_STATUS: // 仪表蓝牙状态设置
        HmiIPC::SyncMemoryItem(EmFunctionItem::BLUETOOTH_STATUS, decode[1]);
        break;
    case SET_IC_MILEAGE_MODE: // 仪表里程模式设置
        HmiIPC::SyncMemoryItem(EmFunctionItem::MILEAGE_MODE, decode[1]);
        break;
    case SET_IC_PRESSUNIT_MODE: //压力单位
    {
        HmiIPC::SyncMemoryItem(EmFunctionItem::UNIT_PRESS, decode[1]);
        break;
    }
    case SET_IC_MENU_ITEM_CFG_LOW:
    {
        HmiIPC::SyncMemoryItem(EmFunctionItem::MENU_ITEM_CFG_LOW, decode[1]);
        break;
    }
    case SET_IC_MENU_ITEM_CFG_HIGH:
    {
        HmiIPC::SyncMemoryItem(EmFunctionItem::MENU_ITEM_CFG_HIGH, decode[1]);
        break;
    }
    case SET_IC_SPEED_WARNValue:
    {
        HmiIPC::SyncMemoryItem(EmFunctionItem::SPEED_WARNING_VALUE, decode[1]);
        break;
    }
    case SET_IC_BRIGHTNESS_AUTO:
    {
        if(0 == decode[1])
        {
            HmiIPC::SyncMemoryItem(EmFunctionItem::BRIGHTNESS, 6);
            s_brightnessValue = 6;
        }
        break;
    }
    case SET_IC_DAYLIGHTAUTO:
    {
        //EmViewMode viewMode = decode[1] == 0 ? EmViewMode::DAY:EmViewMode::NIGHT;
        
        LOG_SDEBUG("%s case SET_IC_DAYLIGHTAUTO: decode[1] = %d\n", __func__, decode[1]);
        break;
    }
    default:
        break;
    }
}

void CMsgService::GetUpgrade(void)
{
    // printf("update: %d  %d \n", decode[0], decode[1]);
    // if ((0XA3 == decode[0]) && ((8 == decode[1]) || (16 == decode[1]) || (32 == decode[1])))
    // {
    //     o_mcuUpdate.EnterUpgradeMode(0, &mMsgDev);
    //     mMsgDev.swapCore(&o_mcuUpdate);
    //     // printf("-----------swap ok----------\n");
    //     NotifyMcuIntoUpgradeMode(0XA0);
    // }
}

void CMsgService::simulatedData(void)
{
    printf("*****  simulatedData begin.  *****\n");
    uint32_t lCounter = 0;

    while (true)
    {
        if (lCounter % 100 == 0)
        {
            static bool reverse = true;
            if (reverse)
            {
                HmiIPC::UpdateTurnLampSts(0,0,0); 
                reverse = false;
            }
            else
            {
                HmiIPC::UpdateTurnLampSts(1,1,0); 
                reverse = true;
            }
        }
        
        if (lCounter % 5 == 0)
        {
            static bool reverse = true;
            if (reverse)
            {
                HmiIPC::UpdateDispVehicleSpeed(160, true);
                HmiIPC::UpdateDispPowerValue(100, true);
                HmiIPC::UpdateDispOdograph(1000);
                HmiIPC::UpdateDispSOCValue(100, false, true);
                reverse = false;
            }
            else
            {
                HmiIPC::UpdateDispVehicleSpeed(0, true);
                HmiIPC::UpdateDispPowerValue(-100, true);
                HmiIPC::UpdateDispOdograph(10);
                HmiIPC::UpdateDispSOCValue(0, false, true);
                reverse = true;
            }
        }
        if (lCounter % 10 == 0)
        {
            static bool reverse = true;
            if (reverse)
            {
               // HmiIPC::UpdateDispFuel(80.f, true, EmWarningState::NORMAL);
                reverse = false;
            }
            else
            {
               // HmiIPC::UpdateDispFuel(0.f, true, EmWarningState::NORMAL);
                reverse = true;
            }
        }
        #if 0
		HmiIPC::UpdateDispFuel(int(lCounter), true, EmWarningState::NORMAL);
		HmiIPC::UpdateDispGear(static_cast<EmGearsValue>(lCounter % int(EmGearsValue::INVALID)));
		HmiIPC::UpdateDispCurTime(std::to_string(lCounter));
		HmiIPC::UpdateDispOutsideTemp(int(lCounter), true);
		HmiIPC::UpdateDispDrivingTime(int(lCounter), int(lCounter), true);
		HmiIPC::UpdateDispRechargeMileage(float(lCounter), true);
		HmiIPC::UpdateDispArgFuelConsumption(float(lCounter), true);
		HmiIPC::UpdateDispInstFuelConsumption(float(lCounter),float(lCounter), true);
		HmiIPC::UpdateDispTripA(int(lCounter));
		HmiIPC::UpdateDispTripB(int(lCounter));
		HmiIPC::UpdateDispOdograph(int(lCounter));

		std::vector<StuTelltableLampState> testLamp;
		testLamp.push_back({LAMP_TMPS_FAULT,   static_cast<EmLampState>(lCounter % int(EmLampState::TWINKLE_4HZ) )});
		HmiIPC::UpdateDispLamp(testLamp);
		HmiIPC::UpdateDispPopWarn(static_cast<EmPopWarnID>(lCounter % int(EmPopWarnID::END)));
		std::map<EmPopWarnID, int> popwarn;
		popwarn.insert({static_cast<EmPopWarnID>(lCounter % int(EmPopWarnID::END)), 1});
		std::vector<std::string> fcl;
		fcl.push_back(std::to_string(lCounter));
		HmiIPC::PushFaultCodeList(fcl);
		std::vector<StuCarDoorState> doorState;
		doorState.push_back({static_cast<EmCarDoorID>(lCounter % 6), EmSWState::open});
		HmiIPC::UpdateDispCarDoor(doorState);
usleep(33000);
		StuCarTire tire;
		tire.LFTire.presValue = 1;
		HmiIPC::UpdateDispCarTire(tire);
		HmiIPC::UpdateSysVersion(std::to_string(lCounter),std::to_string(lCounter));
		HmiIPC::UpdateDispKey(static_cast<EmKey>(lCounter % int(EmKey::END)), static_cast<EmKeyState>(lCounter % int(EmKeyState::L_PRESS_NR)));
		HmiIPC::SyncMemoryItem(static_cast<EmFunctionItem>(lCounter % 14), 1);
		HmiIPC::UpdatePowerStatus(static_cast<EmPowerStatus>(lCounter % 5));
		HmiIPC::UpdateDayNightAutoMode(static_cast<EmViewMode>(lCounter % 2));
		HmiIPC::UpdateTurnLampSts(lCounter % 2, lCounter % 2, 0);

		HmiIPC::UpdateScreenStatus(static_cast<EmScreenStatus>(lCounter % 2));
		HmiIPC::UpdateTime(lCounter % 24, lCounter % 60, lCounter % 2);
        #endif
		//
        lCounter++;
        usleep(33000);
		usleep(33000);
    }
}
