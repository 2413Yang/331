#ifndef HUFRAME_STRUCT_H_
#define HUFRAME_STRUCT_H_

#include <stdarg.h>
#include <map>
#include <vector>
#include <initializer_list>
#include <string>

#pragma pack(1)

// 帧结构体定义参考A301项目文档《A双屏互动数据协议V3.3》

// HU发送到IP的数据内容
// 1 状态类
// 1.1 HU状态信息
typedef struct _huf_HUStatus
{
    uint8_t btConnectStatus; // 蓝牙连接状态
    uint8_t btPhoneStatus; // 电话1状态
    uint8_t naviStatus; // 导航状态
    uint8_t naviAudioStatus; // 导航语音状态
    uint8_t sourceStatus; // 当前源状态
    uint8_t btPhoneStatus2; // 电话2状态
    uint8_t sourceHDD; // HDD源
    uint8_t sourceUSB; // USB源
    uint8_t sourceBT; // 蓝牙源
    uint8_t isAutoFullScreen; // 是否自动全屏
    uint8_t isNaviUI; // 地图是否在前台显示
    uint8_t themeSettingMode; // 主题设置方式
    uint8_t naviRenderStatus; // 导航渲染状态
    uint8_t videoLockStatus; // 视频锁定状态
    uint8_t linkStatus; // 链路状态
} HUF_HUStatus;

// 1.2 多媒体状态信息
typedef struct _huf_HUMediaStatus
{
    uint8_t mediaType; // 多媒体类型
    uint8_t playStatus; // 播放状态
    uint8_t repeatMode; // 重复模式
    uint8_t randomMode; // 随机模式
    struct {
        uint32_t index; // 位置
        uint32_t type; // 类型
        uint32_t id; // id
        uint8_t title[128]; // 音乐ID3 title
        uint8_t album[128]; // 专辑名
        uint8_t artist[128]; // 歌手
        uint8_t name[128]; // 文件名
    } currentMusic; // 当前音乐
    uint8_t albumPicData; // 专辑封面
} HUF_HUMediaStatus;

// 1.3 收音机信息
typedef struct _huf_HURadioStatus
{
    uint32_t freQuENT; // 当前频率
    uint8_t presetIndex; // 当前频点在预存台列表中的位置
    uint8_t highLightIndex; // 当前频点的高亮位置
    uint8_t currentBand; // 当前波段
    uint8_t tunerStatus; // 收音机状态
    uint8_t isstereo; // 是否立体声
    uint8_t isLocal; // 是否本地台
    uint8_t signalLength; //信号强度 0~3
    uint8_t extra; // 其他：
} HUF_HURadioStatus;

// 1.4 当前在线电台信息
typedef struct _huf_HURadioCurStatus
{
    uint32_t state; // 播放状态：
    uint32_t index; // 所在的位置
    uint32_t frequent; // 频率
    uint8_t radioName[128]; // 电台名字
} HUF_HURadioCurStatus;

// 1.5 语音状态
typedef struct _huf_HUVoiceStatus
{
    uint32_t state; // 0 开启
} HUF_HUVoiceStatus;

// 2 电话类
// 2.1 手机名称
typedef struct _huf_HUPhoneInfo
{
    uint8_t name[128]; // 电话名字
} HUF_HUPhoneInfo;

// 2.2 通话信息
typedef struct _huf_HUCallConnectInfo
{
    uint32_t contactId; // 联系人id
    uint8_t contactName[128]; // 联系人名字
    uint8_t number[32]; // 联系人电话号码
    uint32_t lineStatus; // 电话状态
    uint32_t handfree; // 0 电话通话, 1 车机通话
    uint32_t isMicrophoneMute; // 麦克风静音
    uint8_t timeStr[16]; // 通话时长
} HUF_HUCallConnectInfo;

// 2.3 通讯录联系人数量
typedef struct _huf_HUContactCount
{
    uint32_t count; // 联系人数量
} HUF_HUContactCount;

