#include "TTIconProcess.h"
#include "hmi/WarnIdDefine.h"
#include "hmi/MsgInterface.h"

CTTIconProcess::CTTIconProcess()
{
	//不自检
	//mTTMap[LAMP_LEFT_TURN] = {EnTTSelfCheckType::NONE, 0}; //左转向指示灯
	//mTTMap[LAMP_RIGHT_TURN] = {EnTTSelfCheckType::NONE,	0};//右转向指示灯
	mTTMap[LAMP_BATTERY_FAULT] = {EnTTSelfCheckType::NONE,	0};//12V蓄电池故障指示灯
	mTTMap[LAMP_EPB] = {EnTTSelfCheckType::NONE,	0};//手刹工作指示灯
	mTTMap[LAMP_HIGH_BEAM] = {EnTTSelfCheckType::NONE,	0};//远光灯开关状态		5
	mTTMap[LAMP_LOW_BEAM] = {EnTTSelfCheckType::NONE,	0};//近光灯开关状态
	mTTMap[LAMP_REAR_FOG] = {EnTTSelfCheckType::NONE,	0};//后雾灯开关状态
	mTTMap[LAMP_POSITION] = {EnTTSelfCheckType::NONE,	0};//位置灯开关状态
	mTTMap[LAMP_MAIN_SEATBELT_INDICATOR] = {EnTTSelfCheckType::NONE,	0};//主驾安全带未系报警灯	
	mTTMap[LAMP_READY_STATUS] = {EnTTSelfCheckType::NONE,	0};//READY状态指示灯
	mTTMap[LAMP_CHARGECABLE_CONNECT] = {EnTTSelfCheckType::NONE,	0};//充电线连接状态指示灯
	mTTMap[LAMP_CHARGE_STATUS] = {EnTTSelfCheckType::NONE,	0};//充电状态指示灯		15
	mTTMap[LAMP_POWER_LIMIT] = {EnTTSelfCheckType::NONE,	0};//功率限制报警灯		
//	mTTMap[LAMP_MAIN_ALARM] = {EnTTSelfCheckType::NONE,	0};//主报警指示灯
	mTTMap[LAMP_DOOR_OPEN] = {EnTTSelfCheckType::NONE,	0};//门开报警灯
	mTTMap[LAMP_BATTERY_CHARGE_HEAT] = {EnTTSelfCheckType::NONE,	0};//电池充电加热指示灯	25'
	//内部自检
	mTTMap[LAMP_BRAKE_FLUID_LEVEL] = {EnTTSelfCheckType::INTERNAL,	0};//制动系统故障指示灯   0
	mTTMap[LAMP_ABS_FAULT] = {EnTTSelfCheckType::INTERNAL,	0};//ABS故障指示灯
	mTTMap[LAMP_EPS_FAULT] = {EnTTSelfCheckType::INTERNAL,	0};//EPS故障指示灯
	mTTMap[LAMP_TMPS_FAULT] = {EnTTSelfCheckType::INTERNAL,	0};//胎压系统报警灯
	mTTMap[LAMP_LOWCHARGE] = {EnTTSelfCheckType::INTERNAL,	0};//低电荷状态报警灯
	mTTMap[LAMP_POWER_BATTERY_FAULT] = {EnTTSelfCheckType::INTERNAL,	0};//动力电池故障指示灯
	mTTMap[LAMP_POWER_BATTERY_HIGHTEMP] = {EnTTSelfCheckType::INTERNAL,	0};//动力蓄电池高温报警灯	20
	mTTMap[LAMP_DRIVE_MOTOR_FAULT] = {EnTTSelfCheckType::INTERNAL,	0};//驱动电机故障指示灯	
	mTTMap[LAMP_SYSTEM_FAULT_YELLOW] = {EnTTSelfCheckType::INTERNAL,	0};//系统故障指示灯_黄灯
	mTTMap[LAMP_SYSTEM_FAULT_RED] = {EnTTSelfCheckType::INTERNAL,	0};//系统故障指示灯_红灯
	//外部自检
	mTTMap[LAMP_EPB_FAULT] = {EnTTSelfCheckType::EXTERNAL,	0};//EPB故障指示灯
	mTTMap[LAMP_AIR_BAG_FAULT] = {EnTTSelfCheckType::EXTERNAL,	0};//安全气囊报警灯		10


	mTimer = new STimer((3*1000), std::bind(&CTTIconProcess::TTIconSelfCheck_OnTimer, this));
	STimerManager::getInstance()->addTimerNode(mTimer);

	mSelfCheckStatus = EnTTSelfCheckStatus::STS_NONE;
	mSelfCheckChangeStsNotify = []()->void{
		LOG_SDEBUG("(%s,%d)init callback\n", __func__, __LINE__);
	};
	mbHmiPageReady = false;
}
CTTIconProcess::~CTTIconProcess()
{
	STimerManager::getInstance()->removeTimerNode(mTimer);
	delete mTimer;
}

