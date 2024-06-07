#ifndef _CClusterSignalDelayProcess_H_
#define _CClusterSignalDelayProcess_H_
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include "STimer.h"
#include <mutex>
#include <functional>

using namespace ZH::BaseLib;
class CSigDelayProcessor
{
private:
	uint32_t*	mArraySaveMcuData;
	uint32_t	mArrayDataLen;
	STimer*		mTimer;
	std::function<void(uint32_t*, uint32_t)> mFuncProcessSignal;
	std::function<void(void)> mFuncCB;
	bool		mbTimerUpdateFlag;
	std::string mName;
private:
	void processMCUSignal_OnTimer();
public:
	static std::mutex	sMutex;
	CSigDelayProcessor(std::function<void(uint32_t*, uint32_t)> func,uint32_t duration = 100, std::string name = "");
	~CSigDelayProcessor();
	void updateSignal(const std::vector<uint32_t>& vecSigValue);
};



#endif //!_CClusterSignalDelayProcess_H_