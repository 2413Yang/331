#ifndef _CHIMEDATADEF_H_
#define _CHIMEDATADEF_H_
#include <stdint.h>
#include "LogServiceApi.h"
#include "mylogCtrl.h"
namespace chime
{
	enum class EnChimeCategory
	{
		CHIME_CATEGORY_Beep,
		CHIME_CATEGORY_Voice,
		CHIME_CATEGORY_BeepOrVoice,
		CHIME_CATEGORY_BODY_CTRL,
		CHIME_CATEGORY_PROMPT,
		CHIME_CATEGORY_TURN_LAMP,
		CHIME_CATEGORY_TURN_SYS,
		CHIME_CATEGORY_CAR_DISP_SYS,
	};
	enum class EnWorkPowerSts
	{
		PM_IGN_UNKNOWN,
		PM_IGN_OFF = 1,
		PM_IGN_ON = 2,
		PM_IGN_ALL = (PM_IGN_OFF | PM_IGN_ON),
	};
	enum class EnSoundType
	{
		Sound_Beep,
		Sound_Voice,
		Sound_End,
	};

	enum class EnChimeID
	{
		CHIME_ID_FAULT_1,//动力中断语音提示 (动力系统故障，请靠边停车)		//关联弹窗
		CHIME_ID_FAULT_2,//功率限制语音提示 (动力限制，请减速慢行) 			//关联弹窗
		CHIME_ID_FAULT_3,//制动真空助力故障 (制动助力不足，请谨慎驾驶)		//关联弹窗
		CHIME_ID_FAULT_4,//制动液位低      (制动液位低，请及时补充)			//关联指示灯
		CHIME_ID_FAULT_5,//续航里程低提醒   (续航里程低，请及时充电	)	//关联弹窗 
		CHIME_ID_FAULT_6,//续航里程低提醒   (续航里程低，请立即充电	)	//续航里程低于10就报一次

		CHIME_ID_BODYCTRL_0,//跟随回家开启提示		//CHIME_ID_BODYCTRL类的都关联BcmBuzWarnMod信号
		CHIME_ID_BODYCTRL_1,//速度落锁设置为取消
		CHIME_ID_BODYCTRL_2,//速度落锁设置为激活
		CHIME_ID_BODYCTRL_3,//钥匙学习成功
		CHIME_ID_BODYCTRL_4,//驾驶员门解锁模式设置为取消
		CHIME_ID_BODYCTRL_5,//驾驶员门解锁模式设置为激活
		CHIME_ID_BODYCTRL_6,//天窗未关提示
		CHIME_ID_BODYCTRL_7,//钥匙在车内
		CHIME_ID_BODYCTRL_8,//未检测到钥匙报警
		CHIME_ID_BODYCTRL_9,//退出防盗激活状态提示保留
		CHIME_ID_BODYCTRL_A,//门未关闭锁
		CHIME_ID_BODYCTRL_B,//遥控电池电量低
		CHIME_ID_BODYCTRL_C,//副驾驶安全带未系
		CHIME_ID_BODYCTRL_D,//灯未关提示； 
		CHIME_ID_BODYCTRL_E,//主驾驶安全带未系； 
		CHIME_ID_BODYCTRL_F,//门未关提示；

		CHIME_ID_PROMPT_VEHICLE_POWERING,	//车辆未下电
		CHIME_ID_PROMPT_OVER_SPD_WARN,		//车速较快，请谨慎驾驶
		CHIME_ID_PROMPT_READY_ICON,			//整车进入可行驶状态(ready灯)
		CHIME_ID_PROMPT_BRAKE_CHANGE_GEAR,	//未踩制动换挡提醒
		CHIME_ID_PROMPT_AC_ON,				//空调开启
		CHIME_ID_PROMPT_AC_HIGH_ENERGY_STS,	//空调高能耗状态
		CHIME_ID_PROMPT_AC_LOW_POWER_STS,	//空调低能耗状态
		CHIME_ID_PROMPT_RESERVATION_AC,		//预约空调
		CHIME_ID_PROMPT_RESERVATION_CHARGE,	//预约充电
		CHIME_ID_PROMPT_VOICE_ON,			//语音播报开启
		CHIME_ID_PROMPT_VOICE_OFF,			//语音播报关闭
		CHIME_ID_PROMPT_BRAKE_START_GEAR,	//钥匙拧到START后再换挡
		CHIME_ID_PROMPT_ENGINE_FAIL_4S,		//换挡器故障
		CHIME_ID_PROMPT_PWS_OFF,			//行人警示系统已关闭
		CHIME_ID_PROMPT_START_BRAKE,		//启动采刹车提示

		CHIME_ID_TURN_OFF,				//转向灯-滴
		CHIME_ID_TURN_ON,			//右转向-嗒
		CHIME_ID_TIRE_ADJUST_LEFT,		//请向左打方向盘，调整轮胎
		CHIME_ID_TIRE_ADJUST_RIGHT,		//请向右打方向盘，调整轮胎
		//CHIME_ID_welcome,				//迎宾声音

		END
	};

}


#endif //!_CHIMEDATADEF_H_