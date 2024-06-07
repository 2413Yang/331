
#include "mlog.h"
#include "SpiDevice.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

CSpiDevice::CSpiDevice()
{
}

CSpiDevice::~CSpiDevice()
{
    close(iDevFD);
}

void CSpiDevice::start()
{
    int i = 20 * 3;
    while ((iDevFD = ::open("/dev/iccom0", O_RDWR)) <= 0)
    {
        i--;
        usleep(50 * 1000);
        if (i <= 0)
            break;
    }
    this->addFD(iDevFD);
}

void CSpiDevice::stop()
{
    this->delFD(iDevFD);
    ::close(iDevFD);
}

void CSpiDevice::send(std::string str)
{
    LOGHEX(str.data(), str.length(), "SEND TO MCU");
    CCommunicDevice::send(str);
}
