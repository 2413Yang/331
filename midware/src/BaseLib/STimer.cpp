#include "STimer.h"
#include <unistd.h>
#include "mlog.h"

using namespace ZH::BaseLib;

STimer::STimer(uint32_t durationMs, pfuncOnTimer pfunc,int tag, bool cycleFalg):
mStartTime(0),
mDuration(durationMs),
mStatus(simpleTimerStatus::timer_stop),
mFuncCb(pfunc),
mTag(tag),
mCycleFlag(cycleFalg)
{
}

STimer::~STimer()
{
	stop();
}

void STimer::start()
{
	std::lock_guard<std::mutex> lock(mMutex);
	if((nullptr == this->mFuncCb) || (simpleTimerStatus::timer_running == this->mStatus))
	{
		LOGERR("## Error...start(), mStatus = %d", this->mStatus);
		return;
	}
	this->mStatus = simpleTimerStatus::timer_running;
	this->mStartTime = ZH::BaseLib::getWorldTimeMS();
}
void STimer::start_resetTimer(uint32_t duration)
{
	do
	{
		std::lock_guard<std::mutex> lock(mMutex);
		this->mDuration = duration;
	} while (0);
	this->start();
}

void STimer::stop()
{
	std::lock_guard<std::mutex> lock(mMutex);
	this->mStatus = simpleTimerStatus::timer_stop;
}
void STimer::restart()
{
	stop();
	start();
}


void STimer::restart_resetTimer(uint32_t duration)
{ 
	do
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mDuration = duration;
	} while (0);
	restart();
}
simpleTimerStatus STimer::getStatus()
{
	std::lock_guard<std::mutex> lock(mMutex);
	return mStatus;
}
void STimer::setStatus(simpleTimerStatus sts)
{
	std::lock_guard<std::mutex> lock(mMutex);
	mStatus = sts;
}

uint32_t STimer::getExpireTime()
{
	std::lock_guard<std::mutex> lock(mMutex);
	return (mStartTime + mDuration);
}
void STimer::operator()()
{
	if(nullptr == this->mFuncCb)
	{
		return;
	}
	mFuncCb(mTag);
}

std::atomic<STimerManager*>	STimerManager::spAtoTimerManager(nullptr);
std::mutex STimerManager::sMutex;
STimerManager* STimerManager::getInstance()
{
	STimerManager* pSTimerManager = spAtoTimerManager;
	if(!pSTimerManager)
	{
		std::lock_guard<std::mutex> lock(sMutex);
		if((pSTimerManager = spAtoTimerManager) == nullptr)
		{
			spAtoTimerManager = pSTimerManager = new STimerManager();
		}
	}
	return pSTimerManager;

}
STimerManager::STimerManager():
mThreadExitFlag(false),
mThread(&STimerManager::MainCheckTimerOut, this)
{
	
}
STimerManager::~STimerManager()
{
	mThreadExitFlag = true;
	mThread.join();
}
void STimerManager::simpleTimerExpire(STimer* pTimerNode)
{
	if(simpleTimerStatus::timer_running == pTimerNode->getStatus())
	{
		if(pTimerNode->mCycleFlag)
		{
			pTimerNode->restart();
		}
		else
		{
			pTimerNode->setStatus(simpleTimerStatus::timer_expired);
		}
		(*pTimerNode)();
	}
}
void STimerManager::MainCheckTimerOut()
{
	this->mThreadExitFlag = false;
	while (!this->mThreadExitFlag)
	{
		uint32_t now = ZH::BaseLib::getWorldTimeMS();
		std::list<std::function<void(void)>> taskList;
		do
		{
			std::lock_guard<std::mutex> lock(mMutex2);
			for(auto ite = this->m_list.begin(); ite != m_list.end(); ++ite)
			{
				if(simpleTimerStatus::timer_running == (*ite)->getStatus())
				{
					uint32_t expiredTimeMs = (*ite)->getExpireTime();
					if(expiredTimeMs < now)
					{
						taskList.push_back(std::bind(&STimerManager::simpleTimerExpire, this, *ite));
						//simpleTimerExpire(*ite);
					}
				}
			}
		} while (0);	
		for(auto iter = taskList.begin(); iter != taskList.end();)
		{
			(*iter)();
			iter = taskList.erase(iter);
		}
		usleep(30*1000);
	}	
}
void STimerManager::addTimerNode(STimer* pNode)
{
	if(nullptr == pNode)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(mMutex2);
	this->m_list.push_back(pNode);
}
bool STimerManager::removeTimerNode(STimer* pNode)
{
	std::lock_guard<std::mutex> lock(mMutex2);
	int32_t ret = -1;
	for(auto ite = this->m_list.begin(); ite != m_list.end(); ite++)
	{
		if( (*ite) == pNode)
		{
			ret = 0;
			ite = this->m_list.erase(ite);
			break;
		}
	}
	return ret;
}