// 2.4 通讯录联系人信息
typedef struct _huf_HUContactInfo
{
    uint32_t start; // 开始位置
    uint32_t count; // 数量
    struct {
        uint32_t index; // 联系人顺序
        uint32_t count; // 电话数量
        uint8_t name[128]; // 名字
        uint8_t number1[32]; // 电话1
        uint8_t number2[32]; // 电话2
        uint8_t number3[32]; // 电话3
        uint8_t number4[32]; // 电话4
        uint8_t number5[32]; // 电话5
    } contacts[1];
} HUF_HUContactInfo;

// 2.5 通话记录联系人数量
typedef struct _huf_HUCallContactCount
{
    uint32_t count; // 通话记录数量
} HUF_HUCallContactCount;

// 2.6 通话记录信息
typedef struct _huf_HUCallHistory
{
    uint32_t start; // 开始位置
    uint32_t count; // 数量
    struct {
        uint32_t index; // 在列表中的位置
        uint32_t callType; // 通话类型
        uint8_t name[128]; // 名字
        uint8_t number[32]; // 电话号码
        uint8_t strTime[16]; // 通话时间
        uint32_t count; // 通话条数
    } calls[1];
} HUF_HUCallHistory;

// 2.7 废除

// 2.8 多个通话状态联系人
typedef struct _huf_HUCallConnectInfoList 
{
    uint32_t count; // 通话个数
    struct {
        uint32_t contactId; // 联系人id
        uint8_t contactName[128]; // 联系人名字
        uint8_t number[32]; // 联系人电话号码
        uint32_t lineStatus; // 电话状态
        uint32_t handfree; // 0 电话通话, 1 车机通话
        uint32_t isMicrophoneMute; // 麦克风静音
        uint8_t timeStr[8]; // 通话时长
    } mList[1];
} HUF_HUCallConnectInfoList ;

// 3 多媒体类
// 3.1 播放列表中歌曲数量
typedef struct _huf_HUPlayListCount
{
    uint32_t count; // 音乐个数
} HUF_HUPlayListCount;

// 3.2 播放列表中歌曲信息
typedef struct _huf_HUMediaInfo 
{
    uint32_t start; // 开始位置
    uint32_t count; // 条数
    struct {
        uint32_t index; // 位置
        uint32_t type; // 类型
        uint32_t id; // ID
        uint8_t title[128]; // 音乐ID3 title
        uint8_t album[128]; // 专辑名
        uint8_t artist[128]; // 歌手
        uint8_t name[128]; // 文件名
    } medias[1];
} HUF_HUMediaInfo;

// 3.3 播放时间
typedef struct _huf_HUMediaPlayTime 
{
    uint32_t id; // 音乐id
    uint32_t currentMs; // 当前时间
    uint32_t totalMs; // 总时间
} HUF_HUMediaPlayTime;

// 3.4 废除
// 3.5 废除

// 4 收音机类
// 4.1 收音机列表中频道数量
typedef struct _huf_HURaidoChannelCount 
{
    uint32_t count; // 收音机个数
} HUF_HURaidoChannelCount;

// 4.2 收音机频道信息
typedef struct _huf_HURaidoChannelInfo 
{
    uint32_t start; // 开始位置
    uint32_t count; // 个数
    struct {
        uint32_t index; // 所在列表中的位置
        uint32_t frequence; // 频率
        uint32_t band; // 波段
    } radios[1];
} HUF_HURaidoChannelInfo;

// 4.3 在线收音机的数量
typedef struct _huf_HURaidoChannelOnlineCount
{
    uint32_t count; // 在线收音机个数
} HUF_HURaidoChannelOnlineCount;

// 4.4 在线收音机列表
typedef struct _huf_HURaidoChannelOnlineList 
{
    uint32_t start; // 开始位置
    uint32_t count; // 个数
    struct {
        uint8_t dataid[8]; // 音频ID
        uint32_t radioType; // 音频类型
        uint32_t index; // 位置
        uint8_t radioName[128]; // 电台名称
        uint8_t radioDesc[128]; // 电台简介
    } radios[1];
} HUF_HURaidoChannelOnlineList;

// 5.1 HU版本信息
typedef struct _huf_HUVersion 
{
    uint8_t version[32]; // 版本号
    uint8_t date[32]; // 时间
} HUF_HUVersion;

// 6.1 HU请求IP版本信息
typedef struct _huf_HUIPVersion 
{
    uint32_t cmd; // 暂无意义
} HUF_HUIPVersion;

