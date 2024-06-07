#include <string>
#include <fstream>
#include "HUMsgService.h"
#include "HUMsgDefine.h"
#include "LogServiceApi.h"
#include "mylogCtrl.h"
#include "DoShellCmd.hpp"

#define BIND(func) [&]( const fsm::args &args ) { func(args); }
#define GETDATA(fname) const HUF_##fname *pData = &((const HUFData *)args[0].data())->fname
#define HEX2STR(h) ((h>=0x30 && h<=0x39) ? std::to_string(h-0x30).data() : "")

void CHUMsgService::initFSM(void)
{
    mFSM.on(statIdle, CHUFrame::FID_HUStatus) = BIND(HUStatus);
    mFSM.on(statIdle, CHUFrame::FID_HUMediaStatus) = BIND(HUMediaStatus);
    mFSM.on(statIdle, CHUFrame::FID_HURadioStatus) = BIND(HURadioStatus);
    mFSM.on(statIdle, CHUFrame::FID_HURadioCurStatus) = BIND(HURadioCurStatus);
    mFSM.on(statIdle, CHUFrame::FID_HUVoiceStatus) = BIND(HUVoiceStatus);
    mFSM.on(statIdle, CHUFrame::FID_HUPhoneInfo) = BIND(HUPhoneInfo);
    mFSM.on(statIdle, CHUFrame::FID_HUCallConnectInfo) = BIND(HUCallConnectInfo);
    mFSM.on(statIdle, CHUFrame::FID_HUContactCount) = BIND(HUContactCount);
    mFSM.on(statIdle, CHUFrame::FID_HUContactInfo) = BIND(HUContactInfo);
    mFSM.on(statIdle, CHUFrame::FID_HUCallContactCount) = BIND(HUCallContactCount);
    mFSM.on(statIdle, CHUFrame::FID_HUCallHistory) = BIND(HUCallHistory);
    mFSM.on(statIdle, CHUFrame::FID_HUCallConnectInfoList ) = BIND(HUCallConnectInfoList );
    mFSM.on(statIdle, CHUFrame::FID_HUPlayListCount) = BIND(HUPlayListCount);
    mFSM.on(statIdle, CHUFrame::FID_HUMediaInfo) = BIND(HUMediaInfo);
    mFSM.on(statIdle, CHUFrame::FID_HUMediaPlayTime) = BIND(HUMediaPlayTime);
    mFSM.on(statIdle, CHUFrame::FID_HURaidoChannelCount) = BIND(HURaidoChannelCount);
    mFSM.on(statIdle, CHUFrame::FID_HURaidoChannelInfo) = BIND(HURaidoChannelInfo);
    mFSM.on(statIdle, CHUFrame::FID_HURaidoChannelOnlineCount) = BIND(HURaidoChannelOnlineCount);
    mFSM.on(statIdle, CHUFrame::FID_HURaidoChannelOnlineList) = BIND(HURaidoChannelOnlineList);
    mFSM.on(statIdle, CHUFrame::FID_HUVersion) = BIND(HUVersion);
    mFSM.on(statIdle, CHUFrame::FID_HUIPVersion) = BIND(HUIPVersion);
    mFSM.on(statIdle, CHUFrame::FID_HUNavTurnInfo) = BIND(HUNavTurnInfo);
    mFSM.on(statIdle, CHUFrame::FID_HUNavRemain) = BIND(HUNavRemain);
    mFSM.on(statIdle, CHUFrame::FID_HUNavRoad) = BIND(HUNavRoad);
    mFSM.on(statIdle, CHUFrame::FID_HUNavRoadConditionBarGraph) = BIND(HUNavRoadConditionBarGraph);
    mFSM.on(statIdle, CHUFrame::FID_HUNavRoadChange) = BIND(HUNavRoadChange);
    mFSM.on(statIdle, CHUFrame::FID_HUNavSpeedLimit) = BIND(HUNavSpeedLimit);
    mFSM.on(statIdle, CHUFrame::FID_HUNavGlobalMapRequest) = BIND(HUNavGlobalMapRequest);
    mFSM.on(statIdle, CHUFrame::FID_HUNavFeedbackScrrenOutputMode) = BIND(HUNavFeedbackScrrenOutputMode);
    mFSM.on(statIdle, CHUFrame::FID_HUNavRoadArrive) = BIND(HUNavRoadArrive);
    mFSM.on(statIdle, CHUFrame::FID_HUNavRoadJam) = BIND(HUNavRoadJam);
    mFSM.on(statIdle, CHUFrame::FID_HUNavAroundSearch) = BIND(HUNavAroundSearch);
    mFSM.on(statIdle, CHUFrame::FID_HUNavAlongSearch) = BIND(HUNavAlongSearch);
    mFSM.on(statIdle, CHUFrame::FID_HUNavTipsCancel) = BIND(HUNavTipsCancel);
    mFSM.on(statIdle, CHUFrame::FID_HUNavPark) = BIND(HUNavPark);
    mFSM.on(statIdle, CHUFrame::FID_HUNavOpenMeterRequest) = BIND(HUNavOpenMeterRequest);
    mFSM.on(statIdle, CHUFrame::FID_HUNavOilLowLevelAlert) = BIND(HUNavOilLowLevelAlert);
    mFSM.on(statIdle, CHUFrame::FID_HUNavThemeStatus) = BIND(HUNavThemeStatus);
    mFSM.on(statIdle, CHUFrame::FID_HUFileInfo) = BIND(HUFileInfo);
    mFSM.on(statIdle, CHUFrame::FID_HUFileData) = BIND(HUFileData);
    mFSM.on(statIdle, CHUFrame::FID_HUHeartbeatAck) = BIND(HUHeartbeatAck);
    mFSM.on(statIdle, CHUFrame::FID_HUUpgradeRequest) = BIND(HUUpgradeRequest);
    mFSM.on(statIdle, CHUFrame::FID_HULogRequest) = BIND(HULogRequest);
    mFSM.on(statIdle, CHUFrame::FID_HUUpdateListFlag) = BIND(HUUpdateListFlag);
    mFSM.on(statIdle, CHUFrame::FID_HUKeyMode) = BIND(HUKeyMode);
    mFSM.on(statIdle, CHUFrame::FID_HUEventSlide) = BIND(HUEventSlide);
    mFSM.on(statIdle, CHUFrame::FID_HUManualThemeSetting) = BIND(HUManualThemeSetting);
    mFSM.on(statIdle, CHUFrame::FID_HUDebugMode) = BIND(HUDebugMode);
    mFSM.on(statIdle, CHUFrame::FID_HUBrightness) = BIND(HUBrightness);

    mFSM.set(statIdle);
}

