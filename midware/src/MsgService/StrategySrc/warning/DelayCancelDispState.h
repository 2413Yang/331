#ifndef _DELAYCANCELDISPSTATE_H_
#define _DELAYCANCELDISPSTATE_H_

#include "BaseState.h"
#include "STimer.h"

namespace warning
{

class CDelayCancelDispState:public CBaseState
{
private:
	ZH::BaseLib::STimer*		mDelayCancelDispTimer;
public:
	CDelayCancelDispState();
	virtual ~CDelayCancelDispState();
	virtual void enter();
	virtual void leave();
private:
	void onTimerCompleted(int tag);

};



}


#endif //!_DELAYCANCELDISPSTATE_H_