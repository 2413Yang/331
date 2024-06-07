
#ifndef MSGINTERFACE__H__
#define MSGINTERFACE__H__

#include "DataDefine.h"
#include "WarnIdDefine.h"
#include "IPC/IPCCore.h"

namespace HmiIPC
{

	/**
	* UpdateDispVehicleSpeed
	*
	* @param  {int}                   : 车速值
	* @param  {bool}                    : 有效标志
	*/
	USER_DEFINED(void, UpdateDispVehicleSpeed, int, bool);
	/**
	* UpdateDispSOCValue
	*
	* @param  {int}						: 电池电量
	* @param  {bool}					: 低电量状态（0：正常，1：低电量）
	* @param  {bool}                    : 有效标志
	*/
	USER_DEFINED(void, UpdateDispSOCValue, int, bool, bool);
	/**
	* UpdateDispGear
	*
	* @param  {EmGearsValue}            : 挡位值
	*/
	USER_DEFINED(void, UpdateDispGear, EmGearsValue);
	/**
	* UpdateDispCurTime
	*
	* @param  {std::string}             : 当前时间
	*/
	USER_DEFINED(void, UpdateDispCurTime, std::string);
	/**
	* UpdateDispOutsideTemp
	*
	* @param  {int}                     : 室外温度
	* @param  {bool}                    : 有效标志
	*/
	USER_DEFINED(void, UpdateDispOutsideTemp, int, bool);
	/**
	* UpdateDispDrivingTime
	*
	* @param  {int}                     : 驾驶时间（时）
	* @param  {int}                     : 驾驶时间（分）
	* @param  {bool}                    : 有效标志
	*/
	USER_DEFINED(void, UpdateDispDrivingTime, int, int, bool);
	/**
	* UpdateDispRechargeMileage
	*
	* @param  {float}                   : 续航里程值
	* @param  {bool}                    : 有效标志
	*/
	USER_DEFINED(void, UpdateDispRechargeMileage, float, bool);
	USER_DEFINED(void, UpdateDispTripA, float);
	USER_DEFINED(void, UpdateDispTripB, float);
	USER_DEFINED(void, UpdateDispOdograph, int);
	USER_DEFINED(void, UpdateDispLamp, std::vector<StuTelltableLampState>);
	/**
	* UpdateDispPopWarn
	*
	* @param  {EmPopWarnID}            : 文字提示id
	* @param  {int}                    : 门开状态（bit0:左前门状态,bit1:右前门状态,bit2:左后门状态,bit3:右后门状态,bit4:后备箱状态,bit5:引擎盖状态,0:关闭,1:打开）
	*/
	USER_DEFINED(void, UpdateDispPopWarn, EmPopWarnID, int);
	USER_DEFINED(void, UpdateHistoryWarnList, std::vector<EmPopWarnID>);

	USER_DEFINED(void, UpdateDispCarTire, StuCarTire);
	/**
	* UpdateSysVersion
	*
	* @param  {std::string}            : 系统版本号
	* @param  {std::string}            : MCU版本号
	*/
	USER_DEFINED(void, UpdateSysVersion, std::string, std::string);

	USER_DEFINED(void, UpdateDispTheme, EmTheme);
	USER_DEFINED(void, UpdateDispKey, EmKey, EmKeyState);

	USER_DEFINED(void, SelfCheckState, int);

	/**
	* SystemUpdateMessaage
	*
	* @param  {EmSWState}             : 升级模式是否打开
	* @param  {std::string}           : 升级类型
	* @param  {int}                   : 倒计时时长
	* @param  {int}                   : 进度条进度
	* @param  {std::string}           : 错误信息
	*/
	USER_DEFINED(void, SystemUpdateMessaage, EmSWState, std::string, std::string, int);
	/**
	* SetCurrentClock            : 设置保存时间
	*
	* @param  {int}              : 年
	* @param  {int}              : 月
	* @param  {int}              : 日
	* @param  {int}              : 时
	* @param  {int}              : 分
	* @param  {int}              : 秒
	* @param  {int}              : 时间制式（12/24制式）
	*/
	USER_DEFINED(void, SetCurrentClock, int, int, int, int, int, int, int);
	USER_DEFINED(void, SyncCurrentClock, int, int, int, int, int, int, int);

	/**
	* SetCurrentUnit            : 设置保存单位制式
	*
	* @param  {EmFunctionItem}    : 功能项
	* @param  {int}               : 记忆\同步 的参数
	*/
	USER_DEFINED(void, SetMemoryItem, EmFunctionItem, int);
	USER_DEFINED(void, SyncMemoryItem, EmFunctionItem, int);
	/**
	* ResetFunctionItem
	*
	* @param  {EmFunctionItem}     : 重置\清零 对应功能项
	*/
	USER_DEFINED(void, ResetFunctionItem, EmFunctionItem);
	/**
	* UpdatePowerStatus
	*
	* @param  {EmPowerStatus}      : 电源档位值
	*/
	USER_DEFINED(void, UpdatePowerStatus, EmPowerStatus);

