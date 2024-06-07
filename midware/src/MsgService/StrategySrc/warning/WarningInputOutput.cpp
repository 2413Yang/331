
#include "WarningInputOutput.h"
#include <string.h>
#include "WarningEngine.h"


using namespace warning;

CWarningInputOutput::CWarningInputOutput()
{
	mPreEpbPopsup = 0;
	memset(mMCUInputWarnData, 0, sizeof(mMCUInputWarnData));
}

CWarningInputOutput::~CWarningInputOutput()
{
}

void CWarningInputOutput::registerChangeWarn(std::function<void(EmPopWarnID)> cb)
{
	CWarningEngine::getInstance()->registerChangeWarn(cb);
}
void CWarningInputOutput::registeNotifyHistoryWarnList(std::function<void(std::vector<EmPopWarnID>&)> cb)
{
	CWarningEngine::getInstance()->registeNotifyHistoryWarnList(cb);
}

void CWarningInputOutput::warningInputData(const char* inputData)
{
	char xorBytes[sizeof(mMCUInputWarnData)];
	bool bChangeFlag = false;
	for (size_t i = 0; i < sizeof(mMCUInputWarnData); i++)
	{
		xorBytes[i] = mMCUInputWarnData[i] ^ inputData[i];
		if(xorBytes[i])
		{
			LOG_RECORD("_warn_%s xorBytes[%d]=0x%02x, input:0x%02x\n", __func__, i, xorBytes[i], inputData[i]);
			bChangeFlag = true;
		}
	}
	if(bChangeFlag)
	{
		//byte0 TPMS
		if(xorBytes[0])
		{
			processWarnInputBitwise(xorBytes, inputData, 0);
			mMCUInputWarnData[0] = inputData[0];
		}
		//byte1 动力系统文本提示VcuInfoDisp
		if(xorBytes[1])
		{
			EmPopWarnID preWarnId = (mMCUInputWarnData[1] < GET_ARRAY_ELEMENT_SIZE(cVcuInfoDispPops)) ? cVcuInfoDispPops[uint8_t(mMCUInputWarnData[1])] : EmPopWarnID::NONE;
			EmPopWarnID triggerWarnId = (inputData[1] < GET_ARRAY_ELEMENT_SIZE(cVcuInfoDispPops)) ? cVcuInfoDispPops[uint8_t(inputData[1])] : EmPopWarnID::NONE;
			CWarningEngine::getInstance()->checkTriggerWarn(preWarnId, false);
			CWarningEngine::getInstance()->checkTriggerWarn(triggerWarnId, true);
			mMCUInputWarnData[1] = inputData[1];
		}
		//byte2 动力系统文本提示VcuIInfoDisp2
		if(xorBytes[2])
		{
			if(mMCUInputWarnData[2] == 0x09)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::VEHICLE_POWERING, false);
			}
			else if(mMCUInputWarnData[2] == 0x08)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::ENGINE_FAIL_4S, false);
			}
			if(inputData[2] == 0x09)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::VEHICLE_POWERING, true);
			}
			else if(inputData[2] == 0x08)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::ENGINE_FAIL_4S, true);
			}
			mMCUInputWarnData[2] = inputData[2];
		}
		/*
		 * byte3 EPB文字提示 bit0~3:EpbTxtDispLe; bit4~7:EpbTxtDispRi
		 * 当EPbTxtDisple≠EpbTxtDispRi时，仪表根据两个信号值中优先级高的显示相应文字提示
		 * 优先级如下：02 > 01 > 03>05>00>0xF信号丢失时默认为0xF
		*/
		if(xorBytes[3])
		{
			uint8_t curEPbTxtDisple = inputData[3] & 0x0f;
			uint8_t curEpbTxtDispRi = (inputData[3] >> 4) & 0x0f;
			uint8_t curEpbTxtDispVaild = curEPbTxtDisple;
			int lePriority = 0xff,riPriority = 0xff;
			if(curEPbTxtDisple != curEpbTxtDispRi)
			{
				if(curEPbTxtDisple < sizeof(cEPB_PriorityList) / sizeof(cEPB_PriorityList[0]))
				{
					lePriority = cEPB_PriorityList[curEPbTxtDisple];
				}
				if(curEpbTxtDispRi < sizeof(cEPB_PriorityList) / sizeof(cEPB_PriorityList[0]))
				{
					riPriority = cEPB_PriorityList[curEpbTxtDispRi];
				}
				if(lePriority < riPriority)
				{
					curEpbTxtDispVaild = curEPbTxtDisple;
				}
				else
				{
					curEpbTxtDispVaild = curEpbTxtDispRi;
				}
			}
			LOG_RECORD_DEBUG("_warn_(%s,%d) mPreEpbPopsup[%d] curEpbTxtDispVaild=%d,Priority(L:%d,R:%d)\n", __func__, __LINE__, mPreEpbPopsup, curEpbTxtDispVaild,lePriority,riPriority);
			if(mPreEpbPopsup != curEpbTxtDispVaild)
			{
				EmPopWarnID preWarnId = (mPreEpbPopsup < GET_ARRAY_ELEMENT_SIZE(cEpbTxtDispPops)) ? cEpbTxtDispPops[mPreEpbPopsup] : EmPopWarnID::NONE;
				EmPopWarnID triggerWarnId = (curEpbTxtDispVaild < GET_ARRAY_ELEMENT_SIZE(cEpbTxtDispPops)) ? cEpbTxtDispPops[curEpbTxtDispVaild] : EmPopWarnID::NONE;
				CWarningEngine::getInstance()->checkTriggerWarn(preWarnId, false);
				CWarningEngine::getInstance()->checkTriggerWarn(triggerWarnId, true);
				mPreEpbPopsup = curEpbTxtDispVaild;
			}
			mMCUInputWarnData[3] = inputData[3];
		}
		//byte4 BCM文字提示
		if(xorBytes[4])
		{
			if(mMCUInputWarnData[4] == 0x15)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::DOOR_CLOSE_LOCK, false);
			}
			else if(mMCUInputWarnData[4] == 0x10)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::LIGHT_OFF, false);
			}
			else if(mMCUInputWarnData[4] == 0x17)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::FASTEN_SEAT_BELT_BCM_1, false);
			}
			else if(mMCUInputWarnData[4] == 0x19)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::FASTEN_SEAT_BELT_BCM_2, false);
			}
			//
			if(inputData[4] == 0x15)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::DOOR_CLOSE_LOCK, true);
			}
			else if(inputData[4] == 0x10)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::LIGHT_OFF, true);
			}
			else if(inputData[4] == 0x17)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::FASTEN_SEAT_BELT_BCM_1, true);
			}
			else if(inputData[4] == 0x19)
			{
				CWarningEngine::getInstance()->checkTriggerWarn(EmPopWarnID::FASTEN_SEAT_BELT_BCM_2, true);
			}
			mMCUInputWarnData[4] = inputData[4];
		}
		//byte5
		if(xorBytes[5])
		{
			processWarnInputBitwise(xorBytes, inputData, 5);
			mMCUInputWarnData[5] = inputData[5];
		}
		//byte6
		if(xorBytes[6])
		{
			processWarnInputBitwise(xorBytes, inputData, 6);
			mMCUInputWarnData[6] = inputData[6];
		}
		//byte7
		if(xorBytes[7])
		{
			char doorSts = inputData[7] >> 1 & 0x3f;
			CWarningEngine::getInstance()->updateDoorOpenWarn(doorSts);
			mMCUInputWarnData[7] = inputData[7];
		}
	}
}
void CWarningInputOutput::processWarnInputBitwise(const char* xorValues, const char* inputValue, int dataIndex)
{
	char xorValue = xorValues[dataIndex];

	while (xorValue)
	{
		uint8_t lowestBit = CWarningEngine::cLowestBitmap[uint8_t(xorValue)];
		int iKey = dataIndex*8 + lowestBit;
		EmPopWarnID warnID = CWarningData::getMapWarnInputBitwise(iKey);
		LOG_RECORD_DEBUG("_warn_%d, iKey:%d, warnID:%d\n", __LINE__, iKey, warnID);
		if(inputValue[dataIndex] & (1 << lowestBit))
		{
			CWarningEngine::getInstance()->checkTriggerWarn(warnID, true);
			//LOG_RECORD("_warn_(GetPopUpAlarm,line:%d), iKey:%d, warnID:%d\n", __LINE__, iKey, warnID);
		}
		else
		{
			CWarningEngine::getInstance()->checkTriggerWarn(warnID, false);
		}
		xorValue &= xorValue - 1;
	}
}

