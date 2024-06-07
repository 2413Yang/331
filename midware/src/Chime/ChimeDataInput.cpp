#include "ChimeDataInput.h"
#include "ChimeService.h"
#include "LogServiceApi.h"

using namespace chime;

CChimeDataInput::CChimeDataInput():
	mBcmBuzWarnMod(0),
	mbReadyIconSts(false),
	mbLowBrakeFluidIconSts(false),
	mbTurnLeftIconSts(false),
	mbTurnRightIconSts(false),
	mChimeIconData(0),
	mVolume(EnVolume::VOLUME_INVALID),
	mVoiceSw(2),
	mbFirstFlag(true),
	mLastDiplayWarnID(EmPopWarnID::NONE),
	mDTE(0xffff),
	mOverSpdFlag(false),
	mWarnData6(0),
	mWarnData8(0),
	mIPC("ChimeService"),
	mLogIPC("LogService")
{
	//mMapPop2Chime[EmPopWarnID::ENGINE_FAIL_STOP] = EnChimeID::CHIME_ID_FAULT_1;//动力系统故障，请靠边停车
	//mMapPop2Chime[EmPopWarnID::ENGINE_LIMIT_SLOW] = EnChimeID::CHIME_ID_FAULT_2;//动力限制，请减速慢行
	//mMapPop2Chime[EmPopWarnID::BRAKE_BOOSTER_REDUCE] = EnChimeID::CHIME_ID_FAULT_3;//制动助力不足，请谨慎驾驶
	//mMapPop2Chime[EmPopWarnID::LOW_BATTERY_CHARGE] = EnChimeID::CHIME_ID_FAULT_5;//动力电池电量低，请及时充电
	mMapPop2Chime[EmPopWarnID::VEHICLE_POWERING] = EnChimeID::CHIME_ID_PROMPT_VEHICLE_POWERING;//车辆未下电
	mMapPop2Chime[EmPopWarnID::BRAKE_CHANGE_GEAR] = EnChimeID::CHIME_ID_PROMPT_BRAKE_CHANGE_GEAR;//换挡时请踩下制动踏板
	mMapPop2Chime[EmPopWarnID::BRAKE_START_GEAR] = EnChimeID::CHIME_ID_PROMPT_BRAKE_START_GEAR;//请启动车辆后再换挡
	mMapPop2Chime[EmPopWarnID::START_BRAKE] = EnChimeID::CHIME_ID_PROMPT_START_BRAKE;//启动时请踩下制动踏板
	mMapPop2Chime[EmPopWarnID::TIRE_ADJUST_LEFT] = EnChimeID::CHIME_ID_TIRE_ADJUST_LEFT;//请向左打方向盘，调整轮胎
	mMapPop2Chime[EmPopWarnID::TIRE_ADJUST_RIGHT] = EnChimeID::CHIME_ID_TIRE_ADJUST_RIGHT;//请向右打方向盘，调整轮胎
	mMapPop2Chime[EmPopWarnID::LOW_SPEED_WARN_MAN_OFF] = EnChimeID::CHIME_ID_PROMPT_PWS_OFF;//行人警示系统已关闭
	mMapPop2Chime[EmPopWarnID::GEAR_SYS_4S] = EnChimeID::CHIME_ID_PROMPT_ENGINE_FAIL_4S;//换挡器故障
	//
	mMapBcmSignal2Chime[0x01] = EnChimeID::CHIME_ID_BODYCTRL_0;//跟随回家开启提示
	mMapBcmSignal2Chime[0x02] = EnChimeID::CHIME_ID_BODYCTRL_1;//速度落锁设置为取消
	mMapBcmSignal2Chime[0x03] = EnChimeID::CHIME_ID_BODYCTRL_2;//速度落锁设置为激活
	mMapBcmSignal2Chime[0x04] = EnChimeID::CHIME_ID_BODYCTRL_3;//钥匙学习成功
	mMapBcmSignal2Chime[0x05] = EnChimeID::CHIME_ID_BODYCTRL_4;//驾驶员门解锁模式设置为取消
	mMapBcmSignal2Chime[0x06] = EnChimeID::CHIME_ID_BODYCTRL_5;//驾驶员门解锁模式设置为取消
	mMapBcmSignal2Chime[0x11] = EnChimeID::CHIME_ID_BODYCTRL_6;//天窗未关提示
	mMapBcmSignal2Chime[0x12] = EnChimeID::CHIME_ID_BODYCTRL_7;//钥匙在车内
	mMapBcmSignal2Chime[0x13] = EnChimeID::CHIME_ID_BODYCTRL_8;//未检测到钥匙报警
	mMapBcmSignal2Chime[0x14] = EnChimeID::CHIME_ID_BODYCTRL_9;//退出防盗激活状态提示保留
	mMapBcmSignal2Chime[0x15] = EnChimeID::CHIME_ID_BODYCTRL_A;//门未关闭锁
	mMapBcmSignal2Chime[0x16] = EnChimeID::CHIME_ID_BODYCTRL_B;//遥控电池电量低
	mMapBcmSignal2Chime[0x17] = EnChimeID::CHIME_ID_BODYCTRL_C;//副驾驶安全带未系
	mMapBcmSignal2Chime[0x10] = EnChimeID::CHIME_ID_BODYCTRL_D;//灯未关提示； 
	mMapBcmSignal2Chime[0x19] = EnChimeID::CHIME_ID_BODYCTRL_E;//主驾驶安全带未系； 
	mMapBcmSignal2Chime[0x1B] = EnChimeID::CHIME_ID_BODYCTRL_F;//门未关提示；

	mDelayProcessSignalTimer = new STimer(1000,std::bind(&CChimeDataInput::delayProcessSignal_OnTime, this));
	STimerManager::getInstance()->addTimerNode(mDelayProcessSignalTimer);

	subscriber::chimeSettings_VoiceVolumeCtrl(mIPC, *this);
	subscriber::chimeInput_BcmBuzWarnMod(mIPC, *this);
	subscriber::chimeInput_Icon(mIPC, *this);
	subscriber::chimeInput_SyncDisplayWarning(mIPC, *this);
	subscriber::chimeInput_DTEValue(mIPC, *this);
	subscriber::chimeInput_OverSpdSts(mIPC, *this);
	subscriber::chimeInput_Data6_8(mIPC, *this);
	subscriber::chime_CurrentPowerMode(mIPC, *this);
	subscriber::chimeInput_HmiReady(mIPC, *this);

    publisher::chimeOutput_ChimeReady(mIPC);
	mIPC.start();
	ZH::logService::publisher::LogRecord(mLogIPC);
	mLogIPC.start();
	CChimeService::getInstance();

    chimeOutput_ChimeReady(0);
}

