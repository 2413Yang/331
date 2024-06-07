#ifndef _MINDISPSTATE_H_
#define _MINDISPSTATE_H_

#include "BaseState.h"
#include "STimer.h"

namespace warning
{

class CMinDispState:public CBaseState
{
private:
	ZH::BaseLib::STimer*		mMinDispTimer;
public:
	CMinDispState();
	virtual ~CMinDispState();
	virtual void enter();
	virtual void leave();
private:
	void onTimerCompleted(int tag);

};



}


#endif //!_MINDISPSTATE_H_