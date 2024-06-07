#include "WarningEngine.h"
#include "NoneDispState.h"
#include "MinDispState.h"
#include "CycleDispState.h"
#include "NormalDispState.h"
#include "AutoConfirmDispState.h"
#include "WarningData_DoorOpen.h"

using namespace warning;

std::atomic<CWarningEngine*> CWarningEngine::sWarningEngine(nullptr);
std::mutex CWarningEngine::sMutex;
CWarningEngine* CWarningEngine::getInstance()
{
	CWarningEngine* pWarningEngine = sWarningEngine;
	if(!pWarningEngine)
	{
		std::lock_guard<std::mutex> lock(sMutex);
		if((pWarningEngine = sWarningEngine) == nullptr)
		{
			sWarningEngine = pWarningEngine = new CWarningEngine();
		}
	}
	return pWarningEngine;
}

CWarningEngine::CWarningEngine()
{
	memset(mTriggerWarningBitMap, 0, sizeof(mTriggerWarningBitMap));
	mWarnStateSet[EnWarnStateID::STS_NONE] = new CNoneDispState();
	mWarnStateSet[EnWarnStateID::STS_MINDISP] = new CMinDispState();
	mWarnStateSet[EnWarnStateID::STS_CYCLEDISP] = new CCycleDispState();
	mWarnStateSet[EnWarnStateID::STS_NORMALDISP] = new CNormalDispState();
	mWarnStateSet[EnWarnStateID::STS_AUTOCONFIRM] = new CAutoConfirmDispState();
	mCurWarnSts = mLastWarnSts = EnWarnStateID::STS_NONE;
	mCurDispWarnID = EmPopWarnID::NONE;
	mCurWorkPowerSts = EnWarnWorkPowerSts::PM_IGN_ON;
	mDoorOpenSts = 0;
	mHmiReady = false;
	warnMapInit();
}

CWarningEngine::~CWarningEngine()
{
	for(auto it : mWarnStateSet)
	{
		delete it;
	}
	warnMapDeInit();
}

char CWarningEngine::getDoorOpenSts()
{
	return mDoorOpenSts;
}
void CWarningEngine::updateDoorOpenWarn(char inputDoorSts)
{
	LOG_RECORD_DEBUG("_warn_%s line:%d inputDoorSts=0x%02x\n", __func__,__LINE__,inputDoorSts);
	char xorDoorSts = inputDoorSts ^ mDoorOpenSts;
	mDoorOpenSts = inputDoorSts;
	bool changeFlag =false;
	if(xorDoorSts & 0x0f)
	{//四个车门的变化
		if(inputDoorSts & 0x0f)
		{
			this->retriggerWarning(EmPopWarnID::DOOR_OPEN);
		}
		else
		{
			this->checkTriggerWarn(EmPopWarnID::DOOR_OPEN, false);
		}
		changeFlag = true;
	}
	if(xorDoorSts & 0x10)
	{
		if(inputDoorSts & 0x10)
		{
			this->checkTriggerWarn(EmPopWarnID::CAR_TRUNK_DOOR_OPEN, true);
		}
		else
		{
			this->checkTriggerWarn(EmPopWarnID::CAR_TRUNK_DOOR_OPEN, false);
		}
		changeFlag = true;
	}
	if(xorDoorSts & 0x20)
	{
		if(inputDoorSts & 0x20)
		{
			this->checkTriggerWarn(EmPopWarnID::CAR_FRONT_DOOR_OPEN, true);
		}
		else
		{
			this->checkTriggerWarn(EmPopWarnID::CAR_FRONT_DOOR_OPEN, false);
		}
		changeFlag = true;
	}
	if(changeFlag)
	{
		if(mMapWarnData.find(mCurDispWarnID) != mMapWarnData.end())
		{
			mNotifyDispWarnCB(mMapWarnData[mCurDispWarnID]->getDisplayPopWarnID());
		}
	}
}