// 7 导航相关协议
// 7.1 转向信息
typedef struct _huf_HUNavTurnInfo 
{
    uint32_t id1; // 系统传给仪表 id
    uint32_t id2; // 需要传两个id的时候使用，默认值为0
    uint32_t message1; // 报文
    uint32_t message2 ; // 需要传两个message的时候使用，默认值为0
    uint8_t disInfo[128]; // 路口距离值 格式:距离值+单位,如 xx,m，距离和单位之间采用分隔符“英文逗号,”
} HUF_HUNavTurnInfo;

// 7.2 废除

// 7.3 导航全程剩余距离和时间
typedef struct _huf_HUNavRemain
{
    uint32_t remainDis; // 剩余的距离
    uint8_t remainTime[128]; // 剩余的时间
} HUF_HUNavRemain;

// 7.4 导航输出车道信息
typedef struct _huf_HUNavRoad
{
    uint8_t driveWay[1]; // 车道信息
} HUF_HUNavRoad;

// 7.5 导航输出路况柱状图信息
typedef struct _huf_HUNavRoadConditionBarGraph
{
    uint8_t tmcSegment[1]; // 路况柱状图信息数据
} HUF_HUNavRoadConditionBarGraph;

// 7.6 导航当前路名和下一路名
typedef struct _huf_HUNavRoadChange
{
    uint8_t curRoad[128]; // 当前道路名称
    uint8_t nextRoad[128]; // 下一道路名称
} HUF_HUNavRoadChange;

// 7.7 导航当前路名和下一路名
typedef struct _huf_HUNavSpeedLimit
{
    uint32_t cameraDis; // 距离最近的电子眼距离
    uint32_t cameraType; // 电子眼类型
    uint32_t cameraSpeed; // 电子眼限速度
    uint32_t cameraIndex; // 下一个将要路过的电子眼编号
    uint32_t limitSpeed; // 当前道路速度限制
} HUF_HUNavSpeedLimit;

// 7.8 全屏地图请求
typedef struct _huf_HUNavGlobalMapRequest
{
    uint32_t cmd; // 0 小屏, 1 大屏
} HUF_HUNavGlobalMapRequest;

// 7.9 导航反馈屏幕输出模式
typedef struct _huf_HUNavFeedbackScrrenOutputMode
{
    uint32_t mode; // 1 大屏模式,2 常显模式（地图和路口放大图）,3 非常显小图模式
} HUF_HUNavFeedbackScrrenOutputMode;

// 7.10 达到/已过路口
typedef struct _huf_HUNavRoadArrive
{
    uint32_t state; // 1 到达路口, 2 已过路口
} HUF_HUNavRoadArrive;

// 7.11 堵塞提醒
typedef struct _huf_HUNavRoadJam
{
    uint8_t msg[128]; // 堵塞提醒信息
} HUF_HUNavRoadJam;

// 7.12 周边搜索
typedef struct _huf_HUNavAroundSearch
{
    uint32_t count; // 数据条数
    struct {
        uint8_t distance[32]; // 距离
        uint8_t longitude[32]; // 经度
        uint8_t latitude[32]; // 纬度
        uint8_t entry_latitude[32]; // 到达点经度
        uint8_t entry_longitude[32]; // 到达点纬度
        uint8_t addr[128]; // 地址
        uint8_t name[128]; // 名字
    } mList[1];
} HUF_HUNavAroundSearch;

// 7.13 沿途搜索
typedef struct _huf_HUNavAlongSearch
{
    uint32_t count; // 数据条数
    struct {
        uint8_t poi_distance[32]; // 距离
        uint8_t poi_longitude[32]; // 经度
        uint8_t poi_latitude[32]; // 纬度
        uint8_t entry_latitude[32]; // 到达点经度
        uint8_t entry_longitude[32]; // 到达点纬度
        uint8_t poi_addr[128]; // 地址
        uint8_t poi_name[128]; // 名字
    } mList[1];
} HUF_HUNavAlongSearch;

// 7.14 仪表取消显示提示
typedef struct _huf_HUNavTipsCancel
{
    uint32_t cmd; // 暂无意义
} HUF_HUNavTipsCancel;

