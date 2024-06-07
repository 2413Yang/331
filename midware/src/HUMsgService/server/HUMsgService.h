
#ifndef HUMSGSERVICE__H__
#define HUMSGSERVICE__H__

#include "Application.h"
#include "IPC/IPCCore.h"
#include "Hook.h"
#include "HUNetMsg.h"
#include "HUFrameDecoder.h"
#include "HUFrameEncoder.h"
#include "fsm.hpp"
#include "simpleTimer.h"
#include "HUMsgDefine.h"

#define IP_TO_HU_CMD(IP, c) \
    { \
        HUFData d = { IP : { cmd : (uint32_t)(c) } }; \
        std::string f = mFrameEncoder(CHUFrame::FID_##IP, &d, sizeof(d.IP)); \
        mDevMsg.send(f); \
        printf("%s_%s_%d IP_TO_HU_CMD(%s) :", __FILE__, __func__, __LINE__, #IP); \
        for (uint32_t i=0;i<f.length();++i) \
        { \
            printf("%02x ",f[i]); \
        } \
        printf("\n"); \
    }

#define IP_TO_HU_START_COUNT(IP, s, c) \
    { \
        HUFData d = { IP : { start : (uint32_t)(s), count: (uint32_t)(c) } }; \
        std::string f = mFrameEncoder(CHUFrame::FID_##IP, &d, sizeof(d.IP)); \
        mDevMsg.send(f); \
        printf("%s_%s_%d IP_TO_HU_START_COUNT(%s) :", __FILE__, __func__, __LINE__, #IP); \
        for (uint32_t i=0;i<f.length();++i) \
        { \
            printf("%02x ",f[i]); \
        } \
        printf("\n"); \
    }

#define IP_TO_HU_SRC_START_COUNT(IP, src, s, c) \
    { \
        HUFData d = { IP : { source: (uint32_t)src, start : (uint32_t)(s), count: (uint32_t)(c) } }; \
        std::string f = mFrameEncoder(CHUFrame::FID_##IP, &d, sizeof(d.IP)); \
        mDevMsg.send(f); \
        printf("%s_%s_%d IP_TO_HU_SRC_START_COUNT(%s) :", __FILE__, __func__, __LINE__, #IP); \
        for (uint32_t i=0;i<f.length();++i) \
        { \
            printf("%02x ",f[i]); \
        } \
        printf("\n"); \
    }

#define IP_TO_HU_PRINT(IP, f) \
    { \
        printf("%s_%s_%d IP_TO_HU(%s) :", __FILE__, __func__, __LINE__, #IP); \
        for (uint32_t i=0;i<f.length();++i) \
        { \
            printf("%02x ",f[i]); \
        } \
        printf("\n"); \
    }

struct MusicCell {
    std::string sName;
    uint32_t nIndex;
    uint32_t nType;
    uint32_t nId;
    uint8_t title[128];
    uint8_t album[128];
    uint8_t artist[128];
    uint8_t name[128];
    bool operator==(const MusicCell &r) { return ((this->sName==r.sName) && (this->nIndex==r.nIndex)); }
};

enum MusicType {
    MusicType_NONE = 0,
    MusicType_HDD = 10,
    MusicType_USB = 11,
    MusicType_OL = 12,
    MusicType_BT = 47
};

struct ContactCell {
    uint32_t index = -1; // ����:0��ʼ
    uint32_t count = -1; // �绰��������
    uint8_t name[128]; // ��ϵ������
    std::vector<std::string> nums; // ��ϵ�˵绰�б�(���5��)
};

struct CallHistory {
    uint32_t index = -1; // ����:0��ʼ
    uint32_t type = 0; // 1 �ѽ�     2 �Ѳ�     3 δ��
    uint8_t name[128]; // ��ϵ������
    std::string number; // �绰����
    std::string time; // ͨ��ʱ��
    uint32_t count = 0; // ͨ������
};

struct RadioCell {
    uint32_t index = -1; // ����:0��ʼ
    uint32_t frq = 0; // Ƶ��(����,���)
    uint32_t band = 0; // ����(����) 0:FM1 1:FM2 2:FM3 3:AM1 4:AM2
    uint8_t id[8]; // ��Ƶid(���ߵ�̨,���)
    uint32_t type; // ��Ƶ����(���ߵ�̨) 0:ֱ����̨ 1:�㲥��Ƶ (���) 1:AM 2:FM 3:���ߵ�̨
    uint8_t name[128]; // ��̨����(���ߵ�̨,���)
    uint8_t dscr[128]; // ��̨���(���ߵ�̨)
    uint8_t propram[128]; // ��Ŀ��Ϣ(���߽�Ŀ)
};

enum RadioType {
    RadioType_NONE = -1,
    RadioType_FM = 0,
    RadioType_AM = 1,
    RadioType_OL = 7, // ���ߵ�̨
    RadioType_POL, // ���߽�Ŀ
    RadioType_MIX
};

class CHUMsgService : public ZH::BaseLib::CApplication
{
public:
    enum
    {
        statIdle = 0,
        statUpgrade,
    };

    static const std::string SERVER_NAME_SELF;     
    static const std::string UPGRADE_MOUNTPATH; 
    static const std::string UPGRADE_DIR;
    static const std::string ARCHIVE_PREFIX;
    static const std::string ARCHIVE_SUFFIX;
    static const int HEARTBEAT_TIMEOUT_THRESHOLD = 5;
    static const int CONNECT_RETRY_THRESHOLD = 150;

public:
    // friend CStrategyContext;
    CHUMsgService(int argc, char *argv[]);
    virtual ~CHUMsgService();
    virtual void Init();
    virtual void DoWork();
    virtual void UInit();

    void RecvMsg(std::string);
    void RecvFile(std::string);

    void LogLevelControl(std::string name, LOG_LEV_EN logLevel);
    void HeartbeatSignal(std::string, int);

    void UpdateIVIInfoList(HUHmiIPC::IVIList, int nPage);
    void UpdateIVIInfoPlayIndex(int);
    void UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage, HUHmiIPC::BTPhoneButton, HUHmiIPC::BTPhoneControl);
    void UpdateIVIBtList(HUHmiIPC::BtList eListType, int nPage);
    void UpdateIVIContactIndex(int nIndex);
    void UpdateIVICallRecordIndex(int nIndex);
    void UpdateIVIRadioPlayIndex(int nIndex);
    void UpdateIVIMediaSourceList(int nSrc);
    void UpdateIVIMediaCLose(bool bClose);

private:
    CIPCConnector mSelfIpc;
    CIPCConnector mWatchIpc;
    CIPCConnector mLogIpc;
    CIPCConnector mUpgradeIpc;

    CHUNetMsg mDevMsg;
    CHUNetMsg mDevFile;

    CHUFrameDecoder mFrameDecoder;
    CHUFrameEncoder mFrameEncoder;

    fsm::stack mFSM;

    bool devMsgConnect();
    bool devFileConnect();

    void initCommunicDevice(void);
    void initSelfIPC(void);
    void initWathchIPC(void);
    void initLogIPC(void);
    void initUpgradeIPC(void);
    void initFSM(void);

    // Frame handler
    void HUStatus(const fsm::args &args);
    void HUMediaStatus(const fsm::args &args);
    void HURadioStatus(const fsm::args &args);
    void HURadioCurStatus(const fsm::args &args);
    void HUVoiceStatus(const fsm::args &args);
    void HUPhoneInfo(const fsm::args &args);
    void HUCallConnectInfo(const fsm::args &args);
    void HUContactCount(const fsm::args &args);
    void HUContactInfo(const fsm::args &args);
    void HUCallContactCount(const fsm::args &args);
    void HUCallHistory(const fsm::args &args);
    void HUCallConnectInfoList(const fsm::args &args);
    void HUPlayListCount(const fsm::args &args);
    void HUMediaInfo(const fsm::args &args);
    void HUMediaPlayTime(const fsm::args &args);
    void HURaidoChannelCount(const fsm::args &args);
    void HURaidoChannelInfo(const fsm::args &args);
    void HURaidoChannelOnlineCount(const fsm::args &args);
    void HURaidoChannelOnlineList(const fsm::args &args);
    void HUVersion(const fsm::args &args);
    void HUIPVersion(const fsm::args &args);
    void HUNavTurnInfo(const fsm::args &args);
    void HUNavRemain(const fsm::args &args);
    void HUNavRoad(const fsm::args &args);
    void HUNavRoadConditionBarGraph(const fsm::args &args);
    void HUNavRoadChange(const fsm::args &args);
    void HUNavSpeedLimit(const fsm::args &args);
    void HUNavGlobalMapRequest(const fsm::args &args);
    void HUNavFeedbackScrrenOutputMode(const fsm::args &args);
    void HUNavRoadArrive(const fsm::args &args);
    void HUNavRoadJam(const fsm::args &args);
    void HUNavAroundSearch(const fsm::args &args);
    void HUNavAlongSearch(const fsm::args &args);
    void HUNavTipsCancel(const fsm::args &args);
    void HUNavPark(const fsm::args &args);
    void HUNavOpenMeterRequest(const fsm::args &args);
    void HUNavOilLowLevelAlert(const fsm::args &args);
    void HUNavThemeStatus(const fsm::args &args);
    void HUFileInfo(const fsm::args &args);
    void HUFileData(const fsm::args &args);
    void HUHeartbeatAck(const fsm::args &args);
    void HUUpgradeRequest(const fsm::args &args);
    void HULogRequest(const fsm::args &args);
    void HUUpdateListFlag(const fsm::args &args);
    void HUKeyMode(const fsm::args &args);
    void HUEventSlide(const fsm::args &args);
    void HUManualThemeSetting(const fsm::args &args);
    void HUDebugMode(const fsm::args &args);
    void HUBrightness(const fsm::args &args);

    // Util
    size_t getDiskFreeBytes(const std::string &mountPath); 
    std::string getHexString(std::string const &buffer, std::string const &delimiter);
    std::string getFileMD5(std::string const &filename);

    void IVINaviInfo(int nStatus, int nLessMile, std::string sLessTime, int nDir, std::string sNextRoadMile, std::string sRoad, std::string sRoadNext, std::string mask);
    void IVIMusicPlayInfo(int nSlideSync, int nStatus, int nMediaType, int nPlaySts, std::string sTitle, std::string sAlbum, std::string sSinger, std::string sFolder, int nPlayingIndex, int nTime, int nTimeTotal, bool bList, bool bHDD, bool bUSB, bool bBt, std::string mask);
    void IVIRadioPlayInfo(int nSlideSync, int nType, int nFrq, int nBrand, std::string sName, bool bList, std::string mask);
    void IVIPhoneInfo(int nStatus, std::string sName, std::string sTelNum, int nTelMedia, int nMute, std::string sTime, std::string mask);
    void IVIBtInfo(int nStatus, std::string sPhoneName, std::string mask);
    void IVIRadioList(RadioType eType, std::vector<RadioCell> &list, int nCount, int nReqStart, int nReqCount, int nPlayIndex, int nPlayingIndex, int nReqHUCount, bool bMode, std::string mask);
    void IVIMusicList(MusicType eType, std::vector<MusicCell> &list, int nCount, int nReqPage, int nPlayIndex, int nPlayingIndex, bool bMode, std::string mask);
    void IVIContactList(int nCount, std::vector<ContactCell> &list, int nReqPage, int nReqCallIndex, CallHistory &chReq, int nBtSts, std::string mask);
    void IVICallHistory(int nCount, std::vector<CallHistory> &list, int nReqPage, int nReqCallIndex, int nBtSts, std::string mask);
    void IVIIPStatus(std::string);

    void SplitHourMinute(const std::string &,int &,int &);
    void SplitMile(const std::string &,int &);
    int ConverMusicType(MusicType eType);
    int ConverMediaSource(int nFromHMI);

    HUF_HUFileInfo mUpgradeFileInfo;
    size_t mFileByteReceived;
    std::string mUpgradeFilename;
    std::string mUpgradeFileMD5;
    int mHeartbeatTimeout;
    int mSilence;

    std::vector<std::string> m_vecStrNull;
    std::vector<RadioCell> m_vecRadioNull;
    std::vector<ContactCell> m_vecContactNull;
    std::vector<CallHistory> m_vecCallHNull;
};

#endif /*HUMSGSERVICE__H__*/