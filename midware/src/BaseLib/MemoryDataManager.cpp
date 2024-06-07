

#include "MemoryDataManager.h"
using namespace ZH::BaseLib;

const uint32_t CMemoryDataManager::sMemSize = 4096;
const char*  CMemoryDataManager::sPath = "/tmp";
CMemoryDataManager::CMemoryDataManager():
    mShmid(-1),
    mPtr((void*)-1),
    mpStMemoryDataInfo(nullptr)
{
    //shmInit();
}
int CMemoryDataManager::shmInit()
{
    key_t key = ftok(sPath, 'b');
    bool isCreateShm = false;
    mShmid = shmget(key, sMemSize, IPC_CREAT|IPC_EXCL|0664);
    if(mShmid == -1)
    {
        mShmid = shmget(key, sMemSize, IPC_CREAT | 0664);
        if(mShmid == -1)
        {
            LOGERR("shmget() return -1\n");
            return -1;
        }
    }
    else
    {
        isCreateShm = true;
    }
    mPtr = shmat(mShmid, NULL, 0);
    if(int(mPtr) == -1)
    {
        LOGERR("shmat() return -1\n");
        return -1;
    }
    mpStMemoryDataInfo = (stMemoryDataInfo*)mPtr;
    if(isCreateShm)
    {
        memset(mpStMemoryDataInfo, 0xff, sizeof(stMemoryDataInfo));
    }
}
CMemoryDataManager::~CMemoryDataManager()
{
    if(int(mPtr) != -1)
    {
        shmdt(mPtr);
    }
    if(mShmid != -1)
    {
        shmctl(mShmid, IPC_RMID, NULL);
    }
}

#if 0
bool CMemoryDataManager::DataManager_SetDTE(uint32_t dte)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u32DTE = dte;
    return true;
}
bool CMemoryDataManager::DataManager_GetDTE(uint32_t& dte)
{
    if(!mpStMemoryDataInfo) return false;
    dte = mpStMemoryDataInfo->u32DTE;
    return true;
}
bool CMemoryDataManager::DataManager_SetODO(uint32_t odo)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u32ODO = odo;
    return true;
}
bool CMemoryDataManager::DataManager_GetODO(uint32_t& odo)
{
    if(!mpStMemoryDataInfo) return false;
    odo = mpStMemoryDataInfo->u32ODO;
    return true;
}
bool CMemoryDataManager::DataManager_SetTripA(uint32_t tripA)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u32TripA = tripA;
    return true;
}
bool CMemoryDataManager::DataManager_GetTripA(uint32_t& tripA)
{
    if(!mpStMemoryDataInfo) return false;
    tripA = mpStMemoryDataInfo->u32TripA;
    return true;
}
bool CMemoryDataManager::DataManager_SetTripB(uint32_t tripB)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u32TripA = tripB;
    return true;
}
bool CMemoryDataManager::DataManager_GetTripB(uint32_t& tripB)
{
    if(!mpStMemoryDataInfo) return false;
    tripB = mpStMemoryDataInfo->u32TripA;
    return true;
}
bool CMemoryDataManager::DataManager_SetVehicleSpeed(uint16_t spd)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u16VehSpd = spd;
    return true;
}
bool CMemoryDataManager::DataManager_GetVehicleSpeed(uint16_t& spd)
{
    if(!mpStMemoryDataInfo) return false;
    spd = mpStMemoryDataInfo->u16VehSpd;
    return true;
}
bool CMemoryDataManager::DataManager_SetPowerValue(uint16_t power)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u16PowerValue = power;
    return true;
}
bool CMemoryDataManager::DataManager_GetPowerValue(uint16_t& power)
{
    if(!mpStMemoryDataInfo) return false;
    power = mpStMemoryDataInfo->u16PowerValue;
    return true;
}
bool CMemoryDataManager::DataManager_SetVehMomEgyCnse(uint16_t vehMomEgyCnse)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u16VehMomEgyCnse = vehMomEgyCnse;
    return true;
}
bool CMemoryDataManager::DataManager_GetVehMomEgyCnse(uint16_t& vehMomEgyCnse)
{
    if(!mpStMemoryDataInfo) return false;
    vehMomEgyCnse = mpStMemoryDataInfo->u16VehMomEgyCnse;
    return true;
}
bool CMemoryDataManager::DataManager_SetAVPC(uint16_t avpc)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u16VehAVPC = avpc;
    return true;
}
bool CMemoryDataManager::DataManager_GetAVPC(uint16_t& avpc)
{
    if(!mpStMemoryDataInfo) return false;
    avpc = mpStMemoryDataInfo->u16VehAVPC;
    return true;
}
bool CMemoryDataManager::DataManager_SetTheme(uint8_t theme,uint8_t skin)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8Theme = theme;
    mpStMemoryDataInfo->u8SkinColor = skin;
    return true;
}
bool CMemoryDataManager::DataManager_GetTheme(uint8_t& theme ,uint8_t& skin)
{
    if(!mpStMemoryDataInfo) return false;
    theme = mpStMemoryDataInfo->u8Theme;
    skin = mpStMemoryDataInfo->u8SkinColor;
    return true;
}

