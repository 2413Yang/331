#include "ChimeService.h"
#include <string.h>
#include <sys/syscall.h>

using namespace chime;

std::atomic<CChimeService*>	CChimeService::sChimeService(nullptr);
std::mutex CChimeService::sMutex;

#define GET_CHIME_STATE(DataList,idx)		(((DataList)[(idx) / (sizeof((DataList)[0])*8)] & (1 << ((idx) % (sizeof((DataList)[0])*8)))) == 0x00 ? false : true)
#define SET_CHIME_STATE(DataList,idx)		(DataList)[(idx) / (sizeof((DataList)[0])*8)] |= (1 << ((idx) % (sizeof((DataList)[0])*8)))
#define CLEAR_CHIME_STATE(DataList,idx)		(DataList)[(idx) / (sizeof((DataList)[0])*8)] &= ~(1 << ((idx) % (sizeof((DataList)[0])*8)))


CChimeService::CChimeService():
	mbThreadExitFlag(false),
	mThread(&CChimeService::PlaySoundMainLoop, this),
    mWatchThread(&CChimeService::watchThreadFn, this)
{
	memset(mTriggerChimeBitMap, 0, sizeof(mTriggerChimeBitMap));
	memset(mPlayChimeBitMap, 0, sizeof(mPlayChimeBitMap));
	ChimeDataInit();
	mbVoiceSwtichSts = true;
	mVolume = EnVolume::VOLUME_HIGH;
	mbSoundPlayingFlag = false;
	mPowerModeSts =EnWorkPowerSts::PM_IGN_ON;
	#if USING_GSTREAMER_APP
	// mGstWavApp = new gstWavApp();
	// mGstWavApp->Init(0,nullptr);
    mGstWavApp = nullptr;
	#endif
	mMapChimePMReset[EnChimeID::CHIME_ID_FAULT_1] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_FAULT_2] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_FAULT_3] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_FAULT_5] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_FAULT_6] = true;

	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_0] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_1] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_2] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_3] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_4] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_5] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_6] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_7] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_8] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_9] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_A] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_B] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_C] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_D] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_E] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_BODYCTRL_F] = true;

	mMapChimePMReset[EnChimeID::CHIME_ID_PROMPT_AC_ON] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_PROMPT_RESERVATION_CHARGE] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_PROMPT_READY_ICON] = true;
	mMapChimePMReset[EnChimeID::CHIME_ID_FAULT_4] = true;

	mbHmiReady = false;
	//声音模块启动太慢了，所以需要延迟响
	if(ZH::BaseLib::getWorldTimeMS() < 30*1000)
	{
		mDelayPlayFlag = true;
		mDelayPlayTimer = new ZH::BaseLib::STimer(5*1000, [this](int)->void{
			LOG_RECORD_DEBUG("mDelayPlayTimer_onTime");
			static uint8_t count = 0;
			if(!mbHmiReady)
			{
				if(++count < 20)
				{//保险措施
					mDelayPlayTimer->restart_resetTimer(300);
					return;
				}
				else
				{
					mbHmiReady = true;
				}
			}
			int ret = this->setVolume(mVol);
			if(ret < 0)
			{
				mDelayPlayTimer->restart_resetTimer(300);
				return;
			}
			else
			{
				std::lock_guard<std::mutex> lock(sMutex);
				mDelayPlayFlag = false;
				this->update(false);
			}
			
		});
		ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mDelayPlayTimer);
		mDelayPlayTimer->start();
	}
	else
	{
		mDelayPlayFlag = false;
		mDelayPlayTimer = nullptr;
	}
	mVol = 8;
    mWatchTimer = new ZH::BaseLib::STimer(10*1000, [this](int)->void{
        LOG_RECORD("mWatchTimer terminatePlayback()\n");
        if(this->mGstWavApp)
        {
            this->mGstWavApp->terminatePlayback();
        }
    });
    ZH::BaseLib::STimerManager::getInstance()->addTimerNode(mWatchTimer);
    mWatchGstPlaySts.count = 0;
    mWatchGstPlaySts.isPlaying = false;
    mWatchGstPlaySts.LastCount = 0xffffffff;
}

