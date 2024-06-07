
#ifndef HUNET__H__
#define HUNET__H__

#include <netinet/in.h>
#include <string>

class CHUNet
{
public:
    CHUNet();
    virtual ~CHUNet(){};
    virtual int start(std::string IP, int port);
    virtual void stop(int);

private:
    struct sockaddr_in stLocalAddr;
};

#endif /* HUNET__H__ */