CChimeDataInput::~CChimeDataInput()
{
	if(mDelayProcessSignalTimer)
	{
		STimerManager::getInstance()->removeTimerNode(mDelayProcessSignalTimer);
		delete mDelayProcessSignalTimer;
	}
}
void CChimeDataInput::chimeSettings_VoiceVolumeCtrl(int sw, EnVolume volume)
{
	const uint8_t covertVolumeList[4] = {9, 8, 6, 8};
	LOG_RECORD("(%s,%d), sw:%d,volume:%d\n",__func__, __LINE__,sw, int(volume));
	if(sw != mVoiceSw)
	{
		mVoiceSw = sw;
        bool voiceSw = sw ? false : true;
		CChimeService::getInstance()->chimeSettings_VoiceAndVolume(voiceSw,volume);
		if(mbFirstFlag)
		{
			mbFirstFlag = false;
		}
		else
		{
			if(0 == sw)
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_VOICE_OFF, false);
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_VOICE_ON, true);
			}
			else
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_VOICE_OFF, true);
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_VOICE_ON, false);
			}
		}
		
	}
	
	if(volume != mVolume)
	{
		mVolume = volume;
		int vol = covertVolumeList[uint8_t(volume) % 3];
		CChimeService::getInstance()->setVolume(vol);
	}
}
void CChimeDataInput::chimeInput_BcmBuzWarnMod(int BcmBuzWarnMod)
{//根据BCM信号触发声音
	LOG_RECORD_DEBUG("(%s,%d), BcmBuzWarnMod:%d\n",__func__, __LINE__,BcmBuzWarnMod);
	if(mBcmBuzWarnMod != BcmBuzWarnMod)
	{
		if(mMapBcmSignal2Chime.find(mBcmBuzWarnMod) != mMapBcmSignal2Chime.end())
		{//解除触发
			CChimeService::getInstance()->checkTriggerChime(mMapBcmSignal2Chime[mBcmBuzWarnMod], false);
		}
		mBcmBuzWarnMod = BcmBuzWarnMod;
		#if 0
		if(mDelayProcessSignalTimer->getStatus() == timer_running)
		{
			return;
		}
		else
		{
			//process
			delayProcessSignal_OnTime();
			mDelayProcessSignalTimer->start();
		}
		#else
		delayProcessSignal_OnTime();
		#endif
	}
}
	
