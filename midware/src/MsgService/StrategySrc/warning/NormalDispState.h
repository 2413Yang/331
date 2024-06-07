#ifndef _NORMALDISPSTATE_H_
#define _NORMALDISPSTATE_H_

#include "BaseState.h"

namespace warning
{


class CNormalDispState:public CBaseState
{
private:
	//
public:
	CNormalDispState();
	virtual ~CNormalDispState();
	virtual void enter();
	virtual void leave();

	

};



}


#endif //!_NORMALDISPSTATE_H_
