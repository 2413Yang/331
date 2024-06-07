/*
 * @Descripttion: 
 * @version: 
 * @Date: 2020-10-16 14:07:32
 * @LastEditTime: 2020-12-09 14:47:44
 */

#ifndef IPCINTERFACECORE__H__
#define IPCINTERFACECORE__H__

#include <atomic>
#include "ArgumentsTable.hpp"
#include "IPCCallback.hpp"

#define USER_DEFINED(returnType, funcName, ...)                                                                     \
    class Ipc##funcName                                                                                             \
    {                                                                                                               \
    public:                                                                                                         \
        using Indices = argvs::ArgIndexTuple<std::tuple_size<std::tuple<__VA_ARGS__>>::value>::__type;              \
                                                                                                                    \
        template <typename... T>                                                                                    \
        void operator()(T... args)                                                                                  \
        {                                                                                                           \
            if (mArguments(std::forward<T>(args)...))                                                               \
            {                                                                                                       \
                if (m_ptrPublisher.load())                                                                          \
                    m_ptrPublisher.load()->publish(std::string(#funcName), mArguments.data());                      \
                else                                                                                                \
                    mIPCcb();                                                                                       \
            }                                                                                                       \
        }                                                                                                           \
                                                                                                                    \
        static Ipc##funcName &GetObj()                                                                              \
        {                                                                                                           \
            static Ipc##funcName it;                                                                                \
            return it;                                                                                              \
        }                                                                                                           \
                                                                                                                    \
    private:                                                                                                        \
        Ipc##funcName() : m_ptrPublisher(NULL), mArguments(#funcName), mIPCcb(mArguments) {}                        \
        ~Ipc##funcName() {}                                                                                         \
                                                                                                                    \
    public:                                                                                                         \
        std::atomic<CPublisher *> m_ptrPublisher;                                                                   \
        CArgumentsTable<__VA_ARGS__> mArguments;                                                                    \
        CIPCCallback<__VA_ARGS__> mIPCcb;                                                                           \
    };                                                                                                              \
                                                                                                                    \
    template <typename... T>                                                                                        \
    returnType funcName(T... args)                                                                                  \
    {                                                                                                               \
        Ipc##funcName::GetObj()(args...);                                                                           \
    }                                                                                                               \
                                                                                                                    \
    namespace subscriber                                                                                            \
    {                                                                                                               \
        template <typename ICallback>                                                                               \
        void funcName(ICallback &cb)                                                                                \
        {                                                                                                           \
            std::function<std::function<void(void)>(void)> mReg = [&cb]() -> std::function<void(void)> {            \
                return Ipc##funcName::GetObj().mIPCcb.Register(&ICallback::funcName, cb, Ipc##funcName::Indices()); \
            };                                                                                                      \
            Ipc##funcName::GetObj().mIPCcb << mReg;                                                                 \
        }                                                                                                           \
                                                                                                                    \
        template <typename subscrib, typename ICallback>                                                            \
        void funcName(subscrib &Sub, ICallback &cb)                                                                 \
        {                                                                                                           \
            subscriber::funcName(cb);                                                                               \
            Sub.regSubscriber(#funcName, &Ipc##funcName::GetObj().mIPCcb);                                          \
        }                                                                                                           \
    }                                                                                                               \
                                                                                                                    \
    namespace publisher                                                                                             \
    {                                                                                                               \
        template <typename publish>                                                                                 \
        void funcName(publish &args)                                                                                \
        {                                                                                                           \
            Ipc##funcName::GetObj().m_ptrPublisher = &args;                                                         \
        }                                                                                                           \
    }

#endif /*IPCINTERFACECORE__H__*/
