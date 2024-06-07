#include "MinDispState.h"
#include "WarningEngine.h"
#include "WarningEngine.h"

namespace warning
{

CMinDispState::CMinDispState():
	CBaseState(EnWarnStateID::STS_MINDISP)
{
	mMinDispTimer = new ZH::BaseLib::STimer(getWarnMinTime(),
		std::bind(&CMinDispState::onTimerCompleted, this, std::placeholders::_1));
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mMinDispTimer);
}

CMinDispState::~CMinDispState()
{
	if(mMinDispTimer)
	{
		ZH::BaseLib::STimerManager::getInstance()->removeTimerNode(mMinDispTimer);
		delete mMinDispTimer;
	}
}
void CMinDispState::onTimerCompleted(int tag)
{
	(void)tag;
	CWarningEngine::getInstance()->minDispalyTimeroutProcess();
}

void CMinDispState::enter()
{
	mMinDispTimer->start();
}
void CMinDispState::leave()
{
	mMinDispTimer->stop();
}

}//!namespace warning