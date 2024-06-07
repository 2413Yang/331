#ifndef _CHIMEDATAINPUT_H_
#define _CHIMEDATAINPUT_H_
#include "ChimeApi.h"
#include "ChimeDataDef.h"
#include <map>
#include "STimer.h"
#include "IPC/IPCCore.h"
using namespace ZH::BaseLib;

#define GET_BITWISE_STATAS(u32Data,bitPos)		(((u32Data) & (1 << int(bitPos))) ? true:false)
namespace chime
{
	class CChimeDataInput
	{
	private:
		int			mBcmBuzWarnMod;
		bool		mbReadyIconSts;//false:Ready灯灭；true:Ready灯亮
		bool		mbLowBrakeFluidIconSts;//false:制动液位灯灭；true:制动液位灯亮
		bool		mbTurnLeftIconSts;//false:左转向灯灭；true:左转向灯亮
		bool		mbTurnRightIconSts;//false:右转向灯灭；true:右转向灯亮
		int			mChimeIconData;
		EnVolume	mVolume;
		int		    mVoiceSw;
		bool		mbFirstFlag;
		EmPopWarnID	mLastDiplayWarnID;
		int			mDTE;
		bool		mOverSpdFlag;
		int			mWarnData6;
		int			mWarnData8;
		CIPCConnector	mIPC;
		CIPCConnector	mLogIPC;
		std::map<EmPopWarnID, EnChimeID> 	mMapPop2Chime;
		std::map<int, EnChimeID> 			mMapBcmSignal2Chime;
		STimer*		mDelayProcessSignalTimer;
	public:
		CChimeDataInput();
		~CChimeDataInput();
	public:
		void chimeSettings_VoiceVolumeCtrl(int, EnVolume);
		void chimeInput_BcmBuzWarnMod(int);
		void chimeInput_Icon(int iconBitwise);
		void chimeInput_SyncDisplayWarning(EmPopWarnID);
		void chimeInput_DTEValue(int dte);
		void chimeInput_OverSpdSts(bool flag);
		void chimeInput_Data6_8(int warnData6, int warnData8);
		void chime_CurrentPowerMode(int mode);
		void chimeInput_HmiReady(int);
	private:
		void delayProcessSignal_OnTime();
	};
	
}

#endif //_CHIMEDATAINPUT_H_