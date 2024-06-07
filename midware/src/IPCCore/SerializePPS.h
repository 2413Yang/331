/*
 * @Descripttion: 
 * @version: 
 * @Date: 2020-10-21 18:48:47
 * @LastEditTime: 2020-10-24 21:04:12
 */

#ifndef SERIALIZE_PPS__H__
#define SERIALIZE_PPS__H__
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <sys/pps.h>
#include "mlog.h"
#include "DataDefine.h"
// #include "json/json.h"

namespace Ipps
{
#define StrValue(param) (param.length() > 0 ? tmpName.c_str() : NULL)

    void serialize(bool bParam, std::string tmpName, pps_encoder_t &out);
    void serialize(int iParam, std::string tmpName, pps_encoder_t &out);
    void serialize(unsigned int uiParam, std::string tmpName, pps_encoder_t &out);
    void serialize(long lParam, std::string tmpName, pps_encoder_t &out);
    void serialize(unsigned long ulParam, std::string tmpName, pps_encoder_t &out);
    void serialize(double fParam, std::string tmpName, pps_encoder_t &out);
    void serialize(float fParam, std::string tmpName, pps_encoder_t &out);
    void serialize(const char *pParam, std::string tmpName, pps_encoder_t &out);
    void serialize(const std::string pParam, std::string tmpName, pps_encoder_t &out);

    /*******************************************************************************************************
********************************************************************************************************/
    void deserialize(bool &bParam, std::string tmpName, pps_decoder_t &in);
    void deserialize(int &iParam, std::string tmpName, pps_decoder_t &in);
    void deserialize(unsigned int &iParam, std::string tmpName, pps_decoder_t &in);
    void deserialize(long &iParam, std::string tmpName, pps_decoder_t &in);
    void deserialize(unsigned long &iParam, std::string tmpName, pps_decoder_t &in);
    void deserialize(float &fParam, std::string tmpName, pps_decoder_t &in);
    void deserialize(double &fParam, std::string tmpName, pps_decoder_t &in);
    void deserialize(char *pParam, std::string tmpName, pps_decoder_t &in);
    void deserialize(std::string &pParam, std::string tmpName, pps_decoder_t &in);

    //自定义枚举
    template <typename T>
    void serialize(T iParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_int(&out, StrValue(tmpName), (int)iParam);
    }
    template <typename T>
    void deserialize(T &iParam, std::string tmpName, pps_decoder_t &in)
    {
        int iTmp = -1;
        pps_decoder_get_int(&in, StrValue(tmpName), &iTmp);
        iParam = (T)iTmp;
    }

    void serialize(const StuTelltableLampState pParam, std::string tmpName, pps_encoder_t &out);
    void deserialize(StuTelltableLampState &pParam, std::string tmpName, pps_decoder_t &in);
    void serialize(const StuCarDoorState pParam, std::string tmpName, pps_encoder_t &out);
    void deserialize(StuCarDoorState &pParam, std::string tmpName, pps_decoder_t &in);
    void serialize(const StuCarTire::tire pParam, std::string tmpName, pps_encoder_t &out);
    void deserialize(StuCarTire::tire &pParam, std::string tmpName, pps_decoder_t &in);
    void serialize(const StuCarTire pParam, std::string tmpName, pps_encoder_t &out);
    void deserialize(StuCarTire &pParam, std::string tmpName, pps_decoder_t &in);

    // 基础pair
    template <typename T1, typename T2>
    void serialize(std::pair<T1, T2> param, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_start_object(&out, StrValue(tmpName));
        serialize(param.first, std::string("first"), out);
        serialize(param.second, std::string("second"), out);
        pps_encoder_end_object(&out);
    }

    template <typename T1, typename T2>
    void deserialize(std::pair<T1, T2> &pParam, std::string tmpName, pps_decoder_t &in)
    {
        T1 firstIt;
        T2 secondIt;
        pps_decoder_push_object(&in, StrValue(tmpName));
        deserialize(pParam, std::string("first"), in);
        deserialize(secondIt, std::string("second"), in);
        pps_decoder_pop(&in);
        pParam = {firstIt, secondIt};
    }

    // 基础向量
    template <typename T>
    void serialize(std::vector<T> param, std::string tmpName, pps_encoder_t &out)
    {
        int sub = 0;
        pps_encoder_start_array(&out, tmpName.data());
        for (auto it : param)
        {
            serialize(it, std::string(""), out);
            sub++;
        }
        pps_encoder_end_array(&out);
    }

    template <typename T>
    void deserialize(std::vector<T> &pParam, std::string tmpName, pps_decoder_t &in)
    {
        T it;
        pps_decoder_push_array(&in, tmpName.data());
        int sub = pps_decoder_length(&in);
        pParam.clear();
        for (int i = 0; i < sub; i++)
        {
            deserialize(it, std::string(""), in);
            pParam.push_back(it);
        }

        pps_decoder_pop(&in);
    }

    // 基础映射
    template <typename T1, typename T2>
    void serialize(std::map<T1, T2> param, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_start_array(&out, tmpName.data());
        for (auto it : param)
        {
            pps_encoder_start_object(&out, StrValue(tmpName));
            serialize(it.first, std::string("first"), out);
            serialize(it.second, std::string("second"), out);
            pps_encoder_end_object(&out);
        }
        pps_encoder_end_array(&out);
    }

    template <typename T1, typename T2>
    void deserialize(std::map<T1, T2> &pParam, std::string tmpName, pps_decoder_t &in)
    {
        T1 firstIt;
        T2 secondIt;
        pps_decoder_push_array(&in, tmpName.data());
        int sub = pps_decoder_length(&in);
        pParam.clear();
        for (int i = 0; i < sub; i++)
        {
            pps_decoder_push_object(&in, StrValue(tmpName));
            deserialize(firstIt, std::string("first"), in);
            deserialize(secondIt, std::string("second"), in);
            pps_decoder_pop(&in);
            pParam[firstIt] = secondIt;
        }

        pps_decoder_pop(&in);
    }

} // namespace Ipps

#endif /*SERIALIZE__H__*/