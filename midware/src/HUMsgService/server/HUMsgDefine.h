#ifndef HUMSGDEFINE__H__
#define HUMSGDEFINE__H__

// HUMsgService(IVI消息解析服务) 与 HMI 交互消息定义

#include "IPC/IPCCore.h"
#include "DataDefine.h"

namespace HUHmiIPC
{
	enum IVIList 
	{
		IVI_Music_List,
		IVI_Radio_LIst,
	};

	enum BtList {
		BtList_Contact,     // 通讯录
		BtList_CallHistory, // 通话记录
	};

	enum BTPhonePage 
	{
		BTPhone_Incoming,//来电页面
		BTPhone_Dial,//拨打页面
		BTPhone_InCalling,//通话中页面
	};

	enum BTPhoneButton 
	{
		BTPhone_Answer,//接听
		BTPhone_CallUp,//挂断
		BTPhone_Sound,//麦克风
		BTPhone_Moblie,//扩音
	};

	enum BTPhoneControl 
	{
		BTPhone_Close,//关
		BTPhone_Open,//开
	};

	/**
	* UpdateIVISourceStatus:HU当前音源状态
	*
	* @param  {int}                   : 当前音源状态
	*/
	USER_DEFINED(void, UpdateIVISourceStatus, int);

    /**
	* UpdateIVINaviInfo
	*
	* @param  {bool}					: 导航状态（false：关闭导航(默认)，true：打开导航）
	* @param  {int}						: 全程剩余路程距离（默认值:0 单位:m）
	* @param  {int}						: 剩余小时（默认值:0）
	* @param  {int}						: 剩余分钟（默认值:0）
	* @param  {int}						: 方向（默认值:-1 0:直行 1:左前方 2:右前方 3:左转 4:右转 5:调头）
	* @param  {string}					: 下个路口距离（默认值:""）
	* @param  {string}					: 当前路名（默认值:""）
	* @param  {string}					: 下个路口路名（默认值:""）
	*/
	USER_DEFINED(void, UpdateIVINaviInfo, bool, int, int, int, int, std::string, std::string, std::string);

	/**
	* UpdateIVIMusicPlayInfo
	*
	* @param  {bool}					: 投屏状态（false：关闭(默认)，true：打开）
	* @param  {int}						: 播放状态（0：播放，1：暂停, 2:停止 -1:无效(默认)）
	* @param  {int}						: 音乐类型（0：本地音乐，1：蓝牙音乐，2：usb音乐，3：在线音乐, -1:无效(默认)）
	* @param  {string}					: 音乐标题（默认值:""）
	* @param  {string}					: 专辑名（默认值:""）
	* @param  {string}					: 歌手（默认值:""）
	* @param  {string}					: 文件名（默认值:""）
	* @param  {int}						: 当前播放歌曲索引值（0开始）
	* @param  {string}					: 当前时间（默认值:"00:00"）
	* @param  {string}					: 总时间（默认值:"00:00"）
	* @param  {int}					    : 时间进度百分比（默认值:0）
	*/
	USER_DEFINED(void, UpdateIVIMusicPlayInfo, bool, int, int, std::string, std::string, std::string, std::string, int, std::string, std::string, int);

	/**
	* MW->HMI：关闭多媒体
	* @param  {bool}					: 有效状态 1:关闭 0:不关闭
	*/
	USER_DEFINED(void, UpdateIVIMediaCLose, bool);
	/*
	* MW->HMI：音乐列表数据
	* @param  {int}						: 音乐类型（0：本地音乐，1：蓝牙音乐，2：usb音乐，3：在线音乐, -1:无效(默认)）
	* @param  {int}						: 当前播放歌曲的索引（0开始）
	* @param  {int}						: 页码
	* @param  {std::vector}				: 歌曲名称列表
	*/
	USER_DEFINED(void, UpdateIVIMusicPlayList, int, int, int, std::vector<std::string>);

	/*
	* MW->HMI：音乐设备状态
	* @param  {int}						: 音乐类型（0：本地音乐，1：蓝牙音乐，2：usb音乐，3：在线音乐, -1:无效(默认)）
	* @param  {bool}					: 有效状态 1:有效 0:无效
	*/
	USER_DEFINED(void, UpdateIVIMusicDevice, int, bool);

	/**
	* UpdateIVIRadioPlayInfo
	*
	* @param  {bool}					: 投屏状态（false：关闭(默认)，true：打开）
	* @param  {int}						: 收音机类型（0：AM，1：FM，2：在线电台 -1:无效(默认)）
	* @param  {string}					: 频率值（默认值:""）
	* @param  {string}					: 电台名字（默认值:""）
	*/
	USER_DEFINED(void, UpdateIVIRadioPlayInfo, bool, int, std::string, std::string);

	/**
	* UpdateIVIRadioList
	*
	* @param  {int}						: 收音机类型（0：AM，1：FM，2：在线电台 -1:无效(默认)）
	* @param  {int}						: 当前播放电台的索引（0开始）
	* @param  {string}					: 频率(本地)/电台名称(在线电台,混合)
	*/
	USER_DEFINED(void, UpdateIVIRadioList, int, int, std::vector<std::string>);

