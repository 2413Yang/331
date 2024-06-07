#ifndef _CYCLESTATE_H_
#define _CYCLESTATE_H_

#include "BaseState.h"

namespace warning
{


class CCycleDispState:public CBaseState
{
private:
	ZH::BaseLib::STimer*	mCycleDispTimer;
public:
	CCycleDispState();
	virtual ~CCycleDispState();
	virtual void enter();
	virtual void leave();
private:
	void onTimerOut(int tag);

};




}


#endif //!_CYCLESTATE_H_