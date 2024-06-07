#include "mlog.h"
#include "ProtocolDecode.h"
#include "ProtocolDefine.h"

namespace ZH
{
    namespace ProtocolCodec
    {

        CDecoder::CDecoder()
        {
            addFrameDef(UP_MINOR_TRIP_ODO_MILEAGE, {16, 24, 24});
            addFrameDef(UP_MINOR_VEHICLE_LIGHT_STATUS, {1, 1, 1, 1, 4,
                                                        1, 1, 1, 1, 4,
                                                        4, 4,
                                                        1, 1, 1, 1, 1, 1, 1, 1,
                                                        1, 1, 1, 1, 1, 1, 1, 1});
            addFrameDef(UP_MINOR_TURN_LIGHT_STATUS, {3,3,2});

#ifdef A301
            addFrameDef(UP_MINOR_DOOR_STATUS, {1, 1, 1, 1, 1, 2, 1,
                                               1, 1, 1, 1, 4,
                                               4, 4});
#else
            addFrameDef(UP_MINOR_DOOR_STATUS, {1, 1, 1, 1, 1, 1, 1, 1,
                                               1, 1, 1, 1, 1, 1, 1, 1,
                                               2, 1, 1, 1, 1, 1, 1});
#endif
            
            addFrameDef(UP_MINOR_WINDOW_STATUS, {2, 2, 4, 2, 2, 4, 4, 4});
            addFrameDef(UP_MINOR_VEHICLE_SPEED, {16, 8, 16});
            addFrameDef(UP_MINOR_ENGINE_SPEED, {16});
            addFrameDef(UP_MINOR_ENERGY_CONSUMPTION, {16, 16, 16});

#ifdef A301
            addFrameDef(UP_MINOR_POWER_GEAR_STATUS, {4, 1, 3, 3, 5});
#else
            addFrameDef(UP_MINOR_POWER_GEAR_STATUS, {4, 1, 3, 3, 5});
#endif
            
            addFrameDef(UP_MINOR_CUR_TIME, {8, 8, 8, 8, 8, 8, 1, 2, 5});

#ifdef A301
            addFrameDef(UP_MINOR_TEMP, {8, 8});
#else
            addFrameDef(UP_MINOR_TEMP, {8, 8, 8, 8, 8});
#endif
            
            addFrameDef(UP_MAJOR_RTC_TIME, {8, 8, 8, 8, 8, 8, 8});
            addFrameDef(UP_MINOR_SEATBELT_AIRBAG, {4,4,4,4});
            addFrameDef(UP_MINOR_RADAR_INFOR, {8,8,8,1,1,6});
            #if 1
            addFrameDef(UP_MINOR_TIRE_STATUS, {1, 1, 6, 8 ,
                                               1, 1, 6, 8 ,
                                               1, 1, 6, 8 ,
                                               1, 1, 6, 8 ,});
            #else
            addFrameDef(UP_MINOR_TIRE_STATUS, {8,8,8,8,8,8,8,8,8});
            #endif
            addFrameDef(UP_MINOR_FAULT_CODE, {1, 1, 1, 1, 1, 1, 1, 1,
                                              1, 1, 1, 1, 1, 1, 1, 1,
                                              1, 1, 1, 1, 1, 1, 1, 1,
                                              1, 1, 1, 1, 1, 1, 1, 1,
                                              1, 1, 1, 1, 1, 1, 1, 1,
                                              1, 1, 1, 1, 1, 1, 1, 1,
                                              1, 1, 1, 1, 1, 1, 1, 1,
                                              1, 1, 1, 1, 1, 1, 1, 1});
            addFrameDef(UP_MINOR_FUEL_CONSUMPTION, {8, 8, 8, 8, 8, 8});
            addFrameDef(UP_MINOR_WATER_TEMP_AND_FUEL, {8, 8});
            addFrameDef(UP_MINOR_ENERGY_VALUE, {8,4,4,2,6});
            addFrameDef(UP_MAJOR_KEY, {8});
            addFrameDef(UP_MAJOR_SET_CONFIG, {8, 8});
            addFrameDef(UP_MAJOR_POWER_MODE, {8});
            addFrameDef(MAJOR_ENTER_UPGRADE_MODE2, {8, 8});
            addFrameDef(UP_MINOR_FUEL, {5, 3});
            addFrameDef(UP_MINOR_ELECTRIC_INFOR, {7, 1, 16, 16, 1, 1, 1, 1, 1, 3, 8, 8, 16, 8});
            addFrameDef(UP_MAJOR_MCU_VERSION, 	{8, 8, 8, 8, 8, 8, 8, 8, 
												8, 8, 8, 8, 8, 8, 8, 8,
												8, 8, 8, 8, 8, 8, 8, 8,
												8, 8, 8, 8, 8, 8, 8, 8,
												8, 8, 8, 8, 8, 8, 8, 8});
            addFrameDef(UP_MINOR_IVI_INTERACTIVE, {8, 8, 8});
            addFrameDef(UP_MINOR_IVI_OTA, {8, 6, 2, 8,8, 8,8, 8});
            addFrameDef(UP_MAJOR_SCREEN_MODE,{8});
			addFrameDef(UP_MAJOR_DATA_SYNC_COMPLETE,{8});
            addFrameDef(UP_MINOR_IVI_TIME, {8,8,8,8,8,8,8,8});

#ifdef A301
            addFrameDef(UP_MINOR_POPUPS_WARNING, { 8,8,8,8,8,8,8,8,
													8});
#else
            addFrameDef(UP_MINOR_POPUPS_WARNING, {  8,8,8,8,8,8,8,8,
                                                    8,8,8,8,8,8,8,8,
                                                    8,8,8,8,8,8,8,8,
                                                    8,8,8,8,8,8,8,8,
                                                    8,8,8,8,8,8,8,8
                                                });
#endif

            addFrameDef(UP_MINOR_CHARGE_ORDER, {8,8,8,8,8,2,2,1,1,2,8,8,8,8,8});
            addFrameDef(UP_MINOR_CHARGE_STATUS, {4,4,8,16,16,16});
            addFrameDef(UP_MINOR_VOICE_WARNING, { 8 });
            addFrameDef(UP_MINOR_MEMORY_INFO, { 2,2,1,2,1,
												8 });
            addFrameDef(UP_MINOR_RESET_ACK, {1,1,1,1,1,3});
            addFrameDef(UP_MINOR_MEMORY_INFO_ACK, { 2,2,1,2,1,
												    8 });
        }

