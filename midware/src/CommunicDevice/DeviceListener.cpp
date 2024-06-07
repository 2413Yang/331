
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include "DeviceListener.h"

CSelectListener::CSelectListener()
{
    mPairPipeFd[0] = 0, mPairPipeFd[1] = 0;
    if (pipe(mPairPipeFd))
        LOGERR("pipe error: %s", strerror(errno));
}
CSelectListener::~CSelectListener()
{
    close(mPairPipeFd[0]), close(mPairPipeFd[1]);
}

CSelectListener &CSelectListener::getInstance()
{
    static CSelectListener om;
    return om;
}

int CSelectListener::operator()(CCommunicDevice *tmpIt)
{
    int iDevFd = -1;
    std::lock_guard<std::mutex> guard(mLock);
    for (auto it : mFdTable)
    {
        if (it.second == tmpIt)
        {
            iDevFd = it.first;
            break;
        }
    }
    return iDevFd;
}

std::vector<CSelectListener::HandleType> CSelectListener::operator()()
{
    fd_set mReadFds;
    int mRfdMax = 0, ret = 0;
    std::vector<HandleType> retTable;
    FD_ZERO(&mReadFds);
    {
        mRfdMax = mPairPipeFd[0];
        FD_SET(mPairPipeFd[0], &mReadFds);
        std::lock_guard<std::mutex> guard(mLock);
        for (auto it : mFdTable)
        {
            FD_SET(it.first, &mReadFds);
            (it.first > mRfdMax) ? (mRfdMax = it.first) : 0;
        }
    }

    if (mRfdMax <= 0)
        return retTable;

    if ((ret = select(mRfdMax + 1, &mReadFds, NULL, NULL, NULL)) > 0)
    {
        if (FD_ISSET(mPairPipeFd[0], &mReadFds))
        {
            char pipeBuf[30] = {0};
            read(mPairPipeFd[0], pipeBuf, sizeof(pipeBuf));
        }
        std::lock_guard<std::mutex> guard(mLock);
        for (auto it : mFdTable)
            if (FD_ISSET(it.first, &mReadFds))
                retTable.push_back(it);
    }
    else if (ret <= 0)
    {
        LOGERR("select()=-1 mRfdMax=%d pid=%d; %s", mRfdMax, getpid(), strerror(errno));
        usleep(100000);
    }
    return retTable;
}

void CSelectListener::operator<<(const HandleType &fds)
{
    std::lock_guard<std::mutex> guard(mLock);
    for (auto it : mFdTable)
    {
        if (fds.first == it.first || fds.first == -1)
        {
            LOGERR("Add repeated handle. [%d]", fds.first);
            return;
        }
    }
    mFdTable.push_back(fds);
}

void CSelectListener::operator>>(const HandleType &fds)
{
    std::lock_guard<std::mutex> guard(mLock);
    for (auto it = mFdTable.begin(); it != mFdTable.end(); it++)
    {
        if (it->first == fds.first)
        {
            LOGDBG("del fd from select table: %d .", fds.first);
            it = mFdTable.erase(it);
        }
        if (it == mFdTable.end())
            break;
    }
}

int32_t CSelectListener::operator[](const CCommunicDevice *ptrCore)
{
    int32_t fd = -1;
    std::lock_guard<std::mutex> guard(mLock);
    for (auto it = mFdTable.begin(); it != mFdTable.end(); it++)
    {
        if (it->second == ptrCore)
        {
            fd = it->first;
            break;
        }
    }
    return fd;
}

void CSelectListener::reset()
{
    int ret = write(mPairPipeFd[1], "notify", 10);
    if(ret <= 0)
    {
        LOGERR("interrupt select failed. : %s", strerror(errno));
    }
    //(ret > 0) ? 0 : LOGERR("interrupt select failed. : %s", strerror(errno));
}