// 7.15 停车场列表信息
typedef struct _huf_HUNavPark
{
    uint32_t count; // 数据条数
    struct {
        uint32_t parkIndex; // 顺序
        uint32_t parkDistance; // 距离
        uint8_t parkName[128]; // 停车场名称
        uint32_t num_space_f; // 空闲数
        uint32_t parkPrice; // 价格
        uint8_t tag_category[32]; // 停车场类型
    } mList[1];
} HUF_HUNavPark;

// 7.16 地图请求打开仪表LVDS
typedef struct _huf_HUNavOpenMeterRequest
{
    uint32_t cmd; // 暂无意义
} HUF_HUNavOpenMeterRequest;

// 7.17 低油量提醒
typedef struct _huf_HUNavOilLowLevelAlert
{
    uint8_t msg[128]; // 提醒信息
} HUF_HUNavOilLowLevelAlert;

// 7.18 导航当前主题状态
typedef struct _huf_HUNavThemeStatus
{
    uint32_t cmd; // 1 经典模式, 2 运动模式, 3 科技模式
} HUF_HUNavThemeStatus;

// 8 文件相关协议
// 8.1 文件文件信息(端口10002发送)
typedef struct _huf_HUFileInfo
{
    uint8_t fileName [128]; // 文件名字
    uint8_t fileMd5 [128]; // 文件的MD5值
    uint32_t fileLength; // 文件长度
    uint32_t fileType; // 文件类型
} HUF_HUFileInfo;

// 8.2 文件数据(端口10002发送/接收,主机为服务端，仪表为客户端)
typedef struct _huf_HUFileData
{
    uint8_t fileData [1]; // 文件数据
} HUF_HUFileData;

// 8.3 主机对仪表心跳的反馈
typedef struct _huf_HUHeartbeatAck
{
    uint32_t cmd; // 1 暂无意义
} HUF_HUHeartbeatAck;

// 8.4 请求升级
typedef struct _huf_HUUpgradeRequest
{
    uint32_t cmd; // 暂无意义
} HUF_HUUpgradeRequest;

// 8.5 请求Log数据上传
typedef struct _huf_HULogRequest
{
    uint32_t cmd; // 暂无意义
} HUF_HULogRequest;

// 9 更新列表标志
// 9.1 更新列表标志
typedef struct _huf_HUUpdateListFlag
{
    uint32_t cmd;
} HUF_HUUpdateListFlag;

// 10 其他控制项
// 10.1 Mode键
typedef struct _huf_HUKeyMode
{
    uint32_t state;
} HUF_HUKeyMode;

// 10.2 滑动事件
typedef struct _huf_HUEventSlide
{
    uint32_t uiId;
    uint32_t eventType;
} HUF_HUEventSlide;

// 10.3 手动切换模式下主题显示设置
typedef struct _huf_HUManualThemeSetting
{
    uint32_t mode;
    uint32_t type;
} HUF_HUManualThemeSetting;

// 10.4 仪表进入调试模式
typedef struct _huf_HUDebugMode
{
    uint32_t cmd;
} HUF_HUDebugMode;

// 10.8 屏幕亮度调节
typedef struct _huf_HUBrightness
{
    uint32_t cmd;
} HUF_HUBrightness;

// IP 发送到HU的数据内容
// 17 状态类
// 17.1 IP状态
typedef struct _huf_IPStatus
{
    uint32_t count; // 心跳数
    uint32_t screenMode; // 1, 大屏模式 2, 常显模式（地图和路口放大图）3, 非常显小图模式
    uint8_t version[32] ; // 仪表的版本号
    uint32_t IP_DisplayMode; // 仪表显示模式
    // uint32_t reserve; // 心跳数
    uint32_t IP_AutoDisplayMode; // 自动切换模式下主题显示设置
    uint8_t videoLockStatus; // 视频锁定状态
    uint8_t linkStatus; // 链路状态
    uint32_t brightness; // 仪表亮度值
} HUF_IPStatus;

// 18 电话类
// 18.1 请求通讯录联系人数量
typedef struct _huf_IPContactCount
{
    uint32_t cmd; // 暂无意义
} HUF_IPContactCount;

// 18.2 请求通讯录联系人信息
typedef struct _huf_IPContactInfo
{
    uint32_t start; // 开始位置
    uint32_t count; // 数量
} HUF_IPContactInfo;

