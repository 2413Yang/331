
#include "SerializePPS.h"

namespace Ipps
{
    void serialize(bool bParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_bool(&out, StrValue(tmpName), bParam);
    }

    void serialize(int iParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_int(&out, StrValue(tmpName), iParam);
    }

    void serialize(unsigned int uiParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_int(&out, StrValue(tmpName), uiParam);
    }

    void serialize(long lParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_int64(&out, StrValue(tmpName), lParam);
    }

    void serialize(unsigned long ulParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_int64(&out, StrValue(tmpName), ulParam);
    }

    void serialize(double fParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_double(&out, StrValue(tmpName), fParam);
    }

    void serialize(float fParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_double(&out, StrValue(tmpName), fParam);
    }

    void serialize(const char *pParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_string(&out, StrValue(tmpName), pParam);
    }

    void serialize(const std::string pParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_add_string(&out, StrValue(tmpName), pParam.c_str());
    }
    /*******************************************************************************************************
********************************************************************************************************/
    void deserialize(bool &bParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_get_bool(&in, StrValue(tmpName), &bParam);
    }

    void deserialize(int &iParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_get_int(&in, StrValue(tmpName), &iParam);
    }

    void deserialize(unsigned int &iParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_get_int(&in, StrValue(tmpName), (int *)&iParam);
    }

    void deserialize(long &iParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_get_int64(&in, StrValue(tmpName), (int64_t *)&iParam);
    }

    void deserialize(unsigned long &iParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_get_int64(&in, StrValue(tmpName), (int64_t *)&iParam);
    }

    void deserialize(float &fParam, std::string tmpName, pps_decoder_t &in)
    {
        double tmpParam = 0.0;
        pps_decoder_get_double(&in, StrValue(tmpName), &tmpParam);
        fParam = tmpParam;
    }

    void deserialize(double &fParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_get_double(&in, StrValue(tmpName), &fParam);
    }

    void deserialize(char *pParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_get_string(&in, StrValue(tmpName), (const char **)&pParam);
    }

    void deserialize(std::string &pParam, std::string tmpName, pps_decoder_t &in)
    {
        const char *pChar = NULL;
        pps_decoder_get_string(&in, StrValue(tmpName), (const char **)&pChar);
        pParam = pChar;
    }

    void serialize(const StuTelltableLampState pParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_start_object(&out, StrValue(tmpName));
        serialize(pParam.strID, "strID", out);
        serialize(pParam.state, "state", out);
        pps_encoder_end_object(&out);
    }

    void deserialize(StuTelltableLampState &pParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_push_object(&in, StrValue(tmpName));
        deserialize(pParam.strID, "strID", in);
        deserialize(pParam.state, "state", in);
        pps_decoder_pop(&in);
    }

    void serialize(const StuCarDoorState pParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_start_object(&out, StrValue(tmpName));
        serialize(pParam.ID, "EmID", out);
        serialize(pParam.state, "state", out);
        pps_encoder_end_object(&out);
    }

    void deserialize(StuCarDoorState &pParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_push_object(&in, StrValue(tmpName));
        deserialize(pParam.ID, "EmID", in);
        deserialize(pParam.state, "state", in);
        pps_decoder_pop(&in);
    }

    void serialize(const StuCarTire::tire pParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_start_object(&out, StrValue(tmpName));
        serialize(pParam.presWarn, "presWarn", out);
        serialize(pParam.tempWarn, "tempWarn", out);
        serialize(pParam.presValue, "presValue", out);
        serialize(pParam.tempValue, "tempValue", out);
        pps_encoder_end_object(&out);
    }

    void deserialize(StuCarTire::tire &pParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_push_object(&in, StrValue(tmpName));
        deserialize(pParam.presWarn, "presWarn", in);
        deserialize(pParam.tempWarn, "tempWarn", in);
        deserialize(pParam.presValue, "presValue", in);
        deserialize(pParam.tempValue, "tempValue", in);
        pps_decoder_pop(&in);
    }

    void serialize(const StuCarTire pParam, std::string tmpName, pps_encoder_t &out)
    {
        pps_encoder_start_object(&out, StrValue(tmpName));
        serialize(pParam.LFTire, "LFTire", out);
        serialize(pParam.RFTire, "RFTire", out);
        serialize(pParam.LRTire, "LRTire", out);
        serialize(pParam.RRTire, "RRTire", out);
        pps_encoder_end_object(&out);
    }

    void deserialize(StuCarTire &pParam, std::string tmpName, pps_decoder_t &in)
    {
        pps_decoder_push_object(&in, StrValue(tmpName));
        deserialize(pParam.LFTire, "LFTire", in);
        deserialize(pParam.RFTire, "RFTire", in);
        deserialize(pParam.LRTire, "LRTire", in);
        deserialize(pParam.RRTire, "RRTire", in);
        pps_decoder_pop(&in);
    }

} // namespace Ipps