void CHUMsgService::RecvFile(std::string frame)
{
    std::string data = mFrameDecoder(frame);
    unsigned int fid = mFrameDecoder.getID();
    mFSM.command(fid, data);
    mHeartbeatTimeout = 0;
}

void CHUMsgService::RecvMsg(std::string frame)
{
    std::string data = mFrameDecoder(frame);
    unsigned int fid = mFrameDecoder.getID();
    mFSM.command(fid, data);
}

void CHUMsgService::HUStatus(const fsm::args &args)
{
    GETDATA(HUStatus);
    LOGINF("IVI Source:%d,HDD:%d,USB:%d\n",pData->sourceStatus, pData->sourceHDD, pData->sourceUSB);
    HUHmiIPC::UpdateIVISourceStatus(pData->sourceStatus);
    IVIMusicPlayInfo(0, pData->sourceStatus, 0, 0, "", "", "", "", 0, 0, 0, false, (pData->sourceHDD == 1), (pData->sourceUSB == 1), (pData->sourceBT==1), "010000000000111");
    IVINaviInfo(pData->naviStatus, 0, "", 0, "", "", "", "1");
    IVIRadioPlayInfo(0, pData->sourceStatus, 0, 0, "", false, "01");
    IVIPhoneInfo(pData->btPhoneStatus, "", "", -1, -1, "", "1");
    IVIBtInfo(pData->btConnectStatus, "", "1");
    HUHmiIPC::UpdateIVIBtStatus(pData->btConnectStatus);

    static bool bFirst = true;
    if (bFirst)
    {
        #if 0
        std::ifstream f("/tmp/A301-SOC-Version.txt", std::ios::in);
        #else
        const char* otaVersion = "/usr/sbin/zhapp/midware/cfg/OTA_Display_Version.txt";
        std::ifstream f(otaVersion, std::ios::in);
        #endif
        if (f.is_open())
        {
            std::string sVer;
            std::string temp; 
            while(getline(f,temp)){
                sVer += temp;
            }
            f.close();

            printf("%s_%s_%d sVer:%s\n", __FILE__, __func__, __LINE__, sVer.data());
            IVIIPStatus(sVer);
        }

        bFirst = false;
    }
}

