#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <unistd.h>

#include "mlog.h"
#include "HUNetMsg.h"


CHUNetMsg::CHUNetMsg(std::string ipAddr, int port) 
    : CCommunicDevice(1152)
{
    mIPAddr = ipAddr;
    mPort = port;
    mSocketFD = -1;
}

void CHUNetMsg::start()
{
    mSocketFD = CHUNet::start(mIPAddr, mPort);
    if (mSocketFD != -1)
    {
        printf("\nConnect Success: IP=%s, Port=%d\n", mIPAddr.data(), mPort);
        this->addFD(mSocketFD);
    }
    else
    {
        printf("\nConnect Fail: IP=%s, Port=%d\n", mIPAddr.data(), mPort);
    }
}

void CHUNetMsg::send(std::string str)
{
    LOGHEX(str.c_str(), str.length(), "send: ");
    if (mSocketFD != -1)
    {
        CCommunicDevice::send(mSocketFD, str);
    }
}

void CHUNetMsg::stop()
{
    this->delFD(mSocketFD);
    CHUNet::stop(mSocketFD);
    mSocketFD = -1;
}

bool CHUNetMsg::isConnect()
{
    return (mSocketFD != -1);
}

// for debug
std::string CHUNetMsg::getHexString(std::string const &buffer, std::string const &delimiter)
{
    std::stringstream ss;
    std::string space = "";
    for (size_t i = 0; i < buffer.length(); i++) {
        ss << space << std::hex << std::setw(2) << std::setfill('0') << int(buffer[i]);
        space = delimiter;
    }
    return ss.str();
}

bool CHUNetMsg::getFrame(std::string &cacheFrame, std::string &frame)
{
    std::size_t cacheLen = cacheFrame.length();
    std::size_t headOffset = 0;
    std::size_t tailOffset = 0;
    std::size_t frameLen = 0;
    std::size_t dataLen = 0;
    std::size_t offset = 0;
    std::string frameHeader = {0xA5, 0xFF, 0x5A, 0xFF};
    std::string frameTailer = {0x5A, 0xFF, 0xA5, 0xFF};
    const unsigned FRAME_MIN_SIZE = 24;
    const unsigned FRAME_LEN_OFFSET = 16;
    bool ret = false;

    if (std::string::npos != (headOffset = cacheFrame.find(frameHeader, 0)))
    {
        if (cacheLen >= (headOffset + FRAME_MIN_SIZE))
        {
            dataLen = (cacheFrame[headOffset + FRAME_LEN_OFFSET + 3] << 24 & 0xFF000000) |
                (cacheFrame[headOffset + FRAME_LEN_OFFSET + 2] << 16 & 0xFF0000) | 
                (cacheFrame[headOffset + FRAME_LEN_OFFSET + 1] << 8 & 0xFF00) | 
                (cacheFrame[headOffset + FRAME_LEN_OFFSET + 0] & 0xFF);

            frameLen = FRAME_MIN_SIZE + dataLen;
            if (cacheLen >= (headOffset + frameLen))
            {
                tailOffset = headOffset + frameLen - frameTailer.length();
                offset = cacheFrame.find(frameTailer, tailOffset);
                if (std::string::npos != offset)
                {
                    frameLen = offset - headOffset + frameTailer.length();
                    frame.assign(cacheFrame, headOffset, frameLen);
                    cacheFrame.assign(cacheFrame, headOffset + frameLen, cacheLen);

                    ret = true;
                    { // For debug.
                       std::size_t pos;
                       if (std::string::npos != (pos = frame.find(frameHeader, frameHeader.length())))
                       {
                           printf("++++++++++++++ Fake frame header detect!\n");
                           printf("++++++++++++++ %s\n", getHexString(frame.substr(0, 32), " ").data());
                           printf("++++++++++++++ %s\n", getHexString(frame.substr(pos, 32), " ").data());
                           printf("++++++++++++++\n");
                       }
                       ret = true;
                    }

                }
                else
                {
                    // Do nothing, The frame is not complete. 
                }
            }
        }
    }

    if (!ret)
    {
        frame = std::string("");
    }

    return ret;
}