#ifndef HUFRAME_H_
#define HUFRAME_H_

#include <stdarg.h>
#include <map>
#include <vector>
#include <initializer_list>
#include <string>
#include "HUFrameStruct.h"

#define HUFID(type, subtype) ((((type) & 0xFF) << 8) | ((subtype) & 0xFF))

class CHUFrame
{
public:
    enum {
        FID_HUStatus = HUFID(0x01, 0x01),
        FID_HUMediaStatus = HUFID(0x01, 0x02),
        FID_HURadioStatus = HUFID(0x01, 0x03),
        FID_HURadioCurStatus = HUFID(0x01, 0x04),
        FID_HUVoiceStatus = HUFID(0x01, 0x05),
        FID_HUPhoneInfo = HUFID(0x02, 0x01),
        FID_HUCallConnectInfo = HUFID(0x02, 0x2),
        FID_HUContactCount = HUFID(0x02, 0x03),
        FID_HUContactInfo = HUFID(0x02, 0x04),
        FID_HUCallContactCount = HUFID(0x02, 0x05),
        FID_HUCallHistory = HUFID(0x02, 0x06),
        FID_HUCallConnectInfoList  = HUFID(0x02, 0x08),
        FID_HUPlayListCount = HUFID(0x03, 0x01),
        FID_HUMediaInfo = HUFID(0x03, 0x02),
        FID_HUMediaPlayTime = HUFID(0x03, 0x03),
        FID_HURaidoChannelCount = HUFID(0x04, 0x01),
        FID_HURaidoChannelInfo = HUFID(0x04, 0x02),
        FID_HURaidoChannelOnlineCount = HUFID(0x04, 0x03),
        FID_HURaidoChannelOnlineList = HUFID(0x04, 0x04),
        FID_HUVersion = HUFID(0x05, 0x01),
        FID_HUIPVersion = HUFID(0x06, 0x01),
        FID_HUNavTurnInfo = HUFID(0x07, 0x01),
        FID_HUNavRemain = HUFID(0x07, 0x03),
        FID_HUNavRoad = HUFID(0x07, 0x04),
        FID_HUNavRoadConditionBarGraph = HUFID(0x07, 0x05),
        FID_HUNavRoadChange = HUFID(0x07, 0x06),
        FID_HUNavSpeedLimit = HUFID(0x07, 0x07),
        FID_HUNavGlobalMapRequest = HUFID(0x07, 0x08),
        FID_HUNavFeedbackScrrenOutputMode = HUFID(0x07, 0x09),
        FID_HUNavRoadArrive = HUFID(0x07, 0x0A),
        FID_HUNavRoadJam = HUFID(0x07, 0x0B),
        FID_HUNavAroundSearch = HUFID(0x07, 0x0C),
        FID_HUNavAlongSearch = HUFID(0x07, 0x0D),
        FID_HUNavTipsCancel = HUFID(0x07, 0x0E),
        FID_HUNavPark = HUFID(0x07, 0x0F),
        FID_HUNavOpenMeterRequest = HUFID(0x07, 0x10),
        FID_HUNavOilLowLevelAlert = HUFID(0x07, 0x11),
        FID_HUNavThemeStatus = HUFID(0x07, 0x12),
        FID_HUFileInfo = HUFID(0x08, 0x01),
        FID_HUFileData = HUFID(0x08, 0x02),
        FID_HUHeartbeatAck = HUFID(0x08, 0x03),
        FID_HUUpgradeRequest = HUFID(0x08, 0x04),
        FID_HULogRequest = HUFID(0x08, 0x05),
        FID_HUUpdateListFlag = HUFID(0x09, 0x01),
        FID_HUKeyMode = HUFID(0x0A, 0x01),
        FID_HUEventSlide = HUFID(0x0A, 0x02),
        FID_HUManualThemeSetting = HUFID(0x0A, 0x03),
        FID_HUDebugMode = HUFID(0x0A, 0x04),
        FID_HUBrightness = HUFID(0x0A, 0x08),
        FID_IPStatus = HUFID(0x11, 0x01),
        FID_IPContactCount = HUFID(0x12, 0x01),
        FID_IPContactInfo = HUFID(0x12, 0x02),
        FID_IPCallContactCount = HUFID(0x12, 0x03),
        FID_IPCallHistory = HUFID(0x12, 0x04),
        FID_IPPhoneCall = HUFID(0x12, 0x05),
        FID_IPPhoneAnswer = HUFID(0x12, 0x06),
        FID_IPPhoneHangup = HUFID(0x12, 0x07),
        FID_IPPhoneSwitch = HUFID(0x12, 0x08),
        FID_IPPhoneMute = HUFID(0x12, 0x09),
        FID_IPPhoneHandfree = HUFID(0x12, 0x0A),
        FID_IPPlayListCount = HUFID(0x13, 0x01),
        FID_IPMediaInfo = HUFID(0x13, 0x02),
        FID_IPMediaPlayback = HUFID(0x13, 0x03),
        FID_IPMediaPause = HUFID(0x13, 0x04),
        FID_IPMediaFF = HUFID(0x13, 0x05),
        FID_IPMediaFB = HUFID(0x13, 0x06),
        FID_IPMediaPrevious = HUFID(0x13, 0x07),
        FID_IPMediaNext = HUFID(0x13, 0x08),
        FID_IPMediaSource = HUFID(0x13, 0x09),
        FID_IPMediaCurInfo = HUFID(0x13, 0x0A),
        FID_IPRaidoChannelCount = HUFID(0x14, 0x01),
        FID_IPRaidoChannelInfo = HUFID(0x14, 0x02),
        FID_IPRaidoPlayback = HUFID(0x14, 0x03),
        FID_IPRaidoPause = HUFID(0x14, 0x04),
        FID_IPRaidoPrevious = HUFID(0x14, 0x05),
        FID_IPRaidoNext = HUFID(0x14, 0x06),
        FID_IPRaidoBandSwitch = HUFID(0x14, 0x07),
        FID_IPRaidoOnlinePlayback = HUFID(0x14, 0x08),
        FID_IPRaidoOnlinePause = HUFID(0x14, 0x09),
        FID_IPRaidoInfo = HUFID(0x14, 0x0A),
        FID_IPVersion = HUFID(0x15, 0x01),
        FID_IPHUVersion = HUFID(0x16, 0x01),
        FID_IPNavExit = HUFID(0x17, 0x01),
        FID_IPNavRoadJam = HUFID(0x17, 0x02),
        FID_IPNavAroundSearch = HUFID(0x17, 0x03),
        FID_IPNavAlongSearch = HUFID(0x17, 0x04),
        FID_IPNavPark = HUFID(0x17, 0x05),
        FID_IPNavMapOpen = HUFID(0x17, 0x06),
        FID_IPFileNext = HUFID(0x18, 0x01),
        FID_IPFileEnd = HUFID(0x18, 0x02),
        FID_IPUpgradeStatus = HUFID(0x18, 0x03),
        FID_IPHeartbeat = HUFID(0x18, 0x05),
        FID_IPUpgradeAccept = HUFID(0x18, 0x06),
        FID_IPThemeSwitchStatus = HUFID(0x1A, 0x01),
        FID_IPThemeSetMode = HUFID(0x1A, 0x02),
        FID_IPManualThemeSetting = HUFID(0x1A, 0x03),
    };

public:
    CHUFrame();
    virtual unsigned int getID(void);
    void updateFrame(std::string &frame);

protected:
    std::string mFrame;
};

#endif // HUFRAME_H_