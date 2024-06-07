
#ifndef DEVICELISTENER__H__
#define DEVICELISTENER__H__

#include <vector>
#include <condition_variable>
#include <string.h>
#include <errno.h>
#include "CommunicDevice.h"
#include "mlog.h"

class CSelectListener
{
    CSelectListener();
    ~CSelectListener();

public:
    typedef std::pair<int, CCommunicDevice *> HandleType;

    static CSelectListener &getInstance();
    int operator()(CCommunicDevice *tmpIt);
    std::vector<HandleType> operator()();
    void operator<<(const HandleType &fds);
    void operator>>(const HandleType &fds);
    int32_t operator[](const CCommunicDevice *ptrCore);
    void reset();

private:
    std::mutex mLock;
    int mPairPipeFd[2];
    std::vector<HandleType> mFdTable;
};

#endif /*DEVICELISTENER__H__*/