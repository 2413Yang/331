#include "WatchInterface.h"
#include "LogServiceApi.h"
#include "UpgradeInterface.h"
#include "HUMsgService.h"
#include "HUMsgDefine.h"

void CHUMsgService::initCommunicDevice()
{
    mDevMsg.registerCB(std::bind(&CHUMsgService::RecvMsg, this, std::placeholders::_1));
    mDevFile.registerCB(std::bind(&CHUMsgService::RecvFile, this, std::placeholders::_1));
}

void CHUMsgService::initSelfIPC()
{
    // TODO: Add IPC of self.
    HUHmiIPC::publisher::UpdateIVISourceStatus(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVINaviInfo(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIMusicPlayInfo(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIRadioPlayInfo(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIRadioList(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIPhoneInfo(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIMusicPlayList(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIContactList(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVICallRecordList(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIThemeColor(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIMusicDevice(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIKeyMode(mSelfIpc);
	HUHmiIPC::publisher::UpdateIVIBrightness(mSelfIpc);
    HUHmiIPC::publisher::UpdateIVIBtStatus(mSelfIpc);

    HUHmiIPC::subscriber::UpdateIVIInfoList(mSelfIpc, *this);
    HUHmiIPC::subscriber::UpdateIVIInfoPlayIndex(mSelfIpc, *this);
    HUHmiIPC::subscriber::UpdateIVIBtPhoneControl(mSelfIpc, *this);
    HUHmiIPC::subscriber::UpdateIVIBtList(mSelfIpc, *this);
    HUHmiIPC::subscriber::UpdateIVIContactIndex(mSelfIpc, *this);
    HUHmiIPC::subscriber::UpdateIVICallRecordIndex(mSelfIpc, *this);
    HUHmiIPC::subscriber::UpdateIVIRadioPlayIndex(mSelfIpc, *this);
    HUHmiIPC::subscriber::UpdateIVIMediaSourceList(mSelfIpc, *this);
    HUHmiIPC::subscriber::UpdateIVIMediaCLose(mSelfIpc, *this);

    mSelfIpc.start();
}

void CHUMsgService::initWathchIPC()
{
    WatchIpc::subscriber::LogLevelControl(mWatchIpc, *this);
    WatchIpc::subscriber::HeartbeatSignal(mWatchIpc, *this);

    WatchIpc::publisher::HeartbeatStart(mWatchIpc);
    WatchIpc::publisher::HeartbeatSignal_Resp(mWatchIpc);
    WatchIpc::publisher::HeartbeatStop(mWatchIpc);

    mWatchIpc.start();
    WatchIpc::HeartbeatStart(SERVER_NAME_SELF);
}

void CHUMsgService::initLogIPC()
{
    ZH::logService::publisher::LogRecord(mLogIpc);
    mLogIpc.start();
}

void CHUMsgService::LogLevelControl(std::string name, LOG_LEV_EN logLevel)
{
    if (name == SERVER_NAME_SELF)
    {
        setPrintLevel(logLevel);
    }
}

void CHUMsgService::HeartbeatSignal(std::string moudleName, int count)
{
    if (moudleName == SERVER_NAME_SELF)
    {
        LOGDBG("#");
        int countResp = count + 1;
        WatchIpc::HeartbeatSignal_Resp(SERVER_NAME_SELF, countResp);
    }
}

void CHUMsgService::initUpgradeIPC()
{
    upgrade::publisher::upgrade_IVICtrl(mUpgradeIpc);
    
    mUpgradeIpc.start();    
}
