
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include "UartDevice.h"
#include "mlog.h"

CUartDevice::CUartDevice()
{
}

CUartDevice::~CUartDevice()
{
}

void CUartDevice::start()
{
    close(open("/dev/ttyS2", O_RDWR | O_NOCTTY));
    while (true)
    {
        iDevFD = open("/dev/ttyS2", O_RDWR | O_NOCTTY);
        if (-1 == iDevFD)
            LOGERR("Can't Open Serial Port /dev/ttyS2");
        else
            break;
    }
    LOGDBG("open success /dev/ttyS2.");
    this->setSerialOption(8, 'N', 38400, 1);
    this->addFD(iDevFD);
    debugId = 1;
}

void CUartDevice::send(std::string str)
{
    LOGHEX(str.data(), str.length(), "SEND TO MCU");
    // uint8_t buf[] = {0xFF, 0xAA, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0xEF, 0x03, 0x20, 0x00, 0x00, 0x1F, 0x0A};
    // uint8_t buf[] = {0xFF,0xAA,0xE5,0x0,0x0,0x50,0xB,0x1,0x0,0x41,0xA};
    CCommunicDevice::send(iDevFD, str); ///std::string((char *)buf, sizeof(buf)));
}

void CUartDevice::setSerialOption(uint8_t nBits, char nEvent, uint32_t nSpeed, uint8_t nStop)
{
    struct termios newtio, oldtio;
    if (tcgetattr(iDevFD, &oldtio) != 0)
        LOGERR("error: SetupSerial \n");
    memset(&newtio, 0, sizeof(newtio));
    //使能串口接收
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    newtio.c_lflag &= ~ICANON; //原始模式
    //newtio.c_lflag |=ICANON; //标准模式

    //设置串口数据位
    switch (nBits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }
    //设置奇偶校验位
    switch (nEvent)
    {
    case 'O':
        newtio.c_cflag |= PARENB, newtio.c_cflag |= PARODD, newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP), newtio.c_cflag |= PARENB, newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }
    switch (nSpeed)
    {
    case 2400:
        cfsetispeed(&newtio, B2400), cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800), cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600), cfsetospeed(&newtio, B9600);
        break;
    case 38400:
        cfsetispeed(&newtio, B38400), cfsetospeed(&newtio, B38400);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200), cfsetospeed(&newtio, B115200);
        break;
    default:
        cfsetispeed(&newtio, B9600), cfsetospeed(&newtio, B9600);
        break;
    }

    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;

    newtio.c_cc[VTIME] = 1; // 100ms
    newtio.c_cc[VMIN] = 16;

    if (tcsetattr(iDevFD, TCSANOW, &newtio) != 0)
        LOGDBG("com set error\n");

    tcflush(iDevFD, TCIOFLUSH);
}

void CUartDevice::stop()
{
    this->delFD(iDevFD);
    close(iDevFD);
}
