
#ifndef WATCHINTERFACE__H__
#define WATCHINTERFACE__H__

#include "IPC/IPCCore.h"
#include "mlog.h"

namespace WatchIpc
{
    // With the MCU heartbeat mechanism
    //
    //    1s(timeout:2s)
    // ARM   ------->   MCU
    USER_DEFINED(void, NotifyMcuHeartbeatPacket, int);

    /**
     * HeartbeatStart   开启心跳监测
     * 
     * @param  {std::string}  : 开启心跳的实体名称
     */
    USER_DEFINED(void, HeartbeatStart, std::string);

    /**
     * HeartbeatSignal   心跳信号
     * 
     * @param  {std::string}  : 发出心跳信号的实体名称
	 * @param  {int}  			: 心跳计数累加
     */
    USER_DEFINED(void, HeartbeatSignal, std::string, int);
	/**
     * HeartbeatSignal   心跳信号
     * 
     * @param  {std::string}	: 发出心跳信号的实体名称
	 * @param  {int}  			: 心跳计数累加
     */
    USER_DEFINED(void, HeartbeatSignal_Resp, std::string, int);

    /**
     * HeartbeatStop   停止心跳监测
     * 
     * @param  {std::string}  : 停止心跳的实体名称
     */
    USER_DEFINED(void, HeartbeatStop, std::string);

    /**
     * LogChannelControl   控制log输出的通道
     * 
     * @param  {std::string}  : 实体名称
     * @param  {bool}  : 通道选择，ture-ttyS2, false-xlog
     */
    USER_DEFINED(void, LogChannelControl, std::string, bool);
	/**
     * LogLevelControl   控制log输出级别
     * 
     * @param  {std::string}  : 实体名称
     * @param  {LOG_LEV_EN}  : log等级
     */
    USER_DEFINED(void, LogLevelControl, std::string, LOG_LEV_EN);
	
	/**
     * NotifyMcuReboot   通知mcu重启arm
	 * @param  {int}  : 无
     * 
     */
    USER_DEFINED(void, NotifyMcuReboot, int);
}

#endif /*WATCHINTERFACE__H__*/