
#ifndef MSGSERVICE__H__
#define MSGSERVICE__H__

#include "Application.h"
#include "SpiDevice.h"
#include "UartDevice.h"
#include "Hook.h"
#include "MsgTimer.hpp"
#include "ProtocolDecode.h"
#include "ProtocolEncode.h"
#include "StrategyContext.hpp"
//#include "UpgradeMcuCore.h"
#include "STimer.h"
#include "StrategySrc/clusterSignalDelayProcess.h"
#include "MemoryDataManager.h"

#define OVER_WRITE_TT_SELFCHECK	1
class CMsgService : public ZH::BaseLib::CApplication, public CMsgTimer
{
public:
    // friend CStrategyContext;
    CMsgService(int argc, char *argv[], bool);
    virtual ~CMsgService();
    virtual void Init();
    virtual void DoWork();
    virtual void UInit();

    void recv(std::string);

    void McuAck(unsigned int ID);

    void NotifyMcuBrightness(EmScreenType, int);
    void NotifyMcuDriveMode(EmDriveMode);
    void NotifyMcuHeartbeatPacket(int);
    void NotifyMcuBacklightState(std::vector<int>);
    void NotifyMcuCurrentPopWarn(int);
    void NotifyMcuIntoUpgradeMode(int);
    void UpdateDispBrightness(EmBrightness);
    void SelfCheckState(int);
    void SetCurrentClock(int, int, int, int, int, int, int);
    void SetMemoryItem(EmFunctionItem, int);
    void ResetFunctionItem(EmFunctionItem);
    void SetThemeColor(int, int);
    void SetVoicePlay(int, int);
    void SetClearMileageInfo(int, int);
    void SetPopWarnStatus(int);
    void LogLevelControl(std::string name, LOG_LEV_EN logLevel);
	void SetTCInfoIndex(int idx);
	void TransitIVIBrightness(int brightness);

	void HeartbeatSignal(std::string, int);
	void NotifyMcuReboot(int);
    void chimeOutput_ChimeReady(int);
private:
    void GetVehicleSpeed(void);
    void GetAvgVehicleSpeed(void);
    void GetDrivenTime(void);
    void GetEngineSpeed(void);
    void GetEnergyValue(void);
    void GetEnduranceMileage(void);
    void GetOdograph(void);
    void GetVehicleLightStatus(void);
    void GetTurnLightStatus(void);
    void GetPowerGearStatus(void);
    void GetCurTime(void);
    void GetTemp(void);
    void GetRadarInfor(void);
    void GetTireStatus(void);
    void GetKey(void);
    void GetSetConfig(void);
    void GetsafetyBeltOrAirbag(void);
    void GetFaultCode(void);
    void GetwaterTempAndFuel(void);
    void GetPopUpAlarm(void);
    void GetChargeOrder(void);
    void GetChargeStatus(void);
    void GetVoiceWarning(void);
    void GetResetAck(void);
    void GetPowerMode(void);
    void GetScreenMode(void);
    void GetRechargeMileage(void);
    void GetTxtWarn(void);
    void GetUpgrade(void);
    void GetFuel(void);
    void GetVersion(void);
    void GetBatteryVoltage(void);
    void UpdateCurTime();
    void updateHmiTire();
    std::string convertTirePressValue(uint16_t valueKpa, EmUnitPress unitPress);
    //
    void simulatedData(void);

    void GetMemoryInfo(void);
    void GetMemoryInfo_ACK(void);

    void heartbeat_OnTimer();

    void TimerSelfInspet();
    bool GetSelfInspet(EmPowerStatus ePowerSts,bool bHmiReady,bool bSelfInspet/*,int nSpd,bool bSpdValid,int nPower,bool bPowerValid*/,std::string sMask);
    void UpdNotSelfInspet(std::string sId, EmLampState e);

	void sendDataToMCU(const std::string& frameData, bool isLogRecord = true);

	void repeatSend0x81_Ontime();
	void GetDataSyncComplete(void);
	void NotifyHMICurrentPopWarn(EmPopWarnID id);
	void NotifyHMIHistoryPopWarnList(std::vector<EmPopWarnID>& vecHistoryWarn);
	void ttSelfCheck_OnChangeSts();
	void InstEnergyConsum_OnTimer();
private:
	void GetVehicleLightStatus_Process(uint32_t* pData, uint32_t);
	void GetTurnLightStatus_Process(uint32_t* pData, uint32_t);
	void GetVehicleSpeed_Process(uint32_t* pData, uint32_t);
	void GetEngineSpeed_Process(uint32_t* pData, uint32_t);
	void GetTemp_Process(uint32_t* pData, uint32_t);
	void GetTireStatus_Process(uint32_t* pData, uint32_t);
	void GetsafetyBeltOrAirbag_Process(uint32_t* pData, uint32_t);
	void GetEnergyValue_Process(uint32_t* pData, uint32_t);
	void GetRechargeMileage_Process(uint32_t* pData, uint32_t);
	void GetBatteryVoltage_Process(uint32_t* pData, uint32_t);
	void GetPopUpAlarm_Process(uint32_t* pData, uint32_t);
	void GetChargeOrder_Process(uint32_t* pData, uint32_t);
	void GetChargeStatus_Process(uint32_t* pData, uint32_t);
    void GetPowerMode_Process(uint32_t* pData, uint32_t);
    void GetPowerGear_Process(uint32_t* pData, uint32_t);
private:
	
