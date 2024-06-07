#include "ChimePlayHelper.h"
#include "ChimeService.h"
using namespace chime;

CChimePlayHelper::CChimePlayHelper(uint32_t PeriodTimes, uint32_t playTimes, uint32_t durationMs, uint32_t intervalMs, const char* playFile, EnChimeID ID):
	mPeriodTimes(PeriodTimes),
	mPlayTimes(playTimes),
	mDurationPlayOnceMs(durationMs),
	mIntervalPeriodms(intervalMs),
	mPlaySoundFile(playFile),
	mPlayChimeID(ID)
{
	LOG_RECORD_DEBUG("(%s,%d) entry,mPeriodTimes:%d,mPlayTimes:%d,mDurationPlayOnceMs:%d,mIntervalPeriodms:%d\n",__func__, __LINE__,
		mPeriodTimes, mPlayTimes,mDurationPlayOnceMs, mIntervalPeriodms);
	mPlayCount = 0;
	mPlayPeriodCount = 1;
	if(mPlayTimes == 1)
	{//播放一次不需要定时器
		mSTimer = nullptr;
	}
	else if(mDurationPlayOnceMs)
	{
		LOG_RECORD_DEBUG("(%s,%d)mDurationPlayOnceMs=%d\n",__func__, __LINE__,mDurationPlayOnceMs);
		mSTimer = new STimer(mDurationPlayOnceMs + 120, std::bind(&CChimePlayHelper::play_OnTimer, this));
		STimerManager::getInstance()->addTimerNode(mSTimer);
	}
	if(mPeriodTimes == 1)
	{
		mPeriodSTimer = nullptr;
	}
	else if(mIntervalPeriodms)
	{
		LOG_RECORD_DEBUG("(%s,%d) mPeriodTimes:%d,mIntervalPeriodms=%d\n",__func__, __LINE__, mPeriodTimes,mIntervalPeriodms);
		mPeriodSTimer = new STimer(mIntervalPeriodms, std::bind(&CChimePlayHelper::playPeriod_OnTimer, this));
		STimerManager::getInstance()->addTimerNode(mPeriodSTimer);
	}
	mPlayTimeoutCB = [](int)->void{
		//LOG_RECORD("mPlayTimeoutCB init function\n");
	};
	mPeriodTimeoutCB = [](int)->void{
		//LOG_RECORD("mPeriodTimeoutCB init function\n");
	};
	mPlayOverCB = []()->void{
		//LOG_RECORD("mPlayOverCB init function\n");
	};
}

CChimePlayHelper::~CChimePlayHelper()
{
	LOG_RECORD("(%s,%d)\n",__func__, __LINE__);
	if(mSTimer)
	{
		STimerManager::getInstance()->removeTimerNode(mSTimer);
		delete mSTimer;
	}
	if(mPeriodSTimer)
	{
		STimerManager::getInstance()->removeTimerNode(mPeriodSTimer);
		delete mPeriodSTimer;
	}
}

void CChimePlayHelper::changePlayCount(uint32_t playCount)
{
    if((playCount == mPlayTimes) || (playCount == 0))
    {
        return;
    }
    else if(playCount == 1)
    {
        if(mSTimer)
        {
            STimerManager::getInstance()->removeTimerNode(mSTimer);
            delete mSTimer;
            mSTimer = nullptr;
        }
        mPlayTimes = 1;
    }
    else
    {
        mPlayTimes = playCount;
        if(!mSTimer)
        {
            mSTimer = new STimer(mDurationPlayOnceMs + 100, std::bind(&CChimePlayHelper::play_OnTimer, this));
		    STimerManager::getInstance()->addTimerNode(mSTimer);
        }
    }
}
void CChimePlayHelper::startPlay()
{
	LOG_RECORD("(%s,%d)\n",__func__, __LINE__);
	play_OnTimer();
	if(mPeriodSTimer)
	{
		mPeriodSTimer->start();
	}
}
void CChimePlayHelper::play_OnTimer()
{
	LOG_RECORD_DEBUG("(%s,%d)(%d,%d)\n",__func__, __LINE__, mPlayCount,mPlayTimes);
	CChimeService::getInstance()->pushSound2PlayList(mPlaySoundFile, mPlayChimeID);
	if(0 == mPlayTimes)
	{
		//持续播放
		if(mSTimer)
		{
			mSTimer->restart();
		}
	}
	else 
	{
		if(++mPlayCount < mPlayTimes)
		{
			if(mSTimer)
			{
				mSTimer->restart();
			}
		}
	}
	if(mPlayTimeoutCB)
	{
		mPlayTimeoutCB(mPlayCount);
	}
	if((mPlayCount>= mPlayTimes) && 
		(mPlayPeriodCount >= mPeriodTimes) &&
		(mPlayOverCB))
	{
		mPlayOverCB();
	}
}
void CChimePlayHelper::playPeriod_OnTimer()
{
	LOG_RECORD_DEBUG("(%s,%d)(%d,%d)\n",__func__, __LINE__, mPlayPeriodCount, mPeriodTimes);
    if(mPeriodTimeoutCB)
	{
		mPeriodTimeoutCB(mPlayPeriodCount);
	}
	mPlayCount = 0;
	play_OnTimer();
	if(0 == mPeriodTimes)
	{
		//无限循环
		if(mPeriodSTimer)
		{
			mPeriodSTimer->restart();
		}
	}
	else
	{
		if(++mPlayPeriodCount < mPeriodTimes)
		{
			if(mPeriodSTimer)
			{
				mPeriodSTimer->restart();
			}
		}
	}
}
