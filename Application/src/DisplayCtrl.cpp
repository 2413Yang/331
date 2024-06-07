#include "DisplayCtrl.h"
#include "MenuDisplay.h"
#include "mlog.h"
#include "MsgInterface.h"
#include "HmiLock.h"
#include "WarnIdDefine.h"
#include "kanziUtil.h"
#include "Common.h"
#include <kanzi/core.ui/graphics3d/standard_material.hpp>
#include "lampStartegy.h"
#include <unistd.h>
//#include <fcntl.h>
#include "Application.h"
#include "mylogCtrl.h"
#include "MemoryDataManager.h"
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

#define _LOG_DO_ACTUAL_LOGGING(format,...)\
do{\
	 printf(format,##__VA_ARGS__),fflush(stdout);\
} while (0)

std::mutex mLogMutex;
CDispCtrlScreenLayer::CDispCtrlScreenLayer()
	: opt("MsgService"), opt_update("UpgradeService"), mLock(Hmi::pMsgLock), opt_ivi("HUMsgService"), opt_log("LogService"),
	m_isUpgradeNow(false), m_nCurSportColor(1), m_nIsMusicPlay(false), m_nIsRadioPlay(false), m_nCurMusicType(0), m_nCurRadioType(0), mSyncMemeryFlag(0), PowerStatus(EmPowerStatus::EM_POWER_NONE)
{
	mMemoryTheme = 0, mMemoryColor = 0, mMemoryTcInfo = 0, mMemoryVoiceState = 0, mMemoryVoiceSize = 1;
	isSelfChecking.store(false);
	ZH::logService::publisher::LogRecord(opt_log);
	opt_log.start();
	m_bIsRestart = false;
	static const char* cFlagFile = "/tmp/hmiReStart";
	if (access(cFlagFile, F_OK) == 0)
	{
		m_bIsRestart = true;
		LOG_RECORD("----HMI error----Restart\n");
		isSelfChecking.store(true);
		//RestartMomoryData();
	}
	else
	{
		FILE* fp = fopen(cFlagFile, "w");
		fsync(fileno(fp));
		fclose(fp);
	}


	InitMap();
	HmiIPC::publisher::SelfCheckState(opt);
	HmiIPC::subscriber::UpdateThemeColor(opt, *this);
	HmiIPC::subscriber::UpdateVoicePlay(opt, *this);
	HmiIPC::subscriber::UpdateTCInfoIndex(opt, *this);
	//HmiIPC::publisher::TestResetCommand(opt);
	opt.start();
	mKanzi_tid = 0;
}

void CDispCtrlScreenLayer::PushUpdateKzAttrTaskToMain(std::string id, std::function<bool(void)> task)
{
	static __thread int tid = 0;
	if (tid == 0)
	{
		tid = static_cast<pid_t>(::syscall(SYS_gettid));
	}
	if (tid == mKanzi_tid)
	{
		task();
		return;
	}
	std::lock_guard<std::mutex> lock(mTaskMutex);
	for (auto it = mUpdateKzAttrList.begin(); it != mUpdateKzAttrList.end(); it++)
	{
		if (it->first == id)
			it = mUpdateKzAttrList.erase(it);
		if (it == mUpdateKzAttrList.end())
			break;
	}
	mUpdateKzAttrList[id] = task;
}

#define DiagnosesPtrIsNullptr(ptr)              \
    if (ptr == nullptr)                         \
	    {                                           \
        /*LOGERR("ptr [%s] is nullptr.\n", #ptr);*/ \
        return true;                            \
	    }
#define PushUpdateKzAttrTask(ta) this->PushUpdateKzAttrTaskToMain(std::string(__FUNCTION__), ta)

#define PushUpdateKzAttrTaskByName(name,ta) this->PushUpdateKzAttrTaskToMain(name, ta)

void CDispCtrlScreenLayer::RunTask()
{
	do
	{
		std::list<std::function<bool(void)>> taskList;
		do
		{
			std::lock_guard<std::mutex> lock(mTaskMutex);
			//ZH::BaseLib::CAutoLock lock(*(msgLayer->ptrLock));
			for (auto task = mUpdateKzAttrList.begin(); task != mUpdateKzAttrList.end(); task++)
			{
				taskList.push_back(task->second);
				{
					task = mUpdateKzAttrList.erase(task);
					if (task == mUpdateKzAttrList.end())
						break;
				}
			}
		} while (0);
		for (auto iter : taskList)
		{
			iter();
		}

	} while (0);
}

void CDispCtrlScreenLayer::init(ScreenSharedPtr ptrRoot, Domain *ptrDomain, Hmi *_app)
{
	int count = 0;
	
	while (!m_bIsRestart)
	{
		//阻塞，等同步消息完成再执行下一步
		if ((mSyncMemeryFlag & 0b111) == 0b111)
		{
			break;
		}
		else
		{
			usleep(10 * 1000);
		}
		if (++count > 200)
		{
			LOG_RECORD("---ERROE----mSyncMemeryFlag = %d\n", mSyncMemeryFlag);
			break;
		}
	}
		
	m_pApp = _app;
	m_oRoot = ptrRoot;
	pMenuCtrl = new CMenuDisplay(m_oRoot, this, m_pApp);
	//pLampStartegy = new CLampStartegy(this, m_pApp);
	//func(__func__, __LINE__);
	
	HmiIPC::subscriber::UpdateDispOdograph(opt, *this);
	HmiIPC::subscriber::UpdateDispPowerValue(opt, *this);
	HmiIPC::subscriber::UpdateDispVehicleSpeed(opt, *this);
	HmiIPC::subscriber::UpdateDispRechargeMileage(opt, *this);
	HmiIPC::subscriber::UpdateDispSOCValue(opt, *this);
	InitNode();
	RunTask();//快速从kanzi默认值变目标值
	//CIPCConnector::registerLock(*(ptrLock = new CHmiMutex(Hmi::pMsgLock)));

	HmiIPC::subscriber::UpdateDispGear(opt, *this);
	HmiIPC::subscriber::UpdateDispTripA(opt, *this);
	HmiIPC::subscriber::UpdateDispTripB(opt, *this);
	
	HmiIPC::subscriber::UpdateSysVersion(opt, *this);
	HmiIPC::subscriber::UpdateDispCarTire(opt, *this);
	HmiIPC::subscriber::UpdateDispKey(opt, *this);
	HmiIPC::subscriber::UpdateTurnLampSts(opt, *this);
	HmiIPC::subscriber::UpdateTime(opt, *this);
	HmiIPC::subscriber::UpdatePowerStatus(opt, *this);
	HmiIPC::subscriber::UpdateDispPopWarn(opt, *this); 
	HmiIPC::subscriber::UpdateHistoryWarnList(opt, *this);
	HmiIPC::subscriber::UpdateDispLamp(opt, *this); 
	
	HmiIPC::subscriber::UpdateDispEnergyFlow(opt, *this);
	HmiIPC::subscriber::UpdateDispEnergyRecycle(opt, *this);
	HmiIPC::subscriber::UpdateInstEnergyConsum(opt, *this);
	HmiIPC::subscriber::UpdateAverEnergyConsum(opt, *this);
	HmiIPC::subscriber::UpdateDispDrivingTime(opt, *this);
	HmiIPC::subscriber::UpdateDispMotorSpeed(opt, *this);
	//HmiIPC::subscriber::UpdateChargeSubscribe(opt, *this);
	HmiIPC::subscriber::UpdateCharging(opt, *this);
	HmiIPC::subscriber::CarDoorStatus(opt, *this);

	HmiIPC::publisher::ResetFunctionItem(opt);
	HmiIPC::publisher::SetPopWarnStatus(opt);
	HmiIPC::publisher::UpdateDispSpeed(opt);
	HmiIPC::publisher::SetThemeColor(opt);
	HmiIPC::publisher::SetVoicePlay(opt);
	HmiIPC::publisher::SetClearMileageInfo(opt);
	HmiIPC::publisher::SetTCInfoIndex(opt);
	HmiIPC::publisher::TransitIVIBrightness(opt);

	HmiIPC::subscriber::SystemUpdateMessaage(opt_update, *this);

	HUHmiIPC::subscriber::UpdateIVISourceStatus(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVINaviInfo(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIMusicPlayInfo(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIRadioPlayInfo(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIPhoneInfo(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIBtInfo(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIMusicPlayList(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIRadioList(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIContactList(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVICallRecordList(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIThemeColor(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIMusicDevice(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIKeyMode(opt_ivi, *this);
	HUHmiIPC::subscriber::UpdateIVIBrightness(opt_ivi, *this);

	HUHmiIPC::publisher::UpdateIVIBtPhoneControl(opt_ivi);
	HUHmiIPC::publisher::UpdateIVIInfoList(opt_ivi);
	HUHmiIPC::publisher::UpdateIVIInfoPlayIndex(opt_ivi);
	HUHmiIPC::publisher::UpdateIVIRadioPlayIndex(opt_ivi);
	HUHmiIPC::publisher::UpdateIVIContactIndex(opt_ivi);
	HUHmiIPC::publisher::UpdateIVICallRecordIndex(opt_ivi);
	HUHmiIPC::publisher::UpdateIVIBtList(opt_ivi);
	HUHmiIPC::publisher::UpdateIVIMediaSourceList(opt_ivi);
	HUHmiIPC::publisher::UpdateIVIMediaCLose(opt_ivi);

	opt_update.start();
	opt_ivi.start();
}

void CDispCtrlScreenLayer::InitNode()
{
	mBgNode = m_oRoot->lookupNode<Node2D>("#ResID_Bg");
	mMainView = m_oRoot->lookupNode<Node2D>("#ResID_MainView");
	mLampNode = m_oRoot->lookupNode<Node2D>("#ResID_Lamp");
	mPopNode = m_oRoot->lookupNode<Node2D>("#ResID_Pop");
	mWarnNode = m_oRoot->lookupNode<Node2D>("#ResID_Warn"); 
	mUpgradeNode = m_oRoot->lookupNode<Node2D>("#ResID_Upgrade");
	mNaviNode = m_oRoot->lookupNode<Node2D>("#ResID_Navi");
	mOffNode = m_oRoot->lookupNode<Node2D>("#ResID_OFF");
	mSysVersionNode = m_oRoot->lookupNode<Node2D>("#ResID_SysVersion");
	
	KANZILOADPREFAB(m_oRoot, mBgNode->getChild(0), "bgClassic", "kzb://a301/Prefabs/Prefab_Bg_Classic", mBgClassic);
	KANZILOADPREFAB(m_oRoot, mBgNode->getChild(1), "bgSport", "kzb://a301/Prefabs/Prefab_Bg_Sport", mBgSport);
	KANZILOADPREFAB(m_oRoot, mBgNode->getChild(2), "bgTechnology", "kzb://a301/Prefabs/Prefab_Bg_Technology", mBgTechnology);
	KANZILOADPREFAB(m_oRoot, mMainView->getChild(0), "styleClassic", "kzb://a301/Prefabs/Prefab_Style_Classic", mStyleClassic);
	KANZILOADPREFAB(m_oRoot, mMainView->getChild(1), "styleSport", "kzb://a301/Prefabs/Prefab_Style_Sport", mStyleSport);
	KANZILOADPREFAB(m_oRoot, mMainView->getChild(2), "styleTechnology", "kzb://a301/Prefabs/Prefab_Style_Technology", mStyleTechnology);
	KANZILOADPREFAB(m_oRoot, mLampNode, "Lamp", "kzb://a301/Prefabs/Prefab_LampNode", mLamp);
	KANZILOADPREFAB(m_oRoot, mPopNode, "Pop", "kzb://a301/Prefabs/Prefab_Pop", mPop);
	KANZILOADPREFAB(m_oRoot, mOffNode, "OFF", "kzb://a301/Prefabs/Prefab_OFF", mOff);

	pMenuCtrl->setCurVoicePlay(mMemoryVoiceState.load());
	pMenuCtrl->setCurVoiceSize(mMemoryVoiceSize.load());
	pMenuCtrl->SetThemeColor(mMemoryTheme.load(), mMemoryColor.load());
	SwitchTheme(mMemoryTheme.load());
	pMenuCtrl->MemoryTcPage(mMemoryTcInfo.load());
	mOffNode->setVisible(false);
	mPopNode->setVisible(false);
}

void CDispCtrlScreenLayer::InitMap()
{
	mGearsMap[EmGearsValue::P] = "P";
	mGearsMap[EmGearsValue::R] = "R";
	mGearsMap[EmGearsValue::R_4HZ] = "R_4HZ";
	mGearsMap[EmGearsValue::N] = "N";
	mGearsMap[EmGearsValue::D] = "D";
	mGearsMap[EmGearsValue::S] = "S";
	mGearsMap[EmGearsValue::M] = "M";

	mGearsMap[EmGearsValue::G1] = "M1";
	mGearsMap[EmGearsValue::G2] = "M2";
	mGearsMap[EmGearsValue::G3] = "M3";
	mGearsMap[EmGearsValue::G4] = "M4";
	mGearsMap[EmGearsValue::G5] = "M5";
	mGearsMap[EmGearsValue::G6] = "M6";
	mGearsMap[EmGearsValue::G7] = "M7";
	mGearsMap[EmGearsValue::G8] = "M8";
	mGearsMap[EmGearsValue::G9] = "M9";
	mGearsMap[EmGearsValue::G10] = "M10";

	mGearsMap[EmGearsValue::INVALID] = "";
	mGearsMap[EmGearsValue::NONE] = "";

	mWarnTxtMap[EmPopWarnID::TIRE_MONITOR_RESET] = "请检查胎压监测系统";
	mWarnTxtMap[EmPopWarnID::TIRE_PRESSURE_HIGH] = "轮胎压力过高";
	mWarnTxtMap[EmPopWarnID::TIRE_PRESSURE_LOW] = "轮胎压力过低";
	mWarnTxtMap[EmPopWarnID::TIRE_LEAK] = "轮胎漏气";
	mWarnTxtMap[EmPopWarnID::TIRE_INFO] = "轮胎信息在行驶几分钟\n后显示";
	mWarnTxtMap[EmPopWarnID::TIRE_ADJUST_LEFT] = "请向左打方向盘\n调整轮胎";
	mWarnTxtMap[EmPopWarnID::TIRE_ADJUST_RIGHT] = "请向右打方向盘\n调整轮胎";
	mWarnTxtMap[EmPopWarnID::LOW_SPEED_WARN_MAN_OFF] = "行人警示系统已关闭";
	mWarnTxtMap[EmPopWarnID::PULL_OFF_CHARER] = "驾驶前请先拔充电枪";
	mWarnTxtMap[EmPopWarnID::POWER_SYS_TEMP_HIGH] = "驱动系统过温\n请减速行驶";
	mWarnTxtMap[EmPopWarnID::CHARGER_WAVE_REPLACE] = "电网波动\n请更换充电地点";
	mWarnTxtMap[EmPopWarnID::BRAKE_BOOSTER_REDUCE] = "制动助力不足\n请谨慎驾驶";
	mWarnTxtMap[EmPopWarnID::CHARGE_P_GEAR] = "请在P档下进行充电";
	mWarnTxtMap[EmPopWarnID::POWER_SUPPLY] = "请刷卡或连接电源";
	mWarnTxtMap[EmPopWarnID::BRAKE_ON_UNABLE_CHARGE] = "无法充电，请拉起手刹";
	mWarnTxtMap[EmPopWarnID::ENGINE_FAIL_STOP] = "动力系统故障\n请靠边停车";
	mWarnTxtMap[EmPopWarnID::CHARGER_WAVE_TIME_LONGER] = "电网波动\n充电时间延长";
	mWarnTxtMap[EmPopWarnID::GEAR_SYS_4S] = "换挡器故障\n请联系4S店检查";
	mWarnTxtMap[EmPopWarnID::CHARGER_HAND_UNLOCK] = "充电枪解锁失败\n请手动解锁";
	mWarnTxtMap[EmPopWarnID::CHARGER_UNLOCK_TIME_LONGER] = "充电枪未锁止\n充电时间延长";
	mWarnTxtMap[EmPopWarnID::BRAKE_CHANGE_GEAR] = "换挡时请踩下制动踏板";
	mWarnTxtMap[EmPopWarnID::CHARGER_UNLOCK_STOP] = "充电枪未锁止\n充电停止";
	mWarnTxtMap[EmPopWarnID::BAT_VOL_LOW] = "12V蓄电池电压过低\n请靠边停车";
	mWarnTxtMap[EmPopWarnID::LOW_BATTERY_CHARGE] = "动力电池电量低\n请及时充电";
	mWarnTxtMap[EmPopWarnID::ENGINE_LIMIT_SLOW] = "动力限制，请减速慢行";
	mWarnTxtMap[EmPopWarnID::BRAKE_CHANGE_GEAR_1] = "换挡时请踩下制动踏板";
	mWarnTxtMap[EmPopWarnID::BRAKE_START_GEAR] = "请启动车辆后再换挡";
	mWarnTxtMap[EmPopWarnID::BRAKE_FAIL] = "手刹故障，驻车请注意";
	mWarnTxtMap[EmPopWarnID::CHARGING_NO_GEAR] = "当前车辆处于外接充电\n状态，无法切换挡位";
	mWarnTxtMap[EmPopWarnID::REDUCE_SPEED_SHIFT_GEAR] = "车速太高\n请减速后再换挡";
	mWarnTxtMap[EmPopWarnID::ENGINE_FAIL_4S] = "动力系统故障\n请联系4S店检查";
	mWarnTxtMap[EmPopWarnID::VEHICLE_POWERING] = "车辆未下电";
	mWarnTxtMap[EmPopWarnID::PARKING_RAMP] = "驻车坡道过大\n请更换驻车地点";
	mWarnTxtMap[EmPopWarnID::PARK_BRAKE_LACK_4S] = "手刹夹紧力不足\n请联系4S店检查";
	mWarnTxtMap[EmPopWarnID::FASTEN_SEAT_BELT_EPB] = "请系好安全带";
	mWarnTxtMap[EmPopWarnID::HANDBRAKE_BRAKE] = "释放电子手刹时请踩制动";
	mWarnTxtMap[EmPopWarnID::DOOR_CLOSE_LOCK] = "请关好车门再按遥控闭锁";
	mWarnTxtMap[EmPopWarnID::LIGHT_OFF] = "请关闭车灯";
	mWarnTxtMap[EmPopWarnID::FASTEN_SEAT_BELT_BCM_1] = "请系好安全带";
	mWarnTxtMap[EmPopWarnID::FASTEN_SEAT_BELT_BCM_2] = "请系安全带";
	mWarnTxtMap[EmPopWarnID::KEY_DETECT_FAIL] = "未检测到钥匙";
	mWarnTxtMap[EmPopWarnID::KEY_IN_CAR_UNLOCK] = "钥匙在车内\n车门无法闭锁";
	mWarnTxtMap[EmPopWarnID::KEY_ELEC_LOW] = "钥匙电量低\n请及时更换钥匙电池";
	mWarnTxtMap[EmPopWarnID::STEER_LOCK_FAIL] = "转向锁止未解除";
	mWarnTxtMap[EmPopWarnID::STEER_LOCK_CHECK] = "请检查转向锁止系统";
	mWarnTxtMap[EmPopWarnID::THEFT_AUTH_FAIL] = "防盗认证失败";
	mWarnTxtMap[EmPopWarnID::CHANGE_GEAR_P] = "启动请将挡位切换至P挡";
	mWarnTxtMap[EmPopWarnID::START_BRAKE] = "启动请踩刹车";
	mWarnTxtMap[EmPopWarnID::POWER_DIST_FAIL_4S] = "电源分配故障\n请联系4s店检查";
	mWarnTxtMap[EmPopWarnID::KEY_SYS_4S] = "无钥匙系统故障\n请联系4S店检查";
	mWarnTxtMap[EmPopWarnID::DOOR_OPEN] = "车门打开";
	mWarnTxtMap[EmPopWarnID::LEFT_FRONT_DOOR_OPEN] = "左前门打开";
	mWarnTxtMap[EmPopWarnID::LEFT_REAR_DOOR_OPEN] = "左后门打开";
	mWarnTxtMap[EmPopWarnID::RIGHT_FRONT_DOOR_OPEN] = "右前门打开";
	mWarnTxtMap[EmPopWarnID::RIGHT_REAR_DOOR_OPEN] = "右后门打开";
	mWarnTxtMap[EmPopWarnID::CAR_FRONT_DOOR_OPEN] = "发动机舱盖打开";
	mWarnTxtMap[EmPopWarnID::CAR_TRUNK_DOOR_OPEN] = "行李箱门打开";

	mOffPageLampStatus[LAMP_LEFT_TURN] = EmLampState::NONE;
	mOffPageLampStatus[LAMP_RIGHT_TURN] = EmLampState::NONE;
	mOffPageLampStatus[LAMP_HIGH_BEAM] = EmLampState::NONE;
	mOffPageLampStatus[LAMP_LOW_BEAM] = EmLampState::NONE;
	mOffPageLampStatus[LAMP_REAR_FOG] = EmLampState::NONE;
	mOffPageLampStatus[LAMP_POSITION] = EmLampState::NONE;
	mOffPageLampStatus[LAMP_DOOR_OPEN] = EmLampState::NONE;
	mOffPageLampStatus[LAMP_CHARGECABLE_CONNECT] = EmLampState::NONE;
	mOffPageLampStatus[LAMP_CHARGE_STATUS] = EmLampState::NONE;
	mOffPageLampStatus[LAMP_EPB] = EmLampState::NONE;

	mWarnPageMap[Warn_None] = "";
	mWarnPageMap[Warn_CarDoor] = "Prefab_Warn_CarDoor";
	mWarnPageMap[Warn_Safety] = "Prefab_Warn_Safety";
	mWarnPageMap[Warn_SteeringWheel_Left] = "Prefab_Warn_SteeringWheel_Left";
	mWarnPageMap[Warn_SteeringWheel_Right] = "Prefab_Warn_SteeringWheel_Right";

	mIndicatorStatus[LAMP_BRAKE_FLUID_LEVEL] = EmLampState::NONE;
	mIndicatorStatus[LAMP_BATTERY_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_ABS_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_EPS_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_EPB] = EmLampState::NONE;
	mIndicatorStatus[LAMP_HIGH_BEAM] = EmLampState::NONE;
	mIndicatorStatus[LAMP_LOW_BEAM] = EmLampState::NONE;
	mIndicatorStatus[LAMP_REAR_FOG] = EmLampState::NONE;
	mIndicatorStatus[LAMP_POSITION] = EmLampState::NONE;
	mIndicatorStatus[LAMP_EPB_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_AIR_BAG_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_MAIN_SEATBELT_INDICATOR] = EmLampState::NONE;
	mIndicatorStatus[LAMP_TMPS_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_READY_STATUS] = EmLampState::NONE;
	mIndicatorStatus[LAMP_CHARGECABLE_CONNECT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_CHARGE_STATUS] = EmLampState::NONE;
	mIndicatorStatus[LAMP_POWER_LIMIT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_MAIN_ALARM] = EmLampState::NONE;
	mIndicatorStatus[LAMP_LOWCHARGE] = EmLampState::NONE;
	mIndicatorStatus[LAMP_POWER_BATTERY_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_POWER_BATTERY_HIGHTEMP] = EmLampState::NONE;
	mIndicatorStatus[LAMP_DRIVE_MOTOR_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_SYSTEM_FAULT_YELLOW] = EmLampState::NONE;
	mIndicatorStatus[LAMP_SYSTEM_FAULT_RED] = EmLampState::NONE;
	mIndicatorStatus[LAMP_DOOR_OPEN] = EmLampState::NONE;
	mIndicatorStatus[LAMP_BATTERY_CHARGE_HEAT] = EmLampState::NONE;
}

void CDispCtrlScreenLayer::UpdateThemeColor(int theme, int color)
{
	mMemoryTheme = (theme > 2) ? 0 : theme;
	mMemoryColor = (color > 2) ? 0 : color;
	mSyncMemeryFlag |= 0x01;
	//func(__func__, __LINE__);
}

void CDispCtrlScreenLayer::UpdateVoicePlay(int state, int size)
{
	mMemoryVoiceState = (state > 1) ? 0 : state;
	mMemoryVoiceSize = (size > 2) ? 1 : size;
	mSyncMemeryFlag |= 0x02;
}

void CDispCtrlScreenLayer::UpdateTCInfoIndex(int index)
{
	mMemoryTcInfo = index;
	mSyncMemeryFlag |= 0x04;
}


void CDispCtrlScreenLayer::UpdatePowerStatus(EmPowerStatus powerStatus)
{
	//testLogManager testLog(__func__);
	EmPowerStatus status = powerStatus;
	//LOGWAR("UpdatePowerStatus======%d", status);
	PowerStatus = powerStatus;

	auto UpdateAttrTask = [this, status]() -> bool
	{
		static EmPowerStatus PerPowerStatus = EmPowerStatus::EM_POWER_NONE;
		switch (status)
		{
		case EmPowerStatus::EM_POWER_ON:
			mOffNode->setProperty(DynamicPropertyType<int>("Common.PowerState"), 1);
			mBgNode->setVisible(true);
			mMainView->setVisible(true);
			pMenuCtrl->mMenuNode->setVisible(true);
			mLampNode->setVisible(true);
			//mPopNode->setVisible(true);
			mWarnNode->setVisible(true);
			mUpgradeNode->setVisible(true);
			mNaviNode->setVisible(true);
			mOffNode->setVisible(false);
			//LOGWAR("mOffNodeONStatus = %d\n", mOffNode->isVisible());
			break;
		case EmPowerStatus::EM_POWER_OFF:
			mOffNode->setProperty(DynamicPropertyType<int>("Common.PowerState"), 0);
			mBgNode->setVisible(false);
			mMainView->setVisible(false);
			pMenuCtrl->mMenuNode->setVisible(false);
			mLampNode->setVisible(false);
			//mPopNode->setVisible(false);
			mWarnNode->setVisible(false);
			mUpgradeNode->setVisible(false);
			mNaviNode->setVisible(false);
			mOffNode->setVisible(true);
			//LOGWAR("mOffNodeOFFStatus = %d\n", mOffNode->isVisible());
			//LOGERR("MemoryTcIndex = %d\n",pMenuCtrl->GetMemoryTcIndex());
			if ((mSyncMemeryFlag & 0b100) >= 0b100 && PerPowerStatus == EmPowerStatus::EM_POWER_ON)
			{
				HmiIPC::SetTCInfoIndex(pMenuCtrl->GetMemoryTcIndex());
				LOG_RECORD("SendTCInfoIndex = %d\n", pMenuCtrl->GetMemoryTcIndex());
			}
			break;
		default:
			break;
		}
		PerPowerStatus = status;
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
	
}

void CDispCtrlScreenLayer::UpdateDispVehicleSpeed(const int &vehicleSpeed, bool validity)
{
	//testLogManager testLog(__func__);
	//LOG_RECORD_DEBUG("vehicleSpeed = %d", vehicleSpeed);
	bool selfcheck_temp = isSelfChecking.load();
	if (selfcheck_temp == false)
		return;
	/*static int preSpeed = -1;
	if (vehicleSpeed == preSpeed)
	{
		return;
	}*/
	int tmpValue = vehicleSpeed;
	if (validity == false)
	{
		tmpValue = 0;
	}
	static float arraySpeedValue[SpeedSectionNum] = { 0.0, 80.0, 130.0, 140.0, 150.0, 160.0 };
	static float arraySpeedPointer[SpeedSectionNum] = { -135.0, 0.0, 85.0, 101.8, 118.8, 135.5 };
	static float arraySpeedValue1[12] = {0, 100.0, 105.5, 110.0, 115.0, 120.0, 130.0, 140.0, 145.0, 150.0, 155.0, 160.0 };
	static float arraySpeedFire[12] = { 0, 111.5, 117.2, 121.8, 125.6, 128.7, 134.2, 139.5, 142.4, 145.5, 149.3, 153.5 };
	static float arraySpeedValue2[SpeedSectionNum] = { 0.0, 30.0, 60.0, 90.0, 120.0, 160.0 };
	static float arraySpeedLine[SpeedSectionNum] = { 100.0, 30.0, 18.0, 15.0, 12.0, 8.0 };
	float mSpeedPointer = PointerRotation((float)tmpValue, arraySpeedValue, arraySpeedPointer);
	float mSpeedFire = PointerRotation((float)tmpValue, arraySpeedValue1, arraySpeedFire);
	float mSpeedLine = PointerRotation((float)tmpValue, arraySpeedValue2, arraySpeedLine);
	auto pRoot = this->mMainView;
	auto UpdateAttrTask = [tmpValue, validity, mSpeedPointer, mSpeedFire, mSpeedLine, pRoot, this]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		pRoot->setProperty(DynamicPropertyType<float>("Common.SpeedValue"), (float)(tmpValue + 0.4));//为了kanzi最后的数值插值不那么久，加了0.4
		pRoot->setProperty(DynamicPropertyType<float>("Common.SpeedPointer"), mSpeedPointer);
		pRoot->setProperty(DynamicPropertyType<float>("Common.SpeedFire"), mSpeedFire);
		pRoot->setProperty(DynamicPropertyType<bool>("Common.SpeedValid"), validity);
		//lineRun = (int)((1.0 - tmpValue / 200.0) * 100);
		static int PreSpeed = -3;
		if (tmpValue <= 0)
		{
			if (NULL != m_CarLineToken) removeTimerHandler(m_pApp->getMessageDispatcher(), m_CarLineToken);
			PreSpeed = -3;
		}
		else
		{
			if (abs(tmpValue - PreSpeed) > 3)
			{
				PreSpeed = tmpValue;
				lineRun = (int)mSpeedLine;
				if (NULL != m_CarLineToken) removeTimerHandler(m_pApp->getMessageDispatcher(), m_CarLineToken);
				m_CarLineToken = addTimerHandler(m_pApp->getMessageDispatcher(),
					kanzi::chrono::milliseconds(lineRun),
					KZU_TIMER_MESSAGE_MODE_REPEAT,
					[this](const TimerMessageArguments&){
					static int process = 0;
					process = process + 1;
					if (process > 23)
						process = 0;
					mMainView->setProperty(DynamicPropertyType<int>("Common.CarLine"), process);
				});
			}
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
	//preSpeed = vehicleSpeed;
}


void CDispCtrlScreenLayer::UpdateDispMotorSpeed(const float motorSpeed, bool validity)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  MotorSpeed = %f", __FUNCTION__, motorSpeed);
	auto pRoot = this->pMenuCtrl;
	float value = (int)(motorSpeed * 10)/10.0;
	if (value > 16.0 && validity == true)
	{
		value = 16.0;
	}
	auto UpdateAttrTask = [this, value, validity, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		if (validity == true)
		{
			char tmp[10] = { 0 };
			sprintf(tmp, "%.1f", value);
			pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Common.MotorSpeed"), tmp);
			return true;
		}
		else
		{
			pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Common.MotorSpeed"), "---");
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateDispPowerValue(const int &fPowerValue, bool validity)
{
	//testLogManager testLog(__func__);
	//LOGERR("%s fPowerValue = %d,validity = %d\n ", __FUNCTION__, fPowerValue, validity);
	//LOG_RECORD_DEBUG("PowerValue = %d", fPowerValue);
	bool selfcheck_temp = isSelfChecking.load();
	if (selfcheck_temp == false)
		return;
	/*static int prePower = -1;
	if (fPowerValue == prePower)
	{
		return;
	}*/
	auto pRoot = this->mMainView;
	int tmpValue = fPowerValue;
	if (validity == false)
	{
		tmpValue = 0;
	}
	static float arrayPowerValue[5] = { -100, 0.0, 70.0, 80.0, 100.0 };
	static float arrayPowerPointer[5] = { -134.6, -90.0, 67.5, 90.2, 135.3 };
	static float arrayPowerProcess[5] = { 0.0, 0.166, 0.75, 0.83, 1.0 };
	float mPowerPointer = PointerRotation((float)tmpValue, arrayPowerValue, arrayPowerPointer);
	float mPowerProcess = PointerRotation((float)tmpValue, arrayPowerValue, arrayPowerProcess);
	auto UpdateAttrTask = [tmpValue, mPowerPointer, mPowerProcess, validity, pRoot, this]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		SwitchSportColor(tmpValue);
		pRoot->setProperty(DynamicPropertyType<bool>("Common.PowerValid"), validity);
		if (tmpValue>0)
			pRoot->setProperty(DynamicPropertyType<float>("Common.PowerValue"), (float)(tmpValue + 0.4));//为了kanzi最后的数值插值不那么久，加了0.4
		else
		{
			pRoot->setProperty(DynamicPropertyType<float>("Common.PowerValue"), (float)(tmpValue - 0.4));//为了kanzi最后的数值插值不那么久，加了0.4
		}
		pRoot->setProperty(DynamicPropertyType<float>("Common.PowerPointer"), mPowerPointer);
		pRoot->setProperty(DynamicPropertyType<float>("Common.PowerProcess"), mPowerProcess);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
	//prePower = fPowerValue;
}

void CDispCtrlScreenLayer::UpdateDispEnergyFlow(const int Value)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  DispEnergyFlow = %d", __FUNCTION__, Value);
	auto pRoot = this->pMenuCtrl;
	int value = Value;
	auto UpdateAttrTask = [this, value, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<int>("Common.EnergyFlowValue"), value);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateDispEnergyRecycle(const int Value)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  DispEnergyRecycle = %d", __FUNCTION__, Value);
	auto pRoot = this->mMainView;
	auto UpdateAttrTask = [Value, pRoot, this]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		pRoot->setProperty(DynamicPropertyType<int>("Common.EnergyRecycleState"), Value);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateInstEnergyConsum(const float Value)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  InstEnergyConsum = %f", __FUNCTION__, Value);
	auto pRoot = this->pMenuCtrl;
	auto UpdateAttrTask = [this, Value, pRoot]() -> bool
	{
		float value = Value;
		DiagnosesPtrIsNullptr(pRoot);
		if (value >= 51.1f)
		{
			pRoot->setInstEnergyConsumPage(false);
			if (pRoot->getCurMenuPageID() == Menu_Tc_Sub_InstanEnergyCons)
			{
				pRoot->CloseInstEnergyConsumPage();
			}
		}
		else
		{
			if (value > 50.0f)
				value = 50.0;
			pRoot->setInstEnergyConsumPage(true);
			char tmp[10] = { 0 };
			sprintf(tmp, "%.1f", value);
			pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Common.InstEnergyConsum"), tmp);
			pRoot->mMenuNode->setProperty(DynamicPropertyType<float>("TrendLine.CurveEnd"), value / 50.0);
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateAverEnergyConsum(const float Value)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  AverEnergyConsum = %f", __FUNCTION__, Value);
	auto pRoot = this->pMenuCtrl;
	auto UpdateAttrTask = [this, Value, pRoot]() -> bool
	{
		float value = Value;
		DiagnosesPtrIsNullptr(pRoot);
		if (value >= 51.1f)
			pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Common.AverEnergyConsum"), "---");
		else
		{
			if (value > 50.0f)
				value = 50.0;
			char tmp[10] = { 0 };
			sprintf(tmp, "%.1f", value);
			pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Common.AverEnergyConsum"), tmp);
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}


void CDispCtrlScreenLayer::UpdateDispSOCValue(const int socValue, const bool status, bool validity)
{
	//testLogManager testLog(__func__);
	//LOGERR("socValue = %d", socValue);
	auto pRoot = this->mMainView;
	int nSocValue = socValue;
	if (nSocValue >= 0x7f)
		nSocValue = 0;
	auto UpdateAttrTask = [pRoot, status, nSocValue]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		pRoot->setProperty(DynamicPropertyType<int>("Common.ElectricStatus"), (int)status);
		pRoot->setProperty(DynamicPropertyType<int>("Common.ElectricValue"), nSocValue);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateDispGear(const EmGearsValue &gear)
{
	//testLogManager testLog(__func__);
	
	//LOGDBG("%s  gear = %d", __FUNCTION__, gear);
	auto it = mGearsMap.find(gear);
	if (it != mGearsMap.end())
    {
        auto pRoot = this->mMainView;
		std::string tmpValue = it->second;
		auto UpdateAttrTask = [tmpValue,this, pRoot]() -> bool
        {
            DiagnosesPtrIsNullptr(pRoot);
			if (tmpValue == "R_4HZ")
			{
				pRoot->setProperty(DynamicPropertyType<bool>("Common.GearTwinkle"), true);
			}
			else
			{
				pRoot->setProperty(DynamicPropertyType<bool>("Common.GearTwinkle"), false);
				pRoot->setProperty(DynamicPropertyType<string>("Common.GearValue"), tmpValue);
				if (tmpValue == "R")
				{
					mOffNode->setProperty(DynamicPropertyType<int>("Lamp.Backup" ), 1);
				}
				else
				{
					mOffNode->setProperty(DynamicPropertyType<int>("Lamp.Backup"), 0);
				}
			}
            return true;
        };
        PushUpdateKzAttrTask(UpdateAttrTask);
    }
}


void CDispCtrlScreenLayer::UpdateDispTripA(const float &iTripA)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  iTripA = %f", __FUNCTION__, iTripA);
	/*static float preTripA = -1.0;
	if (iTripA == preTripA)
	{

		return;
	}*/
	auto pRoot = this->pMenuCtrl;
	float value = iTripA;
	if (value > 999.9)
	{
		value = 999.9;
	}
	auto UpdateAttrTask = [this, value, pRoot]() -> bool
    {
        DiagnosesPtrIsNullptr(pRoot);
		char tmp[10] = { 0 };
		sprintf(tmp, "%.1f", value);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Common.TirpAValue"), tmp);
        return true;
    };
    PushUpdateKzAttrTask(UpdateAttrTask);
	//preTripA = iTripA;
}

void CDispCtrlScreenLayer::UpdateDispTripB(const float &iTripB)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  iTripB = %f", __FUNCTION__, iTripB);
	/*static float preTripB = -1.0;
	if (iTripB == preTripB)
	{

		return;
	}*/
	auto pRoot = this->pMenuCtrl;
	float value = iTripB;
	if (value > 9999.9)
	{
		value = 9999.9;
	}
	auto UpdateAttrTask = [this, value, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		char tmp[10] = { 0 };
		sprintf(tmp, "%.1f", value);
		//LOG_RECORD("00000UpdateDispTripB = %f\n", value);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Common.TirpBValue"), tmp);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
	//preTripB = iTripB;
}

void CDispCtrlScreenLayer::UpdateDispDrivingTime(const int hour, const int minute, bool validity)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  DrivingTimeHour = %d, DrivingTimeMinute = %d\n", __FUNCTION__, hour, minute);
	auto pRoot = this->pMenuCtrl;
	if (minute > 59)
	{
		return;
	}
	auto UpdateAttrTask = [this, hour, minute, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Common.TirpTimeHour"), std::to_string(hour));
		pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Common.TirpTimeMinute"), std::to_string(minute));
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateDispOdograph(const int &iOdograph)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  iOdograph = %d", __FUNCTION__, iOdograph);
	/*static float preOdo = -1.0;
	if (iOdograph == preOdo)
	{

		return;
	}*/
	int odographValue = iOdograph;
	if (iOdograph > 999999)
	{
		odographValue = 999999;
	}
	//LOG_RECORD("odographValue Show = %d\n", odographValue);
	auto pRoot = this->mMainView;
	auto UpdateAttrTask = [odographValue, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		pRoot->setProperty(DynamicPropertyType<string>("Common.ODOValue"), std::to_string(odographValue));
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
	//preOdo = iOdograph;
}

void CDispCtrlScreenLayer::SystemUpdateMessaage(EmSWState state, std::string strTitle, std::string strTxt,int progress )
{
	//testLogManager testLog(__func__);
	//LOGDBG("SystemUpdateMessaage  %d , %s, %s, %d", state, strTitle.data(), strTxt.data(), progress);
	if (state == EmSWState::open)
	{
		m_isUpgradeNow = true;
	}
	else if (state == EmSWState::close)
	{
		m_isUpgradeNow = false;
	}

	auto pRoot = this->m_oRoot;
	auto UpdateAttrTask = [this, state, strTitle, strTxt, progress, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		if (state == EmSWState::open)
		{
			if (mUpgradeNode->getChildCount() == 0)
			{
				KANZILOADPREFAB(pRoot, mUpgradeNode, "Upgrade", "kzb://a301/Prefabs/Prefab_UpgradePage", mUpgrade);
			}
			mUpgradeNode->setProperty(DynamicPropertyType<string>("Common.UpgradeTitle"), strTitle);
			mUpgradeNode->setProperty(DynamicPropertyType<string>("Common.UpgradeTxt"), strTxt);
			mUpgradeNode->setProperty(DynamicPropertyType<int>("ProgressBar.ProgressBarProgress"), progress);
		}
		else if (state == EmSWState::close)
		{
			if (mUpgradeNode->getChildCount() != 0)
			{
				mUpgradeNode->removeAllChildren();
			}
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateDispRechargeMileage(float RecMil, bool validity)
{
	//testLogManager testLog(__func__);
	int RechargeMileage = RecMil;
	auto pRoot = this->mMainView;
	auto UpdateAttrTask = [RechargeMileage, validity, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		if (validity == true)
			pRoot->setProperty(DynamicPropertyType<string>("Common.EnduranceMileage"), std::to_string(RechargeMileage));
		else
			pRoot->setProperty(DynamicPropertyType<string>("Common.EnduranceMileage"), "---");
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

//auto Lamp = [](std::string id, int state)->void{
//		{
//			FILE* fp = fopen("/tmp/MsgService.txt", "a+");
//			if (fp)
//			{
//				fprintf(fp, "LampID = %s,LampState = %d\n", id.c_str(), state);
//				fflush(fp);
//				fclose(fp);
//			}
//		} while (0);
//};

void CDispCtrlScreenLayer::UpdateDispLamp(const std::vector<StuTelltableLampState> lamp)
{
	//testLogManager testLog(__func__);
	//uint32_t time = ZH::BaseLib::getWorldTimeMS();
	/*for (auto it : lamp)
	{
		//Lamp(it.strID.c_str(), (int)it.state);
		LOG_RECORD("LampID = %s,LampState = %d\n", it.strID.c_str(), it.state);
	}*/
	//pLampStartegy->updateLampStatus(lamp);
	auto pRoot = this->mLampNode;
	auto UpdateAttrTask = [this,lamp, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		for (auto it : lamp)
		{
			//LOGERR("------%s-----%d\n", it.strID.data(), it.state);
			auto itStatus = mIndicatorStatus.find(it.strID);
			if (itStatus != mIndicatorStatus.end() && itStatus->second == it.state)
			{
				continue;
			}
			//setLampStatus(it.strID, it.state);
			Node2DSharedPtr ImgLamp = pRoot->getChild(0)->lookupNode<Node2D>(it.strID);
			if (ImgLamp != nullptr)
			{
				ImgLamp->setProperty(DynamicPropertyType<int>("Common.LampStatus"), int(it.state));
			}
			if (it.strID == LAMP_HIGH_BEAM || it.strID == LAMP_LOW_BEAM || it.strID == LAMP_REAR_FOG || it.strID == LAMP_POSITION)
			{
				mOffNode->setProperty(DynamicPropertyType<int>("Lamp." + it.strID), int(it.state));
			}
			auto OffLampStatus = mOffPageLampStatus.find(it.strID);
			if (OffLampStatus != mOffPageLampStatus.end())
			{
				mOff->lookupNode<Image2D>("./Lamp/" + it.strID)->setProperty(DynamicPropertyType<int>("Common.LampStatus"), int(it.state));
			}
			mIndicatorStatus[it.strID] = it.state;
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::setLampStatus(std::string lampName, EmLampState lampState)
{
	//testLogManager testLog(__func__);
	auto pRoot = this->mLampNode;

	auto UpdateAttrTask = [lampName, lampState, pRoot,this]() -> bool
	{
			DiagnosesPtrIsNullptr(pRoot);
			Node2DSharedPtr ImgLamp = pRoot->getChild(0)->lookupNode<Node2D>(lampName);
			if (ImgLamp != nullptr)
			{
				ImgLamp->setProperty(DynamicPropertyType<int>("Common.LampStatus"), int(lampState));
			}
			if (lampName == LAMP_HIGH_BEAM || lampName == LAMP_LOW_BEAM || lampName == LAMP_REAR_FOG || lampName == LAMP_POSITION)
			{
				mOffNode->setProperty(DynamicPropertyType<int>("Lamp." + lampName), int(lampState));
			}
			auto OffLampStatus = mOffPageLampStatus.find(lampName);
			if (OffLampStatus != mOffPageLampStatus.end())
			{
				mOff->lookupNode<Image2D>("./Lamp/" + lampName)->setProperty(DynamicPropertyType<int>("Common.LampStatus"), int(lampState));
			}
		return true;
	};
	PushUpdateKzAttrTaskByName(lampName, UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateDispCarTire(StuCarTire carTire)
{
	//testLogManager testLog(__func__);
	//LOGDBG("%s  DrivingTimeHour = %d, DrivingTimeMinute = %d\n", __FUNCTION__, hour, minute);
	auto pRoot = this->pMenuCtrl;
	auto UpdateAttrTask = [this, carTire, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Tire.PressureLeftFrontValue"), carTire.LFTire.presValue);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Tire.PressureLeftRearValue"), carTire.LRTire.presValue);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Tire.PressureRightFrontValue"), carTire.RFTire.presValue);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("Tire.PressureRightRearValue"), carTire.RRTire.presValue);

		pRoot->mMenuNode->setProperty(DynamicPropertyType<int>("Tire.PressureLeftFrontState"), carTire.LFTire.tireStatus);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<int>("Tire.PressureLeftRearState"), carTire.LRTire.tireStatus);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<int>("Tire.PressureRightFrontState"), carTire.RFTire.tireStatus);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<int>("Tire.PressureRightRearState"), carTire.RRTire.tireStatus);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::IsReady(void)
{

}

void CDispCtrlScreenLayer::UpdateSysVersion(std::string os, std::string mcu)
{
	//testLogManager testLog(__func__);
	auto UpdateKey = [os, mcu, this]() -> bool
	{
		mSysVersionNode->setVisible(true);
		mSysVersionNode->lookupNode<TextBlock2D>("./soc")->setText("soc: " + os);
		mSysVersionNode->lookupNode<TextBlock2D>("./mcu")->setText("mcu: "+ mcu);
		if (NULL != m_SysVersionTimer) removeTimerHandler(m_pApp->getMessageDispatcher(), m_SysVersionTimer);
		m_SysVersionTimer = addTimerHandler(m_pApp->getMessageDispatcher(),
			kanzi::chrono::milliseconds(10000),
			KZU_TIMER_MESSAGE_MODE_ONCE,
			[this](const TimerMessageArguments&){
			mSysVersionNode->setVisible(false);
		});
		return true;
	};
	PushUpdateKzAttrTask(UpdateKey);
}

void CDispCtrlScreenLayer::UpdateDispKey(EmKey key,EmKeyState keyState)
{
	//testLogManager testLog(__func__);
	//LOGERR("function %s", __func__);
	if (isSelfChecking.load() == false || PowerStatus != EmPowerStatus::EM_POWER_ON)
		return;
    auto pRoot = this->pMenuCtrl;
	//LOGWAR("key = %d,keyState = %d", key, keyState);

    auto UpdateKey = [key,keyState, pRoot, this]() -> bool
    {
		DiagnosesPtrIsNullptr(pRoot);
        pRoot->UpdateMenuKey(key, keyState);
        return true;
    };
    PushUpdateKzAttrTask(UpdateKey);
	
}



void CDispCtrlScreenLayer::UpdateTurnLampSts(int leftTurn, int rightTurn, int warnLamp)
{
	//testLogManager testLog(__func__);
	auto pRoot = this->mLampNode;
	int left = (leftTurn > 1) ? 0 : leftTurn;
	int right = (rightTurn > 1) ? 0 : rightTurn;
	auto UpdateAttrTask = [left, right, pRoot, this]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		pRoot->getChild(0)->lookupNode<Node2D>("./LetfTurn")->setProperty(DynamicPropertyType<int>("Common.LampStatus"), left);
		pRoot->getChild(0)->lookupNode<Node2D>("./RightTurn")->setProperty(DynamicPropertyType<int>("Common.LampStatus"), right);
		mOffNode->setProperty(DynamicPropertyType<int>("Lamp.LetfTurn"), left);
		mOffNode->setProperty(DynamicPropertyType<int>("Lamp.RightTurn"), right);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::SwitchBg(int index)
{
	//testLogManager testLog(__func__);
	#if 0
	const static char* pngPath = "file://./image/Bg/Scenery_";
	char URL[200];
	sprintf(URL, "%s%d%s", pngPath, index, ".png");
	TextureSharedPtr mSceneryBgTexture = m_pApp->getDomain()->getResourceManager()->tryAcquireResource<Texture>(URL);
	if (pMenuCtrl->getCurStyle() == 0)
	{
		mBgNode->getChild(0)->getChild(0)->lookupNode<Image2D>("./SceneryBg")->setImage(mSceneryBgTexture);
	}
	else if (pMenuCtrl->getCurStyle() == 1)
	{
		mBgNode->getChild(1)->getChild(0)->lookupNode<Image2D>("./SceneryBg")->setImage(mSceneryBgTexture);
	}
	#else
	mBgNode->setProperty(DynamicPropertyType<int>("Common.State"), index);
	#endif
}


void CDispCtrlScreenLayer::SelfCheck()
{
	//testLogManager testLog(__func__);
	HmiIPC::SelfCheckState(1);
	/*do
	{
		FILE* fp = fopen("/tmp/selfcheckHmiSts.txt", "a+");
		if (fp)
		{
			fprintf(fp, "state;1\n");
			fflush(fp);
			fclose(fp);
		}
	} while (0);*/
	isSelfChecking.store(true);
	opt.refresh({ "UpdateDispVehicleSpeed","UpdateDispPowerValue" });
}

void CDispCtrlScreenLayer::startUpdateDispSpeed()
{
	/*
	**************************gl8_statr*************************
	if (isUpdateDispSpeed)
	{
		isUpdateDispSpeed = false;
		removeTimerHandler(m_oRoot->getDomain()->getMessageDispatcher(), m_DispSpeedTimer);
	}

	m_DispSpeedTimer = addTimerHandler(m_pApp->getMessageDispatcher(),
		kanzi::chrono::milliseconds(500),
		KZU_TIMER_MESSAGE_MODE_REPEAT,
		[this](const TimerMessageArguments&){
		int nowSpeed = 0;
		nowSpeed = mStyleSecond->lookupNode<Node2D>("./VehicleSpeed")->getProperty(DynamicPropertyType<int>("VehicleSpeedValue"));
		mNowDispSpeed = nowSpeed;
		HmiIPC::UpdateDispSpeed(mNowDispSpeed);
		//LOGDBG("startUpdateDispSpeed mNowDispSpeed = %d", mNowDispSpeed);
			
	});
	isUpdateDispSpeed = true;
	**************************gl8_end*************************
	*/
}

void CDispCtrlScreenLayer::dispSpeedDone()
{
	/*
	**************************gl8_statr*************************
	if (isUpdateDispSpeed)
	{
		isUpdateDispSpeed = false;
		removeTimerHandler(m_oRoot->getDomain()->getMessageDispatcher(), m_DispSpeedTimer);
	}
	int nowSpeed = 0;
	nowSpeed = mStyleSecond->lookupNode<Node2D>("./VehicleSpeed")->getProperty(DynamicPropertyType<int>("VehicleSpeedValue"));
	mNowDispSpeed = nowSpeed;
	//LOGDBG("dispSpeedDone mNowDispSpeed = %d", mNowDispSpeed);
	HmiIPC::UpdateDispSpeed(mNowDispSpeed);
	**************************gl8_end*************************
	*/
}


void CDispCtrlScreenLayer::UpdateTime(int hour, int minute,int format)
{
	//testLogManager testLog(__func__);
	//LOGDBG("update time hour = %d,minute = %d,format = %d", hour, minute, format);
	bool Validty = true;
	if (hour == 0xff && minute == 0xff)
		Validty = false;
	if ((hour > 23 || minute > 59) && Validty == true)
		return;
	char time[200];
	std::string amPm = "";
	if (format == 1)
	{
		if (hour > 12)
		{
			amPm = "下午";
			hour = hour % 12;
		}
		else
		{
			if (hour == 0)
				hour = 12;
			amPm = "上午";
		}
	}
	sprintf(time, "%02d:%02d", hour, minute);
	auto pRoot = this->mMainView;
	auto UpdateTask = [pRoot, time, Validty, amPm]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		if (Validty)
		{
			pRoot->setProperty(DynamicPropertyType<string>("Common.CurTimeMode"), amPm.data());
			pRoot->setProperty(DynamicPropertyType<string>("Common.CurTime"), time);
		}
		else
		{
			pRoot->setProperty(DynamicPropertyType<string>("Common.CurTimeMode"), "");
			pRoot->setProperty(DynamicPropertyType<string>("Common.CurTime"), "");
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateTask);
}

void CDispCtrlScreenLayer::UpdateDispPopWarn(EmPopWarnID warnId, int doorWarn)
{
	//testLogManager testLog(__func__);
	auto pRoot = this->mPopNode;
	string warnTxt = "";
	auto it = mWarnTxtMap.find(warnId);
	if (it != mWarnTxtMap.end())
	{
		warnTxt = it->second;
	}
	//LOGDBG("UpdatePopupsWarn = %s", warnTxt.data());
	DispWarnImage(warnId, doorWarn);
	auto UpdateTask = [warnTxt, pRoot, this]() -> bool
	{
		if (warnTxt == "")
		{
			pRoot->setVisible(false);
		}
		else
		{
			pRoot->setVisible(true);
			pRoot->setProperty(DynamicPropertyType<string>("TextBlockConcept.Text"), warnTxt);
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateTask);
}

void CDispCtrlScreenLayer::DispWarnImage(EmPopWarnID warnId, int doorWarn)
{
	//testLogManager testLog(__func__);
	//LOGERR("warnId = %d, doorWarn = %d\n", warnId, doorWarn);
	int LFDoorSts = doorWarn & 0x1;
	int RFDoorSts = (doorWarn >> 1) & 0x1;
	int LRDoorSts = (doorWarn >> 2) & 0x1;
	int RRDoorSts = (doorWarn >> 3) & 0x1;
	int Trunk = (doorWarn >> 4) & 0x1;
	int Bonnet = (doorWarn >> 5) & 0x1;
	//LOGERR("LFDoorSts = %d, RFDoorSts = %d,LRDoorSts = %d,RRDoorSts = %d,Trunk = %d,Bonnet = %d\n", LFDoorSts, RFDoorSts, LRDoorSts, RRDoorSts, Trunk, Bonnet);
	auto UpdateTask = [warnId, LFDoorSts, LRDoorSts, RFDoorSts, RRDoorSts, Bonnet, Trunk, this]() -> bool
	{
		switch (warnId)
		{
		case EmPopWarnID::DOOR_OPEN:
		case EmPopWarnID::LEFT_FRONT_DOOR_OPEN:
		case EmPopWarnID::LEFT_REAR_DOOR_OPEN:
		case EmPopWarnID::RIGHT_FRONT_DOOR_OPEN:
		case EmPopWarnID::RIGHT_REAR_DOOR_OPEN:
		case EmPopWarnID::CAR_FRONT_DOOR_OPEN:
		case EmPopWarnID::CAR_TRUNK_DOOR_OPEN:
			WarnImagePage(WarnPage::Warn_CarDoor);
			mWarnNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.DoorLFState"), LFDoorSts);
			mWarnNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.DoorLRState"), LRDoorSts);
			mWarnNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.DoorRFState"), RFDoorSts);
			mWarnNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.DoorRRState"), RRDoorSts);
			mWarnNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.DoorBonnetState"), Bonnet);
			mWarnNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.DoorTrunkState"), Trunk);
			break;
		case EmPopWarnID::FASTEN_SEAT_BELT_EPB:
		case EmPopWarnID::FASTEN_SEAT_BELT_BCM_1:
		case EmPopWarnID::FASTEN_SEAT_BELT_BCM_2:
			WarnImagePage(WarnPage::Warn_Safety);
			break;
		case EmPopWarnID::TIRE_ADJUST_LEFT:
			WarnImagePage(WarnPage::Warn_SteeringWheel_Left);
			break;
		case EmPopWarnID::TIRE_ADJUST_RIGHT:
			WarnImagePage(WarnPage::Warn_SteeringWheel_Right);
			break;
		default:
			WarnImagePage(WarnPage::Warn_None);
			break;
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateTask);
}

void CDispCtrlScreenLayer::WarnImagePage(WarnPage warnId)
{
	//testLogManager testLog(__func__);
	static WarnPage mPerMenuPage = Warn_None;
	if (warnId != mPerMenuPage)
	{
		auto iter = mWarnPageMap.find(warnId);
		if (iter != mWarnPageMap.end())
		{
			if (mWarnNode->getChildCount() != 0)
			{
				if (mPerMenuPage == Warn_CarDoor && warnId == Warn_None)
				{
					DoorCloseTimer = addTimerHandler(m_pApp->getMessageDispatcher(),
						kanzi::chrono::milliseconds(500),
						KZU_TIMER_MESSAGE_MODE_ONCE,
						[this](const TimerMessageArguments&){
						RemoveWarn();
					});
				}
				else
				{
					if (DoorCloseTimer != NULL)
					{
						removeTimerHandler(m_oRoot->getMessageDispatcher(), DoorCloseTimer);
					}
					RemoveWarn();
				}
			}
			if (iter->second != "")
			{
				Node2DSharedPtr subNode;
				std::string url = "kzb://a301/Prefabs/" + iter->second;
				KANZILOADPREFAB(m_oRoot, mWarnNode, "subNode", url.data(), subNode);
				if (pMenuCtrl->getCurStyle() == 2)
				{
					mMainView->getChild(2)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/PowerValue")->setVisible(false);
					if (pMenuCtrl->getCurMenuPageID() == Menu_Media_MusicPlay || pMenuCtrl->getCurMenuPageID() == Menu_Media_RadioPlay || pMenuCtrl->getCurMenuPageID() == Menu_Media_List \
						|| pMenuCtrl->getCurMenuPageID() == Menu_Media_NoList)
					{
						pMenuCtrl->mMenuNode->setVisible(false);
					}
				}
			}
		}
		mPerMenuPage = warnId;
	}
}

void CDispCtrlScreenLayer::RemoveWarn()
{
	if (mWarnNode->getChildCount() != 0)
	{
		mWarnNode->removeAllChildren();
		if (pMenuCtrl->getCurStyle() == 2)
		{
			//关闭报警弹窗图片，如果还有多媒体的话就显示多媒体，否则显示功率信息
			if (pMenuCtrl->getCurMenuPageID() == Menu_Media_MusicPlay || pMenuCtrl->getCurMenuPageID() == Menu_Media_RadioPlay || pMenuCtrl->getCurMenuPageID() == Menu_Media_List \
				|| pMenuCtrl->getCurMenuPageID() == Menu_Media_NoList)
			{
				pMenuCtrl->mMenuNode->setVisible(true);
			}
			else
			{
				mMainView->getChild(2)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/PowerValue")->setVisible(true);
			}
		}
	}
}

void CDispCtrlScreenLayer::CarDoorStatus(int doorWarn)
{
	int LFDoorSts = doorWarn & 0x1;
	int RFDoorSts = (doorWarn >> 1) & 0x1;
	int LRDoorSts = (doorWarn >> 2) & 0x1;
	int RRDoorSts = (doorWarn >> 3) & 0x1;
	int Trunk = (doorWarn >> 4) & 0x1;
	int Bonnet = (doorWarn >> 5) & 0x1;
	auto UpdateTask = [this, LFDoorSts, LRDoorSts, RFDoorSts, RRDoorSts, Bonnet, Trunk]() -> bool
	{
		mOffNode->setProperty(DynamicPropertyType<int>("Common.DoorLFState"), LFDoorSts);
		mOffNode->setProperty(DynamicPropertyType<int>("Common.DoorLRState"), LRDoorSts);
		mOffNode->setProperty(DynamicPropertyType<int>("Common.DoorRFState"), RFDoorSts);
		mOffNode->setProperty(DynamicPropertyType<int>("Common.DoorRRState"), RRDoorSts);
		mOffNode->setProperty(DynamicPropertyType<int>("Common.DoorBonnetState"), Bonnet);
		mOffNode->setProperty(DynamicPropertyType<int>("Common.DoorTrunkState"), Trunk);
		return true;
	};
	PushUpdateKzAttrTask(UpdateTask);
}

void CDispCtrlScreenLayer::UpdateHistoryWarnList(std::vector<EmPopWarnID> warn)
{
	//testLogManager testLog(__func__);
	auto pRoot = this->pMenuCtrl;
	//LOGERR("UpdateHistoryWarnList = %d \n", warn.size());
	/*for (auto it : warn)
	{
		LOGERR("HistoryWarnList = %d \n", it);
	}*/
	auto UpdateTask = [pRoot, warn]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);

		pRoot->mAlarmInfoList = warn;
		if (pRoot->getCurMenuPageID() == Menu_AlarmInfo_Sub)
			pRoot->setAlarmList();
		return true;
	};
	PushUpdateKzAttrTask(UpdateTask);
}


float CDispCtrlScreenLayer::PointerRotation(float x, float arrayX[16], float arrayY[16])
{
	//testLogManager testLog(__func__);
	//由于表盘刻度分布不规则，比如速度0~10指针旋转了17度，速度10~20指针旋转了18度，所以做了分段处理，最多分成16段
	float k = 0.0, b = 0.0;
	if (x >= arrayX[0] && x <= arrayX[1])
	{
		k = (arrayY[1] - arrayY[0]) / (arrayX[1] - arrayX[0]);
		b = arrayY[1] - k * arrayX[1];
	}
	else if (x > arrayX[1] && x <= arrayX[2])
	{
		k = (arrayY[2] - arrayY[1]) / (arrayX[2] - arrayX[1]);
		b = arrayY[2] - k * arrayX[2];
	}
	else if (x > arrayX[2] && x <= arrayX[3])
	{
		k = (arrayY[3] - arrayY[2]) / (arrayX[3] - arrayX[2]);
		b = arrayY[3] - k * arrayX[3];
	}
	else if (x > arrayX[3] && x <= arrayX[4])
	{
		k = (arrayY[4] - arrayY[3]) / (arrayX[4] - arrayX[3]);
		b = arrayY[4] - k * arrayX[4];
	}
	else if (x > arrayX[4] && x <= arrayX[5])
	{
		k = (arrayY[5] - arrayY[4]) / (arrayX[5] - arrayX[4]);
		b = arrayY[5] - k * arrayX[5];
	}
	else if (x > arrayX[5] && x <= arrayX[6])
	{
		k = (arrayY[6] - arrayY[5]) / (arrayX[6] - arrayX[5]);
		b = arrayY[6] - k * arrayX[6];
	}
	else if (x > arrayX[6] && x <= arrayX[7])
	{
		k = (arrayY[7] - arrayY[6]) / (arrayX[7] - arrayX[6]);
		b = arrayY[7] - k * arrayX[7];
	}
	else if (x > arrayX[7] && x <= arrayX[8])
	{
		k = (arrayY[8] - arrayY[7]) / (arrayX[8] - arrayX[7]);
		b = arrayY[8] - k * arrayX[8];
	}
	else if (x > arrayX[8] && x <= arrayX[9])
	{
		k = (arrayY[9] - arrayY[8]) / (arrayX[9] - arrayX[8]);
		b = arrayY[9] - k * arrayX[9];
	}
	else if (x > arrayX[9] && x <= arrayX[10])
	{
		k = (arrayY[10] - arrayY[9]) / (arrayX[10] - arrayX[9]);
		b = arrayY[10] - k * arrayX[10];
	}
	else if (x > arrayX[10] && x <= arrayX[11])
	{
		k = (arrayY[11] - arrayY[10]) / (arrayX[11] - arrayX[10]);
		b = arrayY[11] - k * arrayX[11];
	}
	else if (x > arrayX[11] && x <= arrayX[12])
	{
		k = (arrayY[12] - arrayY[11]) / (arrayX[12] - arrayX[11]);
		b = arrayY[12] - k * arrayX[12];
	}
	else if (x > arrayX[12] && x <= arrayX[13])
	{
		k = (arrayY[13] - arrayY[12]) / (arrayX[13] - arrayX[12]);
		b = arrayY[13] - k * arrayX[13];
	}
	else if (x > arrayX[13] && x <= arrayX[14])
	{
		k = (arrayY[14] - arrayY[13]) / (arrayX[14] - arrayX[13]);
		b = arrayY[14] - k * arrayX[14];
	}
	else if (x > arrayX[14] && x <= arrayX[15])
	{
		k = (arrayY[15] - arrayY[14]) / (arrayX[15] - arrayX[14]);
		b = arrayY[15] - k * arrayX[15];
	}
	else if (x > arrayX[15] && x <= arrayX[16])
	{
		k = (arrayY[16] - arrayY[15]) / (arrayX[16] - arrayX[15]);
		b = arrayY[16] - k * arrayX[16];
	}
	float y = x * k + b;
	return y;
}

void CDispCtrlScreenLayer::UpdateIVINaviInfo(bool state, int residueMile, int hour, int minute, int direction, std::string nextRoadMile, std::string roadname, std::string nextroadname)
{
	//testLogManager testLog(__func__);
	/**
	* UpdateIVINaviInfo
	*
	* @param  {int}						: 全程剩余路程距离
	* @param  {int}						: 剩余小时
	* @param  {int}						: 剩余分钟
	* @param  {int}						: 方向
	* @param  {int}						: 下个路口距离
	* @param  {string}					: 当前路名
	*/
	//LOGDBG("%s  state = %d, residueMile = %d, hour = %d, minute = %d, direction = %d, nextRoadMile = %d, roadname = %s, nextroadname = %s\n", \
		__FUNCTION__, state, residueMile, hour, minute, direction, nextRoadMile, roadname.c_str(), nextroadname.c_str());
	char ResidueMile[10] = { 0 };
	float fResidueMile = residueMile;
	std::string unit = "";
	if (residueMile > 1000 * 100)
	{
		unit = "km";
		fResidueMile = fResidueMile / 1000;
		sprintf(ResidueMile, "%.0f", fResidueMile);
	}
	else if (residueMile > 1000)
	{
		unit = "km";
		fResidueMile = fResidueMile / 1000;
		sprintf(ResidueMile, "%.1f", fResidueMile);
	}
	else
	{
		unit = "m";
		sprintf(ResidueMile, "%.0f", fResidueMile);
	}
	//LOGDBG("ResidueMile = %s\n", ResidueMile);
	auto pRoot = this->mNaviNode;
	auto UpdateAttrTask = [this, state, unit, ResidueMile, hour, minute, direction, nextRoadMile, nextroadname, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		if (state == true)
		{
			if (pRoot->getChildCount() == 0)
			{
				KANZILOADPREFAB(m_oRoot, pRoot, "Navi", "kzb://a301/Prefabs/Prefab_Navi", mNavi);
				pRoot->setProperty(DynamicPropertyType<bool>("Common.IsNavi"), true);
			}
			pRoot->setProperty(DynamicPropertyType<string>("Common.ResidueDistance"), ResidueMile + unit);
			pRoot->setProperty(DynamicPropertyType<int>("Common.Hour"), hour);
			pRoot->setProperty(DynamicPropertyType<int>("Common.Minute"), minute);
			pRoot->setProperty(DynamicPropertyType<int>("Common.DirectionIndex"), direction);
			pRoot->setProperty(DynamicPropertyType<string>("Common.RoadDistance"), nextRoadMile);
			pRoot->setProperty(DynamicPropertyType<string>("Common.RoadName"), nextroadname);
		}
		else
		{
			if (pRoot->getChildCount() != 0)
			{
				pRoot->removeAllChildren();
				pRoot->setProperty(DynamicPropertyType<bool>("Common.IsNavi"), false);
			}
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateIVIMusicPlayInfo(bool state, int playState, int source, std::string Lyric, std::string Album, std::string singer, std::string file, int index, std::string curTime, std::string totalTime, int progress)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	/*@param{ bool }					: 投屏状态（false：关闭，true：打开）
		* @param{ int }						: 播放状态（0：播放，1：暂停）
		* @param{ int }						: 音乐类型（0：本地音乐，1：蓝牙音乐，2：usb音乐，3：在线音乐）
		* @param{ string }					: 音乐标题
		* @param{ string }					: 专辑名
		* @param{ string }					: 歌手
		* @param{ string }					: 文件名
		* @param{ string }					: 当前时间
		* @param{ string }					: 总时间*/
	/*LOGERR("%s  state = %d, playState = %d, source = %d, Lyric = %s, Album = %s, singer = %s, file = %s, index = %d, curTime = %s,totalTime = %s,progress = %d\n", \
		__FUNCTION__, state, playState, source, Lyric.c_str(), Album.c_str(), singer.c_str(), index, file.c_str(), curTime.c_str(), totalTime.c_str(), progress);*/
	//LOGERR("%s  state = %d, playState = %d, source = %d, index = %d, curTime = %s\n", \
		__FUNCTION__, state, playState, source, index, curTime.c_str());

	auto pRoot = this->pMenuCtrl;
	auto UpdateAttrTask = [this, state, playState, source, Lyric, Album, singer, curTime, totalTime, progress, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		int CurPage = pRoot->getCurMenuPageID();
		if (CurPage == Menu_BTPhone_Incoming || CurPage == Menu_BTPhone_Dial || CurPage == Menu_BTPhone_InCalling || CurPage == Menu_BTPhone_CallEnd || CurPage == Menu_ChargeSubscribe\
			|| CurPage == Menu_Charging || CurPage == Menu_ChargeResult)
		{
			//LOGERR("@@@@@@@@@@@@@UpdateIVIMediaCLose(true)\n");
			//HUHmiIPC::UpdateIVIMediaCLose(true);
			return true;
		}
		static int prePlayState = -1;
		if (prePlayState != 1 || playState != 2)
		{
			prePlayState = playState;
			if (state == true)
			{
				//if (m_nIsMusicPlay == false)
				{
					m_nIsMusicPlay = true;
					m_nIsRadioPlay = false;
					pRoot->OpenMusicPlayPage();
					pRoot->stop5sTimer();
					pRoot->setArrowsVisible(true);
					m_nCurMusicType = source;
					m_nMediaSourceType = 0;
				}
				if (pRoot->getCurMenuPageID() == Menu_Media_MusicPlay)
				{

					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), source);
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.Lyric"), Lyric);
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.AlbumName"), Album);
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.Songer"), singer);
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.MusicCurTime"), curTime);
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.MusicAllTime"), totalTime);
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<float>("Common.Progress"), progress / 100.0);
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.PlayState"), playState);
				}
			}
			else
			{
				m_nIsMusicPlay = false;
				pRoot->CloseMusicPlayPage();
			}
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateIVICallRecordList(bool state, int page, std::vector<StuCallRecord> list)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	/*LOGERR("state = %d,CallRecordSize = %d\n", state, list.size());
	for (int i = 0; i < list.size(); i++)
	{
		LOGERR("CallRecordList[%d] = %s\n", i, list[i].name.c_str());
	}*/
	pMenuCtrl->CallRecordList(state, page, list);
}

void CDispCtrlScreenLayer::UpdateIVIContactList(bool state, int page, std::vector<std::string> list)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	/*LOGERR("state = %d,Size = %d\n", state, list.size());
	for (int i = 0; i < list.size(); i++)
	{
		LOGERR("MediaList[%d] = %s\n", i, list[i].c_str());
	}*/
	pMenuCtrl->ContactList(state, page, list);
}

void CDispCtrlScreenLayer::UpdateIVIMusicPlayList(int type, int index, int page, std::vector<std::string> MediaList)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	/*LOGERR("type = %d,index = %d,Size = %d\n", type, index, MediaList.size());
	std::string str = "";
	for (int i = 0; i < MediaList.size(); i++)
	{
		str = MediaList[i];
		LOGERR("MediaList[%d] = %s\n", i, str.c_str());
	}*/
	
	auto UpdateAttrTask = [this, type, index, page, MediaList]() -> bool
	{
		pMenuCtrl->MusicInfoList(type, index, page, MediaList);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateIVIRadioList(int type, int index, std::vector<std::string> MediaList)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	/*LOGERR("type = %d,index = %d,Size = %d\n", type, index, MediaList.size());
	std::string str = "";
	for (int i = 0; i < MediaList.size(); i++)
	{
		str = MediaList[i];
		LOGERR("MediaList[%d] = %s\n", i, str.c_str());
	}*/

	pMenuCtrl->mRadioInfoList = MediaList;
	auto UpdateAttrTask = [this, type, index]() -> bool
	{
		pMenuCtrl->RadioInfoList(type, index);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateIVIMusicDevice(int type, bool state)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	//LOGERR("MusicType = %d,state = %d\n", type, state);
	auto UpdateAttrTask = [this, type, state]() -> bool
	{
		switch (type)
		{
		case 1:
			pMenuCtrl->setBtDeviceState(state);
			if (state == false)
			{
				m_nIsMusicPlay = false;
				pMenuCtrl->CloseMusicPlayPage();
			}
			break;
		case 2:
			pMenuCtrl->setUSBDeviceState(state);
			if (state == false)
			{
				m_nIsMusicPlay = false;
				pMenuCtrl->CloseMusicPlayPage();
			}
			break;
		default:
			break;
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateIVIRadioPlayInfo(bool state, int source, std::string frequency, std::string name)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	LOGERR("state = %d, source = %d, frequency = %s", state, source, frequency.c_str(), name.c_str());
	auto pRoot = this->pMenuCtrl;
	auto UpdateAttrTask = [this, state, source, frequency, name, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		int CurPage = pRoot->getCurMenuPageID();
		if (CurPage == Menu_BTPhone_Incoming || CurPage == Menu_BTPhone_Dial || CurPage == Menu_BTPhone_InCalling || CurPage == Menu_BTPhone_CallEnd || CurPage == Menu_ChargeSubscribe\
			|| CurPage == Menu_Charging || CurPage == Menu_ChargeResult)
		{
			//LOGERR("@@@@@@@@@@@@@UpdateIVIMediaCLose(true)\n");
			//HUHmiIPC::UpdateIVIMediaCLose(true);
			return true;
		}
		if (state == true)
		{
			//if (m_nIsRadioPlay == false)
			{
				m_nIsRadioPlay = true;
				m_nIsMusicPlay = false;
				pRoot->setArrowsVisible(true);
				pRoot->OpenRadioPlayPage();
				pRoot->stop5sTimer();
				m_nCurRadioType = source;
				m_nMediaSourceType = 1;
			}
			if (pRoot->getCurMenuPageID() == Menu_Media_RadioPlay)
			{
				pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.RadioPlayIndex"), source);
				pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.Frequency"), frequency);
				pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.RadioOnlineText"), name);
			}
		}
		else
		{
			m_nIsRadioPlay = false;
			pRoot->CloseRadioPlayPage();
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateChargeSubscribe(int electricType, int chargeStatus, int status, int mode, int statusVaild, int timeVaild, StuChargeSubscribe start, StuChargeSubscribe End)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__); 
	auto pRoot = this->pMenuCtrl;
	char StrData[20];
	char StrTime[10];
	char EndData[20];
	char EndTime[10];
	sprintf(StrData, "%4d-%02d-%02d", start.Year + 2000, start.Month, start.Day);
	sprintf(StrTime, "%02d:%02d", start.Hour, start.Minute);
	sprintf(EndData, "%4d-%02d-%02d", End.Year + 2000, End.Month, End.Day);
	sprintf(EndTime, "%02d:%02d", End.Hour, End.Minute);
	auto UpdateAttrTask = [this, pRoot, electricType, chargeStatus, status, mode, statusVaild, timeVaild, StrData, StrTime, EndData, EndTime]() -> bool
	{
		//LOGERR("electricType = %d,chargeStatus = %d, status = %d,mode = %d,statusVaild = %d,timeVaild = %d\n", electricType, chargeStatus, status, mode, statusVaild, timeVaild);
		DiagnosesPtrIsNullptr(pRoot);
		auto ChargeResultPage = [this, pRoot](int ChargeState)->void{
					switch (ChargeState)
					{
					case 0:
						pRoot->OpenChargePage(MenuPageEnum::Menu_ChargeResult);
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 1);//0:显示预约充电，1：显示充电结果
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 2);
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 0);
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "慢充已连接未充电");
						break;
					case 3:
						pRoot->OpenChargePage(MenuPageEnum::Menu_ChargeResult);
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 1);//0:显示预约充电，1：显示充电结果
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 2);
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 1);
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "充电结束");
						break;
					case 4:
						pRoot->OpenChargePage(MenuPageEnum::Menu_ChargeResult);
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 1);//0:显示预约充电，1：显示充电结果
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 2);
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 1);
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "充电故障");
						break;
					default:
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 0);
						pRoot->CloseChargePage();
						break;
					}
		};
		if (electricType == 0 && pRoot->getCurMenuPageID() != Menu_Charging)
		{
			if (statusVaild == 0 && (status == 1 && mode != 0))//预约充电状态信号有效
			{
				if (chargeStatus == 0 || chargeStatus == 3)
				{
					if (timeVaild == 0)
					{
						pRoot->OpenChargePage(MenuPageEnum::Menu_ChargeSubscribe);
						//off页面
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 3);
						if (pRoot->getCurMenuPageID() == Menu_ChargeSubscribe)
						{
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartDate"), StrData);
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartTime"), StrTime);
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndDate"), EndData);
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndTime"), EndTime);
							switch (mode)
							{
							case 2:
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndDate"), "----------");
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndTime"), "--:--");
								break;
							case 3:
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartDate"), "----------");
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartTime"), "--:--");
								break;
							default:
								break;
							}
						}
					}
					else
					{
						pRoot->OpenChargePage(MenuPageEnum::Menu_ChargeSubscribe);
						//off页面
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 3);
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndDate"), "----------");
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndTime"), "--:--");
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartDate"), "----------");
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartTime"), "--:--");
					}
				}
			}
			else
			{
				ChargeResultPage(chargeStatus);
			}
		}
		
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateCharging(int mode, int status, int hour, int minute, int voltage, int electricity, int SubscribeState, int SubscribeMode, int SubscribeStuVaild, int SubscribeTimeVaild,\
StuChargeSubscribe start, StuChargeSubscribe End)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	//LOGERR("mode = %d,status = %d,hour = %d,minute = %d,voltage = %d,electricity = %d\n", mode, status, hour, minute, voltage, electricity);
	auto pRoot = this->pMenuCtrl;
	//LOGERR("SubscribeState = %d,SubscribeMode = %d,SubscribeStuVaild = %d,SubscribeTimeVaild = %d\n", SubscribeState, SubscribeMode,SubscribeStuVaild, SubscribeTimeVaild);
	auto UpdateAttrTask = [pRoot, mode, status, SubscribeState, SubscribeMode, SubscribeStuVaild, SubscribeTimeVaild, hour, minute, voltage, electricity, start, End, this]() -> bool
	{

		DiagnosesPtrIsNullptr(pRoot);
		char StrData[20];
		char StrTime[10];
		char EndData[20];
		char EndTime[10];
		sprintf(StrData, "%4d-%02d-%02d", start.Year + 2000, start.Month, start.Day);
		sprintf(StrTime, "%02d:%02d", start.Hour, start.Minute);
		sprintf(EndData, "%4d-%02d-%02d", End.Year + 2000, End.Month, End.Day);
		sprintf(EndTime, "%02d:%02d", End.Hour, End.Minute);
		if (mode == 0 || mode == 1)
		{
			mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeFastOrSlow"), mode + 1);
			mOffNode->setProperty(DynamicPropertyType<bool>("Common.PlaySequence"), false);
			switch (status)
			{
			case 0:
			{
				pRoot->OpenChargePage(MenuPageEnum::Menu_ChargeResult);
				if (SubscribeStuVaild == 0 && mode == 0)
				{
					if (SubscribeState == 1 && SubscribeMode != 0)
					{
						if(SubscribeTimeVaild == 1)
						{
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartDate"), "----------");
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartTime"), "--:--");
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndDate"), "----------");
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndTime"), "--:--");
						}
						else
						{
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartDate"), StrData);
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartTime"), StrTime);
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndDate"), EndData);
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndTime"), EndTime);
							switch (SubscribeMode)
							{
							case 2:
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndDate"), "----------");
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndTime"), "--:--");
								break;
							case 3:
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartDate"), "----------");
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartTime"), "--:--");
								break;
							default:
								break;
							}
						}
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 0);//0:显示预约充电，1：显示充电结果
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "慢充已连接未充电");
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 3);//0：无显示 1：正在充电 2：未充电、充电结束、充电故障 3：预约充电
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 0);//0：未充电 1：充电结束、充电故障（仅在ChargeType = 2时起作用）
					}
					else
					{
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 1);
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "慢充已连接未充电");
						//off页面
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 2);//0：无显示 1：正在充电 2：未充电、充电结束、充电故障 3：预约充电
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 1);//0：未充电 1：充电结束、充电故障（仅在ChargeType = 2时起作用）
					}
				}
				else
				{
					mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 1);
					if (mode == 0)
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "慢充已连接未充电");
					else
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "快充已连接未充电");
					//off页面
					mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 2);//0：无显示 1：正在充电 2：未充电、充电结束、充电故障 3：预约充电
					mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 1);//0：未充电 1：充电结束、充电故障（仅在ChargeType = 2时起作用）
				}
			}
				break;
			case 1:
			case 2:
			{
				//LOGERR("aaaaaaaaaaa##################\n");
				if (status - 1 == mode)
				{
					//LOGERR("4444444444444444444444444444\n");
					pRoot->OpenChargePage(MenuPageEnum::Menu_Charging);
					if (hour == 17 && minute == 3)
					{
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeHour"), "-");
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeMinute"), "-");
					}
					else
					{
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeHour"), std::to_string(hour));
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeMinute"), std::to_string(minute));
					}
					if (voltage >= 0x1fff)
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeVoltage"), "---");
					else if (voltage > 0x1f40)
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeVoltage"), "800");
					else
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeVoltage"), std::to_string(int(voltage*0.1)));
					if (electricity >= 0x3fff)
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeElectricity"), "---");
					else if (electricity >= 0x3e80)
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeElectricity"), "800");
					else
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeElectricity"), std::to_string(int(electricity*0.1) - 800));
					//off页面
					mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 1);
					mOffNode->setProperty(DynamicPropertyType<bool>("Common.PlaySequence"), true);
				}
				else
				{
					mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeFastOrSlow"), 0);
					pRoot->CloseChargePage();
					//off页面
					mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 0);
				}
			}
				break;
			case 3:
			{
				//LOGERR("555555555555555555555555\n");
				//LOGERR("SubscribeStuVaild = %d,SubscribeState = %d,SubscribeMode %d\n", SubscribeStuVaild, SubscribeState,SubscribeMode);
				pRoot->OpenChargePage(MenuPageEnum::Menu_ChargeResult);
				if (SubscribeStuVaild == 0 && mode == 0)
				{
					if (SubscribeState == 1 && SubscribeMode != 0)
					{
						if(SubscribeTimeVaild == 1)
						{
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartDate"), "----------");
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartTime"), "--:--");
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndDate"), "----------");
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndTime"), "--:--");
						}
						else
						{
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartDate"), StrData);
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartTime"), StrTime);
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndDate"), EndData);
							mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndTime"), EndTime);
							switch (SubscribeMode)
							{
							case 2:
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndDate"), "----------");
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeEndTime"), "--:--");
								break;
							case 3:
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartDate"), "----------");
								mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeStartTime"), "--:--");
								break;
							default:
								break;
							}
						}
						
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 0);//0:显示预约充电，1：显示充电结果
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "充电结束");
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 3);//0：无显示 1：正在充电 2：未充电、充电结束、充电故障 3：预约充电
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 0);//0：未充电 1：充电结束、充电故障（仅在ChargeType = 2时起作用）
					}
					else
					{
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 1);
						mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "充电结束");
						//off页面
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 2);
						mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 1);
					}
				}
				else
				{
					mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 1);
					mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "充电结束");
					//off页面
					mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 2);
					mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 1);
				}
			}
				break;
			case 4:
			{
				//LOGERR("6666666666666666666666\n");
				pRoot->OpenChargePage(MenuPageEnum::Menu_ChargeResult);
				mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeSubscribeOrResult"), 1);
				mOffNode->setProperty(DynamicPropertyType<string>("Common.ChargeResult"), "充电故障");
				//off页面
				mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 2);
				mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeState"), 1);
			}
				break;
			default:
			{
				//LOGERR("7777777777777777777\n");
				mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeFastOrSlow"), 0);
				pRoot->CloseChargePage();
				//off页面
				mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 0);
			}
				break;
			}
		}
		else
		{
			//LOGERR("11111111111111111111\n");
			if (mode == 2)
			{
				mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeFastOrSlow"), 3);
			}
			else
			{
				mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeFastOrSlow"), 0);
			}
			pRoot->CloseChargePage();
			//off页面
			mOffNode->setProperty(DynamicPropertyType<int>("Common.ChargeType"), 0);
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}


