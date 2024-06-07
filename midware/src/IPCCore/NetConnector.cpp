
#include "Application.h"
#include "NetConnector.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "mlog.h"

#define ZH_IPC_BEGIN_F "##_ZH-B_##"
#define ZH_IPC_END_F "##_ZH-E_##"

bool CSubInterface::isJsonFmtData(const char *encoding)
{
#define JSONFMT "json"
    if (strcasecmp(encoding, JSONFMT) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }

    return false;
}

std::atomic<ZH::BaseLib::CMutex *> CSubscriber::pLockCb;
std::map<std::string, CSubInterface *> CSubscriber::defCBTable;

CSubscriber::CSubscriber()
{
}

CSubscriber::~CSubscriber()
{
}

void CSubscriber::regSubscriber(std::string interfaceName, CSubInterface *pInterface)
{
    if (defCBTable.find(interfaceName) == defCBTable.end() && pInterface != NULL)
        defCBTable[interfaceName] = pInterface;
    this->callback(interfaceName);
}

void CSubscriber::registerLock(ZH::BaseLib::CMutex &op)
{
    pLockCb = &op;
}

CNetConnector::CNetConnector(std::string node)
{
    m_isClientFD = -1;
    m_iLocalSocketFD = -1;
    strNode = node.substr(node.find_last_of("/") + 1);
    strNode = strNode.substr(0, strNode.find_last_of("."));
    char buf[512] = {0};
    std::string file = "/proc/" + std::to_string(getpid()) + "/cmdline";
    int fd = open(file.data(), O_RDONLY);
    if (read(fd, buf, sizeof(buf)) < 0)
        LOGERR("read error. %s", file.data());
    close(fd);
    cmdline = buf;
    LOGDBG("cmdline: %s, strName: %s ", cmdline.data(), strNode.data());
    CCommunicDevice::registerCB(std::bind(&CNetConnector::subscribe, this, std::placeholders::_1));
}

CNetConnector::~CNetConnector()
{
}

std::mutex CNetConnector::s_cacheLock;
std::vector<std::function<bool(void)>> CNetConnector::s_loopTask;
std::thread *CNetConnector::t_ptrKeepConnected = nullptr;

void CNetConnector::start(void)
{
    if (std::string::npos != cmdline.find(strNode))
    {
        m_iLocalSocketFD = this->CNet::start(toPortNumber(strNode.data()));
        this->addFD(m_iLocalSocketFD);
    }
    else
    {
        do
        {
            std::unique_lock<std::mutex> guard(s_cacheLock);
            if (nullptr == t_ptrKeepConnected)
            {
                t_ptrKeepConnected = new std::thread(CNetConnector::keepConnectedThread);
                if (nullptr == t_ptrKeepConnected)
                    LOGERR("t_ptrKeepConnected create a failure.");
                else
                    t_ptrKeepConnected->joinable() ? t_ptrKeepConnected->detach() : void(0);
            }
        } while (0);
        if (!this->connect())
        {
            std::unique_lock<std::mutex> guard(s_cacheLock);
            s_loopTask.push_back(std::bind(&CNetConnector::connect, this));
        }
    }
}

int32_t CNetConnector::connect()
{
    int ret = -1;
    //LOGDBG("strNode = %s,porNum = %d",this->strNode.data(),this->toPortNumber(this->strNode.data()));
    if (-1 != (ret = this->CNet::start("127.0.0.1", this->toPortNumber(this->strNode.data()))))
    {
        this->m_isClientFD = ret;
        for (auto it : this->mapParameterData)
            CCommunicDevice::send(this->m_isClientFD.load(), it.second);
        this->addFD(ret);
        return true;
    }
    return false;
}

void CNetConnector::keepConnectedThread()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> guard(s_cacheLock);
            for (auto it = s_loopTask.begin(); it != s_loopTask.end(); it++)
            {
                if ((*it)())
                {
                    it = s_loopTask.erase(it);
                    if (it == s_loopTask.end())
                        break;
                }
            }
        }
        usleep(300000);
    }
}

void CNetConnector::publish(const std::string id, std::string &arg)
{
    std::string args = ZH_IPC_BEGIN_F + arg + ZH_IPC_END_F;
    mapParameterData[id] = args;
    if (m_iLocalSocketFD > 0)
    {
        if (args.length() > 0)
            for (auto it : vConnect)
            {
                CCommunicDevice::send(it.first, args);
            }
    }
    if (m_isClientFD > 0)
    {
        // for (auto it : this->mapParameterData)
        CCommunicDevice::send(this->m_isClientFD.load(), args);
    }
}

