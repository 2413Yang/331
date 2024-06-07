
#ifndef STRATEGYCONTEXT__H__
#define STRATEGYCONTEXT__H__
#include "MsgService.h"
#include "LocalInterface.h"
#include "IPC/IPCCore.h"
#include "warning/WarningInputOutput.h"
#include "LogServiceApi.h"
#include "ChimeApi.h"
#include "TTIconProcess.h"

class CMsgService;
class CStrategyContext
{
public:
    friend CMsgService;
    CStrategyContext(void)
        : oMsg("MsgService"),         // Self as a publisher.
          oWatch("WatchService"),     // subscribe from "WatchService"
          oUpgrade("UpgradeService"), // subscribe from "UpgradeService"
		  oLog("LogService"),
		  oChime("ChimeService")
    {
    }

    void init(void)
    {

#ifdef _LINUX_TARGET_
        oMsg.start(); //, oIvi.start(), oWatch.start(), oAnimat.start(), oUpgrade.start();
		oLog.start();
		oWatch.start();
#endif
		chime::publisher::chimeSettings_VoiceVolumeCtrl(oChime);
		chime::publisher::chimeInput_BcmBuzWarnMod(oChime);
		chime::publisher::chimeInput_Icon(oChime);
		chime::publisher::chimeInput_SyncDisplayWarning(oChime);
		chime::publisher::chimeInput_DTEValue(oChime);
		chime::publisher::chimeInput_OverSpdSts(oChime);
		chime::publisher::chimeInput_Data6_8(oChime);
		chime::publisher::chime_CurrentPowerMode(oChime);
		chime::publisher::chimeInput_HmiReady(oChime);
		oChime.start();
    }

private:
    CIPCConnector oMsg, oWatch, oUpgrade, oLog, oChime;
	warning::CWarningInputOutput	oWarnIO;
	CTTIconProcess		oTTProcess;
};

#endif /*STRATEGYCONTEXT__H__*/