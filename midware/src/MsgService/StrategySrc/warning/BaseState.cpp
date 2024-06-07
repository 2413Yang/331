#include "BaseState.h"
#include "WarningDatadef.h"

namespace warning
{

CBaseState::CBaseState(EnWarnStateID sts):
mStateID(sts)
{
}

CBaseState::~CBaseState()
{
}

uint16_t CBaseState::getWarnMinTime()
{
	return WARN_MIN_DISPLAY_TIME;
}
uint16_t CBaseState::getWarnCycleTime()
{
	return WARN_CYCLE_DISPLAY_TIME;
}
uint16_t CBaseState::getWarnAutoConfirmTime()
{
	return WARN_AUTOCONFIRM_DISPLAY_TIME;
}
uint16_t CBaseState::getWarnDelayCancelTime()
{
	return WARN_DELAT_CANCEL_DISPLAY_TIME;
}

}
