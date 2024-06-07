#include "lampStartegy.h"
#include <algorithm>
#include "mlog.h"


CLampStartegy::CLampStartegy(CDispCtrlScreenLayer *m_pDisplayCtrl, Hmi *_app) :m_pApp(_app), m_pDisplayCtrl(m_pDisplayCtrl),isTimerRun(false)
{
	LOGERR("init_map\n");
	mIndicatorStatus[LAMP_BRAKE_FLUID_LEVEL] = EmLampState::NONE;
	mIndicatorStatus[LAMP_BATTERY_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_ABS_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_EPS_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_EPB] = EmLampState::NONE;
	mIndicatorStatus[LAMP_HIGH_BEAM] = EmLampState::NONE;
	mIndicatorStatus[LAMP_LOW_BEAM] = EmLampState::NONE;
	mIndicatorStatus[LAMP_REAR_FOG] = EmLampState::NONE;
	mIndicatorStatus[LAMP_POSITION] = EmLampState::NONE;
	mIndicatorStatus[LAMP_EPB_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_AIR_BAG_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_MAIN_SEATBELT_INDICATOR] = EmLampState::NONE;
	mIndicatorStatus[LAMP_TMPS_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_READY_STATUS] = EmLampState::NONE;
	mIndicatorStatus[LAMP_CHARGECABLE_CONNECT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_CHARGE_STATUS] = EmLampState::NONE;
	mIndicatorStatus[LAMP_POWER_LIMIT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_MAIN_ALARM] = EmLampState::NONE;
	mIndicatorStatus[LAMP_LOWCHARGE] = EmLampState::NONE;
	mIndicatorStatus[LAMP_POWER_BATTERY_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_POWER_BATTERY_HIGHTEMP] = EmLampState::NONE;
	mIndicatorStatus[LAMP_DRIVE_MOTOR_FAULT] = EmLampState::NONE;
	mIndicatorStatus[LAMP_SYSTEM_FAULT_YELLOW] = EmLampState::NONE;
	mIndicatorStatus[LAMP_SYSTEM_FAULT_RED] = EmLampState::NONE;
	mIndicatorStatus[LAMP_DOOR_OPEN] = EmLampState::NONE;
	mIndicatorStatus[LAMP_BATTERY_CHARGE_HEAT] = EmLampState::NONE;
}

CLampStartegy::~CLampStartegy()
{
}


/*开启定时器*/
void CLampStartegy::startTimer(long milliseconds, KzuTimerMessageMode times, TimerFunction callBack)
{
	if (isTimerRun == false)
	{
		isTimerRun = true;
		m_TwinkleTimer = addTimerHandler(m_pApp->getDomain()->getMessageDispatcher(), kanzi::chrono::milliseconds(milliseconds), times, callBack);
	}
}

/*停止定时器*/
void CLampStartegy::stopTimer()
{
	if (NULL != m_TwinkleTimer)
	{
		isTimerRun = false;
		removeTimerHandler(m_pApp->getDomain()->getMessageDispatcher(), m_TwinkleTimer);
	}
}

