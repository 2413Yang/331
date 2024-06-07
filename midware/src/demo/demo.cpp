
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include "CarBodyInfo.h"
#include "CaptureStream.h"

#define DEFAULT 0
#define DISPLAY

#ifdef DISPLAY
#include "WinDisplayer.h"
#include "DispWindow.h"
#endif

class CDemo : public CCarBodyInfo
{
private:
    /* data */
public:
    CDemo(/* args */);
    virtual ~CDemo();

    virtual void UpdateVehicleSpeed(const float value, const bool validity)
    {
        std::cout << __FUNCTION__ << " value: " << value << " validity: " << validity << std::endl;
    }
    virtual void UpdateAverageSpeed(const float value, const bool validity)
    {
        std::cout << __FUNCTION__ << " value: " << value << " validity: " << validity << std::endl;
    }
    virtual void UpdateEngineSpeed(const float value, const bool validity)
    {
        std::cout << __FUNCTION__ << " value: " << value << " validity: " << validity << std::endl;
    }
    virtual void UpdateGear(const EmGearsValue value)
    {
        std::cout << __FUNCTION__ << " value: " << (int)value << std::endl;
    }
    virtual void UpdateTurnLightState(const EmSWState lvalue, const EmSWState rvalue)
    {
        std::cout << __FUNCTION__ << " Left value: " << (int)lvalue << " Right value: " << (int)rvalue << std::endl;
    }
    virtual void UpdateCarDoor(const std::vector<StuCarDoorState> doorState)
    {
        std::cout << "size: " << doorState.size() << std::endl;
        for (auto it : doorState)
            std::cout << "door ID: " << (int)it.ID << " state: " << (int)it.state << std::endl;
    }
    virtual void UpdateSteeringWheelCorner(float value, bool validity)
    {
        std::cout << __FUNCTION__ << " value: " << value << " validity: " << validity << std::endl;
    }
};

CDemo::CDemo(/* args */)
{
}

CDemo::~CDemo()
{
}

class CTouchDemo : public CTouchMessage
{
    virtual void TouchEvent(EmTouchEvent value, int x, int y)
    {
        std::cout << __FUNCTION__ << " touch event:" << (int)value << " X:" << x << " Y:" << y << std::endl;
    }

    virtual void Open360AVM(void)
    {
        std::cout << __FUNCTION__ << std::endl;
    }
};

#ifdef DISPLAY
screen_stream_t getScreenStream(int32_t);
#endif

