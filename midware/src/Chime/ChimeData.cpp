#include "ChimeData.h"
#include "ChimeService.h"
#include "ChimePlayHelper.h"

using namespace chime;
CChimeData::CChimeData(EnChimeID id, EnChimeCategory category, uint32_t index, EnWorkPowerSts pmSts):
	mChimeID(id),
	mCategory(category),
	mPriorityIndex(index),
	mWorkPowerSts(pmSts),
	mbVoiceSwitchSts(true),
	mPlayHelper(nullptr)
{
}

CChimeData::~CChimeData()
{
}
void CChimeData::setPlaySoundParameter(uint32_t periodTimes,uint32_t playTimes,uint32_t durationMs,uint32_t IntervalMs)
{
	mPeriodTimes = periodTimes;
	mPlayTimes = playTimes;
	mDurationPlayOnceMs = durationMs;
	mIntervalPeriodms = IntervalMs;
}
void CChimeData::playSound_OnTimer(int count)
{
	if(mPlayHelper)
	{
		if((mPlayHelper->getPlayCount() >= mPlayTimes) &&
			(mPlayHelper->getPlayPeriodCount() >= mPeriodTimes))
		{
			this->stopPlay();
		}
	}
}
void CChimeData::playSound_OnPeriodTimer(int count)
{
	if(mPlayHelper)
	{
		if((mPlayHelper->getPlayCount() >= mPlayTimes) &&
			(mPlayHelper->getPlayPeriodCount() >= mPeriodTimes))
		{
			this->stopPlay();
		}
	}
}

CChimeData_OneSound::CChimeData_OneSound(EnChimeID id, uint32_t index, EnWorkPowerSts pmSts, 
	uint32_t periodTimes,uint32_t playTimes,uint32_t durationMs,uint32_t IntervalMs, const char* sourceFiles, EnSoundType type):
	CChimeData(id, EnChimeCategory::CHIME_CATEGORY_Beep,index,	pmSts),
	mSoundType(type),
	mSoundFile(sourceFiles)
{
	setPlaySoundParameter(periodTimes,playTimes,durationMs,IntervalMs);
}

CChimeData_OneSound::~CChimeData_OneSound()
{
}

const char* CChimeData_OneSound::getPlaySoundSourceFile()
{
	return mSoundFile;
}
void CChimeData_OneSound::voiceSwitch(bool voiceSwitch)//语音播报切换
{
	setVoiceSwitchSts(voiceSwitch);
}

void CChimeData_OneSound::startPlay()
{
	LOG_RECORD_DEBUG("(%s,%d), flag:%d, mPlayHelper:%p\n",__func__, __LINE__, getVoiceSwitchSts(),mPlayHelper);
	if(EnSoundType::Sound_Voice ==  mSoundType)
	{
		if(!getVoiceSwitchSts())
		{
			return;
		}
	}
	if(mPlayHelper)
	{
		delete mPlayHelper;
		mPlayHelper = nullptr;
	}
	if(!mPlayHelper)
	{
		mPlayHelper = new CChimePlayHelper(mPeriodTimes, mPlayTimes, mDurationPlayOnceMs, mIntervalPeriodms, getPlaySoundSourceFile(), getChimeID());
		mPlayHelper->registerPlayOverCallback(std::bind(&CChimeData_OneSound::playFinish, this));
		mPlayHelper->startPlay();
	}
}
void CChimeData_OneSound::playFinish()
{
	LOG_RECORD_DEBUG("(%s,%d), flag:%d, mPlayHelper:%p\n",__func__, __LINE__, getVoiceSwitchSts(),mPlayHelper);
	CChimeService::getInstance()->removeChime2PlayList(getChimeID());
}
void CChimeData_OneSound::stopPlay()
{
	LOG_RECORD_DEBUG("(%s,%d), flag:%d, mPlayHelper:%p\n",__func__, __LINE__, getVoiceSwitchSts(),mPlayHelper);
	if(mPlayHelper)
	{
		delete mPlayHelper;
		mPlayHelper = nullptr;
	}
}

CChimeData_MultiSound::CChimeData_MultiSound(EnChimeID id, uint32_t index, EnWorkPowerSts pmSts, 
	uint32_t periodTimes,uint32_t playTimes,uint32_t durationMs,uint32_t IntervalMs, std::vector<const char*>& vecSourceFiles):
	CChimeData(id, EnChimeCategory::CHIME_CATEGORY_Beep,index,	pmSts),
	mVecSoundSourceFiles(vecSourceFiles)
{
	setPlaySoundParameter(periodTimes,playTimes,durationMs,IntervalMs);
}