void CDispCtrlScreenLayer::UpdateIVISourceStatus(int state)
{
	//LOGERR("$$$$$$$$$$UpdateIVISourceStatus = %d\n", state);
}

void CDispCtrlScreenLayer::UpdateIVIPhoneInfo(int type, std::string name, std::string num, int medium, int mic, std::string time)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	/**@param{ int }                     : 电话状态（ - 1 : no call(默认) 0 : 通话中 1 : 正在拨打 2 : 正在铃声 3 : 正在来电 4 : 第三方来电正在等待 5 : 通过响应和保留 6 : 停止）
	* @param{ string }                  : 联系人名字（"" : 默认）
	* @param{ string }                  : 联系人电话号码（"" : 默认）
	* @param{ int }                     : 通话媒介（ - 1 : 默认 0 : 电话通话 1 : 车机通话）
	* @param{ int }                     : 麦克风状态（ - 1 : 默认 0 : 静音 1 : 非静音）
	* @param{ string }                  : 通话时长（"00:00" : 默认）*/
	auto pRoot = this->pMenuCtrl;
	auto UpdateAttrTask = [this, type, name, num, medium, mic, pRoot]() -> bool
	{
		
		DiagnosesPtrIsNullptr(pRoot);
		static int PreType = -1;
		std::string str = "";
		if (name != "")
			str = name;
		else
			str = num;
		pRoot->mMenuNode->setProperty(DynamicPropertyType<string>("BtPhone.NameOrNum"), str);
		pRoot->mMenuNode->setProperty(DynamicPropertyType<int>("BtPhone.PhoneType"), type);
		if (mNaviNode->getChildCount() == 0 || pRoot->getCurStyle() != 2)
		{
			switch (type)
			{
			case 0:
				pRoot->OpenBTPhonePage(MenuPageEnum::Menu_BTPhone_InCalling);
				if (mic == 0)
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressSound"), true);
				else if (mic == 1)
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressSound"), false);
				if (medium == 0)
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressMobile"), false);
				else if (medium == 1)
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressMobile"), true);
				break;
			case 2:
				pRoot->OpenBTPhonePage(MenuPageEnum::Menu_BTPhone_Dial);
				if (mic == 0)
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressSound"), true);
				else if (mic == 1)
					pRoot->mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressSound"), false);
				break;
			case 3:
				pRoot->OpenBTPhonePage(MenuPageEnum::Menu_BTPhone_Incoming);
				break;
			case 6:
				if (PreType == 0 || PreType == 2 || PreType == 3)
				{
					pRoot->OpenBTPhonePage(MenuPageEnum::Menu_BTPhone_CallEnd);
				}
				break;
			default:
				break;
			}
		}
		if (PreType != type)
		{
			PreType = type;
			pRoot->ResetPageTimer();
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateIVIBtInfo(int btstate, std::string name)
{
	//LOGERR("function %s", __func__);
	//testLogManager testLog(__func__);
	//LOGDBG("btstate = %d, name = %s", btstate, name.c_str());
	auto pRoot = this->pMenuCtrl;
	auto UpdateAttrTask = [btstate, pRoot]() -> bool
	{
		DiagnosesPtrIsNullptr(pRoot);
		if (btstate == 0)
		{
			int CurPage = pRoot->getCurMenuPageID();
			if (CurPage == Menu_BTPhone_Incoming || CurPage == Menu_BTPhone_Dial || CurPage == Menu_BTPhone_InCalling || CurPage == Menu_BTPhone_CallEnd)
				pRoot->CloseChargePage();
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

void CDispCtrlScreenLayer::UpdateIVIBrightness(int iviBrightness)
{
	HmiIPC::TransitIVIBrightness(iviBrightness);
}
void CDispCtrlScreenLayer::UpdateIVIKeyMode(int iviKey)
{
	//LOGERR("iviKey = %d\n", iviKey);
	//testLogManager testLog(__func__);
	auto pMenuDisplay = this->pMenuCtrl;
	auto UpdateKey = [iviKey, pMenuDisplay]() -> bool
	{
		if (iviKey == 2)
		{
			int CurPage = pMenuDisplay->getCurMenuPageID();
			if (CurPage == Menu_BTPhone_Incoming || CurPage == Menu_BTPhone_Dial || CurPage == Menu_BTPhone_InCalling || CurPage == Menu_BTPhone_CallEnd || CurPage == Menu_ChargeSubscribe\
				|| CurPage == Menu_Charging || CurPage == Menu_ChargeResult)
			{
				return true;
			}
			pMenuDisplay->UpdateIVIModeKey();
		}
		return true;
	};
	PushUpdateKzAttrTask(UpdateKey);
}

/*void CDispCtrlScreenLayer::SwitchTheme(int style)
{
	LOGERR("@@@@@@@@@@@@@style = %d\n", style);
	static int preStyle = -1;
	if (style != preStyle)
	{
		if (mMainView->getChild(0)->getChildCount() != 0)
		{
			mMainView->getChild(0)->removeAbstractChildOverride(*(mStyleClassic.get()));
			if (mStylePrefabClassic)
			{
				mStylePrefabClassic.reset();
			}
			if (mStyleClassic)
			{
				mStyleClassic.reset();
			}
			mMainView->getResourceManager()->purge();
		}
		if (mMainView->getChild(1)->getChildCount() != 0)
		{
			mMainView->getChild(1)->removeAbstractChildOverride(*(mStyleSport.get()));
			if (mStylePrefabSport)
			{
				mStylePrefabSport.reset();
			}
			if (mStyleSport)
			{
				mStyleSport.reset();
			}
			mMainView->getResourceManager()->purge();
		}
		if (mMainView->getChild(2)->getChildCount() != 0)
		{
			mMainView->getChild(2)->removeAbstractChildOverride(*(mStyleTechnology.get()));
			if (mStylePrefabTechnology)
			{
				mStylePrefabTechnology.reset();
			}
			if (mStyleTechnology)
			{
				mStyleTechnology.reset();
			}
			m_oRoot->getResourceManager()->purge();
		}
		if (mBgNode->getChild(0)->getChildCount() != 0)
		{
			mMainView->getChild(0)->removeAbstractChildOverride(*(mBgClassic.get()));
			if (mBgPrefabClassic)
			{
				mBgPrefabClassic.reset();
			}
			if (mBgClassic)
			{
				mBgClassic.reset();
			}
			mBgNode->getResourceManager()->purge();
		}
		if (mBgNode->getChild(1)->getChildCount() != 0)
		{
			mMainView->getChild(1)->removeAbstractChildOverride(*(mBgSport.get()));
			if (mBgPrefabSport)
			{
				mBgPrefabSport.reset();
			}
			if (mBgSport)
			{
				mBgSport.reset();
			}
			mBgNode->getResourceManager()->purge();
		}
		if (mBgNode->getChild(2)->getChildCount() != 0)
		{
			mMainView->getChild(2)->removeAbstractChildOverride(*(mBgTechnology.get()));
			if (mBgPrefabTechnology)
			{
				mBgPrefabTechnology.reset();
			}
			if (mBgTechnology)
			{
				mBgTechnology.reset();
			}
			mBgNode->getResourceManager()->purge();
		}
		switch (style)
		{
		case 0:
			KANZILOADPREFAB_THEME(m_oRoot, mBgNode->getChild(0), "bgClassic", "kzb://a301/Prefabs/Prefab_Bg_Classic", mBgClassic, mBgPrefabClassic);
			KANZILOADPREFAB_THEME(m_oRoot, mMainView->getChild(0), "styleClassic", "kzb://a301/Prefabs/Prefab_Style_Classic", mStyleClassic, mStylePrefabClassic);
			break;
		case 1:
			KANZILOADPREFAB_THEME(m_oRoot, mBgNode->getChild(1), "bgSport", "kzb://a301/Prefabs/Prefab_Bg_Sport", mBgSport, mBgPrefabSport);
			KANZILOADPREFAB_THEME(m_oRoot, mMainView->getChild(1), "styleSport", "kzb://a301/Prefabs/Prefab_Style_Sport", mStyleSport, mStylePrefabSport);
			break;
		case 2:
			KANZILOADPREFAB_THEME(m_oRoot, mBgNode->getChild(2), "bgTechnology", "kzb://a301/Prefabs/Prefab_Bg_Technology", mBgTechnology, mBgPrefabTechnology);
			KANZILOADPREFAB_THEME(m_oRoot, mMainView->getChild(2), "styleTechnology", "kzb://a301/Prefabs/Prefab_Style_Technology", mStyleTechnology, mStylePrefabTechnology);
			break;
		default:
			break;
		}
		mMainView->setProperty(DynamicPropertyType<int>("Common.StyleType"), style);
		pMenuCtrl->setCurStyle(style);
		SwitchThemeSyncInfo(style);
		preStyle = style;
	}

}*/

void CDispCtrlScreenLayer::SwitchTheme(int style)
{
	//testLogManager testLog(__func__);
	LOG_RECORD("SetCurStyleType = %d\n", style);
	static int preStyle = -1;
	if (style != preStyle)
	{
		mMainView->setProperty(DynamicPropertyType<int>("Common.StyleType"), style);
		pMenuCtrl->setCurStyle(style);
		preStyle = style;
	}
}

void CDispCtrlScreenLayer::SwitchSportColor(int value)
{
	//testLogManager testLog(__func__);
	/*auto func = [](const char* funcName, int line,int value)->void{
			{
				FILE* fp = fopen("/tmp/MsgService.txt", "a+");
				if (fp)
				{
					fprintf(fp, "(%s,%d): time:%d ,value = %d\n", funcName, line, ZH::BaseLib::getWorldTimeMS(), value);
					fflush(fp);
					fclose(fp);
				}
			} while (0);
	};
	func(__func__, __LINE__, m_nCurSportColor);*/
	auto setSportColor = [this](int value)->void{
			{
				m_nCurSportColor = value;
				LaneAnimation_SetColor(EnLaneColor(value));
				mMainView->setProperty(DynamicPropertyType<int>("Common.ColorSportState"), value);
			}
	};
	switch (m_nCurSportColor)
	{
	case 0:
		if (value > 90)
			setSportColor(3);
		else if (value > 45)
			setSportColor(2);
		else if (value > -15)
			setSportColor(1);
		break;
	case 1:
		if (value > 90)
			setSportColor(3);
		else if (value > 45)
			setSportColor(2);
		else if (value < -45)
			setSportColor(0);
		break;
	case 2:
		if (value > 90)
			setSportColor(3);
		else if (value < -45)
			setSportColor(0);
		else if (value < 15)
			setSportColor(1);
		break;
	case 3:
		if (value < -45)
			setSportColor(0);
		else if (value < 15)
			setSportColor(1);
		else if (value < 65)
			setSportColor(2);
		break;
	default:
		break;
	}
}

void CDispCtrlScreenLayer::SwitchThemeSyncInfo(int theme)
{
	//testLogManager testLog(__func__);
}

void CDispCtrlScreenLayer::UpdateIVIThemeColor(int theme, int color)
{
	//testLogManager testLog(__func__);
	int Theme = 0;
	int Color = 0;
	if (theme > 0)
	{
		Theme = theme - 1;
		if (Theme == 2)
		{
			switch (color)
			{
			case 1:
				Color = 2;
				break;
			case 2:
				Color = 0;
				break;
			case 3:
				Color = 1;
				break;
			default:
				Color = 0;
				break;
			}
		}
		else
		{
			Color = color - 1;
		}
	}
	else
		return;
	HmiIPC::SetThemeColor(Theme, Color);
	auto UpdateAttrTask = [Theme, Color, this]() -> bool
	{
		pMenuCtrl->setCurStyle(Theme);
		pMenuCtrl->SetThemeColor(Theme, Color);
		SwitchTheme(Theme);
		pMenuCtrl->SyncMainInterface(Theme);
		return true;
	};
	PushUpdateKzAttrTask(UpdateAttrTask);
}

uint32_t testLogManager::sLogCount = 0;
testLogManager::testLogManager(const char* log)
{
	mStrLogContent = log;
	setTestLog();
}

testLogManager::~testLogManager()
{
	setTestLog(false);
}
void testLogManager::setTestLog(bool isEnter)
{
	if (sLogCount > 200)
	{
		FILE* fp = fopen("/tmp/hmi.log", "w");
		if (fp)
		{
			fclose(fp);
		}
		sLogCount = 0;
	}

	int fd = open("/tmp/hmi.log", O_WRONLY | O_CREAT | O_APPEND);
	char logbuf[128];
	snprintf(logbuf, sizeof(logbuf), "%s:%s \n", (isEnter ? "Enter" : "Exit"), mStrLogContent.data());
	write(fd, logbuf, strlen(logbuf));
	fsync(fd);
	close(fd);
	sLogCount++;
}

void CDispCtrlScreenLayer::RestartMomoryData()
{
	ZH::BaseLib::CMemoryDataManager momoryDataManager;
	
	uint32_t dte = 0xffffffff;
	momoryDataManager.DataManager_GetDTE(dte);
	bool DTEvalidty = true;
	if (dte != 0xffffffff)
	{
		if (dte >= 0x7FF)
			DTEvalidty = false;
		UpdateDispRechargeMileage(float(dte), DTEvalidty);
	}

	uint32_t odo = 0xffffffff;
	momoryDataManager.DataManager_GetODO(odo);
	if (odo != 0xffffffff)
	{
		UpdateDispOdograph(odo);
	}

	uint32_t tripA = 0xffffffff;
	momoryDataManager.DataManager_GetTripA(tripA);
	if (tripA != 0xffffffff)
	{
		UpdateDispTripA(float(tripA));
	}

	uint32_t tripB = 0xffffffff;
	momoryDataManager.DataManager_GetTripB(tripB);
	if (tripB != 0xffffffff)
	{
		UpdateDispTripB(float(tripB));
	}

	uint16_t spped = 0xffff;
	momoryDataManager.DataManager_GetVehicleSpeed(spped);
	if (spped >= 0 && spped <= 160)
	{
		UpdateDispVehicleSpeed(spped, true);
	}

	uint16_t power = 0xffff;
	momoryDataManager.DataManager_GetPowerValue(power);
	if (power >= -100 && power <= 100)
	{
		UpdateDispPowerValue(power, true);
	}

	uint16_t VehMomEgyCnse = 0xffff;
	momoryDataManager.DataManager_GetVehMomEgyCnse(VehMomEgyCnse);
	if (VehMomEgyCnse != 0xffff)
	{
		UpdateInstEnergyConsum(float(VehMomEgyCnse));
	}

	uint16_t AVPC = 0xffff;
	momoryDataManager.DataManager_GetAVPC(AVPC);
	if (AVPC != 0xffff)
	{
		UpdateAverEnergyConsum(float(AVPC));
	}

	uint8_t theme = 0xff;
	uint8_t color = 0xff;
	momoryDataManager.DataManager_GetTheme(theme, color);
	LOG_RECORD("theme = %d,color = %d\n", theme, color);
	if (theme != 0xff && color != 0xff)
	{
		UpdateThemeColor(theme, color);
	}

	uint8_t voiceState = 0xff;
	uint8_t voiceSwitch = 0xff;
	momoryDataManager.DataManager_GetVoiceSwtich(voiceState, voiceSwitch);
	if (voiceState != 0xff && voiceSwitch != 0xff)
	{
		UpdateVoicePlay(voiceState, voiceSwitch);
	}

	uint8_t socValue = 0xff;
	uint8_t socState = 0xff;
	momoryDataManager.DataManager_GetSocElectricity(socValue, socState);
	if (socValue != 0xff && socState != 0xff)
	{
		UpdateDispSOCValue(socValue, socState, true);
	}

	uint8_t gear = 0xff;
	momoryDataManager.DataManager_GetGear(gear);
	if (gear != 0xff)
	{
		UpdateDispGear(EmGearsValue(gear));
	}

	uint8_t EgyRgnStyle = 0xff;
	momoryDataManager.DataManager_GetEgyRgnStyle(EgyRgnStyle);
	if (EgyRgnStyle != 0xff)
	{
		UpdateDispEnergyRecycle(EgyRgnStyle);
	}

	uint8_t powerMode = 0xff;
	momoryDataManager.DataManager_GetPowerMode(powerMode);
	if (powerMode != 0xff)
	{
		UpdatePowerStatus(EmPowerStatus(powerMode));
	}
}