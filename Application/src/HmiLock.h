
#ifndef HMILOCK__H_
#define HMILOCK__H_

#include <kanzi/kanzi.hpp>
#include "Mutex.h"

class CHmiLock
{
public:
    CHmiLock(KzsThreadLock *Lock)
        : pLock(Lock)
    {
        if (pLock != NULL)
            kzsThreadLockAcquire(pLock);
    }

    ~CHmiLock()
    {
        if (pLock != NULL)
            kzsThreadLockRelease(pLock);
    }

private:
    KzsThreadLock *pLock;
};

class CHmiMutex : public ZH::BaseLib::CMutex
{
public:
    CHmiMutex(KzsThreadLock *lock) : pLock(lock) {}
    virtual ~CHmiMutex() {}
    virtual void Lock()
    {
        if (pLock != NULL)
            kzsThreadLockAcquire(pLock);
    }
    virtual void Unlock()
    {
        if (pLock != NULL)
            kzsThreadLockRelease(pLock);
    }

private:
    KzsThreadLock *pLock;
};
#endif /*HMILOCK__H__*/