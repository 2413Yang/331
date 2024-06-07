#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "Application.h"
#include "mlog.h"

namespace ZH
{
    namespace BaseLib
    {
        CApplication::CApplication()
        {
        }

        CApplication::~CApplication()
        {
        }

        void CApplication::Start(bool bBlock)
        {
            Init();
            DoWork();

            if (bBlock)
            {
                sigset_t stNewSignalSet;
                sigset_t stOldSignalSet;
                sigfillset(&stNewSignalSet);
                pthread_sigmask(SIG_SETMASK, &stNewSignalSet, &stOldSignalSet);
                sigemptyset(&stNewSignalSet);
                sigaddset(&stNewSignalSet, SIGINT);
                sigaddset(&stNewSignalSet, SIGTERM);
                pthread_sigmask(SIG_SETMASK, &stOldSignalSet, NULL);

                LOGDBG("process block ...pid:%d", getpid());
                int iSigValue = 0;
                sigwait(&stNewSignalSet, &iSigValue);
            }

            LOGDBG("process start exit  ...");
            UInit();
        }
        uint32_t getWorldTimeMS()
        {
            struct timespec t;
            clock_gettime(CLOCK_MONOTONIC, &t);
            uint32_t currentTime = (uint32_t) (t.tv_nsec / (1000 * 1000) + ((uint64_t)t.tv_sec  * 1000));
            return currentTime;
        }
    } // namespace BaseLib
} // namespace ZH