CChimeService::~CChimeService()
{
	mbThreadExitFlag = true;
	mThread.join();
    mWatchThread.join();
	#if USING_GSTREAMER_APP
	if(mGstWavApp)
	{
		mGstWavApp->Deinit();
		delete mGstWavApp;
	}
	#endif
	if(mDelayPlayTimer)
	{
		ZH::BaseLib::STimerManager::getInstance()->removeTimerNode(mDelayPlayTimer);
		delete mDelayPlayTimer;
	}
    if(mWatchTimer)
	{
		ZH::BaseLib::STimerManager::getInstance()->removeTimerNode(mWatchTimer);
		delete mWatchTimer;
	}
}
CChimeService* CChimeService::getInstance()
{
	CChimeService* pChimeService = sChimeService;
	if(!pChimeService)
	{
		std::lock_guard<std::mutex> lock(sMutex);
		if((pChimeService = sChimeService) == nullptr)
		{
			sChimeService = pChimeService = new CChimeService();
		}
	}
	return pChimeService;
}
void CChimeService::watchThreadFn()
{
    uint32_t abnormalCount = 0xffffffff;
    while (!mbThreadExitFlag)
    {
        if(mWatchGstPlaySts.isPlaying)
        {
            if(mWatchGstPlaySts.LastCount != mWatchGstPlaySts.count)
            {
                mWatchGstPlaySts.LastCount = mWatchGstPlaySts.count;
                abnormalCount = 0xffffffff;
            }
            else
            {//异常情况
                if(abnormalCount == mWatchGstPlaySts.count)
                {
                    LOG_RECORD("%s mWatchTimer.sts:%d, count:%d, abnormalCount:%d\n", __func__, mWatchTimer->getStatus(), mWatchGstPlaySts.count, abnormalCount);
                    exit(-1);
                }
                else
                {
                    abnormalCount = mWatchGstPlaySts.count;
                }
                LOG_RECORD("%s mWatchTimer.sts:%d, count:%d\n", __func__, mWatchTimer->getStatus(), mWatchGstPlaySts.count);
                if(this->mGstWavApp)
                {
                    this->mGstWavApp->terminatePlayback();
                }
            }
        }
        else
        {
            abnormalCount = 0xffffffff;
        }
        for(int i =0; i < 3;i++)
        {
            sleep(4);
        }
    }
    
}
void CChimeService::pushSound2PlayList(const char* soundFile, EnChimeID ID)
{
	
	if(!soundFile || soundFile[0] == '\0')
	{
		LOG_RECORD("(%s,%d), soundFile:%p\n",__func__, __LINE__, soundFile);
		return;
	}
	LOG_RECORD("(%s,%d), soundFile = %s, ID = %d, BitMap0:0x%x,BitMap1:0x%x, mWatchTimerSts:%d\n",__func__, __LINE__, soundFile, int(ID), mPlayChimeBitMap[0], mPlayChimeBitMap[1], mWatchTimer->getStatus());
	if(GET_CHIME_STATE(mPlayChimeBitMap, int(ID)))
    {//已经处理过了
        return;
    }
	if(ID < EnChimeID::END)
	{
		SET_CHIME_STATE(mPlayChimeBitMap, int(ID));
	}
	mPlaySoundCache.push({ID,soundFile});
}
void CChimeService::PlaySoundMainLoop()
{
	printf("(%s,%d)\n",__func__, __LINE__);
	LOG_RECORD_DEBUG("(%s,%d), tid:%d\n",__func__, __LINE__, int(syscall(SYS_gettid)));
	char filePath[256];
	while (!mbThreadExitFlag)
	{
		mbSoundPlayingFlag = false;
		mPlaySoundCache.wait();
		std::pair<EnChimeID,const char*> playInfo;
		const char* playFileName;
		if(mPlaySoundCache.pull(playInfo))
		{
            mWatchGstPlaySts.count += 1;
            mWatchGstPlaySts.isPlaying = true;
            LOG_RECORD("(%s,%d), id = %d\n",__func__, __LINE__, playInfo.first);
            mWatchTimer->start();
			mbSoundPlayingFlag = true;
			playFileName = playInfo.second;

			#if USING_GSTREAMER_APP
			snprintf(filePath, sizeof(filePath), "/usr/sbin/zhapp/midware/res/A301EV_audio/%s", playFileName);
            if(!mGstWavApp)
            {//晚点启动，过早启动会影响开机动画
                mGstWavApp = new gstWavApp();
	            mGstWavApp->Init(0,nullptr);
            }
			if(mGstWavApp)
            {
                mGstWavApp->playingAudioFile(filePath);
            }
			LOG_RECORD_DEBUG("(%s,%d) mGstWavApp:%p\n",__func__, __LINE__, mGstWavApp);
			#else
			snprintf(filePath, sizeof(filePath), "aplay \'/usr/sbin/zhapp/midware/res/A301EV_audio/%s\'", playFileName);
			printf("##filePath = %s,chimeID = %d\n", filePath, playInfo.first);
			std::string retCmd = mDoshellCmd(filePath);
			printf("@@retCmd = %s\n", retCmd.c_str());
			#endif
			if(playInfo.first < EnChimeID::END)
			{
				CLEAR_CHIME_STATE(mPlayChimeBitMap, int(playInfo.first));
			}
			LOG_RECORD("(%s,%d)\n",__func__, __LINE__);
            mWatchTimer->stop();
            mWatchGstPlaySts.isPlaying = false;
		}
		//sched_yield();
        usleep(20); 
	}
}
void CChimeService::OnWatchPlaySoundTimeout()
{
	//
}
void CChimeService::checkTriggerChime(EnChimeID ID, bool isTrigger)
{
	if(GET_CHIME_STATE(mTriggerChimeBitMap, int(ID)) == isTrigger)
    {//已经处理过了
        return;
    }
	if(mMapChimeData.find(ID) == mMapChimeData.end())
	{
		LOG_RECORD("(%s,%d), id:%d,flag:%d not in Map\n",__func__, __LINE__, int(ID), int(isTrigger));
		return;
	}
	LOG_RECORD_DEBUG("(%s,%d), id:%d,flag:%d\n",__func__, __LINE__, int(ID), int(isTrigger));
	std::lock_guard<std::mutex> lock(mMutex);
	if(isTrigger)
	{
		SET_CHIME_STATE(mTriggerChimeBitMap, int(ID));
	}
	else
	{
		CLEAR_CHIME_STATE(mTriggerChimeBitMap, int(ID));
	}
	//判断电源条件是否匹配
	if(!(int(mPowerModeSts) & int(mMapChimeData[ID]->getWorkPowerMode())))
	{
		LOG_RECORD_DEBUG("(%s,%d) mPowerModeSts = %d, getWorkPowerMode() = %d\n",__func__, __LINE__, int(mPowerModeSts), int(mMapChimeData[ID]->getWorkPowerMode()));
		return;
	}
	if(this->mDelayPlayFlag)
	{
		if(ZH::BaseLib::getWorldTimeMS() > 60*1000)
		{
			this->mDelayPlayFlag = false;
		}
		return;
	}
	if(isTrigger)
	{
		LOG_RECORD_DEBUG("(%s,%d)\n",__func__, __LINE__);
		mChimePlayList.push_back(ID);
		mMapChimeData[ID]->voiceSwitch(mbVoiceSwtichSts);
		mMapChimeData[ID]->startPlay();
	}
	else
	{
		for(auto iter = mChimePlayList.begin(); iter != mChimePlayList.end();)
		{
			if((*iter) == ID)
			{
				iter = mChimePlayList.erase(iter);
				break;
			}
			else
			{
				iter++;
			}
		}
		mMapChimeData[ID]->stopPlay();
	}
}
void CChimeService::update(bool isChangePowerMode)
{
	for(int i = 0; i < int(sizeof(mTriggerChimeBitMap)/sizeof(mTriggerChimeBitMap[0])); i++)
	{
		LOG_RECORD_DEBUG("(%s,%d) bitMap[%d]:%x", __func__, __LINE__, i,mTriggerChimeBitMap[i]);
		int bitLen = sizeof(mTriggerChimeBitMap[0])*8;
		for(int j = 0; j < bitLen;j++)
		{
			if((mTriggerChimeBitMap[i] >> j) & 0x01)
			{
				int idx = i*bitLen + j;
				EnChimeID chimeID = static_cast<EnChimeID>(idx);
				if(mMapChimeData.find(chimeID) != mMapChimeData.end())
				{
					if(isChangePowerMode && 
						(mMapChimePMReset.find(chimeID) == mMapChimePMReset.end()))
					{
						continue;
					}
					if((EnChimeID::CHIME_ID_TURN_OFF == chimeID) ||
						(EnChimeID::CHIME_ID_TURN_ON == chimeID))
					{
						continue;
					}
					LOG_RECORD_DEBUG("%s idx = %d", __func__, idx);
					if(int(mMapChimeData[chimeID]->getWorkPowerMode()) & int(mPowerModeSts))
					{
						mChimePlayList.push_back(chimeID);
						mMapChimeData[chimeID]->voiceSwitch(mbVoiceSwtichSts);
						mMapChimeData[chimeID]->startPlay();
					}
					else
					{
						mMapChimeData[chimeID]->stopPlay();//电源模式不匹配的报警需要停止播报
					}
				}
			}
		}
	}
}
void CChimeService::removeChime2PlayList(EnChimeID ID)
{
	if(mChimePlayList.empty())
	{
		return;
	}
	LOG_RECORD_DEBUG("(%s,%d),id:%d\n",__func__, __LINE__, int(ID));
	for(auto iter = mChimePlayList.begin(); iter != mChimePlayList.end();)
	{
		if((*iter) == ID)
		{
			iter = mChimePlayList.erase(iter);
			break;
		}
		else
		{
			iter++;
		}
	}
}
void CChimeService::chimeSettings_VoiceAndVolume(bool swSts, EnVolume volume)
{
	LOG_RECORD_DEBUG("(%s,%d), swSts:%d,volume:%d\n",__func__, __LINE__, int(swSts), int(volume));
	mbVoiceSwtichSts = swSts;
	mVolume = volume;
    mMapChimeData[EnChimeID::CHIME_ID_BODYCTRL_C]->voiceSwitch(mbVoiceSwtichSts);
    mMapChimeData[EnChimeID::CHIME_ID_BODYCTRL_E]->voiceSwitch(mbVoiceSwtichSts);
}
void CChimeService::setCurrentPowerMode(const EnWorkPowerSts pmSts)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if(pmSts != mPowerModeSts)
	{
		LOG_RECORD_DEBUG("(%s,%d) pmSts:%d", __func__, __LINE__, pmSts);
		mPowerModeSts = pmSts;
		this->update(true);
	}
}
int CChimeService::setVolume(int vol)
{
	int ret = -1;
	std::lock_guard<std::mutex> lock(mVolCtrlMutex);
	mVol = vol;
	LOG_RECORD_DEBUG("setVolume vol:%d", vol);
	ret = access(CChimeService::cVolumeNode, W_OK);
	if(ret == -1)
	{
		return ret;
	}
	FILE* fp = fopen(CChimeService::cVolumeNode, "r+");
	if(fp)
	{
		fprintf(fp, "%u\n", 5);
		fflush(fp);
		fclose(fp);
		ret = 0;
	}
	else
	{
		ret = -2;
	}
	return ret;
}