// 18.3
typedef struct _huf_IPCallContactCount
{
    uint32_t cmd; // 联系人数量
} HUF_IPCallContactCount;

// 18.4 请求通话记录信息
typedef struct _huf_IPCallHistory
{
    uint32_t start; // 开始位置
    uint32_t count; // 数量
} HUF_IPCallHistory;

// 18.5 拨打电话
typedef struct _huf_IPPhoneCall
{
    uint32_t index; // 位置
    uint8_t name[128]; // 名字
    uint32_t number[128]; // 号码
} HUF_IPPhoneCall;

// 18.6 接听电话
typedef struct _huf_IPPhoneAnswer
{
    uint32_t cmd;
} HUF_IPPhoneAnswer;

// 18.7 挂断电话
typedef struct _huf_IPPhoneHangup
{
    uint32_t cmd;
} HUF_IPPhoneHangup;

// 18.8 切换线路
typedef struct _huf_IPPhoneSwitch
{
    uint32_t cmd; // 暂无意义
} HUF_IPPhoneSwitch;

// 18.9 静音控制
typedef struct _huf_IPPhoneMute
{
    uint32_t cmd; // 1 静音, 2 解除静音
} HUF_IPPhoneMute;

// 18.10 免提控制
typedef struct _huf_IPPhoneHandfree
{
    uint32_t cmd; // 0 车机通话, 1 手机通话
} HUF_IPPhoneHandfree;

// 19 多媒体类
// 19.1 请求播放列表中歌曲数量
typedef struct _huf_IPPlayListCount
{
    uint32_t cmd; // 暂无意义
} HUF_IPPlayListCount;

// 19.2 请求播放列表中的歌曲信息
typedef struct _huf_IPMediaInfo
{
    uint32_t source; // 暂无意义
    uint32_t start; // 开始位置
    uint32_t count; // 请求个数
} HUF_IPMediaInfo;

// 19.3 播放歌曲
typedef struct _huf_IPMediaPlayback
{
    uint32_t index; // 位置
    uint32_t type; // 类型
    uint32_t id; // id
    uint8_t title[128]; // 音乐ID3 title
    uint8_t album[128]; // 专辑名
    uint8_t artist[128]; // 歌手
    uint8_t name[128]; // 文件名
} HUF_IPMediaPlayback;

// 19.4 暂停歌曲
typedef struct _huf_IPMediaPause
{
    uint32_t index; // 位置
    uint32_t type; // 类型
    uint32_t id; // id
    uint8_t title[128]; // 音乐ID3 title
    uint8_t album[128]; // 专辑名
    uint8_t artist[128]; // 歌手
    uint8_t name[128]; // 文件名
} HUF_IPMediaPause;

// 19.5 快进
typedef struct _huf_IPMediaFF
{
    uint32_t cmd; // 暂无意义
} HUF_IPMediaFF;

// 19.6 快进
typedef struct _huf_IPMediaFB
{
    uint32_t cmd; // 暂无意义
} HUF_IPMediaFB;

// 19.7 上一曲
typedef struct _huf_IPMediaPrevious
{
    uint32_t cmd; // 暂无意义
} HUF_IPMediaPrevious;

// 19.8 下一曲
typedef struct _huf_IPMediaNext
{
    uint32_t cmd; // 暂无意义
} HUF_IPMediaNext;

// 19.9 切源
typedef struct _huf_IPMediaSource
{
    uint32_t sourceType; // 10 HDD 47 蓝牙音乐 11 USB 0 FM 1 AM 7 在线电台 8 空源 2 待机
} HUF_IPMediaSource;

// 19.10 请求当前歌曲播放信息
typedef struct _huf_IPMediaCurInfo
{
    uint32_t cmd;
} HUF_IPMediaCurInfo;

// 20 收音机类
// 20.1 请求收音机列表中的频道数量
typedef struct _huf_IPRaidoChannelCount
{
    uint32_t cmd; // 0 本地电台 1 在线电台
} HUF_IPRaidoChannelCount;

// 20.2 请求收音机频道信息
typedef struct _huf_IPRaidoChannelInfo
{
    uint32_t source; // 电台音源 0:本地电台 1:在线电台 2:混合电台
    uint32_t start; // 开始位置
    uint32_t count; // 请求个数
} HUF_IPRaidoChannelInfo;

