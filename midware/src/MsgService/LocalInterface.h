
#ifndef LOCALINTERFACE__H__
#define LOCALINTERFACE__H__
#include <map>
#include <string>
#include "IPC/IPCCore.h"
#include "hmi/WarnIdDefine.h"

// 本地接口，只在当前程序内部调用
namespace MsgLocal
{
    USER_DEFINED(void, NotifyBrightness, EmBrightness, int, std::string);

    USER_DEFINED(void, NotifyMcuDriveMode, EmDriveMode);
    USER_DEFINED(void, NotifyMcuBrightness, EmScreenType, int);

    USER_DEFINED(void, SyncBrightness, EmScreenType, int);

    USER_DEFINED(void, PushPopWarn, EmPopWarnID, EmSWState);
    USER_DEFINED(void, PushPopWarnList, std::map<EmPopWarnID, EmSWState>);
    /**
     * UpdateKey 
     * @param  {EmKey}      : 按键键值更新
     */
    USER_DEFINED(void, UpdateKey, EmKey);
    /**
     * PushLampWarnList 
     * @param  {std::vector<StuLampData>} : 通知lamp管理器最新的指示灯状态 
     */
    USER_DEFINED(void, PushLampWarnList, std::vector<std::pair<std::string, int>>);
    /**
     * NotifyMcuBacklightState 
     * ! 禁止其他模块直接调用
     * @param  {int} state : 通知MCU的背光状态值 
     */
    USER_DEFINED(void, NotifyMcuBacklightState, std::vector<int>);
    /**
     * navigatePage 
     * 
     * @param  {std::string} : 主页面ID
     * @param  {std::string} : 副页面ID
     */
    USER_DEFINED(void, navigatePage, std::string, std::string);
    /**
     * goBackPage 
     * 
     * @param  {int}         : 占位符，无意义
     */
    USER_DEFINED(void, gobackOldPage, int);
    /**
     * NotifyMcuCurrentPopWarn 
     * 
     * @param  {int}         : 当前弹窗的ID
     */
    USER_DEFINED(void, NotifyMcuCurrentPopWarn, int);

    /**
     * NotifyHMICurrentPopWarn 
     * 
     * @param  {EmPopWarnID}         : 当前弹窗的ID
     */
    USER_DEFINED(void, NotifyHMICurrentPopWarn, EmPopWarnID);
}

#endif /*LOCALINTERFACE__H__*/