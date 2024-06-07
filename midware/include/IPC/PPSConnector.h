

#ifndef PPSConnector__H__
#define PPSConnector__H__

#include <functional>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <sys/pps.h>
#include "Mutex.h"
#include "PPSNodeDevice.h"

class CPublisher
{
public:
    virtual void publish(std::string, const std::string &) = 0;
};

class CSubscriber;
class CSubInterface
{
public:
    virtual void SubInterface(pps_attrib_t &) = 0;
    static bool isJsonFmtData(const char *encoding);
};

class CSubscriber
{
public:
    CSubscriber(std::string node);
    virtual ~CSubscriber();

    void handleEvent(std::string);
    void regSubscriber(std::string interfaceName, CSubInterface *pInterface);

    static void registerLock(ZH::BaseLib::CMutex &);

protected:
    CPPSNodeDevice devNode;

    static std::map<std::string, CSubInterface *> defCBTable;
    static std::atomic<ZH::BaseLib::CMutex *> pLockCb;
};

/* 
 * @Description: 本地发布订阅连接器
 * 
*/
class CPPSConnector : public CPublisher, public CSubscriber
{
public:
    CPPSConnector(std::string node);
    virtual ~CPPSConnector();

    /* 
    * @Description: 发布
    * @param: 参数表
    * @return: 
    */
    void publish(std::string id, const std::string &args);

    /* 
    * @Description: 开始 发布/订阅
    * @param: 参数表
    * @return: 
    */
    void start(void);

private:
    std::string ppsNode;

    //  std::atomic<bool> bLoop;
};

#endif /*PPSConnector__H__*/