#include "CycleDispState.h"
#include "WarningEngine.h"

namespace warning
{

CCycleDispState::CCycleDispState():
	CBaseState(EnWarnStateID::STS_CYCLEDISP)
{
	mCycleDispTimer = new ZH::BaseLib::STimer(getWarnCycleTime(),
		std::bind(&CCycleDispState::onTimerOut,this, std::placeholders::_1));
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mCycleDispTimer);
}

CCycleDispState::~CCycleDispState()
{
	if(mCycleDispTimer)
	{
		ZH::BaseLib::STimerManager::getInstance()->removeTimerNode(mCycleDispTimer);
	}
}

void CCycleDispState::onTimerOut(int tag)
{
	//to do
	(void)tag;
	CWarningEngine::getInstance()->cycleDispalyTimeroutProcess();
}

void CCycleDispState::enter()
{
	CWarningEngine::getInstance()->add2CycleList(CWarningEngine::getInstance()->getCurrentDisplayPopWarn());
	mCycleDispTimer->start();
}
void CCycleDispState::leave()
{
	mCycleDispTimer->stop();
}


}