void CWarningInputOutput::setCurrentPowerMode(const int curPowerSts)
{
	EnWarnWorkPowerSts enCurPowerSts = (curPowerSts == 0) ? EnWarnWorkPowerSts::PM_IGN_OFF : EnWarnWorkPowerSts::PM_IGN_ON;
	CWarningEngine::getInstance()->setCurrentPowerMode(enCurPowerSts);
}
void CWarningInputOutput::setHmiReady()
{
	CWarningEngine::getInstance()->setHmiReady();
}

char CWarningInputOutput::getDoorSts()
{
	return CWarningEngine::getInstance()->getDoorOpenSts();
}

const EmPopWarnID	CWarningInputOutput::cVcuInfoDispPops[0x1E] = {
	EmPopWarnID::NONE,//0
	EmPopWarnID::PULL_OFF_CHARER,        //驾驶前请先拔充电枪
	EmPopWarnID::NONE,//2
	EmPopWarnID::NONE,//3
	EmPopWarnID::NONE,//4
	EmPopWarnID::POWER_SYS_TEMP_HIGH,    //驱动系统过温，请减速行驶
	EmPopWarnID::CHARGER_WAVE_REPLACE,   //电网波动，请更换充电地点
	EmPopWarnID::BRAKE_BOOSTER_REDUCE,	//制动助力不足，请谨慎驾驶
	EmPopWarnID::CHARGE_P_GEAR,			//请在P档下进行充电
	EmPopWarnID::POWER_SUPPLY,           //请刷卡或连接电源
	EmPopWarnID::NONE,//0x0A
	EmPopWarnID::BRAKE_ON_UNABLE_CHARGE, //无法充电，请拉起手刹
	EmPopWarnID::ENGINE_FAIL_STOP,		//动力系统故障，请靠边停车
	EmPopWarnID::CHARGER_WAVE_TIME_LONGER,//电网波动，充电时间延长
	EmPopWarnID::GEAR_SYS_4S,            //换挡器故障，请联系4S店检查
	EmPopWarnID::CHARGER_HAND_UNLOCK,    //充电枪解锁失败，请手动解锁
	EmPopWarnID::CHARGER_UNLOCK_TIME_LONGER,//充电枪未锁止，充电时间延长
	EmPopWarnID::NONE,//0x11
	EmPopWarnID::BRAKE_CHANGE_GEAR,		//换挡时请踩下制动踏板
	EmPopWarnID::CHARGER_UNLOCK_STOP,    //充电枪未锁止，充电停止
	EmPopWarnID::BAT_VOL_LOW,            //12V蓄电池电压过低，请靠边停车
	EmPopWarnID::LOW_BATTERY_CHARGE,			//动力电池电量低，请及时充电
	EmPopWarnID::ENGINE_LIMIT_SLOW,		//动力限制，请减速慢行
	EmPopWarnID::BRAKE_CHANGE_GEAR,		//换挡时请踩下制动踏板
	EmPopWarnID::BRAKE_START_GEAR,		//请踩制动踏板并按start按钮后再换挡
	EmPopWarnID::BRAKE_FAIL,				//手刹故障，驻车请注意(0x19)
	EmPopWarnID::NONE,//0x1A
	EmPopWarnID::CHARGING_NO_GEAR,		//当前车辆处于外接充电状态，无法切换挡位
	EmPopWarnID::NONE,//0x1C
	EmPopWarnID::REDUCE_SPEED_SHIFT_GEAR,//车速太高，请减速后再换挡(0x1D)
};
const EmPopWarnID	CWarningInputOutput::cEpbTxtDispPops[0x06] = {
	EmPopWarnID::NONE,//0
	EmPopWarnID::PARKING_RAMP,//1
	EmPopWarnID::PARK_BRAKE_LACK_4S,//2
	EmPopWarnID::FASTEN_SEAT_BELT_EPB,//3
	EmPopWarnID::NONE,//4
	EmPopWarnID::HANDBRAKE_BRAKE,//5
};
const uint8_t	CWarningInputOutput::cEPB_PriorityList[7] = {
0xff,	2,	1,	3,	0xff,	4,	0xff
};