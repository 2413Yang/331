#ifndef _MEMORYDATAMANAGER_H_
#define _MEMORYDATAMANAGER_H_

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <mlog.h>

namespace ZH
{
    namespace BaseLib
    {

        class CMemoryDataManager
        {
        private:
            struct stMemoryDataInfo
            {
                uint32_t    u32DTE;             //续航里程
                uint32_t    u32ODO;             //总计里程
                uint32_t    u32TripA;       //小计里程
                uint32_t    u32TripB;       //小计里程
                uint16_t    u16VehSpd;          //车速表
                uint16_t    u16PowerValue;    //功率表
                uint16_t    u16VehMomEgyCnse;//瞬时能耗
                uint16_t    u16VehAVPC;  //平均电耗     
                uint8_t     u8Theme;            //主题
                uint8_t     u8SkinColor;        //肤色
                uint8_t     u8VoiceSwtich;      //语音开关
                uint8_t     u8Volume;         //音量
                uint8_t     u8TCIndex;        //行车信息菜单索引
                uint8_t     u8SocElectricity;//soc电量
                uint8_t     u8SocBatSts;//soc电量状态
                uint8_t     u8TimeMode;     //时间制式
                uint8_t     u8TimeHour;     //时
                uint8_t     u8TimeMinute;   //分
                uint8_t     u8Gear; //档位
                uint8_t     u8EgyRgnStyle;//能量回收等级
                uint8_t     u8PowerMode;    //电源模式 0：熄火；1：点火
                uint8_t     u8LampData0;
                uint8_t     u8LampData1;
                uint8_t     u8LampData2;
                uint8_t     u8LampData3;
                uint8_t     u8LampData4;
            };
        private:
            int     mShmid;
            void*   mPtr;
            stMemoryDataInfo*   mpStMemoryDataInfo;    
        private:
            const static uint32_t sMemSize;
            const static char*  sPath;
        private:
            int shmInit();
        public:
            CMemoryDataManager();
            ~CMemoryDataManager();
        public:
            bool DataManager_SetDTE(uint32_t dte);
            bool DataManager_GetDTE(uint32_t& dte);
            bool DataManager_SetODO(uint32_t odo);
            bool DataManager_GetODO(uint32_t& odo);
            bool DataManager_SetTripA(uint32_t tripA);
            bool DataManager_GetTripA(uint32_t& tripA);
            bool DataManager_SetTripB(uint32_t tripB);
            bool DataManager_GetTripB(uint32_t& tripB);
            bool DataManager_SetVehicleSpeed(uint16_t spd);
            bool DataManager_GetVehicleSpeed(uint16_t& spd);
            bool DataManager_SetPowerValue(uint16_t power);
            bool DataManager_GetPowerValue(uint16_t& power);
            bool DataManager_SetVehMomEgyCnse(uint16_t vehMomEgyCnse);
            bool DataManager_GetVehMomEgyCnse(uint16_t& vehMomEgyCnse);
            bool DataManager_SetAVPC(uint16_t avpc);
            bool DataManager_GetAVPC(uint16_t& avpc);
            bool DataManager_SetTheme(uint8_t theme, uint8_t skin);
            bool DataManager_GetTheme(uint8_t& theme, uint8_t& skin);
            bool DataManager_SetVoiceSwtich(uint8_t voiceSw, uint8_t volume);
            bool DataManager_GetVoiceSwtich(uint8_t& voiceSw, uint8_t& volume);
            bool DataManager_SetTCIndex(uint8_t tcIdx);
            bool DataManager_GetTCIndex(uint8_t& tcIdx);
            bool DataManager_SetSocElectricity(uint8_t socValue, uint8_t socSts);
            bool DataManager_GetSocElectricity(uint8_t& socValue, uint8_t& socSts);
            bool DataManager_SetTime(uint8_t hour, uint8_t minute, uint8_t timeMode);
            bool DataManager_GetTime(uint8_t& hour, uint8_t& minute, uint8_t& timeMode);
            bool DataManager_SetGear(uint8_t gearValue);
            bool DataManager_GetGear(uint8_t& gearValue);
            bool DataManager_SetEgyRgnStyle(uint8_t egyRgnStyle);
            bool DataManager_GetEgyRgnStyle(uint8_t& egyRgnStyle);
            bool DataManager_SetPowerMode(uint8_t powerMode);
            bool DataManager_GetPowerMode(uint8_t& powerMode);
            bool DataManager_SetLampData0(uint8_t lampData);
            bool DataManager_GetLampData0(uint8_t& lampData);
            bool DataManager_SetLampData1(uint8_t lampData);
            bool DataManager_GetLampData1(uint8_t& lampData);
            bool DataManager_SetLampData2(uint8_t lampData);
            bool DataManager_GetLampData2(uint8_t& lampData);
            bool DataManager_SetLampData3(uint8_t lampData);
            bool DataManager_GetLampData3(uint8_t& lampData);
            bool DataManager_SetLampData4(uint8_t lampData);
            bool DataManager_GetLampData4(uint8_t& lampData);
        };
    }
}


#endif //!_MEMORYDATAMANAGER_H_