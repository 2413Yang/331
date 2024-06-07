#ifndef _CHIME_API_H_
#define _CHIME_API_H_
#include "IPC/IPCCore.h"
#include "hmi/WarnIdDefine.h"

#ifndef SET_BITWISE_STATAS
#define SET_BITWISE_STATAS(u32Data,bitPos)		((u32Data) |= (1 << int(bitPos)))
#endif
#ifndef CLEAR_BITWISE_STATAS
#define CLEAR_BITWISE_STATAS(u32Data,bitPos)		((u32Data) &= ~(1 << int(bitPos)))
#endif
namespace chime
{
	enum class EnVolume
	{
		VOLUME_HIGH = 0,
		VOLUME_MEDIUM = 1,
		VOLUME_LOW = 2,
		VOLUME_INVALID,
	};
	enum class	EnChimeLampBitwise
	{
		ICON_BITWISE_READY = 0,			//ready灯
		ICON_BITWISE_LOWBREAKFLUID = 1,	//制动液位低灯
		ICON_BITWISE_TURN_LEFT = 2,		//左转向灯
		ICON_BITWISE_TURN_RIGHT = 3,	//右转向灯
		ICON_END
	};
	/**
	* chimeSettings_VoiceVolumeCtrl
	*
	* @param  {int}                 :语音播报开关，0-开启语音播报；1-关闭语音播报
	* @param  {EnVolume}             :音量调节，低、中、高
	*/
	USER_DEFINED(void, chimeSettings_VoiceVolumeCtrl, int, EnVolume);

	/**
	* chimeInput_BcmBuzWarnMod
	*
	* @param  {int}                 :mcu透传的CAN信号:BcmBuzWarnMod
	*/
	USER_DEFINED(void, chimeInput_BcmBuzWarnMod, int);
	/**
	* chimeInput_Icon
	*
	* @param  {int}                  :每个bit位对应一个指示灯的状态，EnChimeLampBitwise标明bit位和指示灯的对应关系
	*/
	USER_DEFINED(void, chimeInput_Icon, int);

	/**
	* chimeInput_SyncDisplayWarning
	*
	* @param  {EmPopWarnID}                 :当前显示的报警对应的声音
	*/
	USER_DEFINED(void, chimeInput_SyncDisplayWarning, EmPopWarnID);
	/**
	* chimeInput_DTEValue
	*
	* @param  {int}                 :续航里程值，0-100有效，当小于10，则触发“续航里程低，请立即充电”，当10~15则触发“续航里程低，请及时充电”
	*/
	USER_DEFINED(void, chimeInput_DTEValue, int);
	/**
	* chimeInput_OverSpdSts
	*
	* @param  {bool}                 :车速较快触发标志，true-触发，false-解除
	*/
	USER_DEFINED(void, chimeInput_OverSpdSts, bool);
	/**
	* chime_CurrentPowerMode
	*
	* @param  {int}                 :当前电源状态，0：IGN OFF；1：IGN ON
	*/
	USER_DEFINED(void, chime_CurrentPowerMode, int);
	/**
	* chimeInput_Data6_8
	*
	* @param  {int}                 :bit4：空调已开启（声音提示）bit5：充电预约成功（声音提示）
	* @param  {int}                 :bit0~4：空调耗能等级（声音提示）bit5：制动液位低指示灯（声音提示）
	*/
	USER_DEFINED(void, chimeInput_Data6_8, int, int);
	/**
	* chimeInput_HmiReady
	*
	* @param  {int}                 :hmi界面启动标志
	*/
	USER_DEFINED(void, chimeInput_HmiReady, int);
    /**
	* chimeOutput_ChimeReady
	*
	* @param  {int}                 :Chime启动标志
	*/
    USER_DEFINED(void, chimeOutput_ChimeReady, int);
}

#endif //_CHIME_API_H_