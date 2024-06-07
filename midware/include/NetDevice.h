
#ifndef MSGNET__H__
#define MSGNET__H__
#include <atomic>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "CommunicDevice.h"

class CNet
{
public:
    CNet();
    virtual ~CNet(){};
    virtual int start(int port);
    virtual int start(std::string IP, int port);
    virtual void stop(int);

private:
    struct sockaddr_in stLocalAddr;
};

class CNetDevice : public CNet, public CCommunicDevice
{
public:
    CNetDevice();
    virtual ~CNetDevice(){};
    virtual void start();
    virtual void stop();
    virtual void stop(int);
    virtual void send(std::string);
    void setPort(int port);
    int32_t isConnect(int fd);

protected:
    virtual void handleEvent(int fd);
    virtual bool getFrame(std::string &cache, std::string &frame);

    int m_iLocalSocketFD, mPort;
    std::vector<std::pair<int, std::string>> vConnect;
    std::atomic<int> mCurFD;
};

#endif /*MSGNET__H__*/