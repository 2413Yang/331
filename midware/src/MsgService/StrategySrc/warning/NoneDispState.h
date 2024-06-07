#ifndef _NONELESTATE_H_
#define _NONELESTATE_H_

#include "BaseState.h"

namespace warning
{


class CNoneDispState:public CBaseState
{
private:
	//
public:
	CNoneDispState();
	virtual ~CNoneDispState();
	virtual void enter();
	virtual void leave();
};




}


#endif //!_NONELESTATE_H_