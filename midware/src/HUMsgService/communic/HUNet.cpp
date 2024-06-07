#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <atomic>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "HUNet.h"
#include "mlog.h"

CHUNet::CHUNet()
{
}

int CHUNet::start(std::string IP, int port)
{
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOGWAR("socket error : %s", strerror(errno));
        fd = -1;
    }

    struct sockaddr_in stRemoteAddr;
    memset(&stRemoteAddr, 0, sizeof(stRemoteAddr));
    stRemoteAddr.sin_family = AF_INET;
    stRemoteAddr.sin_addr.s_addr = inet_addr(IP.data());
    stRemoteAddr.sin_port = htons(port);
    LOGDBG("connect to: %s:%d\n", IP.data(), port);
    if (0 != connect(fd, (struct sockaddr *)&stRemoteAddr, sizeof(stRemoteAddr)))
    {
        LOGWAR("Connect error : %s", strerror(errno));
        close(fd);
        fd = -1;
    }
    return fd;
}

void CHUNet::stop(int connect)
{
    LOGINF("disconnect fd=%d", connect);
    close(connect);
}
