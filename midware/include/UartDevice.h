
#ifndef UARTDEVICE__H__
#define UARTDEVICE__H__
#include "CommunicDevice.h"

class CUartDevice : public CCommunicDevice
{
public:
    CUartDevice();
    virtual ~CUartDevice();
    virtual void start();
    virtual void stop();
    void send(std::string str);

protected:
    void setSerialOption(uint8_t nBits, char nEvent, uint32_t nSpeed, uint8_t nStop);

private:
    int iDevFD;
};

#endif /*UARTDEVICE__H__*/
