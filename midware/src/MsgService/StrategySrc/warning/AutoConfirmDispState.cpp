
#include "AutoConfirmDispState.h"
#include "WarningEngine.h"

namespace warning
{

CAutoConfirmDispState::CAutoConfirmDispState():CBaseState(EnWarnStateID::STS_AUTOCONFIRM)
{
	mAutoComfirmDispTimer = new ZH::BaseLib::STimer(getWarnAutoConfirmTime(),
		std::bind(&CAutoConfirmDispState::onTimerCompleted, this, std::placeholders::_1));
	ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mAutoComfirmDispTimer);
}
CAutoConfirmDispState::~CAutoConfirmDispState()
{
	if(mAutoComfirmDispTimer)
	{
		ZH::BaseLib::STimerManager::getInstance()->removeTimerNode(mAutoComfirmDispTimer);
		delete mAutoComfirmDispTimer;
	}
}
void CAutoConfirmDispState::enter()
{
	mAutoComfirmDispTimer->start();
}
void CAutoConfirmDispState::leave()
{
	mAutoComfirmDispTimer->stop();
}

void CAutoConfirmDispState::onTimerCompleted(int tag)
{
	(void)tag;
	CWarningEngine::getInstance()->AutoConfirmDispalyTimeroutProcess();
}

}

