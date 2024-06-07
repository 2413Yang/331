#ifndef TEMPLATE_THREAD_H
#define TEMPLATE_THREAD_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

//const int MIN_INTERNAL_TIME = 10000;
namespace ZH
{
    namespace BaseLib
    {
        template <typename TakerType>
        class CTemplateThread
        {
            typedef void (TakerType::*VerboseWork)();

        public:
            CTemplateThread(TakerType *pThreadTaker, VerboseWork pFuncVerboseWork)
                : m_pThreadTaker(pThreadTaker),
                  m_pFuncVerboseWork(pFuncVerboseWork),
                  m_bWork(false),
                  m_bRunning(false)
            {
            }

            ~CTemplateThread()
            {
                DestroyThread();
                pthread_join(m_ThreadID, NULL);
            }

            void StartThread()
            {
                if (m_bWork)
                {
                    //printf("Thread is already working \n");
                    return;
                }
                m_bWork = true;
                m_bRunning = true;

                pthread_mutex_init(&m_objThreadMutex, NULL);
                pthread_cond_init(&m_objResumeCond, NULL);

                if (0 != pthread_create(&m_ThreadID, NULL, &CTemplateThread<TakerType>::WorkInOtherThread, this))
                {
                    printf("Create thread failed \n");
                }
            }

            void DestroyThread()
            {
                if (m_bWork)
                {
                    m_bWork = false;

                    if (!IsRunning())
                    {
                        Resume();
                    }
                }
            }

            void Suspend()
            {
                pthread_mutex_lock(&m_objThreadMutex);
                if (IsRunning())
                {
                    m_bRunning = false;
                }
                pthread_mutex_unlock(&m_objThreadMutex);
            }

            void Resume()
            {
                pthread_mutex_lock(&m_objThreadMutex);
                if (!IsRunning())
                {
                    m_bRunning = true;
                    pthread_cond_signal(&m_objResumeCond);
                }
                pthread_mutex_unlock(&m_objThreadMutex);
            }

            bool IsRunning() { return m_bRunning; };
            unsigned long getID() { return m_ThreadID; }

        private:
            void DoWork()
            {
                if (NULL == m_pThreadTaker || NULL == m_pFuncVerboseWork)
                {
                    return;
                }

                while (m_bWork)
                {
                    pthread_mutex_lock(&m_objThreadMutex);
                    while (!IsRunning())
                    {
                        pthread_cond_wait(&m_objResumeCond, &m_objThreadMutex);
                    }
                    pthread_mutex_unlock(&m_objThreadMutex);

                    if (!m_bWork)
                    {
                        break;
                    }

                    (m_pThreadTaker->*m_pFuncVerboseWork)();
                }
            }

            static void *WorkInOtherThread(void *pThis)
            {
                if (NULL != pThis)
                {
                    //usleep(10000);

                    ((CTemplateThread<TakerType> *)pThis)->DoWork();

                    pthread_cancel(((CTemplateThread<TakerType> *)pThis)->m_ThreadID);
                }

                return NULL;
            };

        private:
            TakerType *m_pThreadTaker;
            pthread_t m_ThreadID;
            pthread_mutex_t m_objThreadMutex;
            pthread_cond_t m_objResumeCond;
            VerboseWork m_pFuncVerboseWork;

            bool m_bWork;
            bool m_bRunning;
        };
    } // namespace BaseLib
} // namespace ZH

#endif
