#ifndef _WARNINGINPUTOUTPUT_H_
#define _WARNINGINPUTOUTPUT_H_
#include "WarningDatadef.h"
#include <functional>

namespace warning
{
class CWarningInputOutput
{
private:
	char			mMCUInputWarnData[8];
	uint8_t			mPreEpbPopsup;
	const static EmPopWarnID	cVcuInfoDispPops[0x1E];//用信号值查表确定触发弹窗的id
	const static EmPopWarnID	cEpbTxtDispPops[0x06];//用信号值查表确定触发弹窗的id
	const static uint8_t		cEPB_PriorityList[7];
public:
	CWarningInputOutput();
	~CWarningInputOutput();
	void registerChangeWarn(std::function<void(EmPopWarnID)> cb);
	void registeNotifyHistoryWarnList(std::function<void(std::vector<EmPopWarnID>&)> cb);
	void warningInputData(const char* inputData);
	void setCurrentPowerMode(const int curPowerSts);
	void setHmiReady();

	char getDoorSts();
private:
	void processWarnInputBitwise(const char* xorValues, const char* inputValue, int dataIndex);
};

}

#endif //!_WARNINGINPUTOUTPUT_H_