// 1.2 多媒体状态信息
void CHUMsgService::HUMediaStatus(const fsm::args &args)
{
    GETDATA(HUMediaStatus);
    // LOGINF("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    printf("%s_%s_%d mediaType:%d playStatus:%d %d %s-%s-%s-%s\n", __FILE__, __func__, __LINE__, pData->mediaType, pData->playStatus, pData->currentMusic.index, pData->currentMusic.title, pData->currentMusic.album, pData->currentMusic.artist, pData->currentMusic.name);
    IVIMusicPlayInfo(0, 8, pData->mediaType, pData->playStatus, (char *)pData->currentMusic.title, (char *)pData->currentMusic.album, (char *)pData->currentMusic.artist, (char *)pData->currentMusic.name, pData->currentMusic.index, 0, 0, false, false, false, false, "001111111");
    std::vector<MusicCell> list;
    IVIMusicList((pData->mediaType==13 ? MusicType_OL : (MusicType)pData->mediaType), list, 0, 0, 0, pData->currentMusic.index, false, "100001");
    // 注：HU->IP协议定义在线音乐13; IP->HU协议定义在线音乐12; IP工程在线音乐使用12, 因此此处13需要转换成12
}

// 1.3 收音机信息
void CHUMsgService::HURadioStatus(const fsm::args &args)
{
    GETDATA(HURadioStatus);
    // LOGINF("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    // HUHmiIPC::UpdateIVIRadioPlayInfo(false,pData->currentBand,std::to_string(pData->freQuENT));
    printf("%s:%d 0x%x currentBand:%d, presetIndex:%d\n", __func__, __LINE__, (unsigned int)pData, pData->currentBand, pData->presetIndex);
    IVIRadioPlayInfo(0, -1, pData->freQuENT, pData->currentBand, "", false, "0011");

    if (pData->currentBand == 0 || pData->currentBand == 1 || pData->currentBand == 2)
        IVIRadioList(RadioType_FM, m_vecRadioNull, 0, 0, 0, 0, pData->presetIndex, 0, false, "1000001");
    else if (pData->currentBand == 3 || pData->currentBand == 4)
        IVIRadioList(RadioType_AM, m_vecRadioNull, 0, 0, 0, 0, pData->presetIndex, 0, false, "1000001");
}

// 1.4 当前在线电台信息
void CHUMsgService::HURadioCurStatus(const fsm::args &args)
{
    GETDATA(HURadioCurStatus);
    printf("%s:%d 0x%x state:%d, frequent:%d, radioName:%s, index:%d\n", __func__, __LINE__, (unsigned int)pData, pData->state, pData->frequent, pData->radioName, pData->index);
    IVIRadioPlayInfo(0, -1, pData->frequent, -1, (char *)pData->radioName, false, "00101");
    IVIRadioList(RadioType_OL, m_vecRadioNull, 0, 0, 0, 0, pData->index, 0, false, "1000001");
}