bool CMemoryDataManager::DataManager_SetVoiceSwtich(uint8_t voiceSw, uint8_t volume)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8VoiceSwtich = voiceSw;
    mpStMemoryDataInfo->u8Volume = volume;
    return true;
}
bool CMemoryDataManager::DataManager_GetVoiceSwtich(uint8_t& voiceSw, uint8_t& volume)
{
    if(!mpStMemoryDataInfo) return false;
    voiceSw = mpStMemoryDataInfo->u8VoiceSwtich;
    volume = mpStMemoryDataInfo->u8Volume;
    return true;
}
bool CMemoryDataManager::DataManager_SetTCIndex(uint8_t tcIdx)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8TCIndex = tcIdx;
    return true;
}
bool CMemoryDataManager::DataManager_GetTCIndex(uint8_t& tcIdx)
{
    if(!mpStMemoryDataInfo) return false;
    tcIdx = mpStMemoryDataInfo->u8TCIndex;
    return true;
}
bool CMemoryDataManager::DataManager_SetSocElectricity(uint8_t socValue, uint8_t socSts)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8SocElectricity = socValue;
    mpStMemoryDataInfo->u8SocBatSts = socSts;
    return true;
}
bool CMemoryDataManager::DataManager_GetSocElectricity(uint8_t& socValue, uint8_t& socSts)
{
    if(!mpStMemoryDataInfo) return false;
    socValue = mpStMemoryDataInfo->u8SocElectricity;
    socSts = mpStMemoryDataInfo->u8SocBatSts;
    return true;
}
bool CMemoryDataManager::DataManager_SetTime(uint8_t hour, uint8_t minute, uint8_t timeMode)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8TimeMode = timeMode;
    mpStMemoryDataInfo->u8TimeHour = hour;
    mpStMemoryDataInfo->u8TimeMinute = minute;
    return true;
}
bool CMemoryDataManager::DataManager_GetTime(uint8_t& hour, uint8_t& minute, uint8_t& timeMode)
{
    if(!mpStMemoryDataInfo) return false;
    timeMode = mpStMemoryDataInfo->u8TimeMode;
    hour = mpStMemoryDataInfo->u8TimeHour;
    minute = mpStMemoryDataInfo->u8TimeMinute;
    return true;
}
bool CMemoryDataManager::DataManager_SetGear(uint8_t gearValue)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8Gear = gearValue;
    return true;
}
bool CMemoryDataManager::DataManager_GetGear(uint8_t& gearValue)
{
    if(!mpStMemoryDataInfo) return false;
    gearValue = mpStMemoryDataInfo->u8Gear;
    return true;
}
bool CMemoryDataManager::DataManager_SetEgyRgnStyle(uint8_t egyRgnStyle)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8EgyRgnStyle = egyRgnStyle;
    return true;
}
bool CMemoryDataManager::DataManager_GetEgyRgnStyle(uint8_t& egyRgnStyle)
{
    if(!mpStMemoryDataInfo) return false;
    egyRgnStyle = mpStMemoryDataInfo->u8EgyRgnStyle;
    return true;
}
bool CMemoryDataManager::DataManager_SetPowerMode(uint8_t powerMode)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8PowerMode = powerMode;
    return true;
}
bool CMemoryDataManager::DataManager_GetPowerMode(uint8_t& powerMode)
{
    if(!mpStMemoryDataInfo) return false;
    powerMode = mpStMemoryDataInfo->u8PowerMode;
    return true;
}
bool CMemoryDataManager::DataManager_SetLampData0(uint8_t lampData)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8LampData0 = lampData;
    return true;
}
bool CMemoryDataManager::DataManager_GetLampData0(uint8_t& lampData)
{
    if(!mpStMemoryDataInfo) return false;
    lampData = mpStMemoryDataInfo->u8LampData0;
    return true;
}
bool CMemoryDataManager::DataManager_SetLampData1(uint8_t lampData)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8LampData1 = lampData;
    return true;
}
bool CMemoryDataManager::DataManager_GetLampData1(uint8_t& lampData)
{
    if(!mpStMemoryDataInfo) return false;
    lampData = mpStMemoryDataInfo->u8LampData1;
    return true;
}
bool CMemoryDataManager::DataManager_SetLampData2(uint8_t lampData)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8LampData2 = lampData;
    return true;
}
bool CMemoryDataManager::DataManager_GetLampData2(uint8_t& lampData)
{
    if(!mpStMemoryDataInfo) return false;
    lampData = mpStMemoryDataInfo->u8LampData2;
    return true;
}
bool CMemoryDataManager::DataManager_SetLampData3(uint8_t lampData)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8LampData3 = lampData;
    return true;
}
bool CMemoryDataManager::DataManager_GetLampData3(uint8_t& lampData)
{
    if(!mpStMemoryDataInfo) return false;
    lampData = mpStMemoryDataInfo->u8LampData3;
    return true;
}
bool CMemoryDataManager::DataManager_SetLampData4(uint8_t lampData)
{
    if(!mpStMemoryDataInfo) return false;
    mpStMemoryDataInfo->u8LampData4 = lampData;
    return true;
}
bool CMemoryDataManager::DataManager_GetLampData4(uint8_t& lampData)
{
    if(!mpStMemoryDataInfo) return false;
    lampData = mpStMemoryDataInfo->u8LampData4;
    return true;
}
#else
bool CMemoryDataManager::DataManager_SetDTE(uint32_t dte)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetDTE(uint32_t& dte)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetODO(uint32_t odo)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetODO(uint32_t& odo)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetTripA(uint32_t tripA)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetTripA(uint32_t& tripA)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetTripB(uint32_t tripB)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetTripB(uint32_t& tripB)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetVehicleSpeed(uint16_t spd)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetVehicleSpeed(uint16_t& spd)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetPowerValue(uint16_t power)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetPowerValue(uint16_t& power)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetVehMomEgyCnse(uint16_t vehMomEgyCnse)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetVehMomEgyCnse(uint16_t& vehMomEgyCnse)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetAVPC(uint16_t avpc)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetAVPC(uint16_t& avpc)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetTheme(uint8_t theme, uint8_t skin)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetTheme(uint8_t& theme, uint8_t& skin)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetVoiceSwtich(uint8_t voiceSw, uint8_t volume)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetVoiceSwtich(uint8_t& voiceSw, uint8_t& volume)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetTCIndex(uint8_t tcIdx)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetTCIndex(uint8_t& tcIdx)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetSocElectricity(uint8_t socValue, uint8_t socSts)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetSocElectricity(uint8_t& socValue, uint8_t& socSts)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetTime(uint8_t hour, uint8_t minute, uint8_t timeMode)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetTime(uint8_t& hour, uint8_t& minute, uint8_t& timeMode)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetGear(uint8_t gearValue)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetGear(uint8_t& gearValue)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetEgyRgnStyle(uint8_t egyRgnStyle)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetEgyRgnStyle(uint8_t& egyRgnStyle)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetPowerMode(uint8_t powerMode)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetPowerMode(uint8_t& powerMode)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetLampData0(uint8_t lampData)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetLampData0(uint8_t& lampData)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetLampData1(uint8_t lampData)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetLampData1(uint8_t& lampData)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetLampData2(uint8_t lampData)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetLampData2(uint8_t& lampData)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetLampData3(uint8_t lampData)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetLampData3(uint8_t& lampData)
{
    return false;
}
bool CMemoryDataManager::DataManager_SetLampData4(uint8_t lampData)
{
    return false;
}
bool CMemoryDataManager::DataManager_GetLampData4(uint8_t& lampData)
{
    return false;
}
#endif