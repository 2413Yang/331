#ifndef _WARNDATADEF_H_
#define _WARNDATADEF_H_
#include "hmi/WarnIdDefine.h"
#include "../../mylogCtrl.h"
#include "LogServiceApi.h"

namespace warning
{

#define WARN_MIN_DISPLAY_TIME			(3*1000)
#define WARN_CYCLE_DISPLAY_TIME			(3*1000)
#define WARN_AUTOCONFIRM_DISPLAY_TIME	(5*1000 - WARN_MIN_DISPLAY_TIME)
#define WARN_DELAT_CANCEL_DISPLAY_TIME	(3*1000)

#define GET_ARRAY_ELEMENT_SIZE(arrayData)	(sizeof(arrayData) / sizeof((arrayData)[0]))	

enum EnWarnStateID
{
	STS_NONE,		//无显示
	STS_MINDISP,	//最小显示
	STS_CYCLEDISP,	//循环显示
	STS_NORMALDISP,	//显满3秒最小显示后，一直显示，直到条件解除
	STS_AUTOCONFIRM,//显示完最小显示时间，进入自动确认状态
	STS_DELAYCANCEL,//门开报警取消后继续显示3秒才退出
	STS_LAST
};

enum class EnWarnCategory
{
	WARN_CATEGORY_A,
	WARN_CATEGORY_B1,
	WARN_CATEGORY_B2,
	WARN_CATEGORY_C,
	WARN_CATEGORY_D,
	WARN_CATEGORY_END
};

enum class EnWarnWorkPowerSts
{
	PM_IGN_UNKNOWN,
	PM_IGN_OFF = 1,
	PM_IGN_ON = 2,
	PM_IGN_ALL = (PM_IGN_OFF | PM_IGN_ON),
};


}


#endif //!_WARNDATADEF_H_