void CTTIconProcess::TTIconSelfCheck_OnTimer()
{
	LOG_RECORD("%s ####", __func__);
	mSelfCheckStatus = EnTTSelfCheckStatus::STS_SELFCHECK_FINISH;
	mSelfCheckChangeStsNotify();
	updateHMIICon();
}
void CTTIconProcess::setTTData(const uint32_t* ttValue, uint32_t len)
{
	if(len < 26)
	{
		return;
	}
	mTTMap[LAMP_BRAKE_FLUID_LEVEL].mcuValue	= ttValue[0];
	mTTMap[LAMP_BATTERY_FAULT].mcuValue = ttValue[1];
	mTTMap[LAMP_ABS_FAULT].mcuValue = ttValue[2];
	mTTMap[LAMP_EPS_FAULT].mcuValue = ttValue[3];
	mTTMap[LAMP_EPB].mcuValue = ttValue[4];
	mTTMap[LAMP_HIGH_BEAM].mcuValue = ttValue[5];
	mTTMap[LAMP_LOW_BEAM].mcuValue = ttValue[6];
	mTTMap[LAMP_REAR_FOG].mcuValue = ttValue[7];
	mTTMap[LAMP_POSITION].mcuValue = ttValue[8];
	mTTMap[LAMP_EPB_FAULT].mcuValue = ttValue[9];
	mTTMap[LAMP_AIR_BAG_FAULT].mcuValue = ttValue[10];
	mTTMap[LAMP_MAIN_SEATBELT_INDICATOR].mcuValue = ttValue[11];
	mTTMap[LAMP_TMPS_FAULT].mcuValue = ttValue[12];
	mTTMap[LAMP_READY_STATUS].mcuValue = ttValue[13];
	mTTMap[LAMP_CHARGECABLE_CONNECT].mcuValue = ttValue[14];
	mTTMap[LAMP_CHARGE_STATUS].mcuValue = ttValue[15];
	mTTMap[LAMP_POWER_LIMIT].mcuValue = ttValue[16];
	mTTMap[LAMP_LOWCHARGE].mcuValue = ttValue[18];
	mTTMap[LAMP_POWER_BATTERY_FAULT].mcuValue = ttValue[19];
	mTTMap[LAMP_POWER_BATTERY_HIGHTEMP].mcuValue = ttValue[20];
	mTTMap[LAMP_DRIVE_MOTOR_FAULT].mcuValue = ttValue[21];
	mTTMap[LAMP_SYSTEM_FAULT_YELLOW].mcuValue = ttValue[22];
	mTTMap[LAMP_SYSTEM_FAULT_RED].mcuValue = ttValue[23];
	mTTMap[LAMP_DOOR_OPEN].mcuValue = ttValue[24];
	mTTMap[LAMP_BATTERY_CHARGE_HEAT].mcuValue = ttValue[25];
	updateHMIICon();
}
void CTTIconProcess::setTTData(const std::vector<unsigned int>& ttValue)
{
	if(ttValue.size() < 26)
	{
		return;
	}
	mTTMap[LAMP_BRAKE_FLUID_LEVEL].mcuValue	= ttValue[0];
	mTTMap[LAMP_BATTERY_FAULT].mcuValue = ttValue[1];
	mTTMap[LAMP_ABS_FAULT].mcuValue = ttValue[2];
	mTTMap[LAMP_EPS_FAULT].mcuValue = ttValue[3];
	mTTMap[LAMP_EPB].mcuValue = ttValue[4];
	mTTMap[LAMP_HIGH_BEAM].mcuValue = ttValue[5];
	mTTMap[LAMP_LOW_BEAM].mcuValue = ttValue[6];
	mTTMap[LAMP_REAR_FOG].mcuValue = ttValue[7];
	mTTMap[LAMP_POSITION].mcuValue = ttValue[8];
	mTTMap[LAMP_EPB_FAULT].mcuValue = ttValue[9];
	mTTMap[LAMP_AIR_BAG_FAULT].mcuValue = ttValue[10];
	mTTMap[LAMP_MAIN_SEATBELT_INDICATOR].mcuValue = ttValue[11];
	mTTMap[LAMP_TMPS_FAULT].mcuValue = ttValue[12];
	mTTMap[LAMP_READY_STATUS].mcuValue = ttValue[13];
	mTTMap[LAMP_CHARGECABLE_CONNECT].mcuValue = ttValue[14];
	mTTMap[LAMP_CHARGE_STATUS].mcuValue = ttValue[15];
	mTTMap[LAMP_POWER_LIMIT].mcuValue = ttValue[16];
	mTTMap[LAMP_LOWCHARGE].mcuValue = ttValue[18];
	mTTMap[LAMP_POWER_BATTERY_FAULT].mcuValue = ttValue[19];
	mTTMap[LAMP_POWER_BATTERY_HIGHTEMP].mcuValue = ttValue[20];
	mTTMap[LAMP_DRIVE_MOTOR_FAULT].mcuValue = ttValue[21];
	mTTMap[LAMP_SYSTEM_FAULT_YELLOW].mcuValue = ttValue[22];
	mTTMap[LAMP_SYSTEM_FAULT_RED].mcuValue = ttValue[23];
	mTTMap[LAMP_DOOR_OPEN].mcuValue = ttValue[24];
	mTTMap[LAMP_BATTERY_CHARGE_HEAT].mcuValue = ttValue[25];
	updateHMIICon();
}
void CTTIconProcess::updateHMIICon()
{
	LOG_RECORD_DEBUG("%s, mSelfCheckStatus:%d", __func__, mSelfCheckStatus);
	#define STATE_TRANSITION(data) (data <= 5 ? static_cast<EmLampState>(data): EmLampState::NONE)
	std::vector<StuTelltableLampState> lamp;
	if(EnTTSelfCheckStatus::STS_SELFCHECKING == mSelfCheckStatus)
	{
		for(auto iter : mTTMap)
		{
			if(EnTTSelfCheckType::INTERNAL == iter.second.type)
			{
				lamp.push_back({iter.first, EmLampState::BRIGHT});
			}
			else
			{
				lamp.push_back({iter.first, STATE_TRANSITION(iter.second.mcuValue)});
			}
		}
	}
	else
	{
		for(auto iter : mTTMap)
		{
			lamp.push_back({iter.first, STATE_TRANSITION(iter.second.mcuValue)});
		}
	}
	HmiIPC::UpdateDispLamp(lamp);
}
void CTTIconProcess::setHMIPageReady()
{
	LOG_RECORD("%s, mSelfCheckStatus:%d, mLastPowerMode:%d", __func__, mSelfCheckStatus, mLastPowerMode);
	mbHmiPageReady = true;
	if(mLastPowerMode == 1)
	{
		mSelfCheckStatus = EnTTSelfCheckStatus::STS_SELFCHECKING;
		mTimer->start();
		updateHMIICon();
	}
}
void CTTIconProcess::setPowerMode(int mode)
{
	LOG_RECORD_DEBUG("%s, mSelfCheckStatus:%d, mLastPowerMode:%d", __func__, mSelfCheckStatus, mLastPowerMode);
	EnTTSelfCheckStatus lastSts = mSelfCheckStatus;
	if(mLastPowerMode != mode)
	{
		mLastPowerMode = mode;
		if(mbHmiPageReady)
		{
			if(mode == 1)
			{
				//start selfCheck
				mSelfCheckStatus = EnTTSelfCheckStatus::STS_SELFCHECKING;
				mTimer->start();
			}
			else
			{
				mTimer->stop();
				mSelfCheckStatus = EnTTSelfCheckStatus::STS_NONE;
			}
		}
	}
	if(lastSts != mSelfCheckStatus)
	{
		mSelfCheckChangeStsNotify();
		updateHMIICon();
	}
}
CTTIconProcess::EnTTSelfCheckType CTTIconProcess::getSelfCheckTpye(std::string Lamp)
{
	if(mTTMap.find(Lamp) == mTTMap.end())
	{
		return EnTTSelfCheckType::NONE;
	}
	else
	{
		return mTTMap[Lamp].type;
	}
}

void CTTIconProcess::setTTValue(std::string Lamp, uint32_t value)
{
	if(mTTMap.find(Lamp) != mTTMap.end())
	{
		mTTMap[Lamp].mcuValue = value;
	}
}