void CChimeDataInput::chimeInput_Icon(int iconBitwise)
{//同步指示灯触发声音
	LOG_RECORD_DEBUG("(%s,%d), iconBitwise:%0x\n",__func__, __LINE__, iconBitwise);
	if(mChimeIconData != iconBitwise)
	{
		mChimeIconData = iconBitwise;
		bool bReadyIconSts = GET_BITWISE_STATAS(iconBitwise, EnChimeLampBitwise::ICON_BITWISE_READY);
		if(mbReadyIconSts != bReadyIconSts)
		{
			if(bReadyIconSts)
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_READY_ICON, true);
			}
			else
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_READY_ICON, false);
			}
            LOG_RECORD("ready Icon state:%d\n", bReadyIconSts);
			mbReadyIconSts = bReadyIconSts;
		}
		bool lowBreakFluidSts = GET_BITWISE_STATAS(iconBitwise, EnChimeLampBitwise::ICON_BITWISE_LOWBREAKFLUID);
		#if 0
		//制动液位故障
		if(mbLowBrakeFluidIconSts != lowBreakFluidSts)
		{
			if(lowBreakFluidSts)
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_4, true);
			}
			else
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_4, false);
			}
			mbLowBrakeFluidIconSts = lowBreakFluidSts;
		}
		#endif
		bool bTurnLeftIconSts = GET_BITWISE_STATAS(iconBitwise, EnChimeLampBitwise::ICON_BITWISE_TURN_LEFT);
		bool turnBeepSts = false;
		if(mbTurnLeftIconSts != bTurnLeftIconSts)
		{
			LOG_RECORD_DEBUG("(%s,%d), mbTurnLeftIconSts:%d,bTurnLeftIconSts=%d\n",__func__, __LINE__, mbTurnLeftIconSts,bTurnLeftIconSts);
			turnBeepSts = true;
			if(!CChimeService::getInstance()->getSoundPlayingFlag())
			{
				if(bTurnLeftIconSts)
				{
					CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_TURN_ON, true);
					CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_TURN_OFF, false);
				}
				else
				{
					CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_TURN_ON, false);
					CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_TURN_OFF, true);
				}
			}
			mbTurnLeftIconSts = bTurnLeftIconSts;
		}
		bool bTurnRightIconSts = GET_BITWISE_STATAS(iconBitwise, EnChimeLampBitwise::ICON_BITWISE_TURN_RIGHT);
		if(mbTurnRightIconSts != bTurnRightIconSts)
		{
			LOG_RECORD_DEBUG("(%s,%d), mbTurnRightIconSts:%d,bTurnRightIconSts=%d\n",__func__, __LINE__, mbTurnRightIconSts,bTurnRightIconSts);
			if(!CChimeService::getInstance()->getSoundPlayingFlag())
			{
				if(turnBeepSts == false)
				{//如果左转向已经触发了响声，则此时右转向就不要再触发
					if(bTurnRightIconSts)
					{
						
						CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_TURN_ON, true);
						CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_TURN_OFF, false);
					}
					else
					{
						CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_TURN_ON, false);
						CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_TURN_OFF, true);
					}
				}
			}
			
			mbTurnRightIconSts = bTurnRightIconSts;
		}
		(void)lowBreakFluidSts;
		LOG_RECORD_DEBUG("(%s,%d), iconBitwise:%0x, l0:%d,l1:%d,l2:%d,l3:%d\n",__func__, __LINE__, iconBitwise,
		bReadyIconSts,lowBreakFluidSts,bTurnLeftIconSts,bTurnRightIconSts);
	}
}
void CChimeDataInput::chimeInput_SyncDisplayWarning(EmPopWarnID popWarnID)
{//同步弹窗报警触发声音
	if(mLastDiplayWarnID != popWarnID)
	{
		mLastDiplayWarnID = popWarnID;
		if(mMapPop2Chime.find(mLastDiplayWarnID) != mMapPop2Chime.end())
		{
			CChimeService::getInstance()->checkTriggerChime(mMapPop2Chime[mLastDiplayWarnID], false);
		}
		if(mMapPop2Chime.find(popWarnID) == mMapPop2Chime.end())
		{
			return;
		}
		LOG_RECORD_DEBUG("(%s,%d), popWarnID:%d\n",__func__, __LINE__, int(popWarnID));
		CChimeService::getInstance()->checkTriggerChime(mMapPop2Chime[popWarnID], true);
		
	}
	
}
void CChimeDataInput::chimeInput_DTEValue(int dte)
{
	if(mDTE != dte)
	{
		LOG_RECORD("(%s,%d) dte:%d, mDTE:%d\n", __func__,__LINE__, dte, mDTE);
		if(dte < 10)
		{
			if(mDTE >= 10)
			{
				if(mDTE <= 15)
				{
					CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_5, false);
				}
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_6, true);
			}
		}
		else if(dte <= 15)
		{
			if(mDTE < 10)
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_6, false);
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_5, true);
			}
			else if(mDTE > 15)
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_5, true);
			}
		}
		else
		{
			CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_5, false);
			CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_6, false);
		}
		mDTE = dte;
	}
}
void CChimeDataInput::chimeInput_OverSpdSts(bool flag)
{
	if(flag != mOverSpdFlag)
	{
		mOverSpdFlag = flag;
		CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_OVER_SPD_WARN, mOverSpdFlag);
		LOG_RECORD("(%s,%d) flag:%d\n", __func__,__LINE__, flag);
	}
}
void CChimeDataInput::chimeInput_Data6_8(int warnData6, int warnData8)
{	
	//warnData6 &= 0xf1;
	int xorBit4_7 = warnData6 ^ mWarnData6;
	if(xorBit4_7)
	{
		LOG_RECORD_DEBUG("(%s,%d) xorBit4_7:0x%02x, warnData6:0x%02x\n", __func__,__LINE__, xorBit4_7, warnData6);
		bool ReservationACFlag = (warnData6 & 0x01) ? true : false;
		CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_RESERVATION_AC, ReservationACFlag);
		#if 0
		bool acFlag = (warnData6 & 0x10) ? true:false;
		CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_AC_ON, acFlag);
		#else
		//制动液位故障
		bool Flag = (warnData6 & 0x10) ? true:false;
		if(Flag)
		{
			CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_4, true);
		}
		else
		{
			CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_FAULT_4, false);
		}
		#endif
		bool chrgReserveFlag = (warnData6 & 0x20) ? true:false;
		CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_RESERVATION_CHARGE, chrgReserveFlag);
		int VcuSysFltSnd = (warnData6 >> 6) & 0x03;
		int preVcuSysFltSnd = (mWarnData6 >> 6) & 0x03;
		const EnChimeID VcuSysFltSndChimeList[4] = {EnChimeID::END,EnChimeID::CHIME_ID_FAULT_1, EnChimeID::CHIME_ID_FAULT_2, EnChimeID::CHIME_ID_FAULT_3};
		if(VcuSysFltSnd != preVcuSysFltSnd)
		{
			CChimeService::getInstance()->checkTriggerChime(VcuSysFltSndChimeList[preVcuSysFltSnd], false);
			CChimeService::getInstance()->checkTriggerChime(VcuSysFltSndChimeList[VcuSysFltSnd], true);
		}
		mWarnData6 = warnData6;
	}
	int xorData8 = warnData8 ^ mWarnData8;
	if(xorData8)
	{
		int AcTLvl = warnData8 & 0x1f;
		int AcFrntWindLvl = warnData8 >> 5;
		LOG_RECORD_DEBUG("(%s,%d) xorData8:0x%02x, warnData8:0x%02x, AcTLvl:%d, AcFrntWindLvl:%d\n", __func__,__LINE__, xorData8, warnData8, AcTLvl, AcFrntWindLvl);
		if(xorData8 & 0xE0)
		{
			//AcFrntWindLvl			
			if(AcFrntWindLvl > 0)
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_AC_ON, true);
			}
			else
			{
				CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_AC_ON, false);
			}
		}
		if(((AcFrntWindLvl >= 5) && ((AcTLvl > 0) && (AcTLvl <= 3))) ||
			((AcFrntWindLvl > 0) && ((AcTLvl >= 14) && (AcTLvl <= 16))))
		{
			//空调高能耗状态
			CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_AC_HIGH_ENERGY_STS, true);
		}
		else
		{
			CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_AC_HIGH_ENERGY_STS, false);
		}
		if( ( ((AcFrntWindLvl == 1) || (AcFrntWindLvl == 2)) && ((AcTLvl >= 6) && (AcTLvl <= 8))) ||
			( (AcFrntWindLvl > 0) &&  ((AcTLvl >= 8) && (AcTLvl <= 11)) )
			)
		{
			CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_AC_LOW_POWER_STS, true);
		}
		else
		{
			CChimeService::getInstance()->checkTriggerChime(EnChimeID::CHIME_ID_PROMPT_AC_LOW_POWER_STS, false);
		}
		mWarnData8 = warnData8;
	}
}
void CChimeDataInput::chimeInput_HmiReady(int)
{
	CChimeService::getInstance()->setHmiReady(true);
}
void CChimeDataInput::chime_CurrentPowerMode(int mode)
{
	if(1 >= mode)
	{
		EnWorkPowerSts newSts;
		if(0 == mode)
		{
			newSts = EnWorkPowerSts::PM_IGN_OFF;
		}
		else
		{
			newSts = EnWorkPowerSts::PM_IGN_ON;
		}
		CChimeService::getInstance()->setCurrentPowerMode(newSts);
	}
}

void CChimeDataInput::delayProcessSignal_OnTime()
{
	if(mMapBcmSignal2Chime.find(mBcmBuzWarnMod) == mMapBcmSignal2Chime.end())
	{
		return;
	}
	LOG_RECORD_DEBUG("(%s,%d), mMapBcmSignal2Chime[%d]:%d\n",__func__, __LINE__,mBcmBuzWarnMod, int(mMapBcmSignal2Chime[mBcmBuzWarnMod]));
	CChimeService::getInstance()->checkTriggerChime(mMapBcmSignal2Chime[mBcmBuzWarnMod], true);
}