void CNetConnector::handleEvent(int fd)
{
    int client = -1;
    if (-1 != (client = CNetDevice::isConnect(fd)))
    {
        LOGDBG("is connect come on");
        // 连接成功(包括重连)后将最新的数据发布出去
        for (auto it : mapParameterData)
            CCommunicDevice::send(client, it.second);
    }
    else
    {
        if (!CCommunicDevice::pReadCache)
        {
            LOGERR("read cache is nullptr.");
            return;
        }
        memset(CCommunicDevice::pReadCache, 0, mSize);
        int n = read(fd, CCommunicDevice::pReadCache, mSize);
        if (n < 0)
            LOGERR("device data read error [%d]. %s", n, strerror(errno));
        else if ((0 == n) && (EINTR != errno))
        {
            CCommunicDevice::delFD(fd);
            CNet::stop(fd);
            if (fd == m_isClientFD.load())
            {
                m_isClientFD = -1;
                std::unique_lock<std::mutex> guard(s_cacheLock);
                s_loopTask.push_back(std::bind(&CNetConnector::connect, this));
            }
            for (auto it = vConnect.begin(); it != vConnect.end(); it++)
            {
                if (it->first == fd)
                {
                    vConnect.erase(it);
                    break;
                }
            }
        }
        else
        {
            // LOGDBG("Net read: %s ", pReadCache);
            if (std::string("___ACK___") == (char *)CCommunicDevice::pReadCache)
            {
            }
            else
            {
                auto task = std::bind(&CNetConnector::subscribe, this, std::string((char *)pReadCache, n));
                taskQueue.push(task);
                // 收到数据回复 ACK
                CCommunicDevice::send(fd, std::string("___ACK___"));
            }
        }
    }
}

void CNetConnector::callback(const std::string func)
{
    std::string args;
    {
        std::unique_lock<std::mutex> guard(mLock);
        if (mapCBCache.find(func) == mapCBCache.end())
            return;
        args = mapCBCache[func];
    }

    Json::Value jsonRoot;
    Json::Reader jsonReader;
    if (jsonReader.parse(args, jsonRoot, false))
    {
        if (defCBTable.find(func) != defCBTable.end())
        {
            if (pLockCb.load())
                pLockCb.load()->Lock();
            defCBTable[func]->SubInterface(jsonRoot);
            if (pLockCb.load())
                pLockCb.load()->Unlock();
        }
    }
}

//出队列第一步调入这里
void CNetConnector::subscribe(const std::string data)
{
    static std::string sPacketCache;
    sPacketCache += data;
    size_t offsetB = 0, offsetE = 0;
    do
    {
        Json::Value jsonRoot;
        Json::Reader jsonReader;

        offsetB = sPacketCache.find(ZH_IPC_BEGIN_F, offsetB);
        offsetE = sPacketCache.find(ZH_IPC_END_F, offsetE);
        if (std::string::npos == offsetB)
        {
            sPacketCache.clear();
            break;
        }
        if (std::string::npos == offsetE)
        {
            sPacketCache = sPacketCache.substr(offsetB, sPacketCache.length() - offsetB);
            break;
        }
        size_t jsonOffset = sPacketCache.find(":json:", offsetB);
        if (std::string::npos == jsonOffset)
            continue;

        // LOGDBG("----------- %d %d %d", offsetB, offsetE, jsonOffset);
        std::string funcID = sPacketCache.substr(offsetB + 10, jsonOffset - offsetB - 10);
        std::string args = sPacketCache.substr(jsonOffset + 6, offsetE - jsonOffset - 6);

        offsetB += 10, offsetE += 10;
        LOGDBG("funcID: %s  >><<  %s", funcID.data(), args.data());

        if (jsonReader.parse(args, jsonRoot, false))
        {
            do
            {
                std::unique_lock<std::mutex> guard(mLock);
                mapCBCache[funcID] = args;
            } while (0);

            if (defCBTable.find(funcID) != defCBTable.end())
            {
                if (pLockCb.load())
                    pLockCb.load()->Lock();
                defCBTable[funcID]->SubInterface(jsonRoot);
                if (pLockCb.load())
                    pLockCb.load()->Unlock();
            }
        }
        else
            LOGERR("Description Failed to parse JSON data.");

    } while (true);
}

void CNetConnector::refresh(std::vector<std::string> table)
{
    Json::Value jsonRoot;
    Json::Reader jsonReader;

    std::unique_lock<std::mutex> guard(mLock);
    for (auto it : mapCBCache)
    {
        for (auto funID : table)
        {
            if ((it.first == funID) && jsonReader.parse(it.second, jsonRoot, false))
            {
                if (defCBTable.find(it.first) != defCBTable.end())
                {
                    if (pLockCb.load())
                    {
                        pLockCb.load()->Unlock();
                        pLockCb.load()->Lock();
                    }
                    defCBTable[it.first]->SubInterface(jsonRoot);
                    if (pLockCb.load())
                        pLockCb.load()->Unlock();
                }
            }
        }
    }
}

// ELFHash
int32_t CNetConnector::toPortNumber(const char *str)
{
    int32_t hash = 0, x = 0, i = 0;
    while (*str)
    {
        if (i < 20)
            hash = (hash << 2) + (*str);
        else
            i = 0, hash = (hash << 2) + (*++str);
        if ((x = hash & 0xF000L) != 0)
        {
            hash ^= (x >> 8);
            hash &= ~x;
        }
        i++;
    }
    return (hash & 0x7FFF) + 50000;
}