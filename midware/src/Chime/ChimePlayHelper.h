#ifndef _CHIMEPLAYHELPER_H_
#define _CHIMEPLAYHELPER_H_
#include <map>
#include "ChimeDataDef.h"
#include "STimer.h"
using namespace ZH::BaseLib;
//一个周期内最大播放的有限次数
#define MAX_PLAY_TIMES_IN_ONE_PERIOD	(10000)
//有限的周期 声音的最大播放的周期数
#define MAX_PLAY_PERIOD_TIMES			(10000)
namespace chime{
class CChimePlayHelper
{
private:
	uint32_t	mPeriodTimes;//周期次数，0：无限循环播放；n(n>0):播放n个周期；
	uint32_t	mPlayTimes;//播放次数，0：持续播放；n(n>0):播放n次；
	uint32_t	mDurationPlayOnceMs;//播放一次的持续时间
	uint32_t	mIntervalPeriodms; //一个周期的时间间隔
	const char*	mPlaySoundFile;	//播放的音源文件
	STimer*		mSTimer;
	STimer*		mPeriodSTimer;
	uint32_t	mPlayCount;//一个周期内播放的次数计数，一个周期内每播放一声加1，下个周期开始清零重新累加
	uint32_t	mPlayPeriodCount;//播放的周期计数，每播完一个周期加1
	std::function<void (int)>	mPlayTimeoutCB;
	std::function<void (int)>	mPeriodTimeoutCB;
	std::function<void (void)>	mPlayOverCB;
	EnChimeID	mPlayChimeID;
public:
	CChimePlayHelper(uint32_t PeriodTimes, uint32_t playTimes, uint32_t durationMs, uint32_t intervalMs, const char* playFile, EnChimeID ID);
	~CChimePlayHelper();

	void setPlaySoundFile(const char* soundFile){mPlaySoundFile = soundFile;}
	void registerTimeroutCallback(std::function<void (int)> playCB){mPlayTimeoutCB = playCB;}
	void registerPeriodTimeroutCallback(std::function<void (int)> CB){mPeriodTimeoutCB = CB;}
	void registerPlayOverCallback(std::function<void (void)> CB){mPlayOverCB = CB;}

	uint32_t getPlayCount() const {return mPlayCount;}
	uint32_t getPlayPeriodCount() const {return mPlayPeriodCount;}

	void startPlay();
	void play_OnTimer();
	void playPeriod_OnTimer();

    void changePlayCount(uint32_t playCount);
};



}


#endif //!_CHIMEPLAYHELPER_H_