	/**
	* HMI->MW: 请求列表数据
	* UpdateIVIInfoList
	*
	* @param  {IVIList}					: 请求列表数据（IVI_Music_List：音乐列表，IVI_Radio_LIst：收音机列表）
	* @param  {int}						: 页码,从1开始
	*/
	USER_DEFINED(void, UpdateIVIInfoList, IVIList, int);

		/**
	* HMI->MW: 请求音源列表
	* UpdateIVIMediaSourceList
	*
	* @param  {int}						: 音源（0：本地音乐，1：蓝牙音乐，2：USB音乐，3：网络音乐，4：FM, 5:AM, 6:网络）
	*/
	USER_DEFINED(void, UpdateIVIMediaSourceList, int);

	/* HMI->MW
	* HMI点击列表播放音乐
	* @param  {int}						: 播放音乐index值
	*/
	USER_DEFINED(void, UpdateIVIInfoPlayIndex, int);

	/* HMI->MW
	* HMI点击列表播放收音机
	* @param  {int}						: 播放电台index值
	*/
	USER_DEFINED(void, UpdateIVIRadioPlayIndex, int);

    /**
    * UpdateIVIPhoneInfo
    *
    * @param  {int}                     : 电话状态（-1:no call(默认) 0:通话中 1:正在拨打 2:正在铃声 3:正在来电 4:第三方来电正在等待 5:通过响应和保留 6:停止）
    * @param  {string}                  : 联系人名字（"":默认）
	* @param  {string}                  : 联系人电话号码（"":默认）
	* @param  {int}                     : 通话媒介（-1:默认 0:电话通话 1:车机通话）
	* @param  {int}                     : 麦克风状态（-1:默认 0:静音 1:非静音）
	* @param  {string}                  : 通话时长（"00:00":默认）
    */
    USER_DEFINED(void, UpdateIVIPhoneInfo, int, std::string, std::string, int, int, std::string);

    /**
    * UpdateIVIBtInfo
    *
    * @param  {int}                     : 蓝牙状态（-1:无效(默认) 0:蓝牙未连接 1:蓝牙连接,联系人未同步完成 2:蓝牙连接,联系人同步完成）
    * @param  {string}                  : 电话名字（"":默认）
    */
	USER_DEFINED(void, UpdateIVIBtInfo, int, std::string);

	 /**
    * UpdateIVIBtPhoneControl
    *
    * @param  {BTPhonePage}             : 
    * @param  {BTPhoneButton}           : 
	* @param  {BTPhoneButton} 			:
    */
	USER_DEFINED(void, UpdateIVIBtPhoneControl, BTPhonePage, BTPhoneButton, BTPhoneControl);

	/*
	* MW->HMI：联系人列表数据
	* @param  {bool}					: 联系人同步状态
	* @param  {int}						: 当前联系人列表页码,从1开始,0为无效页码
	* @param  {std::vector}				: 联系人列表
	*/
	USER_DEFINED(void, UpdateIVIContactList, bool, int, std::vector<std::string>);

	/* HMI->MW
	* HMI向MW请求蓝牙列表
	* @param  {BtList}					: 蓝牙列表类型
	* @param  {int}						: 页码
	*/
	USER_DEFINED(void, UpdateIVIBtList, BtList, int);

	/* HMI->MW
	* HMI点击联系人拨打
	* @param  {int}						: 联系人拨打index值（从0开始）
	*/
	USER_DEFINED(void, UpdateIVIContactIndex, int);
	/*
	* MW->HMI：通话记录列表数据
	* @param  {bool}					: 通话记录同步状态
	* @param  {std::vector}				: 通话记录列表
	*/
	USER_DEFINED(void, UpdateIVICallRecordList, bool, int, std::vector<StuCallRecord>);
	/* HMI->MW
	* HMI点击通话记录拨打
	* @param  {int}						: 通话记录拨打index值（从0开始）
	*/
	USER_DEFINED(void, UpdateIVICallRecordIndex, int);
	/* MW->HMI
	* HU控制IP主题颜色
	* @param  {int}						: 主题（0：经典主题，1：运动主题，2：科技主题）
	* @param  {int}						: 主题对应颜色
	*/
	USER_DEFINED(void, UpdateIVIThemeColor, int, int);

	/* HU->MW(IP)->HMI
	* MODE 键
	* @param  {int}						:
	* 0 NONE(表示需要立即响应)     1 DOWN(按键按下)     2 UP(按键抬起)
	* 3 LONG_EVENT(按键按键如果按下超过 1秒钟没有抬起将发送长按事件)
	* 4 LONG_UP(按键如果已经发送过长按事件后抬起按键时将发送此状态)
	*/
	USER_DEFINED(void, UpdateIVIKeyMode, int);

	/* HU->MW(IP)->HMI
	* 背光控制
	* @param  {int}						:
	* 1~100背光下发
	*/
	USER_DEFINED(void, UpdateIVIBrightness, int);

	/* HU->MW(IP)->HMI
	* 蓝牙状态
	* @param  {int}						: 0(蓝牙未连接) 1:(蓝牙已连接,联系人未同步完成) 2(蓝牙已连接,联系人同步完成)
	*/
	USER_DEFINED(void, UpdateIVIBtStatus, int);
}

#endif