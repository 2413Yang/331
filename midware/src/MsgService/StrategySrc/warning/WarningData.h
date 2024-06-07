#ifndef _WARNINGDATA_H_
#define _WARNINGDATA_H_
#include "WarningDatadef.h"
//#include "BaseState.h"
#include <map>

namespace warning
{

class CWarningData
{
private:
protected:
	EnWarnCategory	mWarnCategory;
	EmPopWarnID		mPopWarnID;
	EnWarnWorkPowerSts	mPowerWorkSts;
	uint16_t		mInputDataIndex;
	char			mInputDataBitwise;
	static	std::map<int, EmPopWarnID>	sMapInputWarnBitwise;//输入bit位对应的弹窗id的映射
public:
	CWarningData(EnWarnCategory warnCategorym, EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx, char inputDataBitwise);
	virtual ~CWarningData(){}

	EnWarnCategory getWarnCategory(void){return mWarnCategory;}
	EmPopWarnID getPopWarnID(void){ return mPopWarnID;}
	EnWarnWorkPowerSts getWorkPowerSts(void){ return mPowerWorkSts;}

	static EmPopWarnID getMapWarnInputBitwise(int bitpos);

	virtual void triggerWarn();
	virtual void minDispalyTimeroutProcess() = 0;
	virtual void cycleDispalyTimeroutProcess(){}
	virtual void AutoConfirmDispalyTimeroutProcess(){}
	virtual EmPopWarnID getDisplayPopWarnID() {return mPopWarnID;}

	virtual std::pair<bool,EmPopWarnID> checkWarnCancelDisplay() = 0;
};

class CWarningData_A:public CWarningData
{
private:
	//
public:
	CWarningData_A(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx = 0, char inputDataBitwise = 0x00);
	virtual ~CWarningData_A(){}

	virtual void minDispalyTimeroutProcess();

	virtual std::pair<bool,EmPopWarnID> checkWarnCancelDisplay();
};

class CWarningData_B1:public CWarningData
{
private:
	//
public:
	CWarningData_B1(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx = 0, char inputDataBitwise = 0x00);
	virtual ~CWarningData_B1(){}

	virtual void minDispalyTimeroutProcess();

	virtual std::pair<bool,EmPopWarnID> checkWarnCancelDisplay();
};

class CWarningData_B2:public CWarningData
{
private:
	//
public:
	CWarningData_B2(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx = 0, char inputDataBitwise = 0x00);
	virtual ~CWarningData_B2(){}
	
	virtual void triggerWarn();
	virtual void minDispalyTimeroutProcess();
	virtual void cycleDispalyTimeroutProcess();

	virtual std::pair<bool,EmPopWarnID> checkWarnCancelDisplay();
};

class CWarningData_C:public CWarningData
{
private:
	//
public:
	CWarningData_C(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx = 0, char inputDataBitwise = 0x00);
	virtual ~CWarningData_C(){}
	virtual void triggerWarn();
	virtual void minDispalyTimeroutProcess();
	virtual void AutoConfirmDispalyTimeroutProcess();

	virtual std::pair<bool,EmPopWarnID> checkWarnCancelDisplay();
};

class CWarningData_D:public CWarningData
{
private:
	//
public:
	CWarningData_D(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx = 0, char inputDataBitwise = 0x00);
	virtual ~CWarningData_D(){}

	virtual void minDispalyTimeroutProcess();

	virtual std::pair<bool,EmPopWarnID> checkWarnCancelDisplay();
};

}

#endif//!_WARNINGDATA_H_

