

#ifndef WARNIDDEFINE__H__
#define WARNIDDEFINE__H__

#define LAMP_LEFT_TURN "LetfTurn"								//左转向指示灯
#define LAMP_RIGHT_TURN "RightTurn"								//右转向指示灯
#define LAMP_BRAKE_FLUID_LEVEL "BrakeSystemFault"				//制动系统故障指示灯   0
#define LAMP_BATTERY_FAULT "ChargingFault"						//12V蓄电池故障指示灯
#define LAMP_ABS_FAULT "ABSFault"								//ABS故障指示灯
#define LAMP_EPS_FAULT "EPSFault"								//EPS故障指示灯
#define LAMP_EPB "EPB"											//手刹工作指示灯
#define LAMP_HIGH_BEAM "HighBeam"								//远光灯开关状态		5
#define LAMP_LOW_BEAM "LowBeam"									//近光灯开关状态
#define LAMP_REAR_FOG "RearFog"									//后雾灯开关状态
#define LAMP_POSITION "Position"								//位置灯开关状态
#define LAMP_EPB_FAULT "EPBFalut"								//EPB故障指示灯
#define LAMP_AIR_BAG_FAULT "AirBagFault"						//安全气囊报警灯		10
#define LAMP_MAIN_SEATBELT_INDICATOR "MainSeatbeltIndicator"	//主驾安全带未系报警灯	
#define LAMP_TMPS_FAULT "TPMSFault"								//胎压系统报警灯
#define LAMP_READY_STATUS "Ready"								//READY状态指示灯
#define LAMP_CHARGECABLE_CONNECT "ChargCableConnectStatus"		//充电线连接状态指示灯
#define LAMP_CHARGE_STATUS "ChargingState"						//充电状态指示灯		15
#define LAMP_POWER_LIMIT "PowerLimit"							//功率限制报警灯		
#define LAMP_MAIN_ALARM "MainAlarm"								//主报警指示灯
#define LAMP_LOWCHARGE "LowCharge"								//低电荷状态报警灯
#define LAMP_POWER_BATTERY_FAULT "PowerBatteryFault"			//动力电池故障指示灯
#define LAMP_POWER_BATTERY_HIGHTEMP "PowerBatteryHighTemp"		//动力蓄电池高温报警灯	20
#define LAMP_DRIVE_MOTOR_FAULT "DriveMotorFault"				//驱动电机故障指示灯	
#define LAMP_SYSTEM_FAULT_YELLOW "SystemFault_Yellow"			//系统故障指示灯_黄灯
#define LAMP_SYSTEM_FAULT_RED "SystemFault_Red"					//系统故障指示灯_红灯
#define LAMP_DOOR_OPEN "DoorOpen"								//门开报警灯
#define LAMP_BATTERY_CHARGE_HEAT "BatteryChargheat"				//电池充电加热指示灯	25


#define LAMP_HANDLE_HEATING "Lamp.HeatGripState"  //手把加热指示
#define LAMP_SEAT_HEATING "Lamp.HeatCushionState" //座椅加热指示
#define LAMP_LOW_BATTERY "Lamp.BatteryState"      //电池低电量报警
#define LAMP_ALARM_PROMPT "Lamp.FaultState"       //故障提示报警
#define LAMP_LEFT_ALARM_STATUS "Lamp.LeftAlarmState"                 //左警灯状态
#define LAMP_RIGHT_ALARM_STATUS "Lamp.RightAlarmState"                //右警灯状态
#define LAMP_FRONT_ALARM_STATUS "Lamp.FrontAlarmState"                //前警灯状态
#define LAMP_AFTER_ALARM_STATUS "Lamp.BehindAlarmState"                //后警灯状态
#define LAMP_ALARM_STATUS "Lamp.TailAlarmState"                      //警灯状态
#define LAMP_BLUETOOTH_STATUS "Common.BtConnectState"      //蓝牙连接状态

// enum class EmPopWarnID
// {
//     NONE,
//     AIRBAG_FAULT,
//     HIGH_WATER_TEMP,
//     KEY_lOW_POWER,
//     OIL_SHORTAGE,
//     TIRE_PRESSURE_ALARM,
//     ABS_FAULT,
//     END,
// };

enum EmListStatus
{
    UN_SELECT,
    SELECT,
};


