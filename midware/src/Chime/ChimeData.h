#ifndef _CHIMEDATA_H_
#define _CHIMEDATA_H_
#include "ChimeDataDef.h"
#include "ChimePlayHelper.h"
#include <map>
#include <vector>
namespace chime{
class CChimeData
{
private:
	EnChimeID			mChimeID;
	EnChimeCategory		mCategory;
	uint32_t			mPriorityIndex;
	EnWorkPowerSts		mWorkPowerSts;
	bool				mbVoiceSwitchSts;
protected:
	uint32_t			mPeriodTimes;//周期次数，0：无限循环播放；n(n>0):播放n个周期；
	uint32_t			mPlayTimes;//播放次数，0：持续播放；n(n>0):播放n次；
	uint32_t			mDurationPlayOnceMs;//播放一次的持续时间
	uint32_t			mIntervalPeriodms; //一个周期的时间间隔
	CChimePlayHelper*	mPlayHelper;
protected:
	void setVoiceSwitchSts(bool sts) { mbVoiceSwitchSts = sts;}
	void setPlaySoundParameter(uint32_t,uint32_t,uint32_t,uint32_t);
public:
	CChimeData(EnChimeID, EnChimeCategory, uint32_t , EnWorkPowerSts);
	~CChimeData();	
	EnChimeID getChimeID() const {return mChimeID;}
	EnChimeCategory getCategory() const { return mCategory;}
	uint32_t getPriority() const{ return mPriorityIndex;}
	bool getVoiceSwitchSts()const {return mbVoiceSwitchSts;}

	uint32_t getPeriodTimes() const {return mPeriodTimes;}
	uint32_t getPlayTimes() const {return mPlayTimes;}
	uint32_t getDurationPlayOnceMs() const {return mDurationPlayOnceMs;}
	uint32_t getIntervalPeriodms() const {return mIntervalPeriodms;}
	EnWorkPowerSts getWorkPowerMode() {return mWorkPowerSts;}

	virtual const char* getPlaySoundSourceFile() = 0;
	virtual void voiceSwitch(bool voiceSwitch) = 0;//语音播报切换
	virtual void startPlay() = 0;
	virtual void stopPlay() = 0;
	virtual void playFinish() = 0;
	virtual void playSound_OnTimer(int count);
	virtual void playSound_OnPeriodTimer(int count);
	
};


class CChimeData_OneSound: public CChimeData
{
private:
	EnSoundType		mSoundType;
	const char*		mSoundFile;
public:
	CChimeData_OneSound(EnChimeID, uint32_t, EnWorkPowerSts,uint32_t,uint32_t,uint32_t,uint32_t, const char* sourceFiles, EnSoundType type);
	~CChimeData_OneSound();
	virtual const char* getPlaySoundSourceFile();
	virtual void voiceSwitch(bool voiceSwitch);
	virtual void startPlay();
	virtual void stopPlay();
	virtual void playFinish();
};

class CChimeData_MultiSound: public CChimeData
{
protected:
	std::vector<const char*>	mVecSoundSourceFiles;
public:
	CChimeData_MultiSound(EnChimeID, uint32_t, EnWorkPowerSts,uint32_t,uint32_t,uint32_t,uint32_t, std::vector<const char*>& vecSourceFiles);
	~CChimeData_MultiSound();
	virtual const char* getPlaySoundSourceFile();
	virtual void voiceSwitch(bool voiceSwitch);
	virtual void startPlay();
	virtual void stopPlay();
	virtual void playFinish();
};

class CChimeData_Seatbelt: public CChimeData_MultiSound
{
private:
    uint32_t    mPlayCount;
    uint32_t    mPlayMaxCount;
public:
	CChimeData_Seatbelt(EnChimeID, uint32_t, EnWorkPowerSts,uint32_t,uint32_t,uint32_t,uint32_t, std::vector<const char*>& vecSourceFiles);
	~CChimeData_Seatbelt();
    virtual void voiceSwitch(bool voiceSwitch);
	virtual const char* getPlaySoundSourceFile();
	virtual void startPlay();
private:
    void cycelPlaySound_OnPeriodTime(int count);
};



}


#endif //!_CHIMEDATA_H_