// 20.3 播放收音机
typedef struct _huf_IPRaidoPlayback
{
    uint32_t index; //位置
    uint32_t frequence; // 频率
    uint32_t band; // 波段
} HUF_IPRaidoPlayback;

// 20.4 暂停收音机
typedef struct _huf_IPRaidoPause
{
    uint32_t index; //位置
    uint32_t frequence; // 频率
    uint32_t band; // 波段
} HUF_IPRaidoPause;

// 20.5 上一频道
typedef struct _huf_IPRaidoPrevious
{
    uint32_t cmd; // 0 本地电台 1 在线电台
} HUF_IPRaidoPrevious;

// 20.6 下一频道
typedef struct _huf_IPRaidoNext
{
    uint32_t cmd; // 0 本地电台 1 在线电台
} HUF_IPRaidoNext;

// 20.7 切换波段
typedef struct _huf_IPRaidoBandSwitch
{
    uint32_t cmd; // 暂无意义
} HUF_IPRaidoBandSwitch;

// 20.8 播放在线电台
typedef struct _huf_IPRaidoOnlinePlayback
{
    uint8_t dataid[8]; // 音频ID
    uint32_t radioType; // 音频类型
    uint32_t index; // 位置
    uint8_t radioName[128]; // 电台名称
    uint8_t radioDesc[128]; // 电台简介
} HUF_IPRaidoOnlinePlayback;

// 20.9 暂停在线电台
typedef struct _huf_IPRaidoOnlinePause
{
    uint32_t dataid; // 音频ID
    uint32_t radioType; // 音频类型
    uint32_t index; // 位置
    uint8_t radioName[128]; // 电台名称
    uint8_t radioDesc[128]; // 电台简介
} HUF_IPRaidoOnlinePause;

// 20.10 请求电台信息
typedef struct _huf_IPRaidoInfo
{
    uint32_t cmd; // 暂无意义
} HUF_IPRaidoInfo;

// 21
// 21.1 IP版本信息
typedef struct _huf_IPVersion
{
    uint8_t version[32]; // 版本信息
    uint8_t date[32]; // 日期
} HUF_IPVersion;

// 22
// 22.1 IP请求HU的版本信息
typedef struct _huf_IPHUVersion
{
    uint32_t cmd; // 暂无意义
} HUF_IPHUVersion;

// 23 导航相关
// 23.1 请求退出导航
typedef struct _huf_IPNavExit
{
    uint32_t cmd; // 暂无意义
} HUF_IPNavExit;

// 23.2 堵塞提醒的反馈
typedef struct _huf_IPNavRoadJam
{
    uint32_t cmd; // 1 确定 0 取消
} HUF_IPNavRoadJam;

// 23.3 周边搜索确认
typedef struct _huf_IPNavAroundSearch
{
    uint8_t distance[32]; // 距离
    uint8_t longitude[32]; // 经度
    uint8_t latitude[32]; // 纬度
    uint8_t entry_latitude[32]; // 到达点经度
    uint8_t entry_longitude[32]; // 到达点纬度
    uint8_t addr[128]; // 地址
    uint8_t name[128]; // 名字
} HUF_IPNavAroundSearch;

// 23.4 沿途搜索确认
typedef struct _huf_IPNavAlongSearch
{
    uint8_t poi_distance[32]; // 距离
    uint8_t poi_longitude[32]; // 经度
    uint8_t poi_latitude[32]; // 纬度
    uint8_t entry_latitude[32]; // 到达点经度
    uint8_t entry_longitude[32]; // 到达点纬度
    uint8_t poi_addr[128]; // 地址
    uint8_t poi_name[128]; // 名字
} HUF_IPNavAlongSearch;

// 23.5 停车场确认
typedef struct _huf_IPNavPark
{
    uint32_t parkIndex; // 顺序
    uint32_t parkDistance; // 距离
    uint8_t parkName[128]; // 停车场名称
    uint32_t num_space_f; // 空闲数
    uint32_t parkPrice; // 价格
    uint8_t tag_category[32]; // 停车场类型
} HUF_IPNavPark;

