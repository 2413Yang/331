
#ifndef CUSTOMTYPEJSON__H__
#define CUSTOMTYPEJSON__H__

#include "BaseTypeJson.h"

namespace Jsoncpp
{

    //自定义枚举
    template <typename T>
    void serialize(std::string tmpName, T iParam, Json::Value &encoder)
    {
        encoder[tmpName] = (int32_t)iParam;
    }
    template <typename T>
    void deserialize(std::string tmpName, T &iParam, Json::Value &decoder)
    {
        iParam = (T)decoder[tmpName].asInt();
    }

    void serialize(std::string tmpName, const StuTelltableLampState pParam, Json::Value &encoder);
    void deserialize(std::string tmpName, StuTelltableLampState &pParam, Json::Value &decoder);

    void serialize(std::string tmpName, const StuCallRecord pParam, Json::Value &encoder);
    void deserialize(std::string tmpName, StuCallRecord &pParam, Json::Value &decoder);

    void serialize(std::string tmpName, const StuCarDoorState pParam, Json::Value &encoder);
    void deserialize(std::string tmpName, StuCarDoorState &pParam, Json::Value &decoder);

    void serialize(std::string tmpName, const StuCarTire::tire pParam, Json::Value &encoder);
    void deserialize(std::string tmpName, StuCarTire::tire &pParam, Json::Value &decoder);

    void serialize(std::string tmpName, const StuCarTire pParam, Json::Value &encoder);
    void deserialize(std::string tmpName, StuCarTire &pParam, Json::Value &decoder);

    void serialize(std::string tmpName, const StuChargeSubscribe pParam, Json::Value &encoder);
    void deserialize(std::string tmpName, StuChargeSubscribe &pParam, Json::Value &decoder);

} // namespace Ipps

#endif /*CUSTOMTYPEJSON__H__*/
