#include "clusterSignalDelayProcess.h"
#include "../mylogCtrl.h"

std::mutex CSigDelayProcessor::sMutex;
CSigDelayProcessor::CSigDelayProcessor(std::function<void(uint32_t*, uint32_t)> func, uint32_t duration, std::string name):
	mArraySaveMcuData(nullptr),
	mArrayDataLen(0),
	mFuncProcessSignal(func),
	mName(name)
{
	if(duration  > 20)
	{
		mTimer = new STimer(duration, std::bind(&CSigDelayProcessor::processMCUSignal_OnTimer, this));
		STimerManager::getInstance()->addTimerNode(mTimer);
	}
	else
	{
		mTimer = nullptr;
	}
	mbTimerUpdateFlag = false;
}

CSigDelayProcessor::~CSigDelayProcessor()
{
	if(mTimer)
	{
		STimerManager::getInstance()->removeTimerNode(mTimer);
		delete mTimer;
	}
	if(mArraySaveMcuData)
	{
		free(mArraySaveMcuData);
	}
}
void CSigDelayProcessor::updateSignal(const std::vector<uint32_t>& vecSigValue)
{
	if(!mArraySaveMcuData)
	{
		mArrayDataLen = vecSigValue.size();
		mArraySaveMcuData = (uint32_t*)malloc(mArrayDataLen*4 + 100);
		mFuncCB = std::bind(mFuncProcessSignal, mArraySaveMcuData, mArrayDataLen);
		if(mArraySaveMcuData == nullptr)
		{
			return;
		}
	}
	if(mArrayDataLen != vecSigValue.size())
	{
		LOG_RECORD_DEBUG("\n\n %s, mArrayDataLen(%d) != vecSize:%d",__func__, mArrayDataLen, vecSigValue.size());
	}
	std::lock_guard<std::mutex> lock(CSigDelayProcessor::sMutex);
	bool changeFlag = false;
	//std::string log(mName + ":updateSignal:");
	for(uint32_t i = 0; i < vecSigValue.size(); i++)
	{
		//if(vecSigValue[i] != mArraySaveMcuData[i]) mArraySaveMcuData未赋初值，所以不能比较
		{
			mArraySaveMcuData[i] = vecSigValue[i];
			//changeFlag = true;
			
		}
		//log += std::to_string(vecSigValue[i]) + " ";
	}
	//LOG_RECORD_DEBUG("%s, mArrayDataLen = %d",log.c_str(), mArrayDataLen);
	//if(changeFlag)
	{
		if(mTimer)
		{
			if(mTimer->getStatus() != simpleTimerStatus::timer_running)
			{
				mTimer->start();
				#if 1
				mFuncCB();
				#else
				mFuncProcessSignal(mArraySaveMcuData, mArrayDataLen);
				#endif
				mbTimerUpdateFlag =false;
				LOG_RECORD_DEBUG("4321updateSignal:%s, mTimer->start()",log.c_str());
			}
			else
			{
				mbTimerUpdateFlag = true;
			}
		}
		else
		{
			mFuncCB();
		}	
	}
}
void CSigDelayProcessor::processMCUSignal_OnTimer()
{
	std::lock_guard<std::mutex> lock(CSigDelayProcessor::sMutex);
	
	if(mbTimerUpdateFlag)
	{
		mbTimerUpdateFlag = false;
		#if 1
		mFuncCB();
		#else
		mFuncProcessSignal(mArraySaveMcuData, mArrayDataLen);
		#endif
		if(mTimer)
		{
			mTimer->start();
			LOG_RECORD_DEBUG("processMCUSignal_OnTimer:%s, mTimer->start()", mName.c_str());
		}
	}
}