// 23.6 仪表请求打开地图
typedef struct _huf_IPNavMapOpen
{
    uint32_t cmd; // 暂无意义
} HUF_IPNavMapOpen;

// 24 IP到HU文件端口
// 24.1 IP请求HU发送升级下一帧数据
typedef struct _huf_IPFileNext
{
    uint32_t cmd; // 暂无意义
} HUF_IPFileNext;

// 24.2 文件发送完成
typedef struct _huf_IPFileEnd
{
    uint32_t cmd; // 暂无意义
} HUF_IPFileEnd;

// 24.3 升级的反馈
enum {
    IPUpgradeStatus_AcceptDone = 1,
    IPUpgradeStatus_AcceptFaile = 2,
    IPUpgradeStatus_Break = 3,
    IPUpgradeStatus_UpgradeDone = 4,
    IPUpgradeStatus_UpgradeFail = 5,
};

typedef struct _huf_IPUpgradeStatus
{
    uint32_t cmd; // 1 接受完成 2 接受失败 3 中断
} HUF_IPUpgradeStatus;

// 24.4 心跳包
typedef struct _huf_IPHeartbeat
{
    uint32_t cmd; // 暂无意义
} HUF_IPHeartbeat;

// 24.5 仪表允许升级反馈
typedef struct _huf_IPUpgradeAccept
{
    uint32_t cmd; // 1：允许升级 0：不允许升级
} HUF_IPUpgradeAccept;

// 26 主题切换
// 26.1 主题切换响应状态
typedef struct _huf_IPThemeSwitchStatus
{
    uint32_t status;
} HUF_IPThemeSwitchStatus;

// 26.2 仪表主题设置方式
typedef struct _huf_IPThemeSetMode
{
    uint32_t mode; // 仪表主题设置方式
} HUF_IPThemeSetMode;

// 26.3 手动切换模式下仪表主题显示设置
typedef struct _huf_IPManualThemeSetting
{
    uint32_t mode; // 手动切换模式下仪表主题显示设置
} HUF_IPManualThemeSetting;

