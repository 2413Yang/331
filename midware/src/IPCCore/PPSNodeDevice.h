
#ifndef PPSNODEDEVICE__H__
#define PPSNODEDEVICE__H__
#include <string>
#include "CommunicDevice.h"

class CPPSNodeDevice : public CCommunicDevice
{
public:
    CPPSNodeDevice(std::string nd);
    ~CPPSNodeDevice();
    virtual void start();
    virtual void stop();

protected:
    virtual bool getFrame(std::string &cache, std::string &frame);

private:
    int iNodeFD;
    std::string mNode;
};

#endif /*PPSNODEDEVICE__H__*/