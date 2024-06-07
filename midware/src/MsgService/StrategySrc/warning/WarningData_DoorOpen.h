#ifndef _WARNINGDATADOOROPEN_H_
#define _WARNINGDATADOOROPEN_H_
#include "WarningData.h"

namespace warning
{

class CWarningData_DoorOpen:public CWarningData_B2
{
private:
	const static EmPopWarnID cDoorOpenPopIdList[4];
public:
	CWarningData_DoorOpen(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx = 0, char inputDataBitwise = 0x00);
	virtual ~CWarningData_DoorOpen(){}

	virtual std::pair<bool,EmPopWarnID> checkWarnCancelDisplay();
	virtual EmPopWarnID getDisplayPopWarnID();
};

}

#endif//!_WARNINGDATADOOROPEN_H_

