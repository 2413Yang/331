
#ifndef TEMPLATETYPEJSON__H__
#define TEMPLATETYPEJSON__H__
#include "CustomTypeJson.h"

namespace Jsoncpp
{
    // 基础pair
    template <typename T1, typename T2>
    void serialize(std::string tmpName, std::pair<T1, T2> param, Json::Value &encoder)
    {
        Json::Value mPair;
        serialize("first", param.first, mPair);
        serialize("second", param.second, mPair);
        encoder[tmpName] = mPair;
    }

    template <typename T1, typename T2>
    void deserialize(std::string tmpName, std::pair<T1, T2> &pParam, Json::Value &decoder)
    {
        T1 firstIt;
        T2 secondIt;
        Json::Value mPair = decoder[tmpName];
        deserialize("first", firstIt, mPair);
        deserialize("second", secondIt, mPair);
        pParam = {firstIt, secondIt};
    }

    // 基础向量
    template <typename T>
    void serialize(std::string tmpName, std::vector<T> param, Json::Value &encoder)
    {
        Json::Value array;

        int sub = 0;
        for (auto it : param)
        {
            serialize(std::string(), it, array[sub]);
            sub++;
        }
        encoder[tmpName] = array;
    }

    template <typename T>
    void deserialize(std::string tmpName, std::vector<T> &pParam, Json::Value &decoder)
    {
        T it;
        pParam.clear();
        Json::Value array = decoder[tmpName];

        for (uint32_t i = 0; i < array.size(); i++)
        {
            deserialize(std::string(), it, array[i]);
            pParam.push_back(it);
        }
    }

    // 基础链表
    template <typename T>
    void serialize(std::string tmpName, std::list<T> param, Json::Value &encoder)
    {
        Json::Value array;

        int sub = 0;
        for (auto it : param)
        {
            serialize(std::string(), it, array[sub]);
            sub++;
        }
        encoder[tmpName] = array;
    }

    template <typename T>
    void deserialize(std::string tmpName, std::list<T> &pParam, Json::Value &decoder)
    {
        T it;
        pParam.clear();
        Json::Value array = decoder[tmpName];

        for (uint32_t i = 0; i < array.size(); i++)
        {
            deserialize(std::string(), it, array[i]);
            pParam.push_back(it);
        }
    }

    // 基础映射
    template <typename T1, typename T2>
    void serialize(std::string tmpName, std::map<T1, T2> param, Json::Value &encoder)
    {
        Json::Value array;
        int sub = 0;
        for (auto it : param)
        {
            serialize("first", it.first, array[sub]);
            serialize("second", it.second, array[sub]);
            sub++;
        }
        encoder[tmpName] = array;
    }

    template <typename T1, typename T2>
    void deserialize(std::string tmpName, std::map<T1, T2> &pParam, Json::Value &decoder)
    {
        T1 firstIt;
        T2 secondIt;

        pParam.clear();
        Json::Value array = decoder[tmpName];

        for (uint32_t i = 0; i < array.size(); i++)
        {
            deserialize("first", firstIt, array[i]);
            deserialize("second", secondIt, array[i]);
            pParam[firstIt] = secondIt;
        }
    }

    

}
#endif /*TEMPLATETYPEJSON__H__*/