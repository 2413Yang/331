#ifndef HUNETMSG__H__
#define HUNETMSG__H__

#include "CommunicDevice.h"
#include "HUNet.h"

class CHUNetMsg : public CHUNet, public CCommunicDevice
{
public:
    CHUNetMsg(std::string ipAddr, int port);
    virtual ~CHUNetMsg(){};
    virtual void start();
    virtual void stop();
    virtual void send(std::string);
    bool isConnect();

    // for debug.
    std::string getHexString(std::string const &buffer, std::string const &delimiter);
protected:
    virtual bool getFrame(std::string &cache, std::string &frame);

    std::string mIPAddr;
    int mPort;
    int mSocketFD;
};

#endif /* HUNETMSG__H__ */