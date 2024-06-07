#ifndef _SIMPLETIMER_H_
#define _SIMPLETIMER_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <list>
#include <time.h>
#include "mlog.h"
#include <mutex>
#include "Application.h"

namespace ZH
{
namespace BaseLib
{

template <typename T>
class CTimerManager;

template <typename T>
class CSimpleTimer
{
public:
    friend CTimerManager<T>;
    typedef void (T::*ptfOnTimer)();
public:
    CSimpleTimer(uint32_t durationMs, ptfOnTimer pfunc, T* pUser, bool cycleFalg = false)
    {
        this->m_startTime = 0;
        this->m_duration = durationMs;
        this->m_status = timer_stop;
        this->pfunOntimer = pfunc;
        this->m_User = pUser;
        this->mCycleFlag = cycleFalg;
    }
    ~CSimpleTimer() {stop();};
    void start()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if((nullptr == this->pfunOntimer) || (timer_running == this->m_status))
        {
            LOGERR("## Error...start(), m_status = %d", this->m_status);
            return;
        }
        this->m_status = timer_running;
        this->m_startTime = getWorldTimeMS();
        //LOGDBG("m_startTime = %d", m_startTime);
    }
    void start_resetTimer(uint32_t duration)
    {
        do
        {
            std::lock_guard<std::mutex> lock(mMutex);
            this->m_duration = duration;
        } while (0);
        
        this->start();
    }
    void stop(){ std::lock_guard<std::mutex> lock(mMutex); this->m_status = timer_stop;}
    
    void restart(){ stop(); start();};
    void restart_resetTimer(uint32_t duration){ 
        do
        {
            std::lock_guard<std::mutex> lock(mMutex);
            m_duration = duration;
        } while (0);
        restart();
    }
    simpleTimerStatus getStatus(){ std::lock_guard<std::mutex> lock(mMutex); return m_status;}
private:
    void setStatus(simpleTimerStatus sts){ std::lock_guard<std::mutex> lock(mMutex); m_status = sts;}
    uint32_t getExpireTime(){std::lock_guard<std::mutex> lock(mMutex); return (m_startTime + m_duration);}
    void operator()()
    {
        if(nullptr == this->pfunOntimer)
        {
            return;
        }
        (m_User->*pfunOntimer)();
    }
    
private:
    T*                  m_User;
    uint32_t            m_startTime;
    uint32_t            m_duration;
    simpleTimerStatus   m_status;
    ptfOnTimer		    pfunOntimer;
    std::mutex          mMutex;
    bool                mCycleFlag;
};

template<typename T>
class CTimerManager
{
public:
    CTimerManager(){};
    ~CTimerManager(){};
    void addNode(CSimpleTimer<T>* pTimerObj){
		if(nullptr == pTimerObj)
		{
			return;
		}
		std::lock_guard<std::mutex> lock(mMutex2); 
		this->m_list.push_back(pTimerObj);
	}
    int32_t removeNode(CSimpleTimer<T>* pTimerObj)
    {
        std::lock_guard<std::mutex> lock(mMutex2);
        int32_t ret = -1;
        for(auto ite = this->m_list.begin(); ite != m_list.end(); ite++)
        {
            if( (*ite) == pTimerObj)
            {
                ret = 0;
                ite = this->m_list.erase(ite);
                break;
            }
        }
        return ret;
    }
    void MainCheckTimerOut()
    {//循环调用
        uint32_t now = getWorldTimeMS();
        std::lock_guard<std::mutex> lock(mMutex2);
        for(auto ite = this->m_list.begin(); ite != m_list.end(); ++ite)
        {
            CSimpleTimer<T>* pSimpleTimer = *ite;
            if(timer_running == pSimpleTimer->getStatus())
            {
                uint32_t expiredTimeMs = pSimpleTimer->getExpireTime();
                if(expiredTimeMs < now)
                {
                    simpleTimerExpire(pSimpleTimer);
                }
            }
        }
    }
private:
    void simpleTimerExpire(CSimpleTimer<T>* pTimerObj)
    {
        if(timer_running == pTimerObj->getStatus())
        {
            if(pTimerObj->mCycleFlag)
            {
                pTimerObj->restart();
            }
            else
            {
                pTimerObj->setStatus(timer_expired);
            }
            (*pTimerObj)();
        }
    }
    std::list<CSimpleTimer<T>*>      m_list;
    std::mutex          mMutex2;
};

}//namespace BaseLib
}//namespace ZH
#endif //_SIMPLETIMER_H_