
#include "PPSConnector.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mlog.h"

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

CSubscriber::CSubscriber(std::string node)
    : devNode(node)
{
    devNode.registerCB(std::bind(&CSubscriber::handleEvent, this, std::placeholders::_1));
}

CSubscriber::~CSubscriber()
{
    devNode.stop();
}

void CSubscriber::handleEvent(std::string cache)
{
    char *pps_data = (char *)cache.c_str();
    pps_status_t pps_status;
    do
    {
        pps_attrib_t info;
        pps_status = ppsparse(&pps_data, NULL, NULL, &info, 0);
        switch (pps_status)
        {
        case PPS_END:
        {
            break;
        }
        break;
        case PPS_ERROR:
        {
            LOGERR("pps_attrib_t: %s value = %s\n",
                   info.attr_name ? info.attr_name : "NULL", info.value ? info.value : "NULL");
        }
        break;
        case PPS_ATTRIBUTE:
        {
            if (defCBTable.find(std::string(info.attr_name)) != defCBTable.end())
            {
                if (pLockCb.load())
                    pLockCb.load()->Lock();
                defCBTable[info.attr_name]->SubInterface(info);
                if (pLockCb.load())
                    pLockCb.load()->Unlock();
            }
        }
        break;
        default:
        {
        }
        break;
        }
    } while (pps_status != PPS_END);
}

void CSubscriber::regSubscriber(std::string interfaceName, CSubInterface *pInterface)
{
    if (defCBTable.find(interfaceName) == defCBTable.end() && pInterface != NULL)
        defCBTable[interfaceName] = pInterface;
}

void CSubscriber::registerLock(ZH::BaseLib::CMutex &op)
{
    pLockCb = &op;
}

CPPSConnector::CPPSConnector(std::string node)
    : CSubscriber(node)
{
}

CPPSConnector::~CPPSConnector()
{
}

void CPPSConnector::start(void)
{
    devNode.start();
}
/* 
    * @Description: 发布
    * @param: 参数表
    * @return: 
    */
void CPPSConnector::publish(std::string id, const std::string &args)
{
    devNode.send(args);
}