void CLampStartegy::updateLampStatus(std::vector<StuTelltableLampState> lampVec)
{
	for (auto it : lampVec)
	{
		//LOGERR("------%s-----%d\n", it.strID.data(), it.state);
		auto itStatus = mIndicatorStatus.find(it.strID);
		if (itStatus != mIndicatorStatus.end() && itStatus->second == it.state)
		{
			continue;
		}
		m_pDisplayCtrl->setLampStatus(it.strID, it.state);
		mIndicatorStatus[it.strID] = it.state;
	}


    //std::vector<StuTelltableLampState> updateLampStateVec;
    /*for(auto it:lampVec)
    {
		//LOGERR("------%s-----%d\n", it.strID.data(), it.state);
		auto itStatus = mIndicatorStatus.find(it.strID);
        if(itStatus != mIndicatorStatus.end() && itStatus->second == it.state)
        {
            continue;
        }

		earseFromTwinkleVec(it.strID);//为了同步闪烁，全部清除后，再重新把闪烁加到数组里
		 switch(it.state)
        {
			case EmLampState::EXTINGUISH:
				m_pDisplayCtrl->setLampStatus(it.strID, it.state);
            break;
            case EmLampState::BRIGHT:
				m_pDisplayCtrl->setLampStatus(it.strID, it.state);
            break;
            case EmLampState::TWINKLE_0_5HZ:
                mTwinkle_0_5_HzVec.push_back(it.strID);
            break;
            case EmLampState::TWINKLE_1HZ:
                mTwinkle_1_HzVec.push_back(it.strID);
            break;
            case EmLampState::TWINKLE_2HZ:
                mTwinkle_2_HzVec.push_back(it.strID);
            break;
            case EmLampState::TWINKLE_4HZ:
                mTwinkle_4_HzVec.push_back(it.strID);
            break;
            default:
            break;
        }
        mIndicatorStatus[it.strID] = it.state;
    }

    if(mTwinkle_0_5_HzVec.size() > 0 || mTwinkle_1_HzVec.size() > 0 || mTwinkle_2_HzVec.size() > 0 || mTwinkle_4_HzVec.size() > 0)
    {
		startTimer(P_LAMP_DURATION, KZU_TIMER_MESSAGE_MODE_REPEAT, std::bind(&CLampStartegy::onTimer, this));
    }
    else
    {
		stopTimer();
    }*/
}

void CLampStartegy::earseFromTwinkleVec(string lampName)
{
    auto it  = mIndicatorStatus.find(lampName);
    if(it != mIndicatorStatus.end())
    {
        switch(it->second)
        {
            case EmLampState::TWINKLE_0_5HZ:
            {
                auto itVec = find(mTwinkle_0_5_HzVec.begin(),mTwinkle_0_5_HzVec.end(),lampName);
                if(itVec != mTwinkle_0_5_HzVec.end())
                {
                    mTwinkle_0_5_HzVec.erase(itVec);
                }
            }
            break;
            case EmLampState::TWINKLE_1HZ:
            {
                auto itVec = find(mTwinkle_1_HzVec.begin(),mTwinkle_1_HzVec.end(),lampName);
                if(itVec != mTwinkle_1_HzVec.end())
                {
                    mTwinkle_1_HzVec.erase(itVec);
                }
            }
            break;
            case EmLampState::TWINKLE_2HZ:
            {
                auto itVec = find(mTwinkle_2_HzVec.begin(),mTwinkle_2_HzVec.end(),lampName);
                if(itVec != mTwinkle_2_HzVec.end())
                {
                    mTwinkle_2_HzVec.erase(itVec);
                }
            }
            break;
            case EmLampState::TWINKLE_4HZ:
            {
                auto itVec = find(mTwinkle_4_HzVec.begin(),mTwinkle_4_HzVec.end(),lampName);
                if(itVec != mTwinkle_4_HzVec.end())
                {
                    mTwinkle_4_HzVec.erase(itVec);
                }
            }
            break;
            default:
            break;
        }
    }
}

void CLampStartegy::handleLampVec(int timeNum,int divisor,std::vector<string>* twinkleVecPtr)
{
    std::vector<StuTelltableLampState> lampVec;
    if((timeNum / divisor) % 2 == 0)
    {
        for(auto it:*twinkleVecPtr)
        {
			m_pDisplayCtrl->setLampStatus(it, EmLampState::EXTINGUISH);
        }
    }
    else if((timeNum / divisor) % 2 == 1)
    {
        for(auto it:*twinkleVecPtr)
        {
			m_pDisplayCtrl->setLampStatus(it, EmLampState::BRIGHT);
        }
    }
}

static int s_timerNum = 0;
void CLampStartegy::onTimer()
{
    if(mTwinkle_0_5_HzVec.empty() == false)
    {
        handleLampVec(s_timerNum,4,&mTwinkle_0_5_HzVec);
    }

    if(mTwinkle_1_HzVec.empty() == false)
    {
        handleLampVec(s_timerNum,2,&mTwinkle_1_HzVec);
    }

    if(mTwinkle_2_HzVec.empty() == false)
    {
        handleLampVec(s_timerNum,1,&mTwinkle_2_HzVec);
    }

    if(mTwinkle_4_HzVec.empty() == false)
    {
        handleLampVec(s_timerNum,1,&mTwinkle_4_HzVec);
    }
    s_timerNum++;
}

