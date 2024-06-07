#ifndef _AUTOCONFIRMDISPSTATE_H_
#define _AUTOCONFIRMDISPSTATE_H_

#include "BaseState.h"
#include "STimer.h"

namespace warning
{

class CAutoConfirmDispState:public CBaseState
{
private:
	ZH::BaseLib::STimer*		mAutoComfirmDispTimer;
public:
	CAutoConfirmDispState();
	virtual ~CAutoConfirmDispState();
	virtual void enter();
	virtual void leave();
private:
	void onTimerCompleted(int tag);

};



}


#endif //!_AUTOCONFIRMDISPSTATE_H_