#ifndef _WARNINGENGINE_H_
#define _WARNINGENGINE_H_
#include "WarningData.h"
#include "BaseState.h"
#include <vector>
#include "STimer.h"
#include <string.h>
#include <map>
#include <atomic>
#include <mutex>
#include <sys/types.h>
#include "WarningInputOutput.h"

namespace warning
{

class CWarningEngine
{
private:
	uint32_t	mTriggerWarningBitMap[(int)EmPopWarnID::END/(8*sizeof(uint32_t)) + 1];
	std::list<EmPopWarnID>	mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_END)];//报警排队待显列表，报警解除或者开始显示则从此list移除
	std::list<EmPopWarnID>	mHistoryWarnList;//报警查看列表
	std::list<EmPopWarnID>	mCycleWarnList;//报警循环列表
	CBaseState*		mWarnStateSet[EnWarnStateID::STS_LAST];
	std::map<EmPopWarnID,CWarningData*>	mMapWarnData;
	EnWarnStateID	mCurWarnSts;
	EnWarnStateID	mLastWarnSts;
	EmPopWarnID		mCurDispWarnID;
	EnWarnWorkPowerSts	mCurWorkPowerSts;
	std::mutex		mMutex;
	std::function<void(EmPopWarnID)> mNotifyDispWarnCB;
	std::function<void(std::vector<EmPopWarnID>&)> mHistoryCB;
	char	mDoorOpenSts;//bit0:左前门，bit0:右前门，bit0:左后门，bit0:右后门，bit0:行李箱门，bit0:发动机舱盖
	bool	mHmiReady;
private:
	static std::atomic<CWarningEngine*>	sWarningEngine;
	static std::mutex          sMutex;
public:
	static const uint8_t cLowestBitmap[256];
private:
	CWarningEngine();
	void warnMapInit();
	void warnMapDeInit();
	void UpdateWarning();	
	void change2PopWarning(EmPopWarnID warnID);
	bool removeWarnInList(std::list<EmPopWarnID>& list, EmPopWarnID WarnID);
	bool isExistInList(std::list<EmPopWarnID>& list, EmPopWarnID WarnID);
public:
	static CWarningEngine* getInstance();
	~CWarningEngine();

	void registerChangeWarn(std::function<void(EmPopWarnID)> cb){ mNotifyDispWarnCB = cb;};
	void registeNotifyHistoryWarnList(std::function<void(std::vector<EmPopWarnID>&)> cb){mHistoryCB = cb;}
	char getDoorOpenSts();
	void updateDoorOpenWarn(char inputDoorSts);

	EmPopWarnID getCurrentDisplayPopWarn() {return mCurDispWarnID;}
	EnWarnStateID getCurrentWarnState() {return mCurWarnSts;}
	EmPopWarnID findFrontWarn(EnWarnCategory category);
	bool getWarnTriggerSts(EmPopWarnID warningId);
	EmPopWarnID findFirstWarning();

	void add2CycleList(EmPopWarnID warnID);
	bool isExistInCycleList(EmPopWarnID warnID);

	void change2State(EnWarnStateID);

	void setCurrentPowerMode(const EnWarnWorkPowerSts enCurPowerSts);
	EnWarnWorkPowerSts getCurrentPowerMode() { return mCurWorkPowerSts;}
	void minDispalyTimeroutProcess();
	void cycleDispalyTimeroutProcess();
	void AutoConfirmDispalyTimeroutProcess();
	void doorOpenAllClosedTimeoutProcess();

	void checkTriggerWarn(EmPopWarnID, bool isTriggered);
	void retriggerWarning(EmPopWarnID);
	void add2HistoryList(EmPopWarnID id);
	void removeFromHistoryList(EmPopWarnID id);
	void setHmiReady();
};




}


#endif //!_WARNINGENGINE_H_