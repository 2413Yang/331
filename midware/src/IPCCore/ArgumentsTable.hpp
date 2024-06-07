
#ifndef ARGUMENTSTABLE__H__
#define ARGUMENTSTABLE__H__
#include <tuple>
#include <iostream>
#include <typeinfo>
#include "mlog.h"
#ifdef _QNX_TARGET_
#include "PPSConnector.h"
#include "SerializePPS.h"
#endif
#ifdef _LINUX_TARGET_
#include "CustomTypeJson.h"
#include "TemplateTypeJson.hpp"
#endif

#define isStrType(vir) ((typeid(vir) == typeid(const char *)) || typeid(vir) == typeid(std::string))

namespace argvs
{
    template <size_t... _Indexes>
    struct ArgTuple
    {
    };

    /// Builds an ArgTuple< 0, 1, 2, ..., _Num - 1 >.
    template <std::size_t _Num, typename _Tuple = ArgTuple<>>
    struct ArgIndexTuple;

    template <std::size_t _Num, size_t... _Indexes>
    struct ArgIndexTuple<_Num, ArgTuple<_Indexes...>>
        : ArgIndexTuple<_Num - 1, ArgTuple<_Indexes..., sizeof...(_Indexes)>>
    {
    };

    template <size_t... _Indexes>
    struct ArgIndexTuple<0, ArgTuple<_Indexes...>>
    {
        typedef ArgTuple<_Indexes...> __type;
    };
};

// 类型检查
template <typename ArgT1, typename ArgT2, size_t N>
class CType
{
public:
    static bool match(ArgT1 &arg1, ArgT2 &arg2)
    {
        bool ret = true;
        if (CType<ArgT1, ArgT2, N - 1>::match(arg1, arg2))
        {
            auto param1 = std::get<N - 1>(arg1);
            auto param2 = std::get<N - 1>(arg2);
            if ((typeid(param1) != typeid(param2)) && (!isStrType(param1) || !isStrType(param2)))
            {
                LOGERR(" ERROE: The parameter tables do not match");
                ret = false;
            }
        }
        return ret;
    }
};
template <typename ArgT1, typename ArgT2>
class CType<ArgT1, ArgT2, 1>
{
public:
    static bool match(ArgT1 &arg1, ArgT2 &arg2)
    {
        bool ret = true;
        auto param1 = std::get<0>(arg1);
        auto param2 = std::get<0>(arg2);
        if ((typeid(param1) != typeid(param2)) && (!isStrType(param1) || !isStrType(param2)))
        {
            LOGERR(" ERROE: The parameter tables do not match");
            ret = false;
        }
        return ret;
    }
};

// 参数操作
template <typename ArgT, size_t N>
class CArgumentsProcess
{
public:
    static void printf(ArgT arg)
    {
        CArgumentsProcess<ArgT, N - 1>::printf(arg);
        auto v = std::get<N - 1>(arg);
        std::cout << "type: " << typeid(v).name() << " value: " << v << std::endl;
    }

#ifdef _QNX_TARGET_
    template <typename SerialT>
    static void serialization(const ArgT &arg, SerialT &param)
    {
        CArgumentsProcess<ArgT, N - 1>::serialization(arg, param);
        auto tmp = std::get<N - 1>(arg);
        Ipps::serialize(tmp, std::string("param") + std::to_string(N - 1), param);
    }

    template <typename DeserialT>
    static void deserialization(DeserialT &param, ArgT &arg)
    {
        CArgumentsProcess<ArgT, N - 1>::deserialization(param, arg);
        auto &tmp = std::get<N - 1>(arg);
        Ipps::deserialize(tmp, std::string("param") + std::to_string(N - 1), param);
    }
#endif
#ifdef _LINUX_TARGET_
    template <typename SerialT>
    static void serialization(const ArgT &arg, SerialT &param)
    {
        CArgumentsProcess<ArgT, N - 1>::serialization(arg, param);
        auto tmp = std::get<N - 1>(arg);
        Jsoncpp::serialize(std::string("param") + std::to_string(N - 1), tmp, param);
    }

