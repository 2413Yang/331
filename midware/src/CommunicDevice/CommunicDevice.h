
#ifndef MSGDEVICE__H__
#define MSGDEVICE__H__
#include <functional>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <string>
#include <condition_variable>
#include "TemplateCache.hpp"

class CCommunicDevice
{
public:
    CCommunicDevice(unsigned size = 255);
    ~CCommunicDevice();
    virtual void start() = 0;
    virtual void registerCB(std::function<void(std::string)>);
    virtual void stop() = 0;
    virtual void send(std::string);
    virtual void send(uint8_t *, uint32_t);
    virtual void send(int, std::string);
    void swapCore(CCommunicDevice *);
	uint32_t getMsgCount();

protected:
    virtual void handleEvent(int fd);
    virtual bool getFrame(std::string &cache, std::string &frame);
    void loopCb(std::string);
    void addFD(int);
    void delFD(int);
    void debug(void);
    bool checkSum(const char* pdata, uint32_t len);

private:
    std::mutex mLock;
    std::function<void(std::string)> mCB;
#if 0
	class threadManger
	{
	private:
		std::thread*	mThrad;
		bool			mThreadExitFlag;
		int				mCount;
		std::mutex		mMutex;
	public:
		threadManger(std::function<void (void)>);
		~threadManger();
		bool getThreadExitFlag(){ return mThreadExitFlag;}
		void increaseCount(){++mCount;};
		int decreaseCount(){ return --mCount;};
	};
	static threadManger*	sProducer;
	static threadManger*	sConsumer;
#endif
protected:
    unsigned int mSize;
    unsigned char *pReadCache;
    std::string mRemainderFrame;

protected:
    static std::thread *pProducer, *pConsumer;
    static ZH::BaseLib::CTemplateCache<std::function<void(void)>> taskQueue;
    int     debugId;
};

#endif /*CMSGDEVICE__H__*/