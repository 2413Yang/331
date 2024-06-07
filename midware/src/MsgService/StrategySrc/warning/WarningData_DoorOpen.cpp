#include "WarningData_DoorOpen.h"
#include "WarningEngine.h"

using namespace warning;
CWarningData_DoorOpen::CWarningData_DoorOpen(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx, char inputDataBitwise):
	CWarningData_B2(popWarnID, workSts)
{}

std::pair<bool,EmPopWarnID> CWarningData_DoorOpen::checkWarnCancelDisplay()
{
	return CWarningData_B2::checkWarnCancelDisplay();
}

EmPopWarnID CWarningData_DoorOpen::getDisplayPopWarnID()
{
	char doorOpenSts = CWarningEngine::getInstance()->getDoorOpenSts();
	
	LOG_RECORD_DEBUG("_warn_%s line:%d,doorOpenSts=0x%02x\n", __func__,__LINE__, doorOpenSts);
	if(CWarningEngine::getInstance()->getCurrentDisplayPopWarn() == EmPopWarnID::DOOR_OPEN)
	{
		uint32_t retCount = 0;
		doorOpenSts &= 0x0f;
		char tempDoorOpenSts = doorOpenSts;
		while (tempDoorOpenSts)
		{
			++retCount;
			tempDoorOpenSts &=(tempDoorOpenSts - 1);
		}
		LOG_RECORD_DEBUG("_warn_%s line:%d,retCount=%d\n", __func__,__LINE__,retCount);
		if(retCount == 0)
		{
			return EmPopWarnID::NONE;
		}
		else if(retCount == 1)
		{
			LOG_RECORD_DEBUG("_warn_%s line:%d,cLowestBitmap[doorOpenSts]=0x%02x\n", __func__,__LINE__,CWarningEngine::cLowestBitmap[uint8_t(doorOpenSts)]);
			return cDoorOpenPopIdList[CWarningEngine::cLowestBitmap[uint8_t(doorOpenSts)] % GET_ARRAY_ELEMENT_SIZE(cDoorOpenPopIdList)];
		}
		else
		{
			return EmPopWarnID::DOOR_OPEN;
		}
	}
	else
	{
		return CWarningEngine::getInstance()->getCurrentDisplayPopWarn();
	}
	return EmPopWarnID::NONE;
}

const EmPopWarnID CWarningData_DoorOpen::cDoorOpenPopIdList[4] = {
	EmPopWarnID::LEFT_FRONT_DOOR_OPEN,	//左前车门打开
	EmPopWarnID::RIGHT_FRONT_DOOR_OPEN,	//右前车门打开
	EmPopWarnID::LEFT_REAR_DOOR_OPEN,	//左后车门打开
	EmPopWarnID::RIGHT_REAR_DOOR_OPEN,	//右后车门打开
};