void CChimeService::ChimeDataInit()
{
	auto funcMapInit_One = [this](EnSoundType type,EnChimeID id, uint32_t index, EnWorkPowerSts pmSts,uint32_t timesPeriod,uint32_t times,uint32_t durationMs,uint32_t intervalMs, std::vector<const char*> vecSourceFiles)->void{
		this->mMapChimeData[id] = new CChimeData_OneSound(id, index, pmSts, timesPeriod,times,durationMs,intervalMs,vecSourceFiles.front(),type);
	};

	auto funcMapInit_Multi = [this](EnChimeID id, uint32_t index, EnWorkPowerSts pmSts,uint32_t timesPeriod,uint32_t times,uint32_t durationMs,uint32_t intervalMs, std::vector<const char*> vecSourceFiles)->void{
		this->mMapChimeData[id] = new CChimeData_MultiSound(id, index, pmSts, timesPeriod,times,durationMs,intervalMs,vecSourceFiles);
	};
    auto funcMapInit_Seatbelt = [this](EnChimeID id, uint32_t index, EnWorkPowerSts pmSts,uint32_t timesPeriod,uint32_t times,uint32_t durationMs,uint32_t intervalMs, std::vector<const char*> vecSourceFiles)->void{
		this->mMapChimeData[id] = new CChimeData_Seatbelt(id, index, pmSts, timesPeriod,times,durationMs,intervalMs,vecSourceFiles);
	};
				/*	EnSoundType						EnChimeID				preority		EnWorkPowerSts	periodTimes,playTimes,durationMs,IntervalMs， soundFile*/
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_FAULT_1,	4,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,		0,		{"37_Voice_PowerFailure.wav"});
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_FAULT_2,	4,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,		0,		{"38_Voice_PowerLimit.wav"});
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_FAULT_3,	4,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,		0,		{"39_Voice_BrakeFailure.wav"});
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_FAULT_4,	5,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,		0,		{"40_Voice_BrakeFluidLow.wav"});
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_FAULT_5,	6,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,		0,		{"41_Voice_LowResiMilgLow.wav"});
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_FAULT_6,	6,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,		0,		{"42_Voice_LowResiMilgVLow.wav"});
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_0,	11,	EnWorkPowerSts::PM_IGN_OFF,		1,		1,		1000,	0,		{"7_Buz_FollowMe.wav"});//跟随回家开启提示
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_1,	11,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"8_Buz_LockSpdOff.wav"});//速度落锁设置为取消
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_2,	11,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"9_Buz_LockSpdOn.wav"});//速度落锁设置为激活
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_3,	11,	EnWorkPowerSts::PM_IGN_ALL,		1,		1,		1000,	0,		{"10_Buz_KeyLearnSuccess.wav"});//钥匙学习成功
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_4,	11,	EnWorkPowerSts::PM_IGN_OFF,		1,		1,		1000,	0,		{"11_Buz_DriverDoorUnlockModeOff.wav"});//驾驶员门解锁模式设置为取消
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_5,	11,	EnWorkPowerSts::PM_IGN_OFF,		1,		1,		1000,	0,		{"12_Buz_DriverDoorUnlockModeOn.wav"});//驾驶员门解锁模式设置为激活
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_6,	11,	EnWorkPowerSts::PM_IGN_OFF,		1,		1,		0,		0,		{"14_Buz_RoofWinOpen.wav"});//天窗未关提示
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_7,	11,	EnWorkPowerSts::PM_IGN_OFF,		1,		3,		2000,	0,		{"15_Buz_KeyInCarAlrm.wav"});//钥匙在车内
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_8,	11,	EnWorkPowerSts::PM_IGN_ALL,		1,		3,		2000,	0,		{"16_Buz_KeyUndetectedAlrm.wav"});//未检测到钥匙报警
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_9,	11,	EnWorkPowerSts::PM_IGN_OFF,		1,		4,		1500,	0,		{"17_Buz_ExitActiveAlrm.wav"});//退出防盗激活状态提示保留
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_A,	11,	EnWorkPowerSts::PM_IGN_OFF,		1,		3,		2000,	0,		{"18_Buz_DoorOpenLock.wav"});//门未关闭锁
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_BODYCTRL_B,	11,	EnWorkPowerSts::PM_IGN_OFF,		1,		9,		2000,	0,		{"19_Buz_LowBatt.wav"});//遥控电池电量低
	funcMapInit_Seatbelt(EnChimeID::CHIME_ID_BODYCTRL_C,	11,	EnWorkPowerSts::PM_IGN_ON,		20,		6,		1200,	15000,	{"20_Buz_PassengerSeatbeltA.wav", "21_Buz_PassengerSeatbeltB.wav","61_Voice_PassengerSeatbelt.wav"});//副驾驶安全带未系
	funcMapInit_Multi(EnChimeID::CHIME_ID_BODYCTRL_D,	11,	EnWorkPowerSts::PM_IGN_OFF,		1,		1,		1000,	0,		{"13_Buz_LightOn.wav","43_Voice_LightOn.wav"});//灯未关提示 ##
	funcMapInit_Seatbelt(EnChimeID::CHIME_ID_BODYCTRL_E,	11,	EnWorkPowerSts::PM_IGN_ON,		20,		6,		1200,	15000,	{"22_Buz_DriverSeatbeltA.wav", "23_Buz_DriverSeatbeltB.wav","44_Voice_DriverSeatbelt.wav"});//主驾驶安全带未系
	funcMapInit_Multi(EnChimeID::CHIME_ID_BODYCTRL_F,	11,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"24_Buz_DoorOpen.wav","45_Voice_DoorOpen.wav"});//门未关提示

	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_PROMPT_VEHICLE_POWERING,	7,	EnWorkPowerSts::PM_IGN_ON,		1,		5,		1000,	0,		{"25_Buz_VehNtPowOff.wav"});////车辆未下电
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_PROMPT_OVER_SPD_WARN,		12,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,		0,		{"46_Voice_AbsVehSpd.wav"});////车速较快，请谨慎驾驶
	funcMapInit_Multi(EnChimeID::CHIME_ID_PROMPT_READY_ICON,		3,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,		0,		{"26_Buz_VcuReady.wav","47_Voice_VcuReady.wav"});////整车进入可行驶状态(ready灯)
	funcMapInit_Multi(EnChimeID::CHIME_ID_PROMPT_BRAKE_CHANGE_GEAR,	9,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"27_Buz_BrakeB4ChangeGear.wav","48_Voice_BrakeB4ChangeGear.wav"});////未踩制动换挡提醒
	funcMapInit_Multi(EnChimeID::CHIME_ID_PROMPT_AC_ON,				13,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"A23.响0.5S停0.5S.wav", "49_Voice_AirConOn.wav"});////空调开启 ##:49_Voice_AirConOn.wav
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_PROMPT_AC_HIGH_ENERGY_STS,13,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"50_Voice_AirConHighPowConsumption.wav"});////空调高能耗状态 ##:50_Voice_AirConHighPowConsumption.wav
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_PROMPT_AC_LOW_POWER_STS,	13,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"51_Voice_AirConLowPowConsumption.wav"});////空调低能耗状态 ##:51_Voice_AirConLowPowConsumption.wav
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_PROMPT_RESERVATION_AC,	14,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"52_Voice_AirConPresetSuccess.wav"});////预约空调   ##:52_Voice_AirConPresetSuccess.wav
	funcMapInit_Multi(EnChimeID::CHIME_ID_PROMPT_RESERVATION_CHARGE,15,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"A23.响0.5S停0.5S.wav","53_Voice_ChargePresetSuccess.wav"});////预约充电 ##:53_Voice_ChargePresetSuccess.wav
	funcMapInit_Multi(EnChimeID::CHIME_ID_PROMPT_VOICE_ON,			14,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"54_Voice_VoiceOn.wav","54_Voice_VoiceOn.wav"});////语音播报开启
	funcMapInit_Multi(EnChimeID::CHIME_ID_PROMPT_VOICE_OFF,			14,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"55_Voice_VoiceOff.wav","55_Voice_VoiceOff.wav"});////语音播报关闭
	funcMapInit_Multi(EnChimeID::CHIME_ID_PROMPT_BRAKE_START_GEAR,	9,	EnWorkPowerSts::PM_IGN_ALL,		1,		1,		1000,	0,		{"28_Buz_KeyStrtB4ChangeGear.wav","56_Voice_KeyStrtB4ChangeGear.wav"});////钥匙拧到START后再换挡
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_PROMPT_ENGINE_FAIL_4S,	4,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"57_Voice_GearFailure.wav"});////换挡器故障   ##:57_Voice_GearFailure.wav
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_PROMPT_PWS_OFF,			10,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		1000,	0,		{"58_Voice_LowSpdWarnOff.wav"});////行人警示系统已关闭  ##:58_Voice_LowSpdWarnOff.wav
	funcMapInit_One(EnSoundType::Sound_Voice,	EnChimeID::CHIME_ID_PROMPT_START_BRAKE,		8,	EnWorkPowerSts::PM_IGN_ALL,		1,		1,		1000,	0,		{"59_Voice_PepsStrtIndcr.wav"});////启动采刹车提示  ##:59_Voice_PepsStrtIndcr.wav

	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_TURN_OFF,				15,	EnWorkPowerSts::PM_IGN_ALL,		1,		1,		0,	0,	{"30_Buz_LeftIndicatorTick.wav"});//转向 滴
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_TURN_ON,				15,	EnWorkPowerSts::PM_IGN_ALL,		1,		1,		0,	0,	{"31_Buz_LeftIndicatorTock.wav"});//转向 嗒
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_TIRE_ADJUST_LEFT,		9,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,	0,	{"34_Buz_SteerLeft.wav"});//请向左打方向盘，调整轮胎
	funcMapInit_One(EnSoundType::Sound_Beep,	EnChimeID::CHIME_ID_TIRE_ADJUST_RIGHT,		9,	EnWorkPowerSts::PM_IGN_ON,		1,		1,		0,	0,	{"35_Buz_SteerRight.wav"});//请向右打方向盘，调整轮胎
}

const char*	CChimeService::cVolumeNode = "/sys/devices/i2c.3/i2c-3/3-001a/wm8988_vol";