#ifndef Mutex_H
#define Mutex_H

#include <pthread.h>

namespace ZH
{
    namespace BaseLib
    {
        class CMutex
        {
        public:
            CMutex();
            virtual ~CMutex();
            virtual void Lock();
            virtual void Unlock();

        private:
            pthread_mutex_t m_oMutex;
        };

        class CAutoLock
        {
        public:
            CAutoLock(CMutex &oMutex) : m_pMutex(&oMutex) { m_pMutex->Lock(); }
            ~CAutoLock() { m_pMutex->Unlock(); }

        private:
            CMutex *m_pMutex;
        };
    } // namespace BaseLib
} // namespace ZH

#endif