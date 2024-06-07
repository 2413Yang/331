
#ifndef SPIDEVICE__H__
#define SPIDEVICE__H__
#include "CommunicDevice.h"

class CSpiDevice : public CCommunicDevice
{
public:
    CSpiDevice();
    virtual ~CSpiDevice();
    virtual void start();
    virtual void stop();
    virtual void send(std::string);

private:
    int iDevFD;
};

#endif /*SPIDEVICE__H__*/