#ifndef _WARNTIME_H_
#define _WARNTIME_H_
#include <mutex>
#include "Application.h"
#include <list>
#include <thread>
#include <atomic>
#include <functional>

namespace ZH
{
namespace BaseLib
{


// enum class STimerStatus
// {
//     TIMERSTS_STOP,
//     TIMERSTS_RUNNING,
//     TIMERSTS_PAUSE,
//     TIMERSTS_EXPIRED,
// };
class STimerManager;

typedef std::function<void(int)> pfuncOnTimer;

class STimer
{
public:
	friend STimerManager;
public:
	STimer(uint32_t durationMs, pfuncOnTimer pfunc,int tag = 0, bool cycleFalg = false);
	~STimer();

	void start();
	void start_resetTimer(uint32_t duration);
	void stop();
	void restart();
	void restart_resetTimer(uint32_t duration);
	simpleTimerStatus getStatus();
private:
	void setStatus(simpleTimerStatus sts);
	uint32_t getExpireTime();
	void operator()();
private:
    uint32_t            mStartTime;
    uint32_t            mDuration;
    simpleTimerStatus	mStatus;
	pfuncOnTimer		mFuncCb;
	int					mTag;
    std::mutex          mMutex;
    bool                mCycleFlag;
};

class STimerManager
{
private:
	STimerManager();
    void simpleTimerExpire(STimer* pTimerNode);
	void MainCheckTimerOut();
public:
	static STimerManager* getInstance();
	~STimerManager();
	void addTimerNode(STimer* pNode);
	bool removeTimerNode(STimer* pNode);
private:
	std::list<STimer*>	m_list;
    std::mutex          mMutex2;
	bool				mThreadExitFlag;
	std::thread			mThread;
	static std::atomic<STimerManager*>	spAtoTimerManager;
	static std::mutex          sMutex;
};

}}

#endif //!_WARNTIME_H_
