#ifndef _VIDEOPLAYERAPI_H_
#define _VIDEOPLAYERAPI_H_

#include "IPC/IPCCore.h"
#include <string>


enum class EnLaneColor
{
	green,
	purple,
	orange,
	red
};

//1.上电本程序自己先调用StartupAnimation_Start函数，开始播开机动画；
//2.msgService程序检测到ignOff 到ign On状态切换，也需发送StartupAnimation_Start事件；
USER_DEFINED(void, StartupAnimation_Start, std::string);
//两种情况下需要发送StartupAnimation_Stop事件
//1.msgService程序检查到ignOn到off时，需要发送StartupAnimation_Stop事件
//2.在ign on条件下，hmi.exe程序收到PLAYING_LASTFRAME状态，且hmi界面已经准备好第一帧显示画面，则发送StartupAnimation_Stop事件，因此开机动画界面
USER_DEFINED(void, StartupAnimation_Stop, int);

//投屏控制 true-开始投屏；false-停止投屏
USER_DEFINED(void, CastScreen_Ctrl, bool);

typedef enum animationSts
{
    PLAYING_NONE,// 当前没有播放动画
    PLAYING_FIRSTFRAME, //正在播放第一帧
    PLAYING_LASTFRAME,  //播放到最后一帧
}tstAnimationSts;

USER_DEFINED(void, StartupAnimation_PlaySts, tstAnimationSts);

//投屏状态反馈，false-无信号；true-正在投屏
USER_DEFINED(void, CastScreen_Status, bool);

USER_DEFINED(void, LaneAnimation_SetColor, EnLaneColor);
USER_DEFINED(void, LaneAnimation_Speed, int);

#endif//!_VIDEOPLAYERAPI_H_