typedef union _huf_data
{
    // HU 发送到 IP
    HUF_HUStatus HUStatus;
    HUF_HUMediaStatus HUMediaStatus;
    HUF_HURadioStatus HURadioStatus;
    HUF_HURadioCurStatus HURadioCurStatus;
    HUF_HUVoiceStatus HUVoiceStatus;
    HUF_HUPhoneInfo HUPhoneInfo;
    HUF_HUCallConnectInfo HUCallConnectInfo;
    HUF_HUContactCount HUContactCount;
    HUF_HUContactInfo HUContactInfo;
    HUF_HUCallContactCount HUCallContactCount;
    HUF_HUCallHistory HUCallHistory;
    HUF_HUCallConnectInfoList HUCallConnectInfoList;
    HUF_HUPlayListCount HUPlayListCount;
    HUF_HUMediaInfo HUMediaInfo;
    HUF_HUMediaPlayTime HUMediaPlayTime;
    HUF_HURaidoChannelCount HURaidoChannelCount;
    HUF_HURaidoChannelInfo HURaidoChannelInfo;
    HUF_HURaidoChannelOnlineCount HURaidoChannelOnlineCount;
    HUF_HURaidoChannelOnlineList HURaidoChannelOnlineList;
    HUF_HUVersion HUVersion;
    HUF_HUIPVersion HUIPVersion;
    HUF_HUNavTurnInfo HUNavTurnInfo;
    HUF_HUNavRemain HUNavRemain;
    HUF_HUNavRoad HUNavRoad;
    HUF_HUNavRoadConditionBarGraph HUNavRoadConditionBarGraph;
    HUF_HUNavRoadChange HUNavRoadChange;
    HUF_HUNavSpeedLimit HUNavSpeedLimit;
    HUF_HUNavGlobalMapRequest HUNavGlobalMapRequest;
    HUF_HUNavFeedbackScrrenOutputMode HUNavFeedbackScrrenOutputMode;
    HUF_HUNavRoadArrive HUNavRoadArrive;
    HUF_HUNavRoadJam HUNavRoadJam;
    HUF_HUNavAroundSearch HUNavAroundSearch;
    HUF_HUNavAlongSearch HUNavAlongSearch;
    HUF_HUNavTipsCancel HUNavTipsCancel;
    HUF_HUNavPark HUNavPark;
    HUF_HUNavOpenMeterRequest HUNavOpenMeterRequest;
    HUF_HUNavOilLowLevelAlert HUNavOilLowLevelAlert;
    HUF_HUNavThemeStatus HUNavThemeStatus;
    HUF_HUFileInfo HUFileInfo;
    HUF_HUFileData HUFileData;
    HUF_HUHeartbeatAck HUHeartbeatAck;
    HUF_HUUpgradeRequest HUUpgradeRequest;
    HUF_HULogRequest HULogRequest;
    HUF_HUUpdateListFlag HUUpdateListFlag;
    HUF_HUKeyMode HUKeyMode;
    HUF_HUEventSlide HUEventSlide;
    HUF_HUManualThemeSetting HUManualThemeSetting;
    HUF_HUDebugMode HUDebugMode;
    HUF_HUBrightness HUBrightness;

    // IP 发送到 HM
    HUF_IPStatus IPStatus;
    HUF_IPContactCount IPContactCount;
    HUF_IPContactInfo IPContactInfo;
    HUF_IPCallContactCount IPCallContactCount;
    HUF_IPCallHistory IPCallHistory;
    HUF_IPPhoneCall IPPhoneCall;
    HUF_IPPhoneAnswer IPPhoneAnswer;
    HUF_IPPhoneHangup IPPhoneHangup;
    HUF_IPPhoneSwitch IPPhoneSwitch;
    HUF_IPPhoneMute IPPhoneMute;
    HUF_IPPhoneHandfree IPPhoneHandfree;
    HUF_IPPlayListCount IPPlayListCount;
    HUF_IPMediaInfo IPMediaInfo;
    HUF_IPMediaPlayback IPMediaPlayback;
    HUF_IPMediaPause IPMediaPause;
    HUF_IPMediaFF IPMediaFF;
    HUF_IPMediaFB IPMediaFB;
    HUF_IPMediaPrevious IPMediaPrevious;
    HUF_IPMediaNext IPMediaNext;
    HUF_IPMediaSource IPMediaSource;
    HUF_IPMediaCurInfo IPMediaCurInfo;
    HUF_IPRaidoChannelCount IPRaidoChannelCount;
    HUF_IPRaidoChannelInfo IPRaidoChannelInfo;
    HUF_IPRaidoPlayback IPRaidoPlayback;
    HUF_IPRaidoPause IPRaidoPause;
    HUF_IPRaidoPrevious IPRaidoPrevious;
    HUF_IPRaidoNext IPRaidoNext;
    HUF_IPRaidoBandSwitch IPRaidoBandSwitch;
    HUF_IPRaidoOnlinePlayback IPRaidoOnlinePlayback;
    HUF_IPRaidoOnlinePause IPRaidoOnlinePause;
    HUF_IPRaidoInfo IPRaidoInfo;
    HUF_IPVersion IPVersion;
    HUF_IPHUVersion IPHUVersion;
    HUF_IPNavExit IPNavExit;
    HUF_IPNavRoadJam IPNavRoadJam;
    HUF_IPNavAroundSearch IPNavAroundSearch;
    HUF_IPNavAlongSearch IPNavAlongSearch;
    HUF_IPNavPark IPNavPark;
    HUF_IPNavMapOpen IPNavMapOpen;
    HUF_IPFileNext IPFileNext;
    HUF_IPFileEnd IPFileEnd;
    HUF_IPUpgradeStatus IPUpgradeStatus;
    HUF_IPHeartbeat IPHeartbeat;
    HUF_IPUpgradeAccept IPUpgradeAccept;
    HUF_IPThemeSwitchStatus IPThemeSwitchStatus;
    HUF_IPThemeSetMode IPThemeSetMode;
    HUF_IPManualThemeSetting IPManualThemeSetting;
} HUFData;

typedef struct _huf_head
{
    uint8_t tag[4];
    uint32_t type;
    uint32_t sub_type;
    uint32_t sub_type_1;
    uint32_t len;
} HUFHead;

typedef struct _huf_frame_struct
{
    HUFHead head;
    HUFData data;
} HUFrameStruct;

#pragma pack()

#endif // HUFRAME_STRUCT_H_