	struct mileageLogInfo
	{
		FILE* 		mileageFp;
		uint32_t	tripAValue;
		uint32_t	OdoValue;
		float		DTE;
		float		fuel;
		float		AFC;
		float		fuel2;
		float		DTE2;
		float		AFC2;
		uint32_t	speed;
	};
    
private:
    bool bAutoSimulateMode;
    int     mLastBrightnessValue;
    int     mLastViewMode;
    char    mTireSaveData[9];
    std::mutex  mPressMutex;
    EmUnitPress mSaveDispUnitPress;
    //ZH::BaseLib::CSimpleTimer<CMsgService>* mpTestTimer;
    ZH::BaseLib::STimer  *mHeartbeatTimer;
    uint32_t	mOTACtrlValue;
    bool		mIsSeatbeltWarnFlag;//安全带报警标志
    char		mMCUPopUpWarnData[40];
    int			mCurrentTimeMode;
    char		mXKeyValue;
    char		mPreformanceMode;
    uint16_t	mLimitSpdValue;
	char		mSwitchDrivingMode;
	uint8_t		mIVI0xA1_SN;		
#ifdef _spi_
    CSpiDevice mMsgDev;
#endif
#ifdef _serial_
    CUartDevice mMsgDev;
#endif
    CStrategyContext o_stgCtx;
    //CUpgradeMcuCore o_mcuUpdate;
    ZH::ProtocolCodec::CDecoder decode;
    ZH::ProtocolCodec::CEncoder encode;
    ZH::BaseLib::CMultifunc<unsigned int> multifuncion;
    CIPCConnector opt;
    CIPCConnector upgradeIPC;
    bool mIsReStartMsgService;

    // 预约充电开始/结束时间
    // 预约充电状态/模式
    //uint32_t m_nChargeOrderSYear, m_nChargeOrderSMonth, m_nChargeOrderSDay, m_nChargeOrderSHour, m_nChargeOrderSMinute;
    //uint32_t m_nChargeOrderEYear, m_nChargeOrderEMonth, m_nChargeOrderEDay, m_nChargeOrderEHour, m_nChargeOrderEMinute;

	StuChargeSubscribe m_nChargeOrderStart;
    StuChargeSubscribe m_nChargeOrderEnd;
    CHARGE_ORDER_MODE m_eChargeOrderMode;
    CHARGE_ORDER m_eChargeOrderSts;
    int ChargStateVilid;
    int ChargTimeVilid;

    CHARGE_DEVICE m_eChargeDC;
    CHARGE_DEVICE m_eChargeAC;
    CHARGING m_eChargingSts;
    int m_nElectricType;

    int m_nCurTheme;
    int m_nCurColor;
    int m_nCurVoiceSwitch;
    int m_nCurVoiceSize;
	int mChimeIconBitwiseData;

    int m_nGear;

    ZH::BaseLib::STimer *m_pTimerSelfInspet; // 开机自检
    std::vector<StuTelltableLampState> m_vecSelfInspet;
    std::vector<StuTelltableLampState> m_vecNotSelfInspet;
    std::mutex m_lockSelfInspet;

	ZH::BaseLib::STimer *mRepeatSend0x810000Timer;
	uint32_t	mRepeatSend0x81Count;
	bool		mChimeSpdOverFlag;
	int			mSaveCurMenuItem;
	int 		mCurPowerMode;
	uint8_t		mCurPopWarnID;
	
	CSigDelayProcessor* mLampSigDelayProcessor;
	CSigDelayProcessor* mTurnLampSigDelayProcessor;
	CSigDelayProcessor* mVehSpdSigDelayProcessor;
	CSigDelayProcessor* mEngineSpdDelayProcessor;
	CSigDelayProcessor* mTempDelayProcessor;
	CSigDelayProcessor* mTPMSDelayProcessor;
	CSigDelayProcessor* mSafeBeltDelayProcessor;
	CSigDelayProcessor* mEnergyDelayProcessor;
	CSigDelayProcessor* mRechargeDelayProcessor;
	CSigDelayProcessor* mBatteryVoltageDelayProcessor;
	CSigDelayProcessor* mPopUpAlarmDelayProcessor;
	CSigDelayProcessor* mChargeOrderDelayProcessor;
	CSigDelayProcessor* mChargeStsDelayProcessor;
    CSigDelayProcessor* mPowerModeProcessor;
    CSigDelayProcessor* mPowerGearProcessor;

	ZH::BaseLib::STimer*	mInstEnergyConsumTimer;
	std::mutex				mInstEnergyConsumMutex;
	uint32_t				mInstEnergyConsumValue;
	std::pair<float, bool>	mDisplaySpeed;
	std::pair<int, bool>	mDisplayPowerValue;
	ZH::BaseLib::STimer*	mIgnDelayUpdateHmiTimer;

	void togetherProcess();
	struct stUpdateCharging
	{
		int32_t ChargeSubscribeData[20];
		int32_t ChargingData[12];
		ZH::BaseLib::STimer*	delayProcessTimer;
	};
	stUpdateCharging mStUpdateCharging;
	ZH::BaseLib::STimer*	mBrightnessSendFilterTimer;
	int			mBrightnessValue;
	std::mutex 	mSendMCUDataMutex;
	int			mBrightnessRepeatSendCount;
	
	ZH::BaseLib::STimer*	mRepeatSendIgnOffWarnIdTimer;
	std::string	mSocVersion;
    bool    mbHmiReady;
    bool    mbSetVoiceFlag;
    struct MemoryDataSync
    {
        ZH::BaseLib::STimer*	repeatSendMemoryDataTimer;//休眠记忆项数据重复发送定时器
        bool    isInitReCoverSts;//false:休眠恢复状态；true:设置项记忆状态
        int     repeatSendCount;
    };
    MemoryDataSync mStMemoryDataSync;
    
    CMemoryDataManager* mMemDataManager;
    uint32_t    mOdoValue;
    bool        mOdoFirstUpdate;
    uint8_t     mOdoDelayCount;

};
#endif /*MSGSERVICE__H__*/