	/**
	* UpdateTurnLampSts
	*
	* @param  {int} 				：左转向灯 0x0:灭 0x1:亮
	* @param  {int}        			：右转向灯 0x0:灭 0x1:亮
	* @param  {int}           		: 报警灯 0x0:左右转向同时灭 0x1:左右转向同时亮 
	*/
	USER_DEFINED(void, UpdateTurnLampSts, int, int, int);

	/**
	* UpdateScreenStatus
	*
	* @param  {EmScreenStatus}      : 屏幕亮灭状态
	*/
	USER_DEFINED(void, UpdateScreenStatus, EmScreenStatus);

	/**
	* UpdateTime
	*
	* @param  int      : 小时
	* @param  int      : 分钟
	*/
	USER_DEFINED(void, UpdateTime, int, int, int);

	/**
	* SetPopWarnStatus
	*
	* @param  {int}                 :0x0 =无弹窗 0x1 = 有弹窗
	*/
	USER_DEFINED(void, SetPopWarnStatus, int);

	/**
	* UpdateDispSpeed
	*
	* @param  {int}         :kanzi界面显示速度
	*/
	USER_DEFINED(void, UpdateDispSpeed, int);

	/*************a301新增接口*************/
	/**
	* UpdateDispPowerValue
	*
	* @param  {int}						: 功率值
	* @param  {bool}                    : 有效标志
	*/
	USER_DEFINED(void, UpdateDispPowerValue, int, bool);
	/**
	* UpdateDispEnergyFlow
	*
	* @param  {int}						: 能量流状态
	*/
	USER_DEFINED(void, UpdateDispEnergyFlow, int);
	/**
	* UpdateDispEnergyRecycle
	*
	* @param  {int}						: 能量回收显示状态
	*/
	USER_DEFINED(void, UpdateDispEnergyRecycle, int);
	/**
	* UpdateInstEnergyConsum
	*
	* @param  {float}						: 瞬时能耗
	*/
	USER_DEFINED(void, UpdateInstEnergyConsum, float);
	
	/**
	* UpdateAverEnergyConsum
	*
	* @param  {float}						: 平均能耗
	*/
	USER_DEFINED(void, UpdateAverEnergyConsum, float);

    /**
    * UpdateRadarDetDist
    *
    * @param  {RADAR_DEVICE}                 : 雷达设备位置
    * @param  {RADAR_DET_DIST}               : 雷达设备距离状态
    * @param  {float}                        : 雷达设备监测距离
    */
    USER_DEFINED(void, UpdateRadarDetDist, RADAR_DEVICE, RADAR_DET_DIST, float);
	/**
    * UpdateAlarmVoice          : 声音提示
    * @param  {ALARM_VOICE}     : 声音ID
    */
   USER_DEFINED(void, UpdateAlarmVoice, ALARM_VOICE);
	/**
    * UpdateResetAck        : 清除ACK
    * @param  {RESET_MODE}      : 清除模块
	* @param  {MACK}             : ACK状态
    */
	USER_DEFINED(void, UpdateResetAck, RESET_MODE, MACK);

	/**
	* UpdateDispMotorSpeed
	*
	* @param  {float}						: 电机转速
	* @param  {bool}						: 有效值
	*/
	USER_DEFINED(void, UpdateDispMotorSpeed, float, bool);

