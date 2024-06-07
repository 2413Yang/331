

#ifndef BASETYPEJSON__H__
#define BASETYPEJSON__H__

#include <map>
#include <list>
#include <vector>
#include <string>
#include <string.h>
#include "mlog.h"
#include "DataDefine.h"
#include "json/json.h"
// #include "TemplateTypeJson.hpp"

namespace Jsoncpp
{
    void serialize(std::string tmpName, bool bParam, Json::Value &encoder);
    void serialize(std::string tmpName, int iParam, Json::Value &encoder);
    void serialize(std::string tmpName, unsigned int uiParam, Json::Value &encoder);
    void serialize(std::string tmpName, long lParam, Json::Value &encoder);
    void serialize(std::string tmpName, unsigned long ulParam, Json::Value &encoder);
    void serialize(std::string tmpName, double fParam, Json::Value &encoder);
    void serialize(std::string tmpName, float fParam, Json::Value &encoder);
    void serialize(std::string tmpName, const char *pParam, Json::Value &encoder);
    void serialize(std::string tmpName, const std::string pParam, Json::Value &encoder);

    /*******************************************************************************************************
********************************************************************************************************/
    void deserialize(std::string tmpName, bool &bParam, Json::Value &decoder);
    void deserialize(std::string tmpName, int &iParam, Json::Value &decoder);
    void deserialize(std::string tmpName, unsigned int &iParam, Json::Value &decoder);
    void deserialize(std::string tmpName, long &iParam, Json::Value &decoder);
    void deserialize(std::string tmpName, unsigned long &iParam, Json::Value &decoder);
    void deserialize(std::string tmpName, float &fParam, Json::Value &decoder);
    void deserialize(std::string tmpName, double &fParam, Json::Value &decoder);
    void deserialize(std::string tmpName, char *pParam, Json::Value &decoder);
    void deserialize(std::string tmpName, std::string &pParam, Json::Value &decoder);

} // namespace Ipps

#endif /*BASETYPEJSON__H__*/