#define GET_WARN_STATE(DataList,idx)		(((DataList)[(idx) / (sizeof((DataList)[0])*8)] & (1 << ((idx) % (sizeof((DataList)[0])*8)))) == 0x00 ? false : true)
#define SET_EN_WARN_STATE(DataList,idx)		(DataList)[(idx) / (sizeof((DataList)[0])*8)] |= (1 << ((idx) % (sizeof((DataList)[0])*8)))
#define SET_DISEN_WARN_STATE(DataList,idx)	(DataList)[(idx) / (sizeof((DataList)[0])*8)] &= ~(1 << ((idx) % (sizeof((DataList)[0])*8)))
void CWarningEngine::checkTriggerWarn(EmPopWarnID warningId, bool isTriggered)
{
	LOG_RECORD_DEBUG("_warn_%s 11 warningId:%d,isTriggered:%d\n", __func__,warningId,isTriggered);
	if(getWarnTriggerSts(warningId) == isTriggered)
    {//已经处理过了
        return;
    }
	if(mMapWarnData.find(warningId) == mMapWarnData.end())
	{
		LOG_SERROR("_warn_%s warningid(%d) not in  mWarningMetaDataMap\n", __func__, warningId);
		return;
	}
	std::lock_guard<std::mutex> lock(mMutex);
	if(isTriggered)
	{
		SET_EN_WARN_STATE(mTriggerWarningBitMap, int(warningId));
	}
	else
	{
		SET_DISEN_WARN_STATE(mTriggerWarningBitMap, int(warningId));
	}
	LOG_RECORD_DEBUG("_warn_%s warningId:%d,BitMap[0]:%08x,BitMap[1]:%08x,\n", __func__, warningId,
		mTriggerWarningBitMap[0], mTriggerWarningBitMap[1]);
	CWarningData* pWarnData = mMapWarnData[warningId];
	if(isTriggered)
	{
		if(int(mCurWorkPowerSts) & int(mMapWarnData[warningId]->getWorkPowerSts()))
		{
			mWarnWaitLists[int(pWarnData->getWarnCategory())].push_back(warningId);
			LOG_RECORD_DEBUG("_warn_%s mWarnWaitLists[%d].push(%d)\n", __func__, int(pWarnData->getWarnCategory()),warningId);
		}
		else
		{
			LOG_RECORD_DEBUG("_warn_%s warningId:%d, getWorkPowerSts(%d) != mCurWorkPowerSts(%d)\n", __func__, warningId, int(mMapWarnData[warningId]->getWorkPowerSts()), int(mCurWorkPowerSts));
		}
	}
	else
	{
		removeWarnInList(mWarnWaitLists[int(pWarnData->getWarnCategory())], warningId);
		removeWarnInList(mCycleWarnList, warningId);
		bool retFlag = removeWarnInList(mHistoryWarnList, warningId);
		if(retFlag)
		{
			std::vector<EmPopWarnID> vecHistoryWarn(mHistoryWarnList.begin(), mHistoryWarnList.end());
			mHistoryCB(vecHistoryWarn);
		}
	}
	this->UpdateWarning();
	
}
void CWarningEngine::retriggerWarning(EmPopWarnID warningId)
{
	//重新触发报警
	if(getWarnTriggerSts(warningId) == false)
    {//
		this->checkTriggerWarn(warningId, true);
        return;
    }
	//重新触发
    if(mMapWarnData.find(warningId) == mMapWarnData.end())
    {
        LOG_SERROR("_warn_%s warningid(%d) not in  mMapWarnData\n", __func__, warningId);
        return;
    }
	LOG_RECORD_DEBUG("_warn_%s warningId:%d\n", __func__,warningId);
	std::lock_guard<std::mutex> lock(mMutex);
	SET_EN_WARN_STATE(mTriggerWarningBitMap, int(warningId));
	if(int(mCurWorkPowerSts) & int(mMapWarnData[warningId]->getWorkPowerSts()))
	{
		CWarningData* pWarnData = mMapWarnData[warningId];
		removeWarnInList(mCycleWarnList, warningId);
		#if 0
		bool retFlag = removeWarnInList(mHistoryWarnList, warningId);
		if(retFlag)
		{
			std::vector<EmPopWarnID> vecHistoryWarn(mHistoryWarnList.begin(), mHistoryWarnList.end());
			mHistoryCB(vecHistoryWarn);
		}
		#endif
		if(mCurDispWarnID == warningId)
		{
			change2PopWarning(warningId);
			return;
		}
		if(isExistInList(mWarnWaitLists[int(pWarnData->getWarnCategory())], warningId))
		{
			return;
		}
		else
		{
			mWarnWaitLists[int(pWarnData->getWarnCategory())].push_back(warningId);
		}
		this->UpdateWarning();
	}
	else
	{
		return;
	}
}