	/**
	* UpdateIVINaviInfo
	*
	* @param  {bool}					: 导航状态（false：关闭导航，true：打开导航）
	* @param  {int}						: 全程剩余路程距离
	* @param  {int}						: 剩余小时
	* @param  {int}						: 剩余分钟
	* @param  {int}						: 方向
	* @param  {int}						: 下个路口距离
	* @param  {string}					: 当前路名
	*/
	//USER_DEFINED(void, UpdateIVINaviInfo, bool, int, int, int, int, int, std::string);
	/**
	* UpdateIVIMusicPlayInfo
	*
	* @param  {bool}					: 投屏状态（false：关闭，true：打开）
	* @param  {int}						: 播放状态（0：播放，1：暂停）
	* @param  {int}						: 音乐类型（0：本地音乐，1：蓝牙音乐，2：usb音乐，3：在线音乐）
	* @param  {string}					: 音乐标题
	* @param  {string}					: 专辑名
	* @param  {string}					: 歌手
	* @param  {string}					: 文件名
	* @param  {string}					: 当前时间
	* @param  {string}					: 总时间
	*/
	//USER_DEFINED(void, UpdateIVIMusicPlayInfo, bool, int, int, std::string, std::string, std::string, std::string, std::string, std::string);
	/**
	* UpdateIVIRadioPlayInfo
	*
	* @param  {bool}					: 投屏状态（false：关闭，true：打开）
	* @param  {int}						: 收音机类型（0：AM，1：FM，2：在线电台）
	* @param  {string}					: 频率值
	*/
	//USER_DEFINED(void, UpdateIVIRadioPlayInfo, bool, int, std::string);
	/**
	* UpdateChargeSubscribe
	*
	* @param  {int}						: 充电模式(0:交流 1:直流 2:无效)
	* @param  {int}						: 预约充电状态
	* @param  {int}						: 预约充电模式
	* @param  {int}						: 预约充电状态有效（0：有效，1：无效）
	* @param  {int}						: 预约充电时间有效（0：有效，1：无效）
	* @param  {int}						: 预约充电开始时间（年）
	* @param  {int}						: 预约充电开始时间（月）
	* @param  {int}						: 预约充电开始时间（日）
	* @param  {int}						: 预约充电开始时间（小时）
	* @param  {int}						: 预约充电开始时间（分钟）
	* @param  {int}						: 预约充电结束时间（年）
	* @param  {int}						: 预约充电结束时间（月）
	* @param  {int}						: 预约充电结束时间（日）
	* @param  {int}						: 预约充电结束时间（小时）
	* @param  {int}						: 预约充电结束时间（分钟）
	*/
	USER_DEFINED(void, UpdateChargeSubscribe, int, int, int, int, int, int, StuChargeSubscribe, StuChargeSubscribe);
	/**
	* UpdateCharging
	*
	* @param  {int}						: 充电模式(0:交流 1:直流 2:无效)
	* @param  {int}						: 充电状态
	* @param  {int}						: 充电小时
	* @param  {int}						: 充电分钟
	* @param  {int}						: 充电电压
	* @param  {int}						: 充电电流
	* @param  {int}						: 预约充电状态
	* @param  {int}						: 预约充电模式
	* @param  {int}						: 预约充电状态有效（0：有效，1：无效）
	* @param  {int}						: 预约充电时间有效（0：有效，1：无效）
	* @param  {StuChargeSubscribe}		: 开始日期
	* @param  {StuChargeSubscribe}		: 结束日期
	*/
	USER_DEFINED(void, UpdateCharging, int, int, int, int, int, int, int, int, int, int, StuChargeSubscribe, StuChargeSubscribe);
	/**
	* UpdateThemeColor
	*
	* @param  {int}						: 主题（0：经典主题，1：运动主题，2：科技主题）
	* @param  {int}						: 主题对应颜色
	*/
	USER_DEFINED(void, UpdateThemeColor, int, int);
	/**
	* SetThemeColor
	*
	* @param  {int}						: 主题（0：经典主题，1：运动主题，2：科技主题）
	* @param  {int}						: 主题对应颜色
	*/
	USER_DEFINED(void, SetThemeColor, int, int);
	/**
	* UpdateVoicePlay
	*
	* @param  {int}						: 语音播报开关(0:开，1：关)
	* @param  {int}						: 音量大小(0：高 1：中 2：低）
	*/
	USER_DEFINED(void, UpdateVoicePlay, int, int);
	/**
	* SetVoicePlay
	*
	* @param  {int}						: 语音播报开关(0:开，1：关)
	* @param  {int}						: 音量大小(0：高 1：中 2：低）
	*/
	USER_DEFINED(void, SetVoicePlay, int, int);
	/**
	* SetVoicePlay
	*
	* @param  {int}						: 0:长期行驶 1：小计里程
	* @param  {int}						: 0：不清除 1：清除
	*/
	USER_DEFINED(void, SetClearMileageInfo, int, int);

	/**
	* SetTCInfoIndex
	*
	* @param  {int}						: 下发记忆项索引
	*/
	USER_DEFINED(void, SetTCInfoIndex, int);
	/**
	* SetTCInfoIndex
	*
	* @param  {int}						: 更新记忆项索引
	*/
	USER_DEFINED(void, UpdateTCInfoIndex, int);
	/**
	* TransitIVIBrightness
	*
	* @param  {int}						: 背光控制值
	*/
	USER_DEFINED(void, TransitIVIBrightness, int);
    /**
	* CarDoorStatus
    * 熄火界面下的门开图片状态
	*
	* @param  {int}						: 门开状态（bit0:左前门状态,bit1:右前门状态,bit2:左后门状态,bit3:右后门状态,bit4:后备箱状态,bit5:引擎盖状态,0:关闭,1:打开）
	*/
	USER_DEFINED(void, CarDoorStatus, int);
	/**
	* ResetSocCommand
	*
	* @param  {int}						: 重启(1：下发重启)
	*/
	USER_DEFINED(void, TestResetCommand, int);
};

#endif /*MSGINTERFACE__H__*/