    template <typename DeserialT>
    static void deserialization(DeserialT &param, ArgT &arg)
    {
        CArgumentsProcess<ArgT, N - 1>::deserialization(param, arg);
        auto &tmp = std::get<N - 1>(arg);
        Jsoncpp::deserialize(std::string("param") + std::to_string(N - 1), tmp, param);
    }
#endif
};
template <typename ArgT>
class CArgumentsProcess<ArgT, 1>
{
public:
    static void printf(ArgT arg)
    {
        auto v = std::get<0>(arg);
        std::cout << "type: " << typeid(v).name() << " value: " << v << std::endl;
    }

#ifdef _QNX_TARGET_
    template <typename SerialT>
    static void serialization(const ArgT &arg, SerialT &param)
    {
        auto tmp = std::get<0>(arg);
        Ipps::serialize(tmp, std::string("param") + std::to_string(0), param);
    }

    template <typename DeserialT>
    static void deserialization(DeserialT &param, ArgT &arg)
    {
        auto &tmp = std::get<0>(arg);
        Ipps::deserialize(tmp, std::string("param") + std::to_string(0), param);
    }
#endif
#ifdef _LINUX_TARGET_
    template <typename SerialT>
    static void serialization(const ArgT &arg, SerialT &param)
    {
        auto tmp = std::get<0>(arg);
        Jsoncpp::serialize(std::string("param") + std::to_string(0), tmp, param);
    }

    template <typename DeserialT>
    static void deserialization(DeserialT &param, ArgT &arg)
    {
        auto &tmp = std::get<0>(arg);
        Jsoncpp::deserialize(std::string("param") + std::to_string(0), tmp, param);
    }
#endif
};

template <typename... T>
class CArgumentsTable
{
public:
    CArgumentsTable(std::string name)
        : mName(name) {}

    bool operator()(T... args)
    {
        bool ret = false;
        auto tmp = (mTable = std::forward_as_tuple(args...));
        if (CType<decltype(tmp), decltype(mTable), sizeof...(T)>::match(tmp, mTable))
            ret = true;
        else
            LOGERR("parameter list type matches error.");
        return ret;
    }
#ifdef _QNX_TARGET_
    bool separate(pps_attrib_t &info)
    {
        bool ret = false;
        if (CSubInterface::isJsonFmtData(info.encoding))
        {
            pps_decoder_initialize(&mDeserializeDtata, NULL);
            if (!pps_decoder_parse_json_str(&mDeserializeDtata, info.value))
            {
                ret = true;
                pps_decoder_push(&mDeserializeDtata, NULL);
                CArgumentsProcess<decltype(mTable), std::tuple_size<std::tuple<T...>>::value>::
                    deserialization(mDeserializeDtata, mTable);
                pps_decoder_pop(&mDeserializeDtata);
            }
            else
                LOGERR("Parse error parsing JSON data.");
            pps_decoder_cleanup(&mDeserializeDtata);
        }
        return ret;
    }

    std::string &data(void)
    {
        pps_encoder_initialize(&mSerializeDtata, false);
        pps_encoder_start_object(&mSerializeDtata, mName.data());
        CArgumentsProcess<decltype(mTable), sizeof...(T)>::serialization(mTable, mSerializeDtata);
        pps_encoder_end_object(&mSerializeDtata);
        mEncodeData = std::string((char *)pps_encoder_buffer(&mSerializeDtata), pps_encoder_length(&mSerializeDtata));
        return mEncodeData;
    }

private:
    std::string mEncodeData;
    pps_encoder_t mSerializeDtata;
    pps_decoder_t mDeserializeDtata;

public:
#endif
#ifdef _LINUX_TARGET_
    bool separate(Json::Value &info)
    {
        CArgumentsProcess<decltype(mTable), std::tuple_size<std::tuple<T...>>::value>::deserialization(info, mTable);
        return true;
    }

    std::string &data(void)
    {
        Json::FastWriter writer;
        CArgumentsProcess<decltype(mTable), sizeof...(T)>::serialization(mTable, jsonEncoder);
        argvsData = mName + ":json:" + writer.write(jsonEncoder); //.toStyledString();
        return argvsData;
    }

private:
    std::string argvsData;
    Json::Value jsonEncoder, jsonDecoder;

public:
#endif
    void printf(void)
    {
        CArgumentsProcess<decltype(mTable), sizeof...(T)>::printf(mTable);
    }

    std::tuple<T...> &operator()()
    {
        return mTable;
    }

    void operator()(std::tuple<T...> &table)
    {
        mTable.swap(table);
    }

    std::string &name()
    {
        return mName;
    }

private:
    std::string mName;
    std::tuple<T...> mTable;
};

#endif /*ARGUMENTSTABLE__H__*/