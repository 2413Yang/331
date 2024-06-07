
#ifndef APPMAIN__H__
#define APPMAIN__H__
#include "Application.h"
#include "IPC/IPCCore.h"
#include "burnMCU.h"
#include "WatchInterface.h"

#define SD1_PATH        "/mnt/sda1/sda1/"  //迈亚本地升级检测目录
#define SD2_PATH        "/mnt/sda2/sda1/" //gl8本地升级检测目录
#define OTA_PATH        "/run/ota/"     //ota升级检测目录
#define OS_FILE_NAME    "update/"
#define MCU_FILE_NAME   "zh_arm_mcu.bin"
#define PROJECT_CHECK	"A301"

class CApp : public ZH::BaseLib::CApplication
{
public:
    enum updateType
    {
        updateType_None = 0,
        updateType_MCU = 1,
        updateType_OS = 2,
        updateType_All = updateType_MCU | updateType_OS,
    };
    enum updatePath
    {
        updatePath_SDA1,
        updatePath_SDA2,
        updatePath_OTA,
        updatePath_Last,
    };
    struct updateFileInfo
    {
        updatePath  enUpdatePath;
        const char* filePath;
    };
    enum otaProcessStep
    {
        OTA_Step_Init,
        OTA_Step_CheckMountPoint, //开始检查ota分区的挂载点是否不在了
        OTA_Step_CheckMountPointComplete,//检查ota分区挂载点完成
        OTA_Step_CheckPack, //开始检查升级包文件是否存在
        OTA_Step_CheckPackTimeOut,
        OTA_Step_Decompression,//开始解压缩升级包
        OTA_Step_DecompressionComplete,//解压缩升级包完成
        OTA_Step_Last,
    };
    enum upgradeHmiHint
    {
        
    };

    typedef enum _mcu_upgrade_flag {
        MCU_UFLAG_NONE = 0,
        MCU_UFLAG_UPGRADEING = 1,
        MCU_UFLAG_DONE = 2,
    } MCUUprgadeFlag;

public:
    CApp(int argc, char *argv[]);
    virtual ~CApp();

    virtual void Init();
    virtual void DoWork();
    virtual void UInit();
    void upgrade_IVICtrl(int value);
    void Upgrade_clusterVersion(std::string armVer, std::string mcuVer);

    void LogLevelControl(std::string name, LOG_LEV_EN logLevel);
	void HeartbeatSignal(std::string name, int count);
	void sendUpgradeInfo2Mcu(uint8_t cmd, uint8_t times = 2);
private:
    updateType isHaveUpgradePackage();
    std::string findOTAArchiveFromOTADir(const char* dir);
	bool	checkProjectFile();
    bool    checkOsMd5();
    void    sendSystemUpdateMessaage(EmSWState sts, std::string title, std::string content, int progress, int debugID);
	bool	copyFile(const char* dstFile, const char* srcFile);
    static bool setMCUUpgradeFlag(MCUUprgadeFlag flag);
    static MCUUprgadeFlag getMCUUpgradeFlag();

private:
    const static char*      cOSFileMd5TextName;
    const static char*      cOTA_ArchivePrefix;
    const static char*      cOTA_ArchiveSuffix[2];
    const static updateFileInfo cUpdateFileInfoList[updatePath_Last];
    static const std::string cMCUBakFilePath;
    static const std::string cMCUBKRecFilename;
	static const char*		cFoolProofFile;
private:
    CIPCConnector oIpc;
    CIPCConnector watchAppIpc;
	CIPCConnector logAppIpc;
    char    mARM_verion[32];
    char    mMCU_version[32];
    bool    mCheckOTAFile;
    int     mOTACtrlValue;
    otaProcessStep mOTAProcessStep;
    updatePath  mUpdatePath;
    int     mDispCount;//升级倒计时
    burnMCU*    mBurnMCU;
    EmSWState   mMsgPopFlag;//升级弹窗是否开启
    uint32_t    mOTACheckPackTimes;
    uint32_t    mOTAWaitPackTimes;//等待中控传输升级包计时
	bool		mVialdVersion;
};

#endif /*APPMAIN__H__*/