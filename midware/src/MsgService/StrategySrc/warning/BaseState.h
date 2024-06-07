#ifndef _BASESTATE_H_
#define _BASESTATE_H_

#include <unistd.h>
#include <stdint.h>
#include <list>
#include <map>
#include <mutex>
#include "STimer.h"
#include "WarningDatadef.h"

namespace warning
{

class CWarningEngine;

class CBaseState
{
private:
	EnWarnStateID		mStateID;
public:
	CBaseState(EnWarnStateID sts);
	virtual ~CBaseState();
	virtual void enter() = 0;
	virtual void leave() = 0;

	uint16_t	getWarnMinTime();
	uint16_t	getWarnCycleTime();
	uint16_t	getWarnAutoConfirmTime();
	uint16_t	getWarnDelayCancelTime();
};





}


#endif //!_BASESTATE_H_