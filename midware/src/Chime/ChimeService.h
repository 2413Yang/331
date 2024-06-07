#ifndef _CHIMESERVICE_H_
#define _CHIMESERVICE_H_
#include <mutex>
#include <atomic>
//#include <unistd.h>
//#include <stdint.h>
#include <list>
#include <map>
#include <thread>
#include "ChimeDataDef.h"
#include "ChimeData.h"
#include "TemplateCache.hpp"
#include "ChimeApi.h"

#define USING_GSTREAMER_APP		(1)
#if USING_GSTREAMER_APP
#include "gstAudioApp.h"
#else
#include "DoShellCmd.hpp"
#endif



namespace chime{

class CChimeService
{
private:
	std::list<EnChimeID>	mChimePlayList;
	std::map<EnChimeID, CChimeData*>	mMapChimeData;
	bool			mbThreadExitFlag;
	std::thread		mThread;
	ZH::BaseLib::CTemplateCache<std::pair<EnChimeID,const char*>> mPlaySoundCache;
#if USING_GSTREAMER_APP
	gstWavApp*		mGstWavApp;
#else
	CDoShellCmd		mDoshellCmd;
#endif
	std::mutex		mMutex;
	uint32_t		mTriggerChimeBitMap[(int)EnChimeID::END/(8*sizeof(uint32_t)) + 1];
	uint32_t		mPlayChimeBitMap[(int)EnChimeID::END/(8*sizeof(uint32_t)) + 1];
	bool			mbVoiceSwtichSts;
	EnVolume		mVolume;
	bool			mbSoundPlayingFlag;
	ZH::BaseLib::STimer*	mWatchPlayTimer;
	EnWorkPowerSts	mPowerModeSts;
	std::map<EnChimeID, bool>	mMapChimePMReset;
	bool			mDelayPlayFlag;
	ZH::BaseLib::STimer* mDelayPlayTimer;
	int mVol;
	std::mutex	mVolCtrlMutex;
	bool mbHmiReady;
    ZH::BaseLib::STimer* mWatchTimer;
    std::thread     mWatchThread;
    struct watchGstPlaySts
    {
        uint32_t    count;
        uint32_t    LastCount;
        bool        isPlaying; //true:正在播放中；false:播放完成
    };
    watchGstPlaySts mWatchGstPlaySts;
private:
	static std::atomic<CChimeService*>	sChimeService;
	static std::mutex          sMutex;
	static const char*	cVolumeNode;
    
private:
	CChimeService();
	void ChimeDataInit();
	void PlaySoundMainLoop();
	void update(bool isChangePowerMode);
	void OnWatchPlaySoundTimeout();
    void watchThreadFn();
public:
	static CChimeService* getInstance();
	~CChimeService();
	void pushSound2PlayList(const char*, EnChimeID ID);
	void checkTriggerChime(EnChimeID ID, bool isTrigger);
	void removeChime2PlayList(EnChimeID ID);
	void chimeSettings_VoiceAndVolume(bool, EnVolume);
	bool getSoundPlayingFlag() {return mbSoundPlayingFlag;}
	void setCurrentPowerMode(const EnWorkPowerSts pmSts);
	int setVolume(int vol);
	void setHmiReady(bool flag) { mbHmiReady = flag;}
};
	
}

#endif//!_CHIMESERVICE_H_