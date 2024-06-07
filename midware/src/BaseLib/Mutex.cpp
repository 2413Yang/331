#include "Mutex.h"

namespace ZH
{
    namespace BaseLib
    {
        CMutex::CMutex()
        {
            pthread_mutex_init(&m_oMutex, NULL);
        }

        CMutex::~CMutex()
        {
            pthread_mutex_destroy(&m_oMutex);
        }

        void CMutex::Lock()
        {
            pthread_mutex_lock(&m_oMutex);
        }

        void CMutex::Unlock()
        {
            pthread_mutex_unlock(&m_oMutex);
        }
    } // namespace BaseLib
} // namespace ZH