CChimeData_MultiSound::~CChimeData_MultiSound()
{
}

const char* CChimeData_MultiSound::getPlaySoundSourceFile()
{
	LOG_RECORD_DEBUG("(%s,%d)\n",__func__, __LINE__);
	if(mVecSoundSourceFiles.size())
	{
		if(!getVoiceSwitchSts())
		{
			return mVecSoundSourceFiles.front();
		}
		else
		{
			return mVecSoundSourceFiles.back();
		}
	}
	return "";
}
void CChimeData_MultiSound::voiceSwitch(bool voiceSwitch)//语音播报切换
{
	LOG_RECORD_DEBUG("(%s,%d), voiceSwitch=%d\n",__func__, __LINE__,voiceSwitch);
	setVoiceSwitchSts(voiceSwitch);
}

void CChimeData_MultiSound::startPlay()
{
	LOG_RECORD_DEBUG("(%s,%d), mPlayHelper=%p\n",__func__, __LINE__,mPlayHelper);
	if(mPlayHelper)
	{
		delete mPlayHelper;
		mPlayHelper = nullptr;
	}
	if(!mPlayHelper)
	{
		uint32_t playTimes = getVoiceSwitchSts() ? 1 : mPlayTimes;
		mPlayHelper = new CChimePlayHelper(mPeriodTimes, playTimes, mDurationPlayOnceMs, mIntervalPeriodms, getPlaySoundSourceFile(), getChimeID());
		mPlayHelper->registerPlayOverCallback(std::bind(&CChimeData_MultiSound::playFinish, this));
		mPlayHelper->startPlay();
	}
}
void CChimeData_MultiSound::playFinish()
{
	LOG_RECORD_DEBUG("(%s,%d), mPlayHelper=%p\n",__func__, __LINE__,mPlayHelper);
	CChimeService::getInstance()->removeChime2PlayList(getChimeID());
}

void CChimeData_MultiSound::stopPlay()
{
	LOG_RECORD_DEBUG("(%s,%d), mPlayHelper=%p\n",__func__, __LINE__,mPlayHelper);
	if(mPlayHelper)
	{
		delete mPlayHelper;
		mPlayHelper = nullptr;
	}
}


CChimeData_Seatbelt::CChimeData_Seatbelt(EnChimeID id, uint32_t index, EnWorkPowerSts pmSts, 
	uint32_t periodTimes,uint32_t playTimes,uint32_t durationMs,uint32_t IntervalMs, std::vector<const char*>& vecSourceFiles):
	CChimeData_MultiSound(id, index, pmSts, periodTimes, playTimes, durationMs, IntervalMs, vecSourceFiles)
{
    mPlayCount = 0;
    mPlayMaxCount = playTimes;
}

CChimeData_Seatbelt::~CChimeData_Seatbelt()
{
}
void CChimeData_Seatbelt::startPlay()
{
    mPlayCount = 0;
	CChimeData_MultiSound::startPlay();
	if(mPlayHelper)
	{
		mPlayHelper->registerPeriodTimeroutCallback(std::bind(&CChimeData_Seatbelt::cycelPlaySound_OnPeriodTime, this, std::placeholders::_1));
	}
}
void CChimeData_Seatbelt::voiceSwitch(bool voiceSwitch)//语音播报切换
{
	CChimeData_MultiSound::voiceSwitch(voiceSwitch);
    mPlayMaxCount = voiceSwitch ? 1:mPlayTimes;
}
const char* CChimeData_Seatbelt::getPlaySoundSourceFile()
{
	LOG_RECORD_DEBUG("(%s,%d) mPlayCount:%d\n",__func__, __LINE__, mPlayCount);
	if(mVecSoundSourceFiles.size())
	{
		if(!getVoiceSwitchSts())
		{
            if(mPlayCount > 0)
            {
                return mVecSoundSourceFiles[1];
            }
			else
            {
                return mVecSoundSourceFiles[0];
            }
		}
		else
		{
			return mVecSoundSourceFiles.back();
		}
	}
	return "";
}

void CChimeData_Seatbelt::cycelPlaySound_OnPeriodTime(int count)
{
    if(mPlayHelper)
    {
        mPlayCount = count;
        mPlayHelper->changePlayCount(mPlayMaxCount);
        mPlayHelper->setPlaySoundFile(getPlaySoundSourceFile());
        LOG_RECORD_DEBUG("(%s,%d) mPlayCount:%d, file:%s\n",__func__, __LINE__, count, getPlaySoundSourceFile());
    }
}