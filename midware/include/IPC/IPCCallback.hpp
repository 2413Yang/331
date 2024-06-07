
#ifndef IPCCALLBACK__H__
#define IPCCALLBACK__H__
#include <vector>
#include "Mutex.h"
#ifdef _QNX_TARGET_
#include "PPSConnector.h"
#endif
#ifdef _LINUX_TARGET_
#include "NetConnector.h"
#endif
#include "ArgumentsTable.hpp"

using namespace std::placeholders;

template <typename... T>
class CIPCCallback : public CSubInterface
{
    using CBVoid = std::function<void(void)>;

public:
    CIPCCallback(CArgumentsTable<T...> &ArgTable)
        : mArgsLocal(&ArgTable) {}
    ~CIPCCallback() {}

    template <typename TCbPtr, typename TCb, size_t... Ind>
    CBVoid Register(TCbPtr ptr, TCb &cb, argvs::ArgTuple<Ind...>)
    {
        return std::bind(ptr, &cb, std::get<Ind>(std::move((*mArgsLocal)()))...);
    }

    void operator<<(std::function<CBVoid(void)> &func)
    {
        mCbTable.push_back(func);
    }

#ifdef _QNX_TARGET_
    void SubInterface(pps_attrib_t &arg)
    {
        if (mArgsLocal->separate(arg))
        {
            ZH::BaseLib::CAutoLock lock(mMutex);
            for (auto it : mCbTable)
                it()();
        }
    }
#endif
#ifdef _LINUX_TARGET_
    void SubInterface(Json::Value &arg)
    {
        if (mArgsLocal->separate(arg))
        {
            ZH::BaseLib::CAutoLock lock(mMutex);
            for (auto it : mCbTable)
                it()();
        }
    }
#endif
    void operator()()
    {
        ZH::BaseLib::CAutoLock lock(mMutex);
        for (auto it : mCbTable)
            it()();
    }

private:
    ZH::BaseLib::CMutex mMutex;
    CArgumentsTable<T...> *mArgsLocal;
    std::vector<std::function<CBVoid(void)>> mCbTable;
};

#endif /*IPCCALLBACK__H__*/