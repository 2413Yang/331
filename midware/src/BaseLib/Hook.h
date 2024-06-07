#pragma once

#include <map>
#include <functional>
#include "Mutex.h"

namespace ZH
{
    namespace BaseLib
    {
        template <typename TakerType, typename FlagType, typename... FuncArgType>
        class CHook
        {
        public:
            typedef void (TakerType::*ProcessFunc)(FuncArgType...);
            typedef std::multimap<FlagType, ProcessFunc> VesselIDToFunc;

        public:
            CHook(TakerType *pHookTaker)
                : m_pHookTaker(pHookTaker)
            {
            }
            virtual ~CHook() {}

            void Connect(const FlagType &objFlag, ProcessFunc pFunc)
            {
                CAutoLock objAutoLock(m_mutexVessel);
                m_mapIDToFunc.insert(std::make_pair(objFlag, pFunc));
            }

            void GetData(const FlagType &objFlag, FuncArgType... Args)
            {
                CAutoLock objAutoLock(m_mutexVessel);

                typename VesselIDToFunc::iterator it = m_mapIDToFunc.begin();

                while (m_mapIDToFunc.end() != it)
                {
                    if (objFlag == it->first && NULL != it->second)
                    {
                        (m_pHookTaker->*(it->second))(std::forward<FuncArgType>(Args)...);
                    }

                    ++it;
                }
            }

        private:
            TakerType *m_pHookTaker;
            VesselIDToFunc m_mapIDToFunc;
            CMutex m_mutexVessel;
        };

        template <typename T1, typename... T2>
        class CMultifunc
        {
        public:
            void Connect(T1 l1, std::function<void(T2...)> l2)
            {
                if (m_map.find(l1) == m_map.end())
                    m_map.insert(std::make_pair(l1, l2));
            };

            bool operator()(T1 l1, T2... args)
            {
                bool ret = false;
                typename std::map<T1, std::function<void(T2...)>>::iterator iter;
                if ((iter = m_map.find(l1)) != m_map.end())
                {
                    iter->second(std::forward<T2>(args)...);
                    ret = true;
                }
                return ret;
            };

        private:
            std::map<T1, std::function<void(T2...)>> m_map;
        };
    } // namespace BaseLib
} // namespace ZH
