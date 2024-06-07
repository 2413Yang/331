
#include "BaseTypeJson.h"

namespace Jsoncpp
{
    void serialize(std::string tmpName, bool bParam, Json::Value &encoder)
    {
        (tmpName.length() > 0) ? (encoder[tmpName] = bParam) : (encoder = bParam);
    }
    void serialize(std::string tmpName, int iParam, Json::Value &encoder)
    {
        (tmpName.length() > 0) ? (encoder[tmpName] = iParam) : (encoder = iParam);
    }
    void serialize(std::string tmpName, unsigned int uiParam, Json::Value &encoder)
    {
        (tmpName.length() > 0) ? (encoder[tmpName] = uiParam) : (encoder = uiParam);
    }
    void serialize(std::string tmpName, long lParam, Json::Value &encoder)
    {
        (tmpName.length() > 0) ? (encoder[tmpName] = (int32_t)lParam) : (encoder = (int32_t)lParam);
    }
    void serialize(std::string tmpName, unsigned long ulParam, Json::Value &encoder)
    {
        (tmpName.length() > 0) ? (encoder[tmpName] = (uint32_t)ulParam) : (encoder = (uint32_t)ulParam);
    }
    void serialize(std::string tmpName, double fParam, Json::Value &encoder)
    {
        (tmpName.length() > 0) ? (encoder[tmpName] = fParam) : (encoder = fParam);
    }
    void serialize(std::string tmpName, float fParam, Json::Value &encoder)
    {
        (tmpName.length() > 0) ? (encoder[tmpName] = (double)fParam) : (encoder = (double)fParam);
    }
    void serialize(std::string tmpName, const char *pParam, Json::Value &encoder)
    {
        (tmpName.length() > 0) ? (encoder[tmpName] = pParam) : (encoder = pParam);
    }
    void serialize(std::string tmpName, const std::string pParam, Json::Value &encoder)
    {
        (tmpName.length() > 0) ? (encoder[tmpName] = pParam) : (encoder = pParam);
    }

    /*******************************************************************************************************
********************************************************************************************************/
    void deserialize(std::string tmpName, bool &bParam, Json::Value &decoder)
    {
        (tmpName.length() > 0) ? (bParam = decoder[tmpName].asBool()) : (bParam = decoder.asBool());
    }
    void deserialize(std::string tmpName, int &iParam, Json::Value &decoder)
    {
        (tmpName.length() > 0) ? (iParam = decoder[tmpName].asInt()) : (iParam = decoder.asInt());
    }
    void deserialize(std::string tmpName, unsigned int &iParam, Json::Value &decoder)
    {
        (tmpName.length() > 0) ? (iParam = decoder[tmpName].asUInt()) : (iParam = decoder.asUInt());
    }
    void deserialize(std::string tmpName, long &lParam, Json::Value &decoder)
    {
        (tmpName.length() > 0) ? (lParam = decoder[tmpName].asInt()) : (lParam = decoder.asInt());
    }
    void deserialize(std::string tmpName, unsigned long &ulParam, Json::Value &decoder)
    {
        (tmpName.length() > 0) ? (ulParam = decoder[tmpName].asUInt()) : (ulParam = decoder.asUInt());
    }
    void deserialize(std::string tmpName, float &fParam, Json::Value &decoder)
    {
        (tmpName.length() > 0) ? (fParam = decoder[tmpName].asDouble()) : (fParam = decoder.asDouble());
    }
    void deserialize(std::string tmpName, double &fParam, Json::Value &decoder)
    {
        (tmpName.length() > 0) ? (fParam = decoder[tmpName].asDouble()) : (fParam = decoder.asDouble());
    }
    void deserialize(std::string tmpName, char *pParam, Json::Value &decoder)
    {
        (tmpName.length() > 0) ? (pParam = (char *)decoder[tmpName].asCString()) : (pParam = (char *)decoder.asCString());
    }
    void deserialize(std::string tmpName, std::string &strParam, Json::Value &decoder)
    {
        (tmpName.length() > 0) ? (strParam = decoder[tmpName].asString()) : (strParam = decoder.asString());
    }

} // namespace Ipps