void CHUMsgService::HUVoiceStatus(const fsm::args &args)
{
    GETDATA(HUVoiceStatus);
    printf("%s_%s:%d 0x%x\n", __FILE__, __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUPhoneInfo(const fsm::args &args)
{
    GETDATA(HUPhoneInfo);
    // printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    IVIBtInfo(-1, (char *)pData->name, "01");
}

void CHUMsgService::HUCallConnectInfo(const fsm::args &args)
{
    GETDATA(HUCallConnectInfo);
    // printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    IVIPhoneInfo(pData->lineStatus, (char *)pData->contactName, (char *)pData->number, pData->handfree, pData->isMicrophoneMute, (char *)pData->timeStr, "111111");
}

// 2.3 通讯录联系人数量
void CHUMsgService::HUContactCount(const fsm::args &args)
{
    GETDATA(HUContactCount);
    printf("%s_%s:%d 0x%x count:%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->count);

    CallHistory c;
    IVIContactList(pData->count, m_vecContactNull, 0, 0, c, -1, "1");
}

// 2.4 通讯录联系人信息
void CHUMsgService::HUContactInfo(const fsm::args &args)
{
    GETDATA(HUContactInfo);
    printf("%s_%s:%d 0x%x start:%d count:%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->start, pData->count);

    std::vector<uint32_t> raw;
    std::vector<ContactCell> cell;
    raw.clear(); cell.clear();
    for (uint32_t i=0; i<pData->count; i++)
    {
        raw.push_back(pData->contacts[i].index);
        raw.push_back(pData->contacts[i].count);
        for (int j=0; j<128; j+=4)
        {
            raw.push_back((pData->contacts[i].name[j] & 0xff) | ((pData->contacts[i].name[j+1] << 8) & 0xff00) | ((pData->contacts[i].name[j+2] << 16) & 0xff0000) | ((pData->contacts[i].name[j+3] << 24) & 0xff000000));
        }
        for (int j=0; j<32; j+=4)
        {
            raw.push_back((pData->contacts[i].number1[j] & 0xff) | ((pData->contacts[i].number1[j+1] << 8) & 0xff00) | ((pData->contacts[i].number1[j+2] << 16) & 0xff0000) | ((pData->contacts[i].number1[j+3] << 24) & 0xff000000));
        }
        for (int j=0; j<32; j+=4)
        {
            raw.push_back((pData->contacts[i].number2[j] & 0xff) | ((pData->contacts[i].number2[j+1] << 8) & 0xff00) | ((pData->contacts[i].number2[j+2] << 16) & 0xff0000) | ((pData->contacts[i].number2[j+3] << 24) & 0xff000000));
        }
        for (int j=0; j<32; j+=4)
        {
            raw.push_back((pData->contacts[i].number3[j] & 0xff) | ((pData->contacts[i].number3[j+1] << 8) & 0xff00) | ((pData->contacts[i].number3[j+2] << 16) & 0xff0000) | ((pData->contacts[i].number3[j+3] << 24) & 0xff000000));
        }
        for (int j=0; j<32; j+=4)
        {
            raw.push_back((pData->contacts[i].number4[j] & 0xff) | ((pData->contacts[i].number4[j+1] << 8) & 0xff00) | ((pData->contacts[i].number4[j+2] << 16) & 0xff0000) | ((pData->contacts[i].number4[j+3] << 24) & 0xff000000));
        }
        for (int j=0; j<32; j+=4)
        {
            raw.push_back((pData->contacts[i].number5[j] & 0xff) | ((pData->contacts[i].number5[j+1] << 8) & 0xff00) | ((pData->contacts[i].number5[j+2] << 16) & 0xff0000) | ((pData->contacts[i].number5[j+3] << 24) & 0xff000000));
        }

        uint32_t len = raw.size();
        // printf("%s_%s_%d raw.len:%d, raw[0]:%x, raw[1]:%x\n", __FILE__, __func__, __LINE__, len, (len>0?raw[0]:-1), (len>1?raw[1]:-1));

        if (raw[1] == 0)
        {
            printf("%s_%s_%d raw[1] == 0 -> break\n", __FILE__, __func__, __LINE__);
            break;
        }
        // 不满一个联系人,继续入栈
        if ((len<2) || len < (2+32+raw[1]*8))
        {
            printf("%s_%s_%d continue\n", __FILE__, __func__, __LINE__);
            continue;
        }

        ContactCell c;
        c.index = raw[0]; // 索引号
        c.count = raw[1]; // 电话数量
        // printf("%s_%s_%d index:%d count:%d\n", __FILE__, __func__, __LINE__, c.index, c.count);

        for (int j=0; j<32; j++)
        {
            c.name[j*4]   = ((raw[j+2]      ) & 0xFF);
            c.name[j*4+1] = ((raw[j+2] >> 8 ) & 0xFF);
            c.name[j*4+2] = ((raw[j+2] >> 16) & 0xFF);
            c.name[j*4+3] = ((raw[j+2] >> 24) & 0xFF);
        }
        // printf("%s_%s_%d name:%s\n", __FILE__, __func__, __LINE__, c.name);

        for (uint32_t j=0; j<c.count; j+=8)
        {
            std::string s = "";
            uint8_t cc = 0x0;
            for (int k=0; k<8; k++)
            {
                cc = (raw[j+2+32+k]       & 0xff); if (cc>=0x30 && cc <=0x39) s += std::to_string(cc - 0x30).data();
                cc = (raw[j+2+32+k] >> 8  & 0xff); if (cc>=0x30 && cc <=0x39) s += std::to_string(cc - 0x30).data();
                cc = (raw[j+2+32+k] >> 16 & 0xff); if (cc>=0x30 && cc <=0x39) s += std::to_string(cc - 0x30).data();
                cc = (raw[j+2+32+k] >> 24 & 0xff); if (cc>=0x30 && cc <=0x39) s += std::to_string(cc - 0x30).data();
            }

            c.nums.push_back(s);
            // printf("%s_%s_%d j:%d s:%s %x-%x-%x-%x-%x-%x-%x-%x\n", __FILE__, __func__, __LINE__, j, s.data(), raw[j+2+32], raw[j+2+32+1], raw[j+2+32+2], raw[j+2+32+3], raw[j+2+32+4], raw[j+2+32+5], raw[j+2+32+6], raw[j+2+32+7]);
        }

        cell.push_back(c);

        for (uint32_t j=0; j<(2+32+c.count*8); j++) raw.erase(raw.begin());

        // printf("%s_%s_%d cell.size:%d, pData->count:%d\n", __FILE__, __func__, __LINE__, cell.size(), pData->count);
        if (cell.size() >= pData->count)
        {
            break;
        }
    }

    int len = cell.size();
    printf("%s_%s_%d len:%d\n", __FILE__, __func__, __LINE__, len);

    for (int i=0; i<len; i++)
    {
        printf("%s_%s_%d %d [%d/%d] [%s]", __FILE__, __func__, __LINE__, i, cell[i].index, cell[i].count, cell[i].name);
        int jlen = cell[i].nums.size();
        for (int j=0; j<jlen; j++)
        {
            printf("-[%s]", cell[i].nums[j].data());
        }
        printf("\n");
    }

    if (len > 0)
    {
        CallHistory c;
        IVIContactList(0, cell, 0, 0, c, -1, "01");
    }
}

// 2.5 通话记录数量
void CHUMsgService::HUCallContactCount(const fsm::args &args)
{
    GETDATA(HUCallContactCount);
    printf("%s_%s:%d 0x%x count:%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->count);

    if (pData->count > 0)
    {
        IVICallHistory(pData->count, m_vecCallHNull, 0, 0, -1, "1");
    }
}

// 2.6 通话记录信息
void CHUMsgService::HUCallHistory(const fsm::args &args)
{
    GETDATA(HUCallHistory);
    printf("%s_%s:%d 0x%x start:%d,count:%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->start, pData->count);

    int j = 0;
    std::vector<CallHistory> cells;
    for (uint32_t i=pData->start; i<pData->count; i++)
    {
        CallHistory c;
        c.index = pData->calls[j].index;
        c.type = pData->calls[j].callType;
        c.count = pData->calls[j].count;
        memcpy(c.name, pData->calls[j].name, sizeof(pData->calls[j].name)/sizeof(pData->calls[j].name[0]));
        c.number = std::string((char *)pData->calls[j].number);
        c.time = std::string((char *)pData->calls[j].strTime);

        cells.push_back(c);

        j++;
    }

    // for (auto itor=cells.begin(); itor!=cells.end(); itor++)
    // {
    //     printf("%s_%s_%d %d:%s %d %s %s %d\n", __FILE__, __func__, __LINE__, itor->index, itor->name, itor->type, itor->number.data(), itor->time.data(), itor->count);
    // }

    IVICallHistory(0, cells, 0, 0, -1, "01");
}

// 2.8 多个通话状态联系人
void CHUMsgService::HUCallConnectInfoList(const fsm::args &args)
{
    GETDATA(HUCallConnectInfoList);
    printf("%s_%s:%d 0x%x\n", __FILE__, __func__, __LINE__, (unsigned int)pData);
}

// 3 多媒体类
// 3.1 播放列表中歌曲数量
void CHUMsgService::HUPlayListCount(const fsm::args &args)
{
    GETDATA(HUPlayListCount);
    printf("%s_%s:%d 0x%x count:%d \n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->count);
    if (pData->count > 0)
    {
        std::vector<MusicCell> list;
        IVIMusicList(MusicType_NONE, list, pData->count, 0, 0, 0, false, "001");
    }
}

// 3.2 播放列表中歌曲信息
void CHUMsgService::HUMediaInfo(const fsm::args &args)
{
    GETDATA(HUMediaInfo);
    printf("%s_%s:%d 0x%x start:%d-count:%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->start,pData->count);

    if (pData->count > 0)
    {
        std::vector<MusicCell> list;
        for (uint32_t i=0; i<pData->count; i++)
        { 
            if (pData->medias[i].index < 0) continue;

            MusicCell d;
            d.sName = (const char *)pData->medias[i].title;
            d.nIndex = pData->medias[i].index;
            d.nType = pData->medias[i].type;
            d.nId = pData->medias[i].id;
            memcpy(d.title, pData->medias[i].title, sizeof(pData->medias[i].title)/sizeof(pData->medias[i].title[0]));
            memcpy(d.album, pData->medias[i].album, sizeof(pData->medias[i].album)/sizeof(pData->medias[i].album[0]));
            memcpy(d.artist, pData->medias[i].artist, sizeof(pData->medias[i].artist)/sizeof(pData->medias[i].artist[0]));
            memcpy(d.name, pData->medias[i].name, sizeof(pData->medias[i].name)/sizeof(pData->medias[i].name[0]));
            list.push_back(d);
        }
        IVIMusicList(MusicType_NONE, list, 0, 0, 0, 0, false, "01");
    }
}

void CHUMsgService::HUMediaPlayTime(const fsm::args &args)
{
    GETDATA(HUMediaPlayTime);
    // printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    IVIMusicPlayInfo(0, 8, 0, 0, "", "", "", "", 0, pData->currentMs, pData->totalMs, false, false, false, false, "00000000011");
}

// 4 收音机类
// 4.1 收音机列表中频道数量
void CHUMsgService::HURaidoChannelCount(const fsm::args &args)
{
    GETDATA(HURaidoChannelCount);
    printf("%s_%s:%d 0x%x count:%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->count);
    IVIRadioList(RadioType_NONE, m_vecRadioNull, pData->count, 0, 0, 0, 0, 0, false, "001");
}

// 4.2 收音机频道信息
// 0:FM1 1:FM2 2:FM3 3:AM1 4:AM2
// #define VBAND(b) ((b==0) || (b==1) || (b==2) || (b==3) || (b==4))
// #define VBF(b,f) ((((b==0) || (b==1) || (b==2)) && f>=8750 && f<=10800) || (((b==3) || (b==4)) && f>530 && f<=1602))

// 0:FM 1:AM
#define VBAND(b) ((b==0) || (b==1) || (b==2))
#define VBF(b,f) (((b==0) && f>=8750 && f<=10800) || ((b==1) && f>530 && f<=1602))
void CHUMsgService::HURaidoChannelInfo(const fsm::args &args)
{
    GETDATA(HURaidoChannelInfo);
    printf("%s_%s:%d 0x%x %d-%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->start, pData->count);

    int j = 0;
    std::vector<RadioCell> cells;
    for (uint32_t i = pData->start; i<pData->count; i++)
    {
        RadioCell c;
        c.index = pData->radios[j].index;
        c.frq = pData->radios[j].frequence;
        c.band = pData->radios[j].band;
        j++;

        if ((c.index<0) || (c.index>100) || !VBAND(c.band) || (c.frq<=0) || !VBF(c.band,c.frq))
        {
            continue;
        }

        cells.push_back(c);
    }

    for (auto i=cells.begin(); i!=cells.end(); i++)
    {
        printf("%s_%s_%d index:%d,frq:%d,band:%d\n", __FILE__, __func__, __LINE__, i->index, i->frq, i->band);
    }
    IVIRadioList(RadioType_NONE, cells, 0, 0, 0, 0, 0, 0, false, "01");
}

// 4.3 在线收音机的数量
void CHUMsgService::HURaidoChannelOnlineCount(const fsm::args &args)
{
    GETDATA(HURaidoChannelOnlineCount);
    printf("%s_%s:%d 0x%x count:%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->count);
    IVIRadioList(RadioType_OL, m_vecRadioNull, pData->count, 0, 0, 0, 0, 0, false, "101");
}

// 4.4 在线收音机列表
void CHUMsgService::HURaidoChannelOnlineList(const fsm::args &args)
{
    GETDATA(HURaidoChannelOnlineList);
    printf("%s_%s:%d 0x%x %d-%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->start, pData->count);

    int j = 0;
    std::vector<RadioCell> cells;
    for (uint32_t i = pData->start; i<pData->count; i++,j++)
    {
        if ((pData->radios[j].index<0) || (pData->radios[j].index>100) || (pData->radios[j].radioType != 1) || (strlen((char *)pData->radios[j].radioName) == 0) || (strlen((char *)pData->radios[j].radioDesc) == 0))
        {
            // printf("%s_%s_%d err index:%d,name:%s,type:%d\n", __FILE__, __func__, __LINE__, c.index, c.name, c.type);
            // continue;
            break;
        }

        RadioCell c;
        c.index = pData->radios[j].index;
        memcpy(c.name, pData->radios[j].radioName, sizeof(pData->radios[j].radioName)/sizeof(pData->radios[j].radioName[0]));
        memcpy(c.dscr, pData->radios[j].radioDesc, sizeof(pData->radios[j].radioDesc)/sizeof(pData->radios[j].radioDesc[0]));
        c.type = pData->radios[j].radioType;

        cells.push_back(c);
    }

    for (auto i=cells.begin(); i!=cells.end(); i++)
    {
        printf("%s_%s_%d index:%d,name:%s,dscr%s,type:%d\n", __FILE__, __func__, __LINE__, i->index, i->name, i->dscr, i->type);
    }
    IVIRadioList(RadioType_NONE, cells, 0, 0, 0, 0, 0, 0, false, "01");
}

void CHUMsgService::HUVersion(const fsm::args &args)
{
    GETDATA(HUVersion);
    printf("%s_%s:%d 0x%x\n", __FILE__, __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUIPVersion(const fsm::args &args)
{
    GETDATA(HUIPVersion);
    printf("%s_%s:%d 0x%x\n", __FILE__, __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavTurnInfo(const fsm::args &args)
{
    GETDATA(HUNavTurnInfo);
    // LOGINF("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    IVINaviInfo(0, 0, "", pData->id1, (char *)pData->disInfo, "", "", "00011");
}

void CHUMsgService::HUNavRemain(const fsm::args &args)
{
    GETDATA(HUNavRemain);
    // LOGINF("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    IVINaviInfo(0, pData->remainDis, (char *)pData->remainTime, 0, "", "", "", "011");
}

void CHUMsgService::HUNavRoad(const fsm::args &args)
{
    GETDATA(HUNavRoad);
    printf("%s_%s:%d 0x%x\n", __FILE__, __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavRoadConditionBarGraph(const fsm::args &args)
{
    GETDATA(HUNavRoadConditionBarGraph);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavRoadChange(const fsm::args &args)
{
    GETDATA(HUNavRoadChange);
    // LOGINF("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    IVINaviInfo(0, 0, "", 0, "", (char *)pData->curRoad, (char *)pData->nextRoad, "0000011");
}

void CHUMsgService::HUNavSpeedLimit(const fsm::args &args)
{
    GETDATA(HUNavSpeedLimit);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavGlobalMapRequest(const fsm::args &args)
{
    GETDATA(HUNavGlobalMapRequest);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavFeedbackScrrenOutputMode(const fsm::args &args)
{
    GETDATA(HUNavFeedbackScrrenOutputMode);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavRoadArrive(const fsm::args &args)
{
    GETDATA(HUNavRoadArrive);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavRoadJam(const fsm::args &args)
{
    GETDATA(HUNavRoadJam);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavAroundSearch(const fsm::args &args)
{
    GETDATA(HUNavAroundSearch);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavAlongSearch(const fsm::args &args)
{
    GETDATA(HUNavAlongSearch);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavTipsCancel(const fsm::args &args)
{
    GETDATA(HUNavTipsCancel);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavPark(const fsm::args &args)
{
    GETDATA(HUNavPark);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavOpenMeterRequest(const fsm::args &args)
{
    GETDATA(HUNavOpenMeterRequest);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavOilLowLevelAlert(const fsm::args &args)
{
    GETDATA(HUNavOilLowLevelAlert);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUNavThemeStatus(const fsm::args &args)
{
    GETDATA(HUNavThemeStatus);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUHeartbeatAck(const fsm::args &args)
{
    // GETDATA(HUHeartbeatAck);
    // printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    printf("<");
    mHeartbeatTimeout = 0;
}

void CHUMsgService::HULogRequest(const fsm::args &args)
{
    GETDATA(HULogRequest);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

// 9 更新列表标志
// 9.1 更新列表标志
void CHUMsgService::HUUpdateListFlag(const fsm::args &args)
{
    GETDATA(HUUpdateListFlag);
    printf("%s:%d 0x%x %d\n", __func__, __LINE__, (unsigned int)pData, pData->cmd);
    
    switch (pData->cmd)
    {
        case 1: // 更新联系人列表
            IP_TO_HU_CMD(IPContactCount,0);
            break;
        case 2: // 更新通话记录列表
            IP_TO_HU_CMD(IPCallContactCount,0);
            break;
        case 3: // 更新电台列表
            IVIRadioList(RadioType_NONE, m_vecRadioNull, 0, 0, 0, 0, 0, 1, false, "00000001");
            break;
        case 4: // 更新音乐列表
            break;
        case 5: // 更新在线电台列表
            IVIRadioList(RadioType_NONE, m_vecRadioNull, 0, 0, 0, 0, 0, 1, false, "10000001");
            break;
    }
}

void CHUMsgService::HUKeyMode(const fsm::args &args)
{
    GETDATA(HUKeyMode);
    printf("%s_%s_%d 0x%x state:%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->state);

    HUHmiIPC::UpdateIVIKeyMode(pData->state);
}

void CHUMsgService::HUEventSlide(const fsm::args &args)
{
    GETDATA(HUEventSlide);
    // printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
    printf("%s_%s line:%d eventType:%d,uiId:%d\n", __FILE__, __func__, __LINE__, pData->eventType, pData->uiId);
    if (pData->eventType == 0) // 双指左滑
    {
        switch (pData->uiId)
        {
            case 1: // 导航界面
                break;
            case 2: // 收音机界面
            case 3: // 音乐界面
            {
                IVIMusicPlayInfo(1, -1, -1, -1, "", "", "", "", 0, 0, 0, false, false, false, false, "1");
                IVIRadioPlayInfo(1, -1, 0, 0, "", false, "1");
            }
                break;
            case 4: // 其他界面
                break;
            default:
                break;
        }
    }
    else if (pData->eventType == 1) // 双指右滑
    {
        switch (pData->uiId)
        {
            case 1: // 导航界面
                break;
            case 2: // 收音机界面
            case 3: // 音乐界面
            {
                IVIMusicPlayInfo(0, -1, -1, -1, "", "", "", "", 0, 0, 0, false, false, false, false, "1");
                IVIRadioPlayInfo(0, -1, 0, 0, "", false, "1");
            }
                break;
            case 4: // 其他界面
                break;
            default:
                break;
        }
    }
}

void CHUMsgService::HUManualThemeSetting(const fsm::args &args)
{
    GETDATA(HUManualThemeSetting);
    printf("%s_%s_%d 0x%x mode:%d,type(color):%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->mode, pData->type);

    HUHmiIPC::UpdateIVIThemeColor(pData->mode, pData->type);
}

void CHUMsgService::HUDebugMode(const fsm::args &args)
{
    GETDATA(HUDebugMode);
    printf("%s:%d 0x%x\n", __func__, __LINE__, (unsigned int)pData);
}

void CHUMsgService::HUBrightness(const fsm::args &args)
{
    GETDATA(HUBrightness);
    printf("%s_%s_%d 0x%x bri:%d\n", __FILE__, __func__, __LINE__, (unsigned int)pData, pData->cmd);
	HUHmiIPC::UpdateIVIBrightness(pData->cmd);
}