enum class EmPopWarnID
{
	NONE = 0,
	/*0x615 DATA0*/
	TIRE_MONITOR_RESET,		//请检查胎压监测系统
	TIRE_LEAK,              //轮胎漏气
	TIRE_PRESSURE_HIGH,     //轮胎压力过高
	TIRE_PRESSURE_LOW,      //轮胎压力过低
	TIRE_INFO,              //轮胎信息在行驶几分钟后显示
	TIRE_ADJUST_LEFT,       //请向左打方向盘，调整轮胎
	TIRE_ADJUST_RIGHT,      //请向右打方向盘，调整轮胎
	LOW_SPEED_WARN_MAN_OFF, //低速行人报警功能已关闭
	/*0x615 DATA1*/
	PULL_OFF_CHARER,        //驾驶前请先拔充电枪
	POWER_SYS_TEMP_HIGH,    //驱动系统过温，请减速行驶
	CHARGER_WAVE_REPLACE,   //电网波动，请更换充电地点
	BRAKE_BOOSTER_REDUCE,	//制动助力不足，请谨慎驾驶
	CHARGE_P_GEAR,			//请在P档下进行充电
	POWER_SUPPLY,           //请刷卡或连接电源
	BRAKE_ON_UNABLE_CHARGE, //无法充电，请拉起手刹
	ENGINE_FAIL_STOP,		//动力系统故障，请靠边停车
	CHARGER_WAVE_TIME_LONGER,//电网波动，充电时间延长
	GEAR_SYS_4S,            //换挡器故障，请联系4S店检查
	CHARGER_HAND_UNLOCK,    //充电枪解锁失败，请手动解锁
	CHARGER_UNLOCK_TIME_LONGER,//充电枪未锁止，充电时间延长
	BRAKE_CHANGE_GEAR,		//换挡时请踩下制动踏板
	CHARGER_UNLOCK_STOP,    //充电枪未锁止，充电停止
	BAT_VOL_LOW,            //12V蓄电池电压过低，请靠边停车
	LOW_BATTERY_CHARGE,			//动力电池电量低，请及时充电
	ENGINE_LIMIT_SLOW,		//动力限制，请减速慢行
	BRAKE_CHANGE_GEAR_1,		//换挡时请踩下制动踏板
	BRAKE_START_GEAR,		//请踩制动踏板并按start按钮后再换挡
	BRAKE_FAIL,				//手刹故障，驻车请注意
	CHARGING_NO_GEAR,		//当前车辆处于外接充电状态，无法切换挡位
	REDUCE_SPEED_SHIFT_GEAR,//车速太高，请减速后再换挡
	/*0x615 DATA2*/
	ENGINE_FAIL_4S,         //动力系统故障，请联系4S店检查
	VEHICLE_POWERING,       //车辆未下电
	/*0x615 DATA3*/
	PARKING_RAMP,           //驻车坡道过大，请更换驻车地点
	PARK_BRAKE_LACK_4S,     //手刹夹紧力不足，请联系4S店检查
	FASTEN_SEAT_BELT_EPB,		//请系好安全带
	HANDBRAKE_BRAKE,		//释放电子手刹时请踩制动
	/*0x615 DATA4*/
	DOOR_CLOSE_LOCK,        //请关好车门再按遥控闭锁
	LIGHT_OFF,              //请关闭车灯
	FASTEN_SEAT_BELT_BCM_1,		//请系好安全带
	FASTEN_SEAT_BELT_BCM_2,    //请系安全带
	/*0x615 DATA5*/
	KEY_DETECT_FAIL,        //未检测到钥匙
	KEY_IN_CAR_UNLOCK,		//钥匙在车内，车门无法闭锁
	KEY_ELEC_LOW,			//钥匙电量低
	STEER_LOCK_FAIL,        //转向锁止未解除
	STEER_LOCK_CHECK,       //请检查转向锁止系统
	THEFT_AUTH_FAIL,        //防盗认证失败
	CHANGE_GEAR_P,			//启动请将挡位切换至P挡
	START_BRAKE,			//启动请踩刹车
	/*0x615 DATA6*/
	POWER_DIST_FAIL_4S,     //电源分配故障，请联系4s店检查
	KEY_SYS_4S,				//无钥匙系统故障，请联系4S店检查
	/*0x615 DATA7*/
	DOOR_OPEN,              //车门打开
	LEFT_FRONT_DOOR_OPEN,	//左前车门打开
	LEFT_REAR_DOOR_OPEN,	//左后车门打开
	RIGHT_FRONT_DOOR_OPEN,	//右前车门打开
	RIGHT_REAR_DOOR_OPEN,	//右后车门打开
	CAR_FRONT_DOOR_OPEN,	//引擎盖打开
	CAR_TRUNK_DOOR_OPEN,	//后备箱打开
	GEAR_FAILURE,			//换挡器故障，请联系4S店检查

	END,
};

enum WarnPage
{
	Warn_None,
	Warn_CarDoor,
	Warn_Safety,
	Warn_SteeringWheel_Left,
	Warn_SteeringWheel_Right,
};

enum ALARM_VOICE {
	ALARM_VOICE_NONE = 0x0, // 关闭
	ALARM_VOICE_FOLLOW_HOME = 0x01, // 跟随回家开启提示
	ALARM_VOICE_SPEED_LOCK_CANCEL = 0x02, // 速度落锁设置为取消
	ALARM_VOICE_SPEED_LOCK_ACTIVE = 0x03, // 速度落锁设置为激活
	ALARM_VOICE_KEY_LEARN = 0x04, // 钥匙学习成功
	ALARM_VOICE_DRIVER_LOCK_CANCEL = 0x05, // 驾驶员门解锁模式设置为取消
	ALARM_VOICE_DRIVER_LOCK_ACTIVE = 0x06, // 驾驶员门解锁模式设置为激活
	// 0x07, // 保留
	ALARM_VOICE_LIGHT = 0x10, // 灯未关提示
	ALARM_VOICE_SKYLIGHT = 0x11, // 天窗未关提示
	ALARM_VOICE_KEY_LEFT_IN_CAR = 0x12, // 钥匙在车内
	ALARM_VOICE_KEY_DETECT_FAIL = 0x13, // 未检测到钥匙报警
	ALARM_VOICE_EXIT_THEFT = 0x14, // 退出防盗激活状态提示保留
	ALARM_VOICE_DOOR_LOCK = 0x15, // 门未关闭锁
	ALARM_VOICE_BAT_LOW_POWER = 0x16, // 遥控电池电量低
	ALARM_VOICE_SECOND_SAFETY_BELT = 0x17, // 副驾驶安全带未系
	// 0x18, // 保留
	ALARM_VOICE_MAIN_SAFETY_BELT = 0x19, // 主驾驶安全带未系
	// 0x1A, // 保留
	ALARM_VOICE_DOOR = 0x1B, // 门未关提示
};

#endif /*WARNIDDEFINE__H__*/
