#ifndef _LAMPSTARTEGY_H_
#define _LAMPSTARTEGY_H_
#include "WarnIdDefine.h"
#include "DataDefine.h"
#include <unistd.h>
#include <stdint.h>
#include <map>
#include <mutex>
#include <vector>
#include "kanziUtil.h"
#include <kanzi/kanzi.hpp>
#include "DisplayCtrl.h"

using namespace kanzi;

//警告灯触发时间周期
#define P_LAMP_DURATION  (250)

class CLampStartegy
{
public:
	CLampStartegy(CDispCtrlScreenLayer *m_pDisplayCtrl,Hmi *_app);
    virtual ~CLampStartegy();
    void updateLampStatus(std::vector<StuTelltableLampState>);

private:

	void handleLampVec(int timeNum, int divisor, std::vector<string>*);
	void earseFromTwinkleVec(string);
	//void pushInTwinkleVec(std::vector<string>*,string);
	void startTimer(long milliseconds, KzuTimerMessageMode times, kanzi::TimerFunction callBack);
	void stopTimer();
	void onTimer();


private:
    map<string, EmLampState> mIndicatorStatus;
	kanzi::TimerSubscriptionToken m_TwinkleTimer;
	Hmi *m_pApp;
	CDispCtrlScreenLayer *m_pDisplayCtrl;

	bool isTimerRun;

    std::vector<string> mTwinkle_0_5_HzVec;
    std::vector<string> mTwinkle_1_HzVec;
    std::vector<string> mTwinkle_2_HzVec;
    std::vector<string> mTwinkle_4_HzVec;
};
    

#endif //_LAMPSTARTEGY_H_