void CWarningEngine::add2HistoryList(EmPopWarnID id)
{
	//显示时存储到报警中心
	if(!isExistInList(mHistoryWarnList, id))
	{
		mHistoryWarnList.push_back(id);
		std::vector<EmPopWarnID> vecHistoryWarn(mHistoryWarnList.begin(), mHistoryWarnList.end());
		mHistoryCB(vecHistoryWarn);
	}
}
void CWarningEngine::removeFromHistoryList(EmPopWarnID id)
{
	removeWarnInList(mHistoryWarnList, id);
}
bool CWarningEngine::removeWarnInList(std::list<EmPopWarnID>& list, EmPopWarnID WarnID)
{
	bool retFlag = false;
	if(list.empty() == false)
	{
		for(auto iter = list.begin(); iter != list.end();)
		{
			if((*iter) == WarnID)
			{
				iter = list.erase(iter);
				retFlag = true;
				break;
			}
			else
			{
				iter++;
			}
		}
	}
	LOG_RECORD_DEBUG("_warn_%s line:%d WarnID=%d, retFlag = %d\n", __func__,__LINE__, WarnID, retFlag);
	return retFlag;
}
bool CWarningEngine::isExistInList(std::list<EmPopWarnID>& list, EmPopWarnID WarnID)
{
	bool retFlag = false;
	if(mMapWarnData.find(WarnID) != mMapWarnData.end())
	{
		if(list.empty() == false)
		{
			for(auto iter : list)
			{
				if(iter == WarnID)
				{
					retFlag = true;
					break;
				}
			}
		}
	}
	LOG_RECORD_DEBUG("_warn_%s line:%d WarnID=%d, retFlag:%d\n", __func__,__LINE__, WarnID, retFlag);
	return retFlag;
}
EmPopWarnID CWarningEngine::findFrontWarn(EnWarnCategory category)
{
	
	EmPopWarnID retWarnID = EmPopWarnID::NONE;
	if(uint(category) >= (sizeof(mWarnWaitLists)/sizeof(mWarnWaitLists[0])))
	{
		LOG_SERROR("%s line:%d category is not invalid", __func__,__LINE__);
	}
	else
	{
		if(!mWarnWaitLists[int(category)].empty())
		{
			retWarnID = mWarnWaitLists[int(category)].front();
		}
	}
	//LOG_RECORD("_warn_%s line:%d category = %d, retWarnID=%d\n", __func__,__LINE__, category, retWarnID);
	return retWarnID;
}
bool CWarningEngine::getWarnTriggerSts(EmPopWarnID warningId)
{
	bool flag = GET_WARN_STATE(mTriggerWarningBitMap, int(warningId)) ? true : false;
	LOG_RECORD_DEBUG("_warn_%s line:%d warningId = %d,BitMap[0]:%08x,BitMap[1]:%08x, flag = %d\n", __func__,__LINE__, warningId, mTriggerWarningBitMap[0], mTriggerWarningBitMap[1],flag);
	return flag;
}
void CWarningEngine::setCurrentPowerMode(const EnWarnWorkPowerSts enCurPowerSts)
{
	LOG_RECORD_DEBUG("_warn_%s line:%d enCurPowerSts=%d\n", __func__,__LINE__, enCurPowerSts);
	std::lock_guard<std::mutex> lock(mMutex);
	if(enCurPowerSts != mCurWorkPowerSts)
	{
		mCurWorkPowerSts = enCurPowerSts;
		#if 1
		if(mMapWarnData.find(mCurDispWarnID) != mMapWarnData.end())
		{
			CWarningData* curWarnData = mMapWarnData[mCurDispWarnID];
			if((false == (int(curWarnData->getWorkPowerSts()) & int(enCurPowerSts))) &&
				(mCurWarnSts != EnWarnStateID::STS_MINDISP))
			{
				//当前显示的报警不匹配当前的电源档位，需要解除该报警
				change2PopWarning(EmPopWarnID::NONE);
			}
		}
		#endif
		for(uint32_t i = 0; i < (sizeof(mWarnWaitLists) / sizeof(mWarnWaitLists[0])); i++)
		{//与当前电源模式不匹配的待显示报警都移除待显示队列
			if(mWarnWaitLists[i].empty() == false)
			{
				for(auto iter = mWarnWaitLists[i].begin(); iter != mWarnWaitLists[i].end();)
				{
					EnWarnWorkPowerSts warnPowerSts = mMapWarnData[(*iter)]->getWorkPowerSts();
					if(false == (int(warnPowerSts) & int(enCurPowerSts)))
					{
						iter = mWarnWaitLists[i].erase(iter);
					}
					else
					{
						iter++;
					}
				}
			}
			
		}
		if(mHistoryWarnList.empty() == false)
		{
			for(auto iter = mHistoryWarnList.begin(); iter != mHistoryWarnList.end();)
			{//与当前电源模式不匹配的历史报警都移除待历史报警队列
				EnWarnWorkPowerSts warnPowerSts = mMapWarnData[(*iter)]->getWorkPowerSts();
				if(false == (int(warnPowerSts) & int(enCurPowerSts)))
				{
					iter = mHistoryWarnList.erase(iter);
					break;
				}
				else
				{
					iter++;
				}
			}
		}
		if(mCycleWarnList.empty() == false)
		{
			for(auto iter = mCycleWarnList.begin(); iter != mCycleWarnList.end();)
			{//与当前电源模式不匹配的循环报警都移除待循环报警队列
				EnWarnWorkPowerSts warnPowerSts = mMapWarnData[(*iter)]->getWorkPowerSts();
				if(false == (int(warnPowerSts) & int(enCurPowerSts)))
				{
					iter = mCycleWarnList.erase(iter);
					break;
				}
				else
				{
					iter++;
				}
			}
		}
		
		for(uint32_t i = 0; i < (sizeof(mTriggerWarningBitMap)/sizeof(mTriggerWarningBitMap[0])); i++)
		{
			auto value = mTriggerWarningBitMap[i];
			while (value)
			{
				int lowBitPos = 0;
				for(uint32_t pos = 0; pos < sizeof(mTriggerWarningBitMap[0]); pos++)
				{
					uint8_t tempValue = (value >> (pos*8)) & 0xff;
					if(tempValue)
					{
						lowBitPos = CWarningEngine::cLowestBitmap[tempValue] + (pos*8);
						break;
					}
				}
				value &= value - 1;
				int warningId = i*(8*sizeof(mTriggerWarningBitMap[0])) + lowBitPos;
				EmPopWarnID enWarnId = static_cast<EmPopWarnID>(warningId);
				if((mMapWarnData.find(enWarnId) == mMapWarnData.end()) ||
					(enWarnId == mCurDispWarnID) ||
					(mMapWarnData[enWarnId]->getWorkPowerSts() == EnWarnWorkPowerSts::PM_IGN_ALL))
				{
					continue;
				}
				CWarningData* warnData = mMapWarnData[enWarnId];
				if(int(enCurPowerSts) & int(warnData->getWorkPowerSts()))
				{
					LOG_RECORD_DEBUG("_warn_(%s,%d), enCurMode = %d, warnMeta->enWorkPoerMode = %d, enWarnId = %d\n", __func__, __LINE__, int(enCurPowerSts), int(warnData->getWorkPowerSts()), enWarnId);
					mWarnWaitLists[int(warnData->getWarnCategory())].push_back(enWarnId);
				}
			}
		}
		this->UpdateWarning();
	}
}

