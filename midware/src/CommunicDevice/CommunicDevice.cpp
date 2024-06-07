
#include "CommunicDevice.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "mlog.h"
#include "DeviceListener.h"

std::thread *CCommunicDevice::pProducer = NULL, //生产者
    *CCommunicDevice::pConsumer = NULL;         //消费者
ZH::BaseLib::CTemplateCache<std::function<void(void)>> CCommunicDevice::taskQueue;

typedef std::pair<int, CCommunicDevice *> HandleType;

//CCommunicDevice::threadManger*	CCommunicDevice::sProducer = nullptr;
//CCommunicDevice::threadManger*	CCommunicDevice::sConsumer = nullptr;
CCommunicDevice::CCommunicDevice(unsigned size) : mRemainderFrame("")
{
    mSize = size;
    pReadCache = new (std::nothrow) unsigned char[mSize];
debugId = 0;
    std::lock_guard<std::mutex> guard(mLock);
    if (NULL == pProducer)
    {
        auto listenRead = []() -> void
        {
            while (true)
            {
                auto table = CSelectListener::getInstance()();
                for (auto it : table)
                {
                    if (NULL != it.second)
                        it.second->handleEvent(it.first);
                    else
                        LOGERR("file handle corresponds to an empty object.");
                }
                sched_yield();
            }
        };
        pProducer = new std::thread(listenRead);
        if (NULL != pProducer)
            pProducer->joinable() ? pProducer->detach() : void(0);
        else
            LOGERR("producer thread creation failed.");
    }

    if (NULL == pConsumer)
    {
        auto actuator = [this]() -> void
        {
            std::function<void(void)> task;
            while (true)
            {
                taskQueue.wait();
                while (taskQueue.pull(task))
                    task();
                sched_yield();
            }
            LOGERR("");
        };
        pConsumer = new std::thread(actuator);
        if (NULL != pConsumer)
            pConsumer->joinable() ? pConsumer->detach() : void(0);
        else
            LOGERR("consumer thread creation failed.");
    }
	#if 0
	if(nullptr == sProducer)
	{
		auto listenRead = [this]() -> void
        {
            while (true)
            {
                auto table = CSelectListener::getInstance()();
                for (auto it : table)
                {
                    if (NULL != it.second)
                        it.second->handleEvent(it.first);
                    else
                        LOGERR("file handle corresponds to an empty object.");
                }
                sched_yield();
            }
        };
		sProducer = new threadManger(listenRead);
	}
	else
	{
		sProducer->increaseCount();
	}
	if(nullptr == sConsumer)
	{
		auto actuator = [this]() -> void
        {
            std::function<void(void)> task;
            while (true)
            {
                taskQueue.wait();
                while (taskQueue.pull(task))
                    task();
                sched_yield();
            }
            LOGERR("");
        };
		sConsumer = new threadManger(actuator);
	}
	else
	{
		sConsumer->increaseCount();
	}
	#endif
}

CCommunicDevice::~CCommunicDevice()
{
	#if 0
	if(sProducer)
	{
		if(sProducer->decreaseCount() == 0)
		{
			delete sProducer;
			sProducer = nullptr;
		}
	}
	#endif
    if (pReadCache)
        delete[] pReadCache;
    pReadCache = NULL;
}
#if 0
CCommunicDevice::threadManger::threadManger(std::function<void (void)> threadFIn):
mThreadExitFlag(false),mCount(0)
{
	std::lock_guard<std::mutex> lock(mMutex); 
	mThrad = new std::thread(threadFIn);
}
CCommunicDevice::threadManger::~threadManger()
{
	std::lock_guard<std::mutex> lock(mMutex); 
	mThreadExitFlag = true;
	mThrad->join();
}
#endif
uint32_t CCommunicDevice::getMsgCount()
{
	return taskQueue.getSize();
}
void CCommunicDevice::handleEvent(int fd)
{
    if (!pReadCache)
    {
        LOGERR("read cache is nullptr.");
        return;
    }
    memset(pReadCache, 0, mSize);
    int n = read(fd, pReadCache, mSize);
    if (n <= 0)
    {
        LOGERR("device data read error [%d]. %s", n, strerror(errno));
        return;
    }
    if(debugId == 1)
    {
        //LOGINF("###..n = %d, taskQueue.size = %u", n, taskQueue.getSize());
    }
    // LOGHEX(pReadCache, n, "read-frame");
    auto task = std::bind(&CCommunicDevice::loopCb, this, std::string((char *)pReadCache, n));
    taskQueue.push(task);
}

void CCommunicDevice::send(std::string str)
{
    int iDevFd = CSelectListener::getInstance()(this);
    int n = write(iDevFd, str.data(), str.length());
    if (n <= 0)
    {
        LOGERR("[iDevFd: %d ] Data transmission failed. %s", iDevFd, strerror(errno));
        LOGHEX(str.data(), str.length(), "SEND FAILED");
    }
}

