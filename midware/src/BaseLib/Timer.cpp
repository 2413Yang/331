#include <stdio.h>
#include <time.h>

#include "Timer.h"
#include "mlog.h"

#ifdef _QNX_TARGET_

namespace ZH
{
    namespace BaseLib
    {
#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL

        typedef union
        {
            struct _pulse pulse;
            /* your other message structures would go
           here too */
        } StuMessage;

        CTimer::CTimer() : m_oRecvPulseThread(this, &CTimer::TimerThread)
        {
            m_bExit = false;
            m_bPause = true;

            m_iChannelID = ChannelCreate(0);

            /* Get our priority. */
            struct sched_param stSchedParams;
            int iPrio = 10;
            if (SchedGet(0, 0, &stSchedParams) != -1)
            {
                iPrio = stSchedParams.sched_priority;
            }

            m_stEvent.sigev_notify = SIGEV_PULSE;
            m_stEvent.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,
                                                 m_iChannelID,
                                                 _NTO_SIDE_CHANNEL, 0);
            m_stEvent.sigev_priority = iPrio;
            m_stEvent.sigev_code = MY_PULSE_CODE;
            if (-1 == timer_create(CLOCK_REALTIME, &m_stEvent, &m_iTimeID))
            {
                LOGDBG("timer_create error ");
            }
        }

        CTimer::~CTimer()
        {
            m_bExit = true;
            m_oRecvPulseThread.DestroyThread();
        }

        void CTimer::TimerThread()
        {
            int iRecvID;
            StuMessage stMsg;
            while (!m_bExit)
            {
                iRecvID = MsgReceive(m_iChannelID, &stMsg, sizeof(stMsg), NULL);
                if (iRecvID == 0)
                {
                    if (stMsg.pulse.code == MY_PULSE_CODE && !m_bPause)
                    {
                        this->OnTimer();
                    }
                }
            }
            timer_delete(m_iTimeID);
        }

        void CTimer::SetTimer(int iMilliSecond)
        {
            if (iMilliSecond <= 0)
            {
                LOGDBG("Invaild timeout duration");
                return;
            }

            static struct itimerspec stTimeout;
            stTimeout.it_value.tv_sec = iMilliSecond / 1000;
            stTimeout.it_value.tv_nsec = (iMilliSecond % 1000) * 1000000;
            stTimeout.it_interval.tv_sec = iMilliSecond / 1000;
            stTimeout.it_interval.tv_nsec = (iMilliSecond % 1000) * 1000000;
            int ret = timer_settime(m_iTimeID, 0, &stTimeout, NULL);
            if (ret == -1)
            {
                LOGWAR("timer_settime error");
                return;
            }

            m_bPause = false;

            if (!m_oRecvPulseThread.IsRunning())
            {
                m_oRecvPulseThread.StartThread();
            }
        }

        void CTimer::KillTimer()
        {
            m_bPause = true;

            struct itimerspec stTimeout;
            stTimeout.it_value.tv_sec = 0;
            stTimeout.it_value.tv_nsec = 0;
            stTimeout.it_interval.tv_sec = 0;
            stTimeout.it_interval.tv_nsec = 0;
            if (-1 == timer_settime(m_iTimeID, 0, &stTimeout, NULL))
            {
                LOGWAR("timer_settime error ");
            }
        }
    } // namespace BaseLib
} // namespace ZH
#endif

#ifdef _LINUX_TARGET_
#include <condition_variable>

namespace ZH
{
    namespace BaseLib
    {
#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL
        static bool s_timerIsReady = false;
        static std::mutex s_timerLock;
        static std::condition_variable s_timerCv;
        void sig_handler(int signo)
        {
            std::unique_lock<std::mutex> guard(s_timerLock);
            s_timerIsReady = true;
            s_timerCv.notify_all();
        }

        CTimer::CTimer() : m_oRecvPulseThread(this, &CTimer::TimerThread)
        {
            m_bExit = false;
            struct sigaction act;
            memset(&act, 0, sizeof(act));
            act.sa_handler = sig_handler;
            act.sa_flags = SA_SIGINFO;

            sigemptyset(&act.sa_mask);

            if (sigaction(SIGUSR1, &act, NULL) == -1)
            {
                perror("fail to sigaction");
                exit(-1);
            }

            memset(&m_stEvent, 0, sizeof(struct sigevent));
            m_stEvent.sigev_signo = SIGUSR1;
            m_stEvent.sigev_notify = SIGEV_SIGNAL;

            if (-1 == timer_create(CLOCK_REALTIME, &m_stEvent, &m_iTimeID))
            {
                LOGDBG("timer_create error ");
            }
        }

        CTimer::~CTimer()
        {
            m_bExit = true;
            m_oRecvPulseThread.DestroyThread();
        }

        void CTimer::TimerThread()
        {
            while (!m_bExit)
            {
                {
                    std::unique_lock<std::mutex> guard(s_timerLock);
                    while (!s_timerIsReady)
                        s_timerCv.wait(guard);

                    s_timerIsReady = false;
                }

                this->OnTimer();
            }
            timer_delete(m_iTimeID);
        }

        void CTimer::SetTimer(int iMilliSecond)
        {
            if (iMilliSecond <= 0)
            {
                LOGDBG("Invaild timeout duration");
                return;
            }

            static struct itimerspec stTimeout;
            stTimeout.it_value.tv_sec = iMilliSecond / 1000;
            stTimeout.it_value.tv_nsec = (iMilliSecond % 1000) * 1000000;
            stTimeout.it_interval.tv_sec = iMilliSecond / 1000;
            stTimeout.it_interval.tv_nsec = (iMilliSecond % 1000) * 1000000;
            int ret = timer_settime(m_iTimeID, 0, &stTimeout, NULL);
            if (ret == -1)
            {
                LOGWAR("timer_settime error");
                return;
            }

            if (!m_oRecvPulseThread.IsRunning())
            {
                m_oRecvPulseThread.StartThread();
            }
        }

        void CTimer::KillTimer()
        {
            struct itimerspec stTimeout;
            stTimeout.it_value.tv_sec = 0;
            stTimeout.it_value.tv_nsec = 0;
            stTimeout.it_interval.tv_sec = 0;
            stTimeout.it_interval.tv_nsec = 0;
            if (-1 == timer_settime(m_iTimeID, 0, &stTimeout, NULL))
            {
                LOGWAR("timer_settime error ");
            }
        }
    } // namespace BaseLib
} // namespace ZH
#endif