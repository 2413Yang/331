
#include "CustomTypeJson.h"

namespace Jsoncpp
{
    void serialize(std::string tmpName, const StuTelltableLampState pParam, Json::Value &out)
    {
        Json::Value item;
        serialize("strID", pParam.strID, item);
        serialize("state", pParam.state, item);
        out[tmpName] = item;
    }

    void deserialize(std::string tmpName, StuTelltableLampState &pParam, Json::Value &in)
    {
        Json::Value item = in[tmpName];
        deserialize("strID", pParam.strID, item);
        deserialize("state", pParam.state, item);
    }

    void serialize(std::string tmpName, const StuChargeSubscribe pParam, Json::Value &out)
    {
        Json::Value item;
        serialize("Year", pParam.Year, item);
        serialize("Month", pParam.Month, item);
        serialize("Day", pParam.Day, item);
        serialize("Hour", pParam.Hour, item);
        serialize("Minute", pParam.Minute, item);
        out[tmpName] = item;
    }

    void deserialize(std::string tmpName, StuChargeSubscribe &pParam, Json::Value &in)
    {
        Json::Value item = in[tmpName];
        deserialize("Year", pParam.Year, item);
        deserialize("Month", pParam.Month, item);
        deserialize("Day", pParam.Day, item);
        deserialize("Hour", pParam.Hour, item);
        deserialize("Minute", pParam.Minute, item);
    }

    void serialize(std::string tmpName, const StuCallRecord pParam, Json::Value &out)
    {
        Json::Value item;
        serialize("type", pParam.type, item);
        serialize("name", pParam.name, item);
        serialize("amount", pParam.amount, item);
        out[tmpName] = item;
    }

    void deserialize(std::string tmpName, StuCallRecord &pParam, Json::Value &in)
    {
        Json::Value item = in[tmpName];
        deserialize("type", pParam.type, item);
        deserialize("name", pParam.name, item);
        deserialize("amount", pParam.amount, item);
    }

    void serialize(std::string tmpName, const StuCarDoorState pParam, Json::Value &out)
    {
        Json::Value item;
        serialize("EmID", pParam.ID, item);
        serialize("state", pParam.state, item);
        out[tmpName] = item;
    }

    void deserialize(std::string tmpName, StuCarDoorState &pParam, Json::Value &in)
    {
        Json::Value item = in[tmpName];
        deserialize("EmID", pParam.ID, item);
        deserialize("state", pParam.state, item);
    }

    void serialize(std::string tmpName, const StuCarTire::tire pParam, Json::Value &out)
    {
        Json::Value item;
        serialize("tireStatus", pParam.tireStatus, item);
        serialize("validFlag", pParam.validFlag, item);
        serialize("presValue", pParam.presValue, item);
        out[tmpName] = item;
    }

    void deserialize(std::string tmpName, StuCarTire::tire &pParam, Json::Value &in)
    {
        Json::Value item = in[tmpName];
        deserialize("presValue", pParam.presValue, item);
        deserialize("tireStatus", pParam.tireStatus, item);
        deserialize("validFlag", pParam.validFlag, item);
    }

    void serialize(std::string tmpName, const StuCarTire pParam, Json::Value &out)
    {
        Json::Value item;
        serialize("LFTire", pParam.LFTire, item);
        serialize("RFTire", pParam.RFTire, item);
        serialize("LRTire", pParam.LRTire, item);
        serialize("RRTire", pParam.RRTire, item);
        out[tmpName] = item;
    }

    void deserialize(std::string tmpName, StuCarTire &pParam, Json::Value &in)
    {
        Json::Value item = in[tmpName];
        deserialize("LFTire", pParam.LFTire, item);
        deserialize("RFTire", pParam.RFTire, item);
        deserialize("LRTire", pParam.LRTire, item);
        deserialize("RRTire", pParam.RRTire, item);
    }
};