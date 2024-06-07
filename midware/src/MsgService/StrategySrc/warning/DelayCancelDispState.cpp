#include "DelayCancelDispState.h"
#include "WarningEngine.h"

using namespace warning;

CDelayCancelDispState::CDelayCancelDispState():CBaseState(EnWarnStateID::STS_DELAYCANCEL)
{
	mDelayCancelDispTimer = new ZH::BaseLib::STimer(getWarnMinTime(),
		std::bind(&CDelayCancelDispState::onTimerCompleted, this, std::placeholders::_1));
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mDelayCancelDispTimer);
}
CDelayCancelDispState::~CDelayCancelDispState()
{
	if(mDelayCancelDispTimer)
	{
		ZH::BaseLib::STimerManager::getInstance()->removeTimerNode(mDelayCancelDispTimer);
		delete mDelayCancelDispTimer;
	}
}
void CDelayCancelDispState::enter()
{
	mDelayCancelDispTimer->start();
}
void CDelayCancelDispState::leave()
{
	mDelayCancelDispTimer->stop();
}

void CDelayCancelDispState::onTimerCompleted(int tag)
{
	(void)tag;
	CWarningEngine::getInstance()->doorOpenAllClosedTimeoutProcess();
}

