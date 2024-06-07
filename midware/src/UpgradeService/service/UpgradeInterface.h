
#ifndef UPGRADEINTERFACE__H__
#define UPGRADEINTERFACE__H__
#include "IPC/IPCCore.h"

namespace upgrade
{
    /**
	* NotifyMcuIntoUpgradeMode
	*
	* @param  {int}                   : 发送给MsgService，通知mcu断电重启
	*/
    USER_DEFINED(void, NotifyMcuIntoUpgradeMode, int);

    /**
	* upgrade_IVICtrl
    * MsgService转发IVI的ota控制信息；
	*
	* @param  {int}                   : 0-无效值；1-请求ota升级（切换U盘模式）；2-ota包防止完毕；3-升级失败
	*/
    USER_DEFINED(void, upgrade_IVICtrl, int);

    /**
	* upgrade_ResponseIVICtrl
    * 反馈给MsgService OTA 升级状态
	*
	* @param  {int}                   : 0-无效值；1-切换完成（切换U盘模式）；2-升级完成；3-升级失败
	*/
    USER_DEFINED(void, upgrade_ResponseIVICtrl, int);

    /**
	*  Upgrade_clusterVersion
    * MsgService发送版本信息
	* @param  {std::string}                 : arm版本号
    * @param  {std::string}                 : mcu版本号
	*/
    USER_DEFINED(void, Upgrade_clusterVersion, std::string, std::string);
}

#endif /*UPGRADEINTERFACE__H__*/