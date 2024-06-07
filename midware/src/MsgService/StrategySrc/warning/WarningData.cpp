#include "WarningData.h"
#include "WarningEngine.h"

namespace warning
{

std::map<int, EmPopWarnID>	CWarningData::sMapInputWarnBitwise;
CWarningData::CWarningData(EnWarnCategory warnCategorym, EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx, char inputDataBitwise): 
	mWarnCategory(warnCategorym),
	mPopWarnID(popWarnID),
	mPowerWorkSts(workSts),
	mInputDataIndex(inputDataIdx),
	mInputDataBitwise(inputDataBitwise)
{
	if(inputDataBitwise)
	{
		int key = inputDataIdx*8 + CWarningEngine::cLowestBitmap[uint8_t(inputDataBitwise)];
		sMapInputWarnBitwise[key] = popWarnID;
	}
}

void CWarningData::triggerWarn()
{
	CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_MINDISP);
}

EmPopWarnID CWarningData::getMapWarnInputBitwise(int bitPos)
{
	if(sMapInputWarnBitwise.find(bitPos) == sMapInputWarnBitwise.end())
	{
		return EmPopWarnID::NONE;
	}
	else
	{
		return sMapInputWarnBitwise[bitPos];
	}
}

CWarningData_A::CWarningData_A(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx, char inputDataBitwise):
	CWarningData(EnWarnCategory::WARN_CATEGORY_A, popWarnID, workSts, inputDataIdx, inputDataBitwise){}

std::pair<bool,EmPopWarnID> CWarningData_A::checkWarnCancelDisplay()
{//A类报警在以下条件取消显示： 1.显满3秒
	LOG_RECORD_DEBUG("_warn_ CWarningData_A::%s line:%d\n", __func__,__LINE__);
	if(EnWarnStateID::STS_MINDISP == CWarningEngine::getInstance()->getCurrentWarnState())
	{
		return {false, EmPopWarnID::NONE};
	}
	else
	{
		return {true, CWarningEngine::getInstance()->findFirstWarning()};
	}
}

void CWarningData_A::minDispalyTimeroutProcess()
{
	//显示完3秒报警消失
	CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_NONE);
}

CWarningData_B1::CWarningData_B1(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx, char inputDataBitwise):
	CWarningData(EnWarnCategory::WARN_CATEGORY_B1, popWarnID, workSts, inputDataIdx, inputDataBitwise){}

void CWarningData_B1::minDispalyTimeroutProcess()
{
	//显示完3秒报警,进入正常显示状态
	if(false == CWarningEngine::getInstance()->getWarnTriggerSts(getPopWarnID()))
	{//报警条件取消
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_NONE);
	}
	else
	{
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_NORMALDISP);
	}
}
std::pair<bool,EmPopWarnID> CWarningData_B1::checkWarnCancelDisplay()
{
/*
B.1在以下条件取消显示：
1.报警条件消失，需显满3秒 
2.被新的B.1报警打断 
3.被A、C、D类报警打断
*/
	LOG_RECORD_DEBUG("_warn_ CWarningData_B1::%s line:%d\n", __func__,__LINE__);
	if(EnWarnStateID::STS_MINDISP == CWarningEngine::getInstance()->getCurrentWarnState())
	{
		return {false, EmPopWarnID::NONE};
	}
	else
	{
		if(false == CWarningEngine::getInstance()->getWarnTriggerSts(getPopWarnID()))
		{//报警条件取消
			return {true, CWarningEngine::getInstance()->findFirstWarning()};
		}
		EmPopWarnID breakID = EmPopWarnID::NONE;
		if(	(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_A))) ||
			(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_B1))) ||
			(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_C))) ||
			(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_D)))	)
		{//被打断
			return {true, breakID};
		}
		else
		{
			return {false, EmPopWarnID::NONE};
		}
	}
	return {false, EmPopWarnID::NONE};
}

CWarningData_B2::CWarningData_B2(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx, char inputDataBitwise):
	CWarningData(EnWarnCategory::WARN_CATEGORY_B2, popWarnID, workSts, inputDataIdx, inputDataBitwise){}

void CWarningData_B2::triggerWarn()
{
	//判断是否为轮显状态
	bool flag = CWarningEngine::getInstance()->isExistInCycleList(getPopWarnID());
	if(flag)
	{
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_CYCLEDISP);
	}
	else
	{
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_MINDISP);
	}
}
void CWarningData_B2::minDispalyTimeroutProcess()
{
	//显示完3秒报警，进入轮显状态
	if(false == CWarningEngine::getInstance()->getWarnTriggerSts(getPopWarnID()))
	{//报警条件取消
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_NONE);
	}
	else
	{
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_CYCLEDISP);
	}
}
void CWarningData_B2::cycleDispalyTimeroutProcess()
{
	CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_CYCLEDISP);
}
std::pair<bool,EmPopWarnID> CWarningData_B2::checkWarnCancelDisplay()
{
/*
B.2在以下条件取消显示：
1.报警条件消失(原表会显满3秒再消失)
2.被新的B.2报警打断   
3.被A、C、D类报警打断   
4.被B.1打断 
5.针对屏幕被文字提示全覆盖的仪表：该类提示显示3秒后自动消失，隔6秒后恢复显示，循环显示至条件不满足；
  针对屏幕未被文字提示全覆盖：该类提示一直显示至条件不满足；
*/
	
	// if(false == CWarningEngine::getInstance()->getWarnTriggerSts(getPopWarnID()))
	// {//报警条件取消
	// 	return true;
	// }
	if(EnWarnStateID::STS_MINDISP == CWarningEngine::getInstance()->getCurrentWarnState())
	{
		LOG_RECORD_DEBUG("_warn_ CWarningData_B2::%s line:%d\n", __func__,__LINE__);
		return {false, EmPopWarnID::NONE};
	}
	else
	{
		if(false == CWarningEngine::getInstance()->getWarnTriggerSts(getPopWarnID()))
		{//报警条件取消
			return {true, CWarningEngine::getInstance()->findFirstWarning()};
		}
		EmPopWarnID breakID = EmPopWarnID::NONE;
		if(	(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_A)))	||
			(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_B1)))	||
			(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_B2)))	||
			(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_C)))	||
			(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_D)))	)
		{//被打断
			return {true, breakID};
		}
		else
		{
			LOG_RECORD_DEBUG("_warn_ CWarningData_B2::%s line:%d\n", __func__,__LINE__);
			return {false, EmPopWarnID::NONE};
		}
	}
	LOG_RECORD_DEBUG("_warn_ CWarningData_B2::%s line:%d\n", __func__,__LINE__);
	return {false, EmPopWarnID::NONE};
}

