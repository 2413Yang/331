

#ifndef NETCONNECTOR__H__
#define NETCONNECTOR__H__

#include <functional>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include "Mutex.h"
#include "json/json.h"
#include "NetDevice.h"
//发布者
class CPublisher
{
public:
    virtual void publish(const std::string, std::string &) = 0;
};

class CSubscriber;
class CSubInterface
{
public:
    virtual void SubInterface(Json::Value &) = 0;
    static bool isJsonFmtData(const char *encoding);
};
//订阅者
class CSubscriber
{
public:
    CSubscriber();
    virtual ~CSubscriber();
    virtual void callback(const std::string func) = 0;
    virtual void subscribe(const std::string args) = 0;
    void regSubscriber(std::string interfaceName, CSubInterface *pInterface);
    static void registerLock(ZH::BaseLib::CMutex &);

protected:
    static std::map<std::string, CSubInterface *> defCBTable;
    static std::atomic<ZH::BaseLib::CMutex *> pLockCb;
};

/* 
 * @Description: 本地发布订阅连接器
 */
class CNetConnector
    : public CPublisher,
      public CSubscriber,
      public CNetDevice
{
public:
    CNetConnector(std::string node);
    virtual ~CNetConnector();

    void start(void);
    void subscribe(const std::string args);
    void publish(const std::string id, std::string &args);
    void refresh(std::vector<std::string>);

private:
    void callback(const std::string func);
    int32_t connect();
    void handleEvent(int fd);
    int32_t toPortNumber(const char *str);
    static void keepConnectedThread();

private:
    std::atomic_int m_isClientFD;
    std::string strNode, cmdline;
    std::vector<uint32_t> mClients;
    std::mutex mLock;
    std::map<std::string, std::string> mapParameterData, mapCBCache;
    static std::thread *t_ptrKeepConnected;
    static std::mutex s_cacheLock;
    static std::vector<std::function<bool(void)>> s_loopTask;
};
#endif /*NETCONNECTOR__H__*/