void CWarningEngine::minDispalyTimeroutProcess()
{
	LOG_RECORD_DEBUG("_warn_%s line:%d\n", __func__,__LINE__);
	std::lock_guard<std::mutex> lock(mMutex);
	mMapWarnData[mCurDispWarnID]->minDispalyTimeroutProcess();
	this->UpdateWarning();
}
void CWarningEngine::cycleDispalyTimeroutProcess()
{
	LOG_RECORD_DEBUG("_warn_%s line:%d\n", __func__,__LINE__);
	std::lock_guard<std::mutex> lock(mMutex);
	mMapWarnData[mCurDispWarnID]->cycleDispalyTimeroutProcess();
#if 0
	for(auto iter : mCycleWarnList)
	{
		LOG_RECORD("_warn_%s line:%d,mCycleWarnList: iter=%d\n", __func__,__LINE__,iter);
	}
#endif
	if(isExistInList(mCycleWarnList, mCurDispWarnID))
	{//将循环列表中当前显示的弹窗报警从队首移动到队尾
		removeWarnInList(mCycleWarnList, mCurDispWarnID);
		mCycleWarnList.push_back(mCurDispWarnID);
#if 0
		for(auto iter : mCycleWarnList)
		{
			LOG_RECORD("_warn_%s line:%d,mCycleWarnList: iter=%d\n", __func__,__LINE__,iter);
		}
#endif
	}
	this->UpdateWarning();
}
void CWarningEngine::AutoConfirmDispalyTimeroutProcess()
{
	LOG_RECORD_DEBUG("_warn_%s line:%d\n", __func__,__LINE__);
	std::lock_guard<std::mutex> lock(mMutex);
	mMapWarnData[mCurDispWarnID]->AutoConfirmDispalyTimeroutProcess();
	this->UpdateWarning();
}
void CWarningEngine::doorOpenAllClosedTimeoutProcess()
{
	LOG_RECORD_DEBUG("_warn_%s line:%d\n", __func__,__LINE__);
	std::lock_guard<std::mutex> lock(mMutex);
	if(EmPopWarnID::DOOR_OPEN == mCurDispWarnID)
	{
		this->change2State(EnWarnStateID::STS_NONE);
	}
	this->UpdateWarning();
}
void CWarningEngine::UpdateWarning()
{
	if(!mHmiReady)
	{
		if(ZH::BaseLib::getWorldTimeMS() > 30*1000)
		{//保险措施
			mHmiReady = true;
			LOG_RECORD("\n\nwarning hmi is no ready");
		}
		else
		{
			return;
		}
	}
	LOG_RECORD_DEBUG("_warn_%s line:%d mCurDispWarnID=%d\n", __func__,__LINE__, mCurDispWarnID);
	if(EnWarnStateID::STS_NONE == mCurWarnSts)
	{
		change2PopWarning(findFirstWarning());
		return;
	}
	else
	{
		std::pair<bool,EmPopWarnID> resultBreak = mMapWarnData[mCurDispWarnID]->checkWarnCancelDisplay();
		LOG_RECORD_DEBUG("_warn_%s line:%d resultBreak:{%d,%d}\n", __func__,__LINE__, resultBreak.first, resultBreak.second);
		if(resultBreak.first)
		{//被打断，或者条件解除并显示时间到期
			//removeWarnInList(mCycleWarnList, mCurDispWarnID);
			change2PopWarning(resultBreak.second);
		}
		else
		{
			if(EnWarnStateID::STS_CYCLEDISP == mCurWarnSts)
			{
				if(mCurDispWarnID != findFirstWarning())
				{
					change2PopWarning(findFirstWarning());
				}
			}
		}
	}
}

