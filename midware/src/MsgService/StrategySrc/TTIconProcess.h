#ifndef _TTICONSELFCHECK_H_
#define _TTICONSELFCHECK_H_
#include <list>
#include <map>
#include "../mylogCtrl.h"
#include "STimer.h"
#include <functional>
#include <DataDefine.h>
#include <vector>

using namespace ZH::BaseLib;
class CTTIconProcess
{
public:
	enum class EnTTSelfCheckType
	{
		NONE,
		INTERNAL,
		EXTERNAL,
	};
	enum class EnTTSelfCheckStatus
	{
		STS_NONE,
		STS_SELFCHECKING,
		STS_SELFCHECK_FINISH,
	};
private:
	
	struct stTTSelfCheckInfo
	{
		EnTTSelfCheckType	type;
		uint32_t			mcuValue;
	};
	std::map<std::string, stTTSelfCheckInfo> mTTMap;
	STimer*				mTimer;
	EnTTSelfCheckStatus	mSelfCheckStatus;
	int					mLastPowerMode;
	std::function<void(void)> mSelfCheckChangeStsNotify;
	bool				mbHmiPageReady;
private:
	void TTIconSelfCheck_OnTimer();
	void updateHMIICon();
public:
	CTTIconProcess();
	~CTTIconProcess();

	void setTTData(const std::vector<unsigned int>& ttValue);
	void setTTData(const uint32_t* ttValue, uint32_t len);
	void setCallBack(std::function<void(void)> cb) {mSelfCheckChangeStsNotify = cb;}
	void setPowerMode(int mode);
	EnTTSelfCheckStatus getSelfCheckStatus(){return mSelfCheckStatus;}
	EnTTSelfCheckType getSelfCheckTpye(std::string Lamp);
	void setHMIPageReady();

	void setTTValue(std::string Lamp, uint32_t value);
};



#endif //!_TTICONSELFCHECK_H_