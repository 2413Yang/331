#ifndef HMIMAIN_H_
#define HMIMAIN_H_

#include <kanzi/kanzi.hpp>
#include "videoPlay/videoPlayer_api.h"
#include <time.h>


using namespace kanzi;

class CDispCtrlScreenLayer;
class CMenuDisplay;
class Hmi : public Application
{
public:
	Hmi();
	~Hmi();
	virtual void onConfigure(ApplicationProperties &configuration) KZ_OVERRIDE;
	virtual void onProjectLoaded() KZ_OVERRIDE;
	virtual void registerMetadataOverride(ObjectFactory & /*factory*/) KZ_OVERRIDE;
	virtual void onUpdate(chrono::milliseconds deltaTime) KZ_OVERRIDE;

	void StartupAnimation_PlaySts(tstAnimationSts status);
	void LogLevelControl(std::string, LOG_LEV_EN);
	void HeartbeatSignal(std::string, int);

public:
	static KzsThreadLock *pMsgLock;

	bool isFirstTimeAni;
	uint32_t mSaveSysTime;
private:
	

private:
	bool m_bSelfCheckState;
	bool isAmiLastframe;
protected:
	ScreenSharedPtr m_oRoot;
	Domain *m_Domain;
	CDispCtrlScreenLayer *msgLayer;
	CIPCConnector opt;
	CIPCConnector opt_watch;
};

#endif 