EmPopWarnID CWarningEngine::findFirstWarning()
{//找出当前触发列表中优先级最高的报警
/*
显示优先级由高到低的顺序为：D、A、B.1、B.2、C
组内不存在优先级，先来先显，后续排队
*/
	//LOG_RECORD("_warn_%s line:%d\n", __func__,__LINE__);
	EmPopWarnID retID = EmPopWarnID::NONE;
	if(!mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_D)].empty())
	{
		retID = mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_D)].front();
	}
	else if(!mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_A)].empty())
	{
		retID = mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_A)].front();
	}
	else if(!mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_B1)].empty())
	{
		retID = mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_B1)].front();
	}
	else if(!mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_B2)].empty())
	{
		retID = mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_B2)].front();
	}
	else if(!mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_C)].empty())
	{
		retID = mWarnWaitLists[int(EnWarnCategory::WARN_CATEGORY_C)].front();
	}
	else if(!mCycleWarnList.empty())
	{
		retID = mCycleWarnList.front();
	}
	LOG_RECORD_DEBUG("_warn_%s line:%d retID=%d\n", __func__,__LINE__, retID);
	return retID;
}
void CWarningEngine::add2CycleList(EmPopWarnID warnID)
{
	if(mMapWarnData.find(warnID) == mMapWarnData.end())
	{
		return;
	}
	if(isExistInList(mCycleWarnList, warnID) == false)
	{
		LOG_RECORD_DEBUG("_warn_%s line:%d warnID:%d, cycyleList.size(%d)\n", __func__, __LINE__, warnID, mCycleWarnList.size());
		mCycleWarnList.push_back(warnID);
	}
}
bool CWarningEngine::isExistInCycleList(EmPopWarnID warnID)
{
	return this->isExistInList(mCycleWarnList, warnID);
}
void CWarningEngine::change2State(EnWarnStateID sts)
{
	LOG_RECORD_DEBUG("_warn_%s line:%d mLastWarnSts=%d,mCurWarnSts=%d,newSts=%d\n", __func__,__LINE__, mLastWarnSts,mCurWarnSts,sts);
	mLastWarnSts = mCurWarnSts;
	if(mWarnStateSet[mLastWarnSts])
	{
		mWarnStateSet[mLastWarnSts]->leave();
	}
	mCurWarnSts = sts;
	if(mWarnStateSet[mCurWarnSts])
	{
		mWarnStateSet[mCurWarnSts]->enter();
	}
}

void CWarningEngine::change2PopWarning(EmPopWarnID warnID)
{
	LOG_RECORD_DEBUG("_warn_%s line:%d warnID = %d\n", __func__,__LINE__, warnID);
	if(mMapWarnData.find(warnID) == mMapWarnData.end())//(EmPopWarnID::NONE == warnID)
	{
		if(EmPopWarnID::NONE != mCurDispWarnID)
		{
			mCurDispWarnID = EmPopWarnID::NONE;
			mNotifyDispWarnCB(mCurDispWarnID);
		}
		change2State(EnWarnStateID::STS_NONE);
	}
	else
	{
		//if(mCurDispWarnID != warnID)//有些报警需要重新触发，并且mNotifyDispWarnCB的显示warnID会有变化，比如门开
		{
			removeWarnInList(mWarnWaitLists[int(mMapWarnData[warnID]->getWarnCategory())], warnID);
			mCurDispWarnID = warnID;
			mNotifyDispWarnCB(mMapWarnData[mCurDispWarnID]->getDisplayPopWarnID());
		}
		mMapWarnData[warnID]->triggerWarn();
	}
	
}