        CDecoder::~CDecoder()
        {
        }

        unsigned int CDecoder::operator[](int sub)
        {
            if (sub < (int)mResult.size())
                return mResult[sub];
            else
                return 0;
        }

        void CDecoder::operator()(std::string &frame, int offset)
        {
            this->mResult.clear();
            this->updateFrame(frame);
            this->decode(offset);
        }

        void CDecoder::operator()(unsigned char *frame, int size)
        {
            this->mResult.clear();
            this->updateFrame(frame, size);
            this->decode();
        }

        void CDecoder::decode(int offset)
        {
            unsigned int ID = getMajorID();
            if (mFrameCfg.find(ID) == mFrameCfg.end())
                ID = getMinorID();
            // int offset = PROTOCOL_BASE_LEN_BIT;
            for (auto it : mFrameCfg[ID])
            {
                unsigned int result = 0, tempResult = 0;
                for (int i = 0; i < it; i++)
                    tempResult = tempResult | (getBit(offset + i) << i);
                result = reorder(tempResult, it);
                mResult.push_back(result);
                // printf("ID:%04X, result: %d, tempResult: %d\n", ID, result, tempResult);
                offset += it;
            }
        }

        CIviDecoder::CIviDecoder()
        {
            addFrameDef(IVI_TO_IC_SET_SOURCE, {8});
            addFrameDef(IVI_TO_IC_ANIMAT_CTRL, {8});

            addFrameDef(IVI_TO_IC_TOUCH_EVENT, {8, 16, 16});
        }

        CIviDecoder::~CIviDecoder()
        {
        }

        unsigned int CIviDecoder::getMajorID(void)
        {
            unsigned char *pFrame = getFrameBuf();
            return ((pFrame[2] << 8) & 0xFF00) | (pFrame[3] & 0xFF);
        }

        void CIviDecoder::decode(void)
        {
            unsigned int ID = getMajorID();

            int offset = 48;
            for (auto it : mFrameCfg[ID])
            {
                unsigned int result = 0, tempResult = 0;
                for (int i = 0; i < it; i++)
                    tempResult = tempResult | (getBit(offset + i) << i);
                result = reorder(tempResult, it);
                mResult.push_back(result);
                // printf("ID:%04X, result: %d, tempResult: %d\n", ID, result, tempResult);
                offset += it;
            }
        }

        unsigned char *CIviDecoder::getData(void)
        {
            unsigned char *pFrame = getFrameBuf();

            return pFrame + 6;
        }

    } // namespace ProtocolCodec
} // namespace ZH