void CCommunicDevice::send(uint8_t *buf, uint32_t length)
{
    int iDevFd = CSelectListener::getInstance()(this);
    int n = write(iDevFd, buf, length);
    if (n <= 0)
    {
        LOGERR("[iDevFd: %d ] Data transmission failed. %s", iDevFd, strerror(errno));
        LOGHEX(buf, length, "SEND FAILED");
    }
}

void CCommunicDevice::send(int iDevFd, std::string str)
{
    int n = write(iDevFd, str.data(), str.length());
    if (n <= 0)
    {
        LOGERR("[iDevFd: %d ] Data transmission failed. %s", iDevFd, strerror(errno));
        LOGHEX(str.data(), str.length(), "SEND FAILED");
    }
}

void CCommunicDevice::addFD(int iDevFD)
{
    CSelectListener::getInstance() << CSelectListener::HandleType(iDevFD, this);
    CSelectListener::getInstance().reset();
}

void CCommunicDevice::delFD(int iDevFD)
{
    CSelectListener::getInstance() >> CSelectListener::HandleType(iDevFD, this);
}

void CCommunicDevice::swapCore(CCommunicDevice *ptrCore)
{
    int fd = CSelectListener::getInstance()[this];
    if (fd > 0)
    {
        CSelectListener::getInstance() >> CSelectListener::HandleType(fd, this);
        CSelectListener::getInstance() << CSelectListener::HandleType(fd, ptrCore);
        CSelectListener::getInstance().reset();
    }
    else
        LOGERR("swap core is error.");
}

void CCommunicDevice::registerCB(std::function<void(std::string)> cb)
{
    taskQueue.lock();
    mCB = cb;
    taskQueue.unlock();
}

bool CCommunicDevice::getFrame(std::string &cacheFrame, std::string &frame)
{
    bool ret = false;
    std::size_t Offset = 0;
    unsigned frameLength = 0;
    if(0 == cacheFrame.length())
    {
        return ret;
    }
    //std::string FrameHeader({(char)0xFF, (char)0xAA}, 2);
    char FrameHeader[3] = {0xFF, 0xAA, 0x00};
    if (std::string::npos != (Offset = cacheFrame.find(FrameHeader)))
    {
        if(cacheFrame.length() > (Offset + 7))
        {
            frameLength = (cacheFrame[Offset + 6] & 0xFF);
            //帧长度最小为9
            if(9 > frameLength)
            {//此帧数据格式有问题，舍弃帧头
                
                LOGERR("recv mcu frameLength = 0, cacheFrame.length() = %d\n", cacheFrame.length());
                cacheFrame.assign(cacheFrame, Offset + 2, cacheFrame.length());
                frame = std::string("");
                return ret;
            }
        }
        else
        {
            frame = std::string("");
            return ret;
        }
        cacheFrame.length() >= (Offset + frameLength)
            ? (frame.assign(cacheFrame, Offset, frameLength),
            cacheFrame.assign(cacheFrame, Offset + frameLength, cacheFrame.length()))
            : (frame = std::string(""));
        frame.length() > 0 ? (ret = true) : (ret = false);
        #if 1
        if(ret)
        {
            ret = checkSum(frame.data(), frameLength);
            if(ret == false)
            {
                LOGHEX(frame.data(), frame.length(), "checkSumErr");
            }
        }
        #endif
    }
    return ret;
}

void CCommunicDevice::loopCb(std::string cacheFrame)
{
    bool ret = false;
    taskQueue.lock();
    auto cb = mCB;
    taskQueue.unlock();

    mRemainderFrame += cacheFrame;
    while (true)
    {
        std::string frame;
        ret = this->getFrame(mRemainderFrame, frame);
        if (ret && cb)
            cb(frame);
        else
        {
            // RemainderFrame = cacheFrame;
            break;
        }
    }
}

void CCommunicDevice::debug(void)
{
#if 0
    char buf1[] = {
        0xFF,
        0xAA,
        0xFF,
        0xAA,
        0x50,
        0x00,
        0x01,
        0x00,

    };
    char buf2[] = {0x0F,
                   0x06,
                   0x06,
                   0x12,
                   0x14,
                   0x05,
                   0x06,
                   0x07,
                   0x0A,
                   0xFF,
                   0xAA,
                   0x50,
                   0xFF,
                   0xFF, 0x00, 0x0F, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x0A};

    taskQueue.push(std::string(buf1, sizeof(buf1)));
    taskQueue.push(std::string(buf2, sizeof(buf2)));
    taskQueue.push(std::string(buf1, sizeof(buf1)));
    taskQueue.push(std::string(buf2, sizeof(buf2)));
#endif
}
bool CCommunicDevice::checkSum(const char* pdata, uint32_t len)
{
    uint8_t u8CheckSum = uint8_t(pdata[len - 2]);
    uint8_t calcCheckSum = 0;
    for(int i = 2; i < len - 2; i++)
    {
        calcCheckSum += pdata[i];
    }
#ifdef _QNX_TARGET_
    return ((~calcCheckSum) + 1) == u8CheckSum;
#endif
#ifdef _LINUX_TARGET_
    return calcCheckSum == u8CheckSum;
#endif
}
