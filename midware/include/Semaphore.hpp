
#ifndef TEMPLATESEMAPHORE__H__
#define TEMPLATESEMAPHORE__H__
#include <semaphore.h>
#include <vector>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Mutex.h"
#include "mlog.h"
#include <condition_variable>

enum class EmSemType
{
    THREAD_SEM,
    PROCESS_SEM_S,
    PROCESS_SEM_C,
};

class CSemaphore
{
public:
    CSemaphore(int count = 0, EmSemType type = EmSemType::THREAD_SEM, std::string name = "")
        : mCount(count), mType(type)
    {
        mFlag = -1, mFlagNum = 0;
        if (type == EmSemType::THREAD_SEM)
            sem_init(&mSemap, 0, 0);
        else
        {
            type == EmSemType::PROCESS_SEM_S
                ? mSemap = *sem_open(name.data(), O_CREAT, 0644, 0)
                : mSemap = *sem_open(name.data(), 0);
        }
    }
    ~CSemaphore()
    {
        if (mType == EmSemType::THREAD_SEM)
            sem_destroy(&mSemap);
        else
            sem_close(&mSemap);
    }

    void addFlag(int flag)
    {
        ZH::BaseLib::CAutoLock lock(mMtx);
        if (-1 == mFlag)
        {
            mFlag = flag;
            mFlagNum = 1;
        }
        else if (mFlag == flag)
        {
            mFlagNum++;
            if (mFlagNum >= mCount)
            {
                for (int i = 0; i < mCount; i++)
                {
                    (*this)++;
                    // LOGDBG("sam Value: %d", (*this)());
                }
                mFlag = -1;
            }
        }
    }

    void operator--(int) //wait
    {
        int rc = sem_wait(&mSemap);
        !rc ? 0 : LOGERR("sem_wait: %s", strerror(errno));
    }

    int operator()(int msecs) //wait
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        long secs = msecs / 1000;
        msecs = msecs % 1000;

        long add = 0;
        msecs = msecs * 1000 * 1000 + ts.tv_nsec;
        add = msecs / (1000 * 1000 * 1000);
        ts.tv_sec += (add + secs);
        ts.tv_nsec = msecs % (1000 * 1000 * 1000);

        int rc = sem_timedwait_monotonic(&mSemap, &ts);
        if (0 != rc)
            LOGERR("sem_timedwait_monotonic: %s, pthread id: %ld", strerror(errno), pthread_self());
        return rc;
    }

    void operator++(int)
    {
        int rc = sem_post(&mSemap);
        !rc ? 0 : LOGERR("sem_post: %s", strerror(errno));
    }

    bool tryMinus()
    {
        bool ret = false;
        if (sem_trywait(&mSemap) == 0)
            ret = true;
        return ret;
    }

    int operator()()
    {
        int value = 0;
        sem_getvalue(&mSemap, &value);
        return value;
    }

private:
    sem_t mSemap;
    int mCount;
    EmSemType mType;
    int mFlag, mFlagNum;
    ZH::BaseLib::CMutex mMtx;
};

class CSynchronize
{
public:
    CSynchronize(uint32_t num) : bReady(false), mSyncNum(num), mCount(0) {}
    void wait(void)
    {
        if ((mCount + 1) >= mSyncNum)
        {
            mCount = 0;
            std::unique_lock<std::mutex> guard(mCacheLock);
            bReady = true;
            cv.notify_all();
        }
        else
        {
            mCount++;
            std::unique_lock<std::mutex> guard(mCacheLock);
            while (!bReady)
                cv.wait(guard);
        }
    }
    bool wait(int time)
    {
        if ((mCount + 1) >= mSyncNum)
        {
            mCount = 0;
            std::unique_lock<std::mutex> guard(mCacheLock);
            bReady = true;
            cv.notify_all();
            return true;
        }
        else
        {
            mCount++;
            std::unique_lock<std::mutex> guard(mCacheLock);
            if (std::cv_status::timeout == cv.wait_for(guard, std::chrono::milliseconds(time)))
                return false;
            else
                return true;
        }
    }

private:
    bool bReady;
    uint32_t mSyncNum, mCount;
    std::mutex mCacheLock;
    std::condition_variable cv;
};

#endif /*TEMPLATESEMAPHORE__H__*/