void CWarningEngine::setHmiReady() 
{
	std::lock_guard<std::mutex> lock(mMutex);
	mHmiReady = true;
	this->UpdateWarning();
}

void CWarningEngine::warnMapInit()
{
	LOG_RECORD_DEBUG("_warn_%s line:%d\n", __func__,__LINE__);
	auto funcMapInit = [this](EnWarnCategory warnType, EmPopWarnID id,	EnWarnWorkPowerSts pmSts, uint8_t dataIdx = 0, char bitwise = 0)->void{
		switch (warnType)
		{
		case EnWarnCategory::WARN_CATEGORY_A :
			this->mMapWarnData[id] = new CWarningData_A(id, pmSts, dataIdx, bitwise);
			break;
		case EnWarnCategory::WARN_CATEGORY_B1 :
			this->mMapWarnData[id] = new CWarningData_B1(id, pmSts, dataIdx, bitwise);
			break;
		case EnWarnCategory::WARN_CATEGORY_B2 :
			if(	(id == EmPopWarnID::DOOR_OPEN) ||
				(id == EmPopWarnID::CAR_FRONT_DOOR_OPEN) ||
				(id == EmPopWarnID::CAR_TRUNK_DOOR_OPEN))
			{
				this->mMapWarnData[id] = new CWarningData_DoorOpen(id, pmSts, dataIdx, bitwise);
			}
			else
			{
				this->mMapWarnData[id] = new CWarningData_B2(id, pmSts, dataIdx, bitwise);
			}
			break;
		case EnWarnCategory::WARN_CATEGORY_C :
			this->mMapWarnData[id] = new CWarningData_C(id, pmSts, dataIdx, bitwise);
			break;
		case EnWarnCategory::WARN_CATEGORY_D :
			this->mMapWarnData[id] = new CWarningData_D(id, pmSts, dataIdx, bitwise);
			break;
		default:
			break;
		}
	};
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::KEY_DETECT_FAIL,			EnWarnWorkPowerSts::PM_IGN_ALL,	5,	0x01);//未检测到钥匙
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::KEY_IN_CAR_UNLOCK, 		EnWarnWorkPowerSts::PM_IGN_OFF,	5,	0x02);//钥匙在车内，车门无法闭锁
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::KEY_ELEC_LOW,				EnWarnWorkPowerSts::PM_IGN_ALL,	5,	0x04);//钥匙电量低
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::STEER_LOCK_FAIL,			EnWarnWorkPowerSts::PM_IGN_ON,	5,	0x08);//转向锁止未解除
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::STEER_LOCK_CHECK,			EnWarnWorkPowerSts::PM_IGN_ALL,	5,	0x10);//请检查转向锁止系统
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::THEFT_AUTH_FAIL,			EnWarnWorkPowerSts::PM_IGN_ON,	5,	0x20);//防盗认证失败
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::CHANGE_GEAR_P,			EnWarnWorkPowerSts::PM_IGN_ALL,	5,	0x40);//启动请将挡位切换至P挡
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::START_BRAKE,				EnWarnWorkPowerSts::PM_IGN_ALL,	5,	0x80);//启动请踩刹车
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::DOOR_CLOSE_LOCK,			EnWarnWorkPowerSts::PM_IGN_OFF);//请关好车门再按遥控闭锁
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::LIGHT_OFF,				EnWarnWorkPowerSts::PM_IGN_OFF);//请关闭车灯
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::REDUCE_SPEED_SHIFT_GEAR,	EnWarnWorkPowerSts::PM_IGN_ALL);//车速太高，请减速后再换挡
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::LOW_BATTERY_CHARGE,		EnWarnWorkPowerSts::PM_IGN_ALL);//动力电池电量低，请及时充电
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::CHARGING_NO_GEAR,			EnWarnWorkPowerSts::PM_IGN_ALL);//当前车辆处于外接充电状态,无法切换挡位
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::BRAKE_START_GEAR,			EnWarnWorkPowerSts::PM_IGN_ALL);//请踩制动踏板并按Start按钮后再换挡
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::PULL_OFF_CHARER,			EnWarnWorkPowerSts::PM_IGN_ALL);//驾驶前请先拔充电枪
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::VEHICLE_POWERING,			EnWarnWorkPowerSts::PM_IGN_ALL);//车辆未下电
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::FASTEN_SEAT_BELT_BCM_1,	EnWarnWorkPowerSts::PM_IGN_ON);//请系好安全带
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::FASTEN_SEAT_BELT_BCM_2,	EnWarnWorkPowerSts::PM_IGN_ON);//请系安全带
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::CHARGER_HAND_UNLOCK,		EnWarnWorkPowerSts::PM_IGN_ALL);//充电枪解锁失败，请手动解锁
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::,				EnWarnWorkPowerSts::PM_IGN_ALL);//跟随回家功能开启 已删除
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::,				EnWarnWorkPowerSts::PM_IGN_ALL);//天窗打开 已删除
	//funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::BRAKE_CHANGE_GEAR,		EnWarnWorkPowerSts::PM_IGN_ALL);//挂出P挡时请踩制动 //换挡时请踩下制动踏板
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::BRAKE_CHANGE_GEAR,		EnWarnWorkPowerSts::PM_IGN_ALL);//换档时请踩下制动踏板
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::,				EnWarnWorkPowerSts::PM_IGN_ALL);//换挡失败,请重新换挡
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::VEHICLE_POWERING,			EnWarnWorkPowerSts::PM_IGN_ALL);//车辆未下电
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::LOW_SPEED_WARN_MAN_OFF,	EnWarnWorkPowerSts::PM_IGN_ON,	0,	0x80);//低速行人报警功能已关闭
	funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::TIRE_INFO,				EnWarnWorkPowerSts::PM_IGN_ON,	0,	0x10);//轮胎信息在行驶几分钟后显示
	
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_A, EmPopWarnID::,				EnWarnWorkPowerSts::PM_IGN_ON);//低速行人报警功能已开启 SRD已删除
	//B1类报警
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::LIGHT_OFF,				EnWarnWorkPowerSts::PM_IGN_OFF);//请关闭车灯
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::FASTEN_SEAT_BELT_EPB,	EnWarnWorkPowerSts::PM_IGN_ON);//请系好安全带（EPB提示）
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::PARKING_RAMP,			EnWarnWorkPowerSts::PM_IGN_ON);//驻车坡道过大，请更换驻车地点
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::PARK_BRAKE_LACK_4S,		EnWarnWorkPowerSts::PM_IGN_ON);//手刹夹紧力不足，请联系4S店检查
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::HANDBRAKE_BRAKE,			EnWarnWorkPowerSts::PM_IGN_ON);//释放电子手刹时请踩制动
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::,	EnWarnWorkPowerSts::PM_IGN_ON);//电子手刹未释放
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::CHARGER_WAVE_REPLACE,	EnWarnWorkPowerSts::PM_IGN_ALL);//电网波动，请更换充电地点
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::POWER_SUPPLY,			EnWarnWorkPowerSts::PM_IGN_ALL);//请刷卡或连接电源
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::BRAKE_ON_UNABLE_CHARGE,	EnWarnWorkPowerSts::PM_IGN_ALL);//无法充电,请拉起手刹
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::CHARGER_WAVE_TIME_LONGER,EnWarnWorkPowerSts::PM_IGN_ALL);//电网波动,充电时间延长
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::CHARGER_UNLOCK_TIME_LONGER,	EnWarnWorkPowerSts::PM_IGN_ALL);//充电枪未锁止，充电时间延长
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::CHARGER_UNLOCK_STOP,		EnWarnWorkPowerSts::PM_IGN_ALL);//充电枪未锁止，充电停止
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::CHARGE_P_GEAR,			EnWarnWorkPowerSts::PM_IGN_ALL);//请在P档下进行充电
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::,	EnWarnWorkPowerSts::PM_IGN_ALL);//放电功能启动中
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::,	EnWarnWorkPowerSts::PM_IGN_ALL);//充电枪连接，放电功能禁止使用
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::,	EnWarnWorkPowerSts::PM_IGN_ALL);//电池功率不足，放电功能禁止使用
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::,	EnWarnWorkPowerSts::PM_IGN_ALL);//动力电池电量低，放电功能禁止使用
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::,	EnWarnWorkPowerSts::PM_IGN_ALL);//系统故障，放电功能禁止使用
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_B1, EmPopWarnID::,	EnWarnWorkPowerSts::PM_IGN_ALL);//未拉手刹，放电功能禁止使用
	//B2类报警(门开报警)
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B2, EmPopWarnID::DOOR_OPEN,				EnWarnWorkPowerSts::PM_IGN_ALL);//车门打开
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B2, EmPopWarnID::CAR_FRONT_DOOR_OPEN,		EnWarnWorkPowerSts::PM_IGN_ALL);//发动机舱盖打开
	funcMapInit(EnWarnCategory::WARN_CATEGORY_B2, EmPopWarnID::CAR_TRUNK_DOOR_OPEN,		EnWarnWorkPowerSts::PM_IGN_ALL);//行李箱门打开
	//C类报警
	funcMapInit(EnWarnCategory::WARN_CATEGORY_C, EmPopWarnID::TIRE_MONITOR_RESET,		EnWarnWorkPowerSts::PM_IGN_ON,	0,	0x01);//胎压监测系统故障
	funcMapInit(EnWarnCategory::WARN_CATEGORY_C, EmPopWarnID::POWER_DIST_FAIL_4S,		EnWarnWorkPowerSts::PM_IGN_ALL,	6,	0x01);//电源分配故障，请联系4s店检查
	funcMapInit(EnWarnCategory::WARN_CATEGORY_C, EmPopWarnID::KEY_SYS_4S,				EnWarnWorkPowerSts::PM_IGN_ALL,	6,	0x02);//无钥匙系统故障，请联系4S店检查
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_C, EmPopWarnID::,	EnWarnWorkPowerSts::PM_IGN_ON);//保养即将到期，请及时保养
	// funcMapInit(EnWarnCategory::WARN_CATEGORY_C, EmPopWarnID::,	EnWarnWorkPowerSts::PM_IGN_ON);//保养已到期，请及时保养
	funcMapInit(EnWarnCategory::WARN_CATEGORY_C, EmPopWarnID::BRAKE_FAIL,				EnWarnWorkPowerSts::PM_IGN_ALL);//手刹故障，驻车请注意 　
	funcMapInit(EnWarnCategory::WARN_CATEGORY_C, EmPopWarnID::TIRE_PRESSURE_HIGH,		EnWarnWorkPowerSts::PM_IGN_ON,	0,	0x04);//4轮轮胎压力异常报警(过压报警)
	funcMapInit(EnWarnCategory::WARN_CATEGORY_C, EmPopWarnID::TIRE_PRESSURE_LOW,		EnWarnWorkPowerSts::PM_IGN_ON,	0,	0x08);//4轮轮胎压力异常报警(欠压报警)
	funcMapInit(EnWarnCategory::WARN_CATEGORY_C, EmPopWarnID::TIRE_LEAK,				EnWarnWorkPowerSts::PM_IGN_ON,	0,	0x02);//4轮轮胎压力异常报警(漏气报警)

	//D类报警
	funcMapInit(EnWarnCategory::WARN_CATEGORY_D, EmPopWarnID::POWER_SYS_TEMP_HIGH,		EnWarnWorkPowerSts::PM_IGN_ALL);//驱动系统过温，请减速行驶
	funcMapInit(EnWarnCategory::WARN_CATEGORY_D, EmPopWarnID::BRAKE_BOOSTER_REDUCE,		EnWarnWorkPowerSts::PM_IGN_ALL);//制动助力不足，请谨慎驾驶
	funcMapInit(EnWarnCategory::WARN_CATEGORY_D, EmPopWarnID::ENGINE_FAIL_STOP,			EnWarnWorkPowerSts::PM_IGN_ALL);//动力系统故障，请靠边停车
	funcMapInit(EnWarnCategory::WARN_CATEGORY_D, EmPopWarnID::GEAR_SYS_4S,				EnWarnWorkPowerSts::PM_IGN_ALL);//档位系统故障,请联系4S店维修
	funcMapInit(EnWarnCategory::WARN_CATEGORY_D, EmPopWarnID::BAT_VOL_LOW,				EnWarnWorkPowerSts::PM_IGN_ALL);//12V蓄电池电压过低，请靠边停车
	funcMapInit(EnWarnCategory::WARN_CATEGORY_D, EmPopWarnID::ENGINE_LIMIT_SLOW,		EnWarnWorkPowerSts::PM_IGN_ALL);//动力限制，请减速慢行
	funcMapInit(EnWarnCategory::WARN_CATEGORY_D, EmPopWarnID::ENGINE_FAIL_4S,			EnWarnWorkPowerSts::PM_IGN_ALL);//动力系统故障，请联系4S店检查
	funcMapInit(EnWarnCategory::WARN_CATEGORY_D, EmPopWarnID::TIRE_ADJUST_LEFT,			EnWarnWorkPowerSts::PM_IGN_ALL,	0,	0x20);//请向左打方向盘，调整轮胎
	funcMapInit(EnWarnCategory::WARN_CATEGORY_D, EmPopWarnID::TIRE_ADJUST_RIGHT,		EnWarnWorkPowerSts::PM_IGN_ALL,	0,	0x40);//请向左打方向盘，调整轮胎
	//

}
void CWarningEngine::warnMapDeInit()
{
	LOG_RECORD("_warn_%s line:%d\n", __func__,__LINE__);
	for(auto it : mMapWarnData)
	{
		delete it.second;
	}
}

const uint8_t CWarningEngine::cLowestBitmap[256] =  
{  
    /* 00 */ 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* 10 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* 20 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* 30 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* 40 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* 50 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* 60 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* 70 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* 80 */ 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* 90 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* A0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* B0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* C0 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* D0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* E0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,  
    /* F0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0  
};