int main(int argc, char *argv[])
{
    printf("[%s] build time: %s %s\n", argv[0], __DATE__, __TIME__);

    // 消息接收对象，当有消息时对应的消息函数将被调用
    CDemo msg;
    CTouchDemo oTouchMsg;
    msg.Start();
    // 获取消息，所有消息函数都将被调用，当所有消息函数返回后 Get 函数才会返回
    msg.Get();

#ifdef DISPLAY
    // 显示代码，可忽略
    CDispWindow *ptrWindowLf = NULL, *ptrWindowRf = NULL, *ptrWindowLr = NULL, *ptrWindowRr = NULL;
    CWinDisplayer *ptrDisplayer = NULL;

    bool isUser = false;
    if ((2 == argc) && (std::string("--user") == argv[1]))
    {
        isUser = true;
        uint8_t *buffFront[10] = {NULL}, *buffRear[10] = {NULL}, *buffLeft[10] = {NULL}, *buffRight[10] = {NULL};
        uint8_t **buffs[4] = {buffFront, buffRear, buffLeft, buffRight};
        for (int i = 0; i < 4; i++)
        {
            uint8_t *addr = (uint8_t *)mmap(0, 1280 * 960 * 2 * 10, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED | MAP_PHYS | MAP_ANON, NOFD, 0);
            // printf("addr: %p \n", addr);
            for (int j = 0; j < 10; j++)
                buffs[i][j] = addr + (1280 * 960 * 2 * j);
        }
        // 创建4个数据流,分别对应4个摄像头
        initStream(buffFront, buffRear, buffLeft, buffRight, 10);

        ptrDisplayer = new CWinDisplayer;
        ptrDisplayer->init();
        ptrWindowLf = new CDispWindow(*ptrDisplayer, 2);
        setWindowID(ptrWindowLf->getHandle(), "360AVM_1");
        ptrWindowLf->setUsage(SCREEN_USAGE_WRITE | SCREEN_USAGE_ROTATION | SCREEN_USAGE_CAPTURE);
        ptrWindowLf->setFormat(SCREEN_FORMAT_UYVY);
        ptrWindowLf->setSize(1280, 960);
        ptrWindowLf->setPosition(0, 0);
        ptrWindowLf->setZorder(20);
        ptrWindowLf->setVisible(true);
        ptrWindowLf->createBuffers(1);
        showWindow("360AVM_1", ptrWindowLf->getHandle());
    }
    else if (1 == argc)
    {
        initStream();

        ptrDisplayer = new CWinDisplayer;
        ptrDisplayer->init();
        ptrWindowLf = new CDispWindow(*ptrDisplayer, 0);
        ptrWindowLf->setUsage(SCREEN_USAGE_WRITE | SCREEN_USAGE_ROTATION | SCREEN_USAGE_CAPTURE);
        ptrWindowLf->setFormat(SCREEN_FORMAT_UYVY);
        ptrWindowLf->setSize(1280, 960);
        ptrWindowLf->setPosition(0, 0);
        ptrWindowLf->setZorder(20);
        ptrWindowLf->setVisible(true);
        if (-1 == screen_share_stream_buffers(ptrWindowLf->getHandle(), getScreenStream(0)))
            printf("screen_share_stream_buffers[%d]: %s \n", __LINE__, strerror(errno));

        ptrWindowRf = new CDispWindow(*ptrDisplayer, 0);
        ptrWindowRf->setUsage(SCREEN_USAGE_WRITE | SCREEN_USAGE_ROTATION | SCREEN_USAGE_CAPTURE);
        ptrWindowRf->setFormat(SCREEN_FORMAT_UYVY);
        ptrWindowRf->setSize(1280, 960);
        ptrWindowRf->setPosition(960, 0);
        ptrWindowRf->setZorder(20);
        ptrWindowRf->setVisible(true);
        if (-1 == screen_share_stream_buffers(ptrWindowRf->getHandle(), getScreenStream(1)))
            printf("screen_share_stream_buffers[%d]: %s \n", __LINE__, strerror(errno));

        ptrWindowLr = new CDispWindow(*ptrDisplayer, 2);
        ptrWindowLr->setUsage(SCREEN_USAGE_WRITE | SCREEN_USAGE_ROTATION | SCREEN_USAGE_CAPTURE);
        ptrWindowLr->setFormat(SCREEN_FORMAT_UYVY);
        ptrWindowLr->setSize(1280, 960);
        ptrWindowLr->setPosition(0, 0);
        ptrWindowLr->setZorder(20);
        ptrWindowLr->setVisible(true);
        if (-1 == screen_share_stream_buffers(ptrWindowLr->getHandle(), getScreenStream(2)))
            printf("screen_share_stream_buffers[%d]: %s \n", __LINE__, strerror(errno));

        ptrWindowRr = new CDispWindow(*ptrDisplayer, 2);
        ptrWindowRr->setUsage(SCREEN_USAGE_WRITE | SCREEN_USAGE_ROTATION | SCREEN_USAGE_CAPTURE);
        ptrWindowRr->setFormat(SCREEN_FORMAT_UYVY);
        ptrWindowRr->setSize(1280, 960);
        ptrWindowRr->setPosition(960, 0);
        ptrWindowRr->setZorder(20);
        ptrWindowRr->setVisible(true);
        if (-1 == screen_share_stream_buffers(ptrWindowRr->getHandle(), getScreenStream(3)))
            printf("screen_share_stream_buffers[%d]: %s \n", __LINE__, strerror(errno));
    }
    else
        printf("Incorrect parameter options.\n");
#endif

    // 4个流开始工作
    startCapture();

    int *ptrBuff = NULL;
    uint8_t *pLf = NULL, *pRf = NULL, *pLr = NULL, *pRr = NULL;
    if (isUser)
        ptrBuff = ptrWindowLf->getPointer();
    while (true)
    {
        // 获取4个摄像头数据， 左前，右前，左后，右后， 最长等待时长 100 ms
        if (-1 == getFrames(pLf, pRf, pLr, pRr, 100))
            printf("Failed to get frame; %p %p %p %p\n", pLf, pRf, pLr, pRr);

        // 模拟算法时长...
        usleep(20000);

        if (isUser)
        {
            if (pLf != NULL)
                memcpy(ptrBuff, pLf, 1280 * 960 * 2);
            ptrWindowLf->windowPost();
        }

        // 释放4个缓冲区，按照 左前，右前，左后，右后 顺序
        releaseFrames(pLf, pRf, pLr, pRr);
    }

    stopCapture();
    uinitStream();
    return EXIT_SUCCESS;
}