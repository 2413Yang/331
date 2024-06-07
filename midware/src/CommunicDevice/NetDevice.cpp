
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mlog.h"
#include "NetDevice.h"

CNet::CNet()
{
}

int CNet::start(int port)
{
    int socketFd = -1;
    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOGWAR("socket error : %s", strerror(errno));
    }

    int iSocketOptVal = 1;
    if (0 > setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &iSocketOptVal, sizeof(iSocketOptVal)))
    {
        LOGWAR("setsockopt error : %s", strerror(errno));
    }

    memset(&stLocalAddr, 0, sizeof(stLocalAddr));
    stLocalAddr.sin_family = AF_INET;
    stLocalAddr.sin_addr.s_addr = INADDR_ANY;
    stLocalAddr.sin_port = htons(port);
    //    LOGDBG("oAddr.GetPort() = %d", oAddr.GetPort());

    if (bind(socketFd, (struct sockaddr *)&stLocalAddr, sizeof(struct sockaddr)) < 0)
    {
        LOGWAR("bind %d error : %s", port, strerror(errno));
    }

    if (0 > listen(socketFd, 255))
    {
        LOGWAR("listen error : %s", strerror(errno));
    }
    printf("listen port: %d fd=%d\n", port, socketFd);

    return socketFd;
}

int CNet::start(std::string IP, int port)
{
    int iConnet = -1;
    if ((iConnet = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOGWAR("socket error : %s", strerror(errno));
        iConnet = -1;
    }

    struct sockaddr_in stRemoteAddr;
    memset(&stRemoteAddr, 0, sizeof(stRemoteAddr));
    stRemoteAddr.sin_family = AF_INET;
    stRemoteAddr.sin_addr.s_addr = inet_addr(IP.data());
    stRemoteAddr.sin_port = htons(port);
    LOGDBG("connect to: %s:%d\n", IP.data(), port);
    if (0 != connect(iConnet, (struct sockaddr *)&stRemoteAddr, sizeof(stRemoteAddr)))
    {
        LOGWAR("Connect error : %s", strerror(errno));
        close(iConnet);
        iConnet = -1;
    }
    return iConnet;

#if 0
    if (false)
    {
        struct sockaddr_in stLocalAddr;
        socklen_t iLen = sizeof(stLocalAddr);
        if (0 != getsockname(m_iLocalSocketFD, (struct sockaddr *)&stLocalAddr, &iLen))
        {
            LOGDBG("getsockname error : %s", strerror(errno));
        }

        struct linger stLinger;
        stLinger.l_onoff = 1;
        stLinger.l_linger = 0;
        if (0 != setsockopt(m_iLocalSocketFD, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(stLinger)))
        {
            LOGWAR("setsockopt SO_LINGER error : %s", strerror(errno));
        }

        close(m_iLocalSocketFD);
        shutdown(m_iLocalSocketFD, SHUT_RDWR);
    }
#endif
}

void CNet::stop(int connect)
{
    LOGINF("disconnect fd=%d", connect);
    close(connect);
    // shutdown(connect, SHUT_RDWR);
}

CNetDevice::CNetDevice() : CCommunicDevice(1152)
{
    mCurFD = -1;
    m_iLocalSocketFD = -1;
}

void CNetDevice::start()
{
    m_iLocalSocketFD = CNet::start(mPort);
    this->addFD(m_iLocalSocketFD);
}

void CNetDevice::setPort(int port)
{
    mPort = port;
}

int32_t CNetDevice::isConnect(int fd)
{
    int32_t ret = -1;
    if (fd == m_iLocalSocketFD)
    {
        struct sockaddr_in stClientAddr;
        socklen_t length = sizeof(stClientAddr);
        int clientFd = accept(fd, (struct sockaddr *)&stClientAddr, &length);
        if (clientFd <= 0)
        {
            LOGERR("accept error : %s", strerror(errno));
        }
        else
            ret = clientFd;
        LOGDBG("clientFd = %d", clientFd);
        mCurFD = clientFd;
        char ipAddr[INET_ADDRSTRLEN] = {0};
        // getpeername(clientFd, (struct sockaddr *)&stClientAddr, &length);
        printf("connected peer address = %s:%d\n",
               inet_ntop(AF_INET, &stClientAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(stClientAddr.sin_port));

        vConnect.push_back({clientFd, inet_ntop(AF_INET, &stClientAddr.sin_addr, ipAddr, sizeof(ipAddr))});

        this->addFD(clientFd);
    }

    return ret;
}

void CNetDevice::handleEvent(int fd)
{
    if (-1 == isConnect(fd))
    {
        CCommunicDevice::handleEvent(fd);
    }
}
void CNetDevice::send(std::string str)
{
    LOGHEX(str.c_str(), str.length(), "send: ");
    CCommunicDevice::send(mCurFD.load(), str);
}

void CNetDevice::stop()
{
    this->delFD(m_iLocalSocketFD);
    CNet::stop(m_iLocalSocketFD);
}

void CNetDevice::stop(int client)
{
    this->delFD(client);
    CNet::stop(client);
}

bool CNetDevice::getFrame(std::string &cacheFrame, std::string &frame)
{
    bool ret = false;
    std::size_t Offset = 0;
    unsigned frameLength = 0;
    std::string FrameHeader({(char)0xFF, (char)0xBB}, 2);
    if (std::string::npos != (Offset = cacheFrame.find(FrameHeader, 0)))
    {
        cacheFrame.length() > (Offset + 7)
            ? (frameLength = ((cacheFrame[Offset + 4] << 8) & 0xFF) | (cacheFrame[Offset + 5] & 0xFF))
            : (frameLength = 0);
        cacheFrame.length() >= (Offset + frameLength)
            ? (frame.assign(cacheFrame, Offset, frameLength),
               cacheFrame.assign(cacheFrame, Offset + frameLength, cacheFrame.length()))
            : (frame = std::string(""));
        frame.length() > 0 ? (ret = true) : (ret = false);
    }

    return ret;
}