CWarningData_C::CWarningData_C(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx, char inputDataBitwise):
	CWarningData(EnWarnCategory::WARN_CATEGORY_C, popWarnID, workSts, inputDataIdx, inputDataBitwise){}

void CWarningData_C::triggerWarn()
{
	//进入报警中心列表
	CWarningEngine::getInstance()->add2HistoryList(this->getPopWarnID());
	CWarningData::triggerWarn();
}
void CWarningData_C::minDispalyTimeroutProcess()
{
	//显示完3秒,进入自动确认状态
	if(false == CWarningEngine::getInstance()->getWarnTriggerSts(getPopWarnID()))
	{//报警条件取消
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_NONE);
	}
	else
	{
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_AUTOCONFIRM);
	}
}
void CWarningData_C::AutoConfirmDispalyTimeroutProcess()
{
	CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_NONE);
}
std::pair<bool,EmPopWarnID> CWarningData_C::checkWarnCancelDisplay()
{
/*
C在以下条件取消显示：
1.报警条件消失，需显满3秒；
2.被B.1打断；3.被B.2打断；4.被其他C类打断；
5.on档时，显示5S自动存储；off档时，显示5秒自动消失。
6、LCD主警告灯
当电源档位为ON档，有C类报警提示时，该指示灯点亮至无C类报警；
当电源档位为off档，有C类报警提示时，该指示灯点亮至C类报警提示取消。
*/
	LOG_RECORD_DEBUG("_warn_ CWarningData_C::%s line:%d\n", __func__,__LINE__);
	if(EnWarnStateID::STS_MINDISP == CWarningEngine::getInstance()->getCurrentWarnState())
	{
		return {false, EmPopWarnID::NONE};
	}
	else
	{
		if(false == CWarningEngine::getInstance()->getWarnTriggerSts(getPopWarnID()))
		{//报警条件取消
			return {true, CWarningEngine::getInstance()->findFirstWarning()};
		}
		EmPopWarnID breakID = EmPopWarnID::NONE;
		if(	(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_B1)))	||
			(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_B2)))	||
			(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_C)))	)
		{//被打断
			return {true, breakID};
		}
		else
		{
			return {false, EmPopWarnID::NONE};
		}
	}
	return {false, EmPopWarnID::NONE};
}

CWarningData_D::CWarningData_D(EmPopWarnID popWarnID, EnWarnWorkPowerSts workSts, uint16_t inputDataIdx, char inputDataBitwise):
	CWarningData(EnWarnCategory::WARN_CATEGORY_D, popWarnID, workSts, inputDataIdx, inputDataBitwise){}

void CWarningData_D::minDispalyTimeroutProcess()
{
	//显示完3秒报警，进入normal状态
	if(false == CWarningEngine::getInstance()->getWarnTriggerSts(getPopWarnID()))
	{//报警条件取消
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_NONE);
	}
	else
	{
		CWarningEngine::getInstance()->change2State(EnWarnStateID::STS_NORMALDISP);
	}
}
std::pair<bool,EmPopWarnID> CWarningData_D::checkWarnCancelDisplay()
{
/*
D在以下条件取消显示：
1.报警条件消失，需显满3秒  
2.被其他D类报警打断
*/
	LOG_RECORD_DEBUG("_warn_ CWarningData_D::%s line:%d\n", __func__,__LINE__);
	if(EnWarnStateID::STS_MINDISP == CWarningEngine::getInstance()->getCurrentWarnState())
	{
		return {false, EmPopWarnID::NONE};
	}
	else
	{
		if(false == CWarningEngine::getInstance()->getWarnTriggerSts(getPopWarnID()))
		{//报警条件取消
			return {true, CWarningEngine::getInstance()->findFirstWarning()};
		}
		EmPopWarnID breakID = EmPopWarnID::NONE;
		if(EmPopWarnID::NONE != (breakID = CWarningEngine::getInstance()->findFrontWarn(EnWarnCategory::WARN_CATEGORY_D)))
		{//被打断
			return {true, breakID};
		}
		else
		{
			return {false, EmPopWarnID::NONE};
		}
	}
	return {false, EmPopWarnID::NONE};
}



}