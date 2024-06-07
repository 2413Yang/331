#ifndef Common__H__
#define Common__H__

#include <string>
#include <mutex>



#define PropertyBuilderByName(type, name, access_permission)\
    access_permission:\
        type m_##name;\
    public:\
    inline void set##name(type v) {\
		std::lock_guard<std::mutex> locker(mutex_);\
        m_##name = v;\
    }\
    inline type get##name() {\
		std::lock_guard<std::mutex> locker(mutex_);\
        return m_##name;\
    }\

class CommonData
{
private:
	
	mutable std::mutex mutex_;

public:
	static CommonData* getInstance()
	{
		static CommonData mInstance;
		return &mInstance;
	}


public:

	PropertyBuilderByName(int, Speed, private);
	PropertyBuilderByName(float, EngSpeed, private);
	PropertyBuilderByName(int, Fuel, private);
	PropertyBuilderByName(string, WaterTemp, private);
	PropertyBuilderByName(string, OilLifetime, private);
	PropertyBuilderByName(string, BatteryVoltage, private);

	PropertyBuilderByName(int, OverSpeed, private);
	PropertyBuilderByName(string, TripA, private);
	PropertyBuilderByName(string, TripB, private);
	PropertyBuilderByName(int, OilConsume, private);
	//PropertyBuilderByName(int, itemTime, private);
	PropertyBuilderByName(int, CoolantTemp, private);
	PropertyBuilderByName(string, OilTemp, private);
	PropertyBuilderByName(int, EngineSupercharging, private);
	PropertyBuilderByName(string, GearboxOilTemp, private);
	PropertyBuilderByName(bool, PopWarnExist, private); 

	PropertyBuilderByName(StuCarTire, TireStu, private);
	PropertyBuilderByName(int, MenuSelectCfg, private);

	PropertyBuilderByName(float, OilTrend, private);
};

enum MenuPageEnum
{
	//kanzi模板索引ID
	Menu_None,								//无菜单
	Menu_Tc_Main,							//常显主页面
	Menu_Tc_Sub_LongMileage,				//长期行驶
	Menu_Tc_Sub_TirpMileage,				//小计里程
	Menu_Tc_Sub_EnergyFlow,					//能量流
	Menu_Tc_Sub_MotorSpeed,					//电机转速
	Menu_Tc_Sub_InstanEnergyCons,			//瞬时能耗
	Menu_Tc_Sub_TireInfo,					//胎压信息
	Menu_Tc_Sub_Power,						//功率
	Menu_Tc_Sub_IntelligentScene,			//智能场景
	Menu_Tc_Third_Reset,                    //重置
	Menu_AlarmInfo_Main,					//报警主页面
	Menu_AlarmInfo_Sub,						//报警子页面
	Menu_Theme_Main,						//主题模式主页面
	Menu_Theme_Sub_Mode,					//主题切换二级页面
	Menu_Theme_Third_Color_Classic,			//主题切换三级页面经典模式颜色选择
	Menu_Theme_Third_Color_Technology,		//主题切换三级页面科技模式颜色选择
	Menu_Set_Main,							//设置主页面
	Menu_Set_Sub_Main,						//设置二级页面
	Menu_Set_Sub_VoicePlay,					//设置三级页面语音播报
	Menu_Set_Sub_VoiceSize,					//设置三级页面声音大小
	Menu_CallRecord_Main,					//通话记录主页面
	Menu_CallRecord_Sub,					//通话记录子页面
	Menu_Contact_Main,						//联系人主页面
	Menu_Contact_Sub,						//联系人子页面
	Menu_Media_MusicPlay,					//音乐播放页面
	Menu_Media_RadioPlay,					//收音机播放页面
	Menu_Media_List,						//多媒体列表
	Menu_Media_NoList,						//多媒体列表
	Menu_Media_Source,						//媒体源
	Menu_BTPhone_Incoming,					//来电页面
	Menu_BTPhone_Dial,						//拨打页面
	Menu_BTPhone_InCalling,					//通话中页面
	Menu_BTPhone_CallEnd,					//结束电话页面
	Menu_ChargeSubscribe,					//充电预约页面
	Menu_Charging,							//正在充电页面
	Menu_ChargeResult,						//充电结果页面（充电故障，充电结束）
};

enum StyleEnum
{
	Classic,
	Sport,
	Technology,
};

enum ClassicColorEnum
{
	mblue,
	mgreen,
	mpurple,
};

enum TechnologyColorEnum
{
	green,
	purple,
	golden,
};
#endif
