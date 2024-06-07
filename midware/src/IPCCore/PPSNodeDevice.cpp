
#include <fcntl.h>
#include "PPSNodeDevice.h"
#include "mlog.h"

CPPSNodeDevice::CPPSNodeDevice(std::string nd)
    : CCommunicDevice(1024 * 5)
{
    mNode = nd.substr(nd.find_last_of("/") + 1);
    mNode = mNode.substr(0, mNode.find_last_of("."));
}

CPPSNodeDevice::~CPPSNodeDevice()
{
}

void CPPSNodeDevice::start()
{
    std::string node = "/pps/qnx/";
    node += (mNode + "?wait,delta,nopersist");
    iNodeFD = ::open(node.c_str(), O_RDWR | O_CREAT | O_NONBLOCK, 666);
    if (-1 == iNodeFD)
        LOGERR("PPS node open fail.");
    else
        this->addFD(iNodeFD);
}
void CPPSNodeDevice::stop()
{
    this->delFD(iNodeFD);
    ::close(iNodeFD);
}

bool CPPSNodeDevice::getFrame(std::string &cache, std::string &frame)
{
    if (cache.length() > 0)
        swap(frame, cache);
    else
        return false;
    return true;
}