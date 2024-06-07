
#include "MenuDisplay.h"
#include "mlog.h"
#include "kanziUtil.h"
#include "MsgInterface.h"
#include "mylogCtrl.h"


#define DiagnosesPtrIsNullptr(ptr)              \
    if (ptr == nullptr)                         \
    {                                           \
        LOGDBG("ptr [%s] is nullptr.\n", #ptr); \
        return ;                           		\
    }
     

CMenuDisplay::CMenuDisplay(ScreenSharedPtr p, CDispCtrlScreenLayer *m_pDisplayCtrl, Hmi *_app) : m_oRoot(p), m_pDisplayCtrl(m_pDisplayCtrl),
m_nPerMenuPage(0), m_nCurMenuPageID(0), m_nPerPassiveMenuPage(0), m_nMemoryTcIndex(Menu_None), m_nTcResetPageIndex(0), m_nThemePageIndex(0),
m_nThemeColorClassicPageIndex(0), m_nThemeColorTechnologyPageIndex(0), m_nSettingMainPageIndex(0), m_nSettingVoicePlayPageIndex(0), m_nSettingVoiceSizePageIndex(0), 
m_bArrowsVisible(true), m_nCurStyle(0), m_nCurClassicColor(0), m_nPreClassicColor(0),  m_nCurTechnologyColor(0), m_nCurVoicePlay(0), m_nCurVoiceSize(0), m_nClassBgIndex(0),
m_bInstEnergyConsumPage(false), m_nCurAlarmPage(0), m_nTotalAlarmPage(0), m_nMusicListIndex(0), m_nCurMusicListPage(0), m_nTotalMusicListPage(0), m_nTotalMusicListNum(0),
m_nEndPageMusicListNum(0), m_nCurPlayIndex(0), m_nContactIndex(0), m_nCurContactPage(0), m_nTotalContactPage(0), m_nTotalContactNum(0), m_nEndPageContactNum(0), m_bSyncContact(false),
m_nMusicListType(0), m_nCallRecordIndex(0), m_bSyncCallRecord(false), m_nRadioListType(0), m_nRadioListIndex(0), m_nCurRadioListPage(0), m_nCurRadioPlayIndex(0), m_nTotalRadioListPage(0),
m_nTotalRadioListNum(0), m_nEndPageRadioListNum(0), m_nMediaSourceIndex(0), m_nBtDeviceState(false), m_nUSBDeviceState(false)
{
	Init();
	mMenuNode = m_oRoot->lookupNode<Node2D>("#ResID_Menu");

	/*auto func = [](const char* funcName, int line, int i, std::string str)->void{
			{
				FILE* fp = fopen("/tmp/MsgService.txt", "a+");
				if (fp)
				{
					fprintf(fp, "(%s,%d),mContactList[%d] = %s\n", funcName, line, i, str.c_str());
					fflush(fp);
					fclose(fp);
				}
			} while (0);
	};
	std::vector<std::string> mList;
	for (int i = 0; i < 22; i++)
	{
		mList.push_back(to_string(i));
	}
	ContactList(true, mMusicInfoList);
	for (int i = 0; i < mMusicInfoList.size(); i++)
	{
		func(__func__, __LINE__, i, mMusicInfoList[i]);
	}*/
}

CMenuDisplay::~CMenuDisplay()
{
}


template<class T>
void CMenuDisplay::updateItemMsg(Node2DSharedPtr ptr, string property, T value)
{
	if (ptr != nullptr)
	{
		ptr->setProperty(DynamicPropertyType<T>(property), value);
	}
}

void CMenuDisplay::Init()
{
	mMenuPageMap[Menu_None] = { "" };
	mMenuPageMap[Menu_Tc_Main] = { "Prefab_Menu_Tc_Main" };
	mMenuPageMap[Menu_Tc_Sub_LongMileage] = { "Prefab_Menu_Tc_Sub_LongMileage" };
	mMenuPageMap[Menu_Tc_Sub_TirpMileage] = { "Prefab_Menu_Tc_Sub_TirpMileage" };
	mMenuPageMap[Menu_Tc_Sub_EnergyFlow] = { "Prefab_Menu_Tc_Sub_EnergyFlow" };
	mMenuPageMap[Menu_Tc_Sub_MotorSpeed] = { "Prefab_Menu_Tc_Sub_MotorSpeed" };
	mMenuPageMap[Menu_Tc_Sub_InstanEnergyCons] = { "Prefab_Menu_Tc_Sub_InstanEnergyCons" };
	mMenuPageMap[Menu_Tc_Sub_TireInfo] = { "Prefab_Menu_Tc_Sub_TireInfo" };
	mMenuPageMap[Menu_Tc_Sub_Power] = { "Prefab_Menu_Tc_Sub_Power" };
	mMenuPageMap[Menu_Tc_Sub_IntelligentScene] = { "Prefab_Menu_Tc_Sub_IntelligentScene" };
	mMenuPageMap[Menu_Tc_Third_Reset] = { "Prefab_Menu_Tc_Sub_Reset" };
	mMenuPageMap[Menu_AlarmInfo_Main] = { "Prefab_Menu_AlarmInfo_Main" };
	mMenuPageMap[Menu_AlarmInfo_Sub] = { "Prefab_Menu_AlarmInfo_Sub" };
	mMenuPageMap[Menu_Theme_Main] = { "Prefab_Menu_Theme_Main" };
	mMenuPageMap[Menu_Theme_Sub_Mode] = { "Prefab_Menu_Theme_Sub" };
	mMenuPageMap[Menu_Theme_Third_Color_Classic] = { "Prefab_Menu_Theme_Sub_Color_Classic" };
	mMenuPageMap[Menu_Theme_Third_Color_Technology] = { "Prefab_Menu_Theme_Sub_Color_Technology" };
	mMenuPageMap[Menu_Set_Main] = { "Prefab_Menu_Set_Main" };
	mMenuPageMap[Menu_Set_Sub_Main] = { "Prefab_Menu_Set_Sub" };
	mMenuPageMap[Menu_Set_Sub_VoicePlay] = { "Prefab_Menu_Set_Sub_VoicePlay" };
	mMenuPageMap[Menu_Set_Sub_VoiceSize] = { "Prefab_Menu_Set_Sub_VoiceSize" };
	mMenuPageMap[Menu_CallRecord_Main] = { "Prefab_Menu_CallRecord _Main" };
	mMenuPageMap[Menu_CallRecord_Sub] = { "Prefab_Menu_CallRecord _Sub" };
	mMenuPageMap[Menu_Contact_Main] = { "Prefab_Menu_Contact_Main" };
	mMenuPageMap[Menu_Contact_Sub] = { "Prefab_Menu_Contact_Sub" };
	mMenuPageMap[Menu_Media_MusicPlay] = { "Prefab_Menu_Media_MusicPlay" }; 
	mMenuPageMap[Menu_Media_RadioPlay] = { "Prefab_Menu_Media_RadioPlay" };
	mMenuPageMap[Menu_Media_List] = { "Prefab_Menu_Media_List" };
	mMenuPageMap[Menu_Media_NoList] = { "Prefab_Menu_Media_NoList" };
	mMenuPageMap[Menu_Media_Source] = { "Prefab_Menu_Media_Source" };
	mMenuPageMap[Menu_BTPhone_Incoming] = { "Prefab_Menu_BTPhone_Incoming" };
	mMenuPageMap[Menu_BTPhone_Dial] = { "Prefab_Menu_BTPhone_Dial" };
	mMenuPageMap[Menu_BTPhone_InCalling] = { "Prefab_Menu_BTPhone_InCalling" };
	mMenuPageMap[Menu_BTPhone_CallEnd] = { "Prefab_Menu_BTPhone_CallEnd" };
	mMenuPageMap[Menu_ChargeSubscribe] = { "Prefab_Menu_ChargeSubscribe" };
	mMenuPageMap[Menu_Charging] = { "Prefab_Menu_Charging" };
	mMenuPageMap[Menu_ChargeResult] = { "Prefab_Menu_ChargeResult" };

	mWarnListMap[EmPopWarnID::TIRE_MONITOR_RESET] = "请检查胎压监测系统";
	mWarnListMap[EmPopWarnID::TIRE_PRESSURE_HIGH] = "轮胎压力过高";
	mWarnListMap[EmPopWarnID::TIRE_PRESSURE_LOW] = "轮胎压力过低";
	mWarnListMap[EmPopWarnID::TIRE_LEAK] = "轮胎漏气";
	mWarnListMap[EmPopWarnID::TIRE_INFO] = "轮胎信息在行驶几分钟后显示";
	mWarnListMap[EmPopWarnID::POWER_DIST_FAIL_4S] = "电源分配故障";
	mWarnListMap[EmPopWarnID::KEY_SYS_4S] = "无钥匙系统故障";
	mWarnListMap[EmPopWarnID::LOW_SPEED_WARN_MAN_OFF] = "低速行人报警功能已关闭";
	mWarnListMap[EmPopWarnID::BRAKE_FAIL] = "手刹故障，驻车请注意";
	mWarnListMap[EmPopWarnID::VEHICLE_POWERING] = "车辆未下电";

}

void CMenuDisplay::UpdateMenuKey(EmKey key, EmKeyState keyState)
{
	//LOGERR("m_nCurMenuPageID **********= %d\n", m_nCurMenuPageID);
	if (m_nCurMenuPageID == Menu_ChargeSubscribe || m_nCurMenuPageID == Menu_Charging || m_nCurMenuPageID == Menu_ChargeResult)
	{
		return;
	}
	if ((m_nCurMenuPageID == Menu_Media_MusicPlay || m_nCurMenuPageID == Menu_Media_RadioPlay) && (key != EmKey::OK))
	{
		//LOGERR("###############\n");
		return;
	}
	//五秒无操作，上下箭头消失，屏蔽除菜单键外的其他按键
	if (m_bArrowsVisible == false)//m_bArrowsVisible = false就说明当前是在常显界面
	{
		switch (key)
		{
		case EmKey::RETURN:
			m_nCurMenuPageID = Menu_Tc_Main;
			m_bArrowsVisible = true;
			break;
		default:
			return;
			break;
		}
	}

	//如按键有用，重新开启定时器，重新定时
	stopTimer(m_MenuCloseToken);
	m_MenuCloseToken = startTimer(5000, KZU_TIMER_MESSAGE_MODE_ONCE, std::bind(&CMenuDisplay::MenuCloseTimer, this));
	
	auto it = mMenuPageMap.find(m_nCurMenuPageID);
	if (it != mMenuPageMap.end())
	{
		//把按键分发到各个页面
		switch (m_nCurMenuPageID)
		{
		case Menu_None:
			PageNoneKey(key, keyState);
			break;
		case Menu_Tc_Main:
			PageTcMainKey(key, keyState);
			break;
		case Menu_Tc_Sub_LongMileage:
			PageTcSubLongMileageKey(key, keyState);
			break;
		case Menu_Tc_Sub_TirpMileage:
			PageTcSubTirpMileageKey(key, keyState);
			break;
		case Menu_Tc_Sub_EnergyFlow:
			PageTcSubEnergyFlowKey(key, keyState);
			break;
		case Menu_Tc_Sub_MotorSpeed:
			PageTcSubMotorSpeedKey(key, keyState);
			break;
		case Menu_Tc_Sub_InstanEnergyCons:
			PageTcSubInstanEnergyConsKey(key, keyState);
			break;
		case Menu_Tc_Sub_TireInfo:
			PageTcSubTireInfoKey(key, keyState);
			break;
		case Menu_Tc_Sub_Power:
			PageTcSubPowerKey(key, keyState);
			break;
		case Menu_Tc_Sub_IntelligentScene:
			PageTcSubIntelligentSceneKey(key, keyState);
			break;
		case Menu_Tc_Third_Reset:
			PageTcSubResetKey(key, keyState);
			break;
		case Menu_AlarmInfo_Main:
			PageAlarmInfoMainKey(key, keyState);
			break;
		case Menu_AlarmInfo_Sub:
			PageAlarmInfoSubKey(key, keyState);
			break;
		case Menu_Theme_Main:
			PageThemeMainKey(key, keyState);
			break;
		case Menu_Theme_Sub_Mode:
			PageThemeSubKey(key, keyState);
			break;
		case Menu_Theme_Third_Color_Classic:
			PageThemeSubColorClassicKey(key, keyState);
			break;
		case Menu_Theme_Third_Color_Technology:
			PageThemeSubColorTechnologyKey(key, keyState);
			break;
		case Menu_Set_Main:
			PageSetMainKey(key, keyState);
			break;
		case Menu_Set_Sub_Main:
			PageSetSubMainKey(key, keyState);
			break;
		case Menu_Set_Sub_VoicePlay:
			PageSetSubVoicePlayKey(key, keyState);
			break;
		case Menu_Set_Sub_VoiceSize:
			PageSetSubVoiceSizeKey(key, keyState);
			break;
		case Menu_CallRecord_Main:
			PageCallRecordMainKey(key, keyState);
			break;
		case Menu_CallRecord_Sub:
			PageCallRecordSubKey(key, keyState);
			break;
		case Menu_Contact_Main:
			PageContactMainKey(key, keyState);
			break;
		case Menu_Contact_Sub:
			PageContactSubKey(key, keyState);
			break;
		case Menu_Media_MusicPlay:
			PageMediaMusicPlayKey(key, keyState);
			break;
		case Menu_Media_RadioPlay:
			PageMediaRadioPlayKey(key, keyState);
			break;
		case Menu_Media_List:
			if (m_nMediaType == 0)
				PageMediaListKeyMusic(key, keyState);
			else if (m_nMediaType == 1)
				PageMediaListKeyRadio(key, keyState);
			break;
		case Menu_Media_Source:
			PageMediaSourceKey(key, keyState);
			break;
		case Menu_BTPhone_Incoming:
			PageBTPhoneIncomingKey(key, keyState);
			break;
		case Menu_BTPhone_Dial:
			PageBTPhoneDialKey(key, keyState);
			break;
		case Menu_BTPhone_InCalling:
			PageBTPhoneInCallingKey(key, keyState);
			break;
		case Menu_BTPhone_CallEnd:

			break;
		default:
			break;
		}
		ActiveLoadMenuPage(m_nCurMenuPageID);
		//LOGDBG("m_nCurMenuPageID ======== %d\n", m_nCurMenuPageID);
	}
}

void CMenuDisplay::ActiveLoadMenuPage(int PageIndex)
{
	//LOGERR("function %s", __func__);
	//LOGERR("MenuPageID@@@@@@@@@@ =  %d\n", PageIndex);
	if (PageIndex != m_nPerMenuPage)
	{
		auto iter = mMenuPageMap.find(PageIndex);
		if (iter != mMenuPageMap.end())
		{
			if (mMenuNode->getChildCount() != 0)
			{
				mMenuNode->removeAllChildren();
				if (m_nPerMenuPage == Menu_Media_List)
				{
					if (!mMusicInfoList.empty())
					{
						mMusicInfoList.clear();
					}
					if (!mRadioInfoList.empty())
					{
						mRadioInfoList.clear();
					}
				}
				MainInterface(true);
			}
			if (iter->second != "")
			{
				Node2DSharedPtr subNode;
				std::string url = "kzb://a301/Prefabs/" + iter->second;
				MainInterface(false);
				KANZILOADPREFAB(m_oRoot, mMenuNode, "subNode", url.data(), subNode);
				//因为每次挂载之后属性都是kanzi设的默认值，所以挂载之后重新赋值，恢复上一次的高亮框索引
				MemoryMenuProperty((MenuPageEnum)PageIndex);
			}
		}
		m_nPerMenuPage = PageIndex;
	}
}

void CMenuDisplay::PassiveLoadMenuPage(int PageIndex)
{
	auto iter = mMenuPageMap.find(PageIndex);
	if (iter != mMenuPageMap.end())
	{
		if (mMenuNode->getChildCount() != 0)
		{
			mMenuNode->removeAllChildren();
			m_pDisplayCtrl->mMainView->getChild(0)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/Power_Scale_Pointer")->setVisible(true);
		}
		if (iter->second != "")
		{
			Node2DSharedPtr subNode;
			std::string url = "kzb://a301/Prefabs/" + iter->second;
			m_pDisplayCtrl->mMainView->getChild(0)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/Power_Scale_Pointer")->setVisible(false);
			KANZILOADPREFAB(m_oRoot, mMenuNode, "subNode", url.data(), subNode);
			//因为每次挂载之后属性都是kanzi设的默认值，所以挂载之后重新赋值，恢复上一次的高亮框索引
			MemoryMenuProperty((MenuPageEnum)PageIndex);
			m_nCurMenuPageID = PageIndex;
			m_nPerPassiveMenuPage = PageIndex; //记忆消息驱动菜单页面
			stopTimer(m_MenuCloseToken); //消息驱动切页面，把五秒定时切回常显界面的定时器关闭掉
		}
	}
}

void CMenuDisplay::MainInterface(bool state)
{
	switch (m_nCurStyle)
	{
	case 0:
		m_pDisplayCtrl->mMainView->getChild(0)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/Power_Scale_Pointer")->setVisible(state);
		mMenuNode->setVisible(true);
		break;
	case 1:
		m_pDisplayCtrl->mMainView->getChild(1)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter")->setVisible(state);
		m_pDisplayCtrl->mMainView->getChild(1)->getChild(0)->lookupNode<Node2D>("./GeneralInfo/Power")->setVisible(state);
		mMenuNode->setVisible(true);
		break;
	case 2:
		PowerPagePriority();
		if (state == false && (m_nCurMenuPageID == Menu_Media_MusicPlay || m_nCurMenuPageID == Menu_Media_RadioPlay || m_nCurMenuPageID == Menu_Media_List || m_nCurMenuPageID == Menu_Media_Source \
			|| m_nCurMenuPageID == Menu_Media_NoList))//科技主题有多媒体要显示智能场景
			return;
		m_pDisplayCtrl->mMainView->getChild(2)->getChild(0)->lookupNode<Node2D>("./DialGauge/IntelligentScene")->setVisible(state);
		break;
	default:
		break;
	}
}

void CMenuDisplay::SyncMainInterface(int theme)
{
	static int preTheme = -1;
	if (theme != preTheme)
	{
		if (m_nCurMenuPageID == Menu_Tc_Sub_Power && theme == 2)
		{
			m_nCurMenuPageID = Menu_None;
			m_nMemoryTcIndex = Menu_None;
			ActiveLoadMenuPage(m_nCurMenuPageID);
		}
		else if (m_nCurMenuPageID == Menu_Tc_Sub_IntelligentScene)
		{
			m_nCurMenuPageID = Menu_Tc_Sub_Power;
			m_nMemoryTcIndex = Menu_Tc_Sub_Power;
			ActiveLoadMenuPage(m_nCurMenuPageID);
			m_bArrowsVisible = false;
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ArrowsVisible"), m_bArrowsVisible);
		}
		if (m_nCurMenuPageID == Menu_None)
		{
			switch (m_nCurStyle)
			{
			case 0:
				m_pDisplayCtrl->mMainView->getChild(0)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/Power_Scale_Pointer")->setVisible(true);
				break;
			case 1:
				m_pDisplayCtrl->mMainView->getChild(1)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter")->setVisible(true);
				m_pDisplayCtrl->mMainView->getChild(1)->getChild(0)->lookupNode<Node2D>("./GeneralInfo/Power")->setVisible(true);
				break;
			case 2:
				PowerPagePriority();
				m_pDisplayCtrl->mMainView->getChild(2)->getChild(0)->lookupNode<Node2D>("./DialGauge/IntelligentScene")->setVisible(true);
				break;
			default:
				break;
			}
		}
		else
		{
			switch (m_nCurStyle)
			{
			case 0:
				m_pDisplayCtrl->mMainView->getChild(0)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/Power_Scale_Pointer")->setVisible(false);
				mMenuNode->setVisible(true);
				break;
			case 1:
				m_pDisplayCtrl->mMainView->getChild(1)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter")->setVisible(false);
				m_pDisplayCtrl->mMainView->getChild(1)->getChild(0)->lookupNode<Node2D>("./GeneralInfo/Power")->setVisible(false);
				mMenuNode->setVisible(true);
				break;
			case 2:
				PowerPagePriority();
				if (m_nCurMenuPageID == Menu_Media_MusicPlay || m_nCurMenuPageID == Menu_Media_RadioPlay || m_nCurMenuPageID == Menu_Media_List || m_nCurMenuPageID == Menu_Media_Source \
					|| m_nCurMenuPageID == Menu_Media_NoList)//科技主题有多媒体要显示智能场景
				{
					m_pDisplayCtrl->mMainView->getChild(2)->getChild(0)->lookupNode<Node2D>("./DialGauge/IntelligentScene")->setVisible(true);
				}
				else
				{
					m_pDisplayCtrl->mMainView->getChild(2)->getChild(0)->lookupNode<Node2D>("./DialGauge/IntelligentScene")->setVisible(false);
				}
				break;
			default:
				break;
			}
		}
	}
	preTheme = theme;
}

void CMenuDisplay::PowerPagePriority()
{
	//优先级：弹窗>多媒体>功率表
	if (m_pDisplayCtrl->mWarnNode->getChildCount() != 0)
	{
		m_pDisplayCtrl->mMainView->getChild(2)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/PowerValue")->setVisible(false);
		if (m_nCurMenuPageID == Menu_Media_MusicPlay || m_nCurMenuPageID == Menu_Media_RadioPlay || m_nCurMenuPageID == Menu_Media_List || m_nCurMenuPageID == Menu_Media_Source \
			|| m_nCurMenuPageID == Menu_Media_NoList)
		{
			mMenuNode->setVisible(false);
		}
		else
		{
			mMenuNode->setVisible(true);
		}
	}
	else
	{
		if (m_nCurMenuPageID == Menu_Media_MusicPlay || m_nCurMenuPageID == Menu_Media_RadioPlay || m_nCurMenuPageID == Menu_Media_List || m_nCurMenuPageID == Menu_Media_Source \
			|| m_nCurMenuPageID == Menu_Media_NoList)
		{
			m_pDisplayCtrl->mMainView->getChild(2)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/PowerValue")->setVisible(false);
		}
		else
		{
			m_pDisplayCtrl->mMainView->getChild(2)->getChild(0)->lookupNode<Node2D>("./DialGauge/PowerMeter/PowerValue")->setVisible(true);
		}
	}
}

void CMenuDisplay::MemoryMenuProperty(MenuPageEnum PageId)
{
	switch (PageId)
	{
	case Menu_Tc_Sub_LongMileage:
		break;
	case Menu_Tc_Sub_TirpMileage:
		break;
	case Menu_Tc_Sub_EnergyFlow:
		break;
	case Menu_Tc_Sub_MotorSpeed:
		break;
	case Menu_Tc_Sub_InstanEnergyCons:
		break;
	case Menu_Tc_Sub_TireInfo:
		break;
	case Menu_Tc_Sub_Power:
		break;
	case Menu_Tc_Sub_IntelligentScene:
		break;
	case Menu_Tc_Third_Reset:
		m_nTcResetPageIndex = 0;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nTcResetPageIndex);
		break;
	case Menu_AlarmInfo_Main:
		break;
	case Menu_AlarmInfo_Sub:
		AlarmInfoList(m_nCurAlarmPage);
		break;
	case Menu_Theme_Sub_Mode:
		m_nThemePageIndex = m_nCurStyle;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nThemePageIndex);
		break;
	case Menu_Theme_Third_Color_Classic:
		m_nThemeColorClassicPageIndex = m_nCurClassicColor;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nThemeColorClassicPageIndex);
		break;
	case Menu_Theme_Third_Color_Technology:
		m_nThemeColorTechnologyPageIndex = m_nCurTechnologyColor;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nThemeColorTechnologyPageIndex);
		break;
	case Menu_Set_Sub_Main:
		m_nSettingMainPageIndex = 0;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nSettingMainPageIndex);
		break;
	case Menu_Set_Sub_VoicePlay:
		m_nSettingVoicePlayPageIndex = m_nCurVoicePlay;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nSettingVoicePlayPageIndex);
		break;
	case Menu_Set_Sub_VoiceSize:
		m_nSettingVoiceSizePageIndex = m_nCurVoiceSize;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nSettingVoiceSizePageIndex);
		break;
	case Menu_CallRecord_Sub:
		m_nCallRecordIndex = 0;
		SetCallRecordInfo(1);
		break;
	case Menu_Contact_Sub:
		m_nContactIndex = 0;
		SetContactInfo(1);
		break;
	case Menu_Media_MusicPlay:
		break;
	case Menu_Media_List:
		
		break;
	case Menu_Media_NoList:
		break;
	case Menu_BTPhone_Incoming:
		break;
	case Menu_BTPhone_Dial:
		break;
	case Menu_BTPhone_InCalling:
		break;
	case Menu_BTPhone_CallEnd:
		break;
	case Menu_ChargeSubscribe:
		break;
	case Menu_Charging:
		break;
	case Menu_ChargeResult:
		break;
	default:
		break;
	}
}

/*开启定时器*/
TimerSubscriptionToken CMenuDisplay::startTimer(long milliseconds, KzuTimerMessageMode times, TimerFunction callBack)
{
	return addTimerHandler(m_oRoot->getMessageDispatcher(), kanzi::chrono::milliseconds(milliseconds), times, callBack);
}

/*停止定时器*/
void CMenuDisplay::stopTimer(TimerSubscriptionToken token)
{
	if (NULL != token)
	{
		removeTimerHandler(m_oRoot->getMessageDispatcher(), token);
	}
}

void CMenuDisplay::ResetPageTimer()
{
	stopTimer(m_MenuCloseToken);
	m_MenuCloseToken = startTimer(5000, KZU_TIMER_MESSAGE_MODE_ONCE, std::bind(&CMenuDisplay::MenuCloseTimer, this));
}

void CMenuDisplay::MenuCloseTimer()
{
	m_nCurMenuPageID = (m_nMemoryTcIndex == Menu_None) ?Menu_None : m_nMemoryTcIndex;
	ActiveLoadMenuPage(m_nCurMenuPageID);
	if (m_nMemoryTcIndex != Menu_None)
	{
		m_bArrowsVisible = false;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ArrowsVisible"), m_bArrowsVisible);
	}
}

void CMenuDisplay::PageNoneKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		break;
	case EmKey::DOWN:
		break;
	case EmKey::OK:
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageTcMainKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_Contact_Main;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_AlarmInfo_Main;
		break;
	case EmKey::OK:
		m_nCurMenuPageID = Menu_Tc_Sub_LongMileage;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageTcSubLongMileageKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = (m_nCurStyle == 2) ? Menu_Tc_Sub_IntelligentScene : Menu_Tc_Sub_Power;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Tc_Sub_TirpMileage;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::OK:
		switch (keyState)
		{
		case EmKeyState::S_PRESS:
			break;
		case EmKeyState::L_PRESS:
			m_nCurMenuPageID = Menu_Tc_Third_Reset;
			break;
		}
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageTcSubTirpMileageKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_Tc_Sub_LongMileage;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Tc_Sub_EnergyFlow;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::OK:
		switch (keyState)
		{
		case EmKeyState::S_PRESS:
			break;
		case EmKeyState::L_PRESS:
			m_nCurMenuPageID = Menu_Tc_Third_Reset;
			break;
		}
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageTcSubEnergyFlowKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_Tc_Sub_TirpMileage;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Tc_Sub_MotorSpeed;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::OK:
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageTcSubMotorSpeedKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_Tc_Sub_EnergyFlow;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Tc_Sub_TireInfo;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::OK:
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageTcSubInstanEnergyConsKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_Tc_Sub_TireInfo;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = (m_nCurStyle == 2) ? Menu_Tc_Sub_IntelligentScene : Menu_Tc_Sub_Power;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::OK:
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageTcSubTireInfoKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_Tc_Sub_MotorSpeed;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::DOWN:
		//瞬时能耗为无效信号，则常显信息界面不显示瞬时能耗页面
		if (m_bInstEnergyConsumPage == true)
		{
			m_nCurMenuPageID = Menu_Tc_Sub_InstanEnergyCons;
		}
		else
		{
			if (m_nCurStyle == 2)
				m_nCurMenuPageID = Menu_Tc_Sub_IntelligentScene;
			else
				m_nCurMenuPageID = Menu_Tc_Sub_Power;
		}
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::OK:
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageTcSubPowerKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = (m_bInstEnergyConsumPage == true) ? Menu_Tc_Sub_InstanEnergyCons : Menu_Tc_Sub_TireInfo;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Tc_Sub_LongMileage;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::OK:
		m_nCurMenuPageID = Menu_None;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}

void CMenuDisplay::PageTcSubIntelligentSceneKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = (m_bInstEnergyConsumPage == true) ? Menu_Tc_Sub_InstanEnergyCons : Menu_Tc_Sub_TireInfo;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Tc_Sub_LongMileage;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::OK:
		m_nCurMenuPageID = Menu_None;
		m_nMemoryTcIndex = m_nCurMenuPageID;
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageTcSubResetKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		(--m_nTcResetPageIndex < 0) ? m_nTcResetPageIndex = 1 : m_nTcResetPageIndex;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nTcResetPageIndex);
		break;
	case EmKey::DOWN:
		(++m_nTcResetPageIndex > 1) ? m_nTcResetPageIndex = 0 : m_nTcResetPageIndex;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nTcResetPageIndex);
		break;
	case EmKey::OK:
		if (m_nTcResetPageIndex == 0)
		{
			if (m_nMemoryTcIndex == Menu_Tc_Sub_LongMileage)
				HmiIPC::SetClearMileageInfo(0, 1);
			else if (m_nMemoryTcIndex == Menu_Tc_Sub_TirpMileage)
				HmiIPC::SetClearMileageInfo(1, 1);
		}

		m_nCurMenuPageID = m_nMemoryTcIndex;
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageAlarmInfoMainKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Theme_Main;
		break;
	case EmKey::OK:
		m_nCurMenuPageID = Menu_AlarmInfo_Sub;
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageAlarmInfoSubKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nTotalAlarmPage = int((mAlarmInfoList.size() / 3.0) + 0.99);
		if (m_nCurAlarmPage  < m_nTotalAlarmPage && m_nCurAlarmPage > 0)
		{
			m_nCurAlarmPage--;
			AlarmInfoList(m_nCurAlarmPage);
		}
		break;
	case EmKey::DOWN:
		m_nTotalAlarmPage = int((mAlarmInfoList.size() / 3.0) + 0.99);//向上取整
		if ((m_nCurAlarmPage + 1) < m_nTotalAlarmPage)
		{
			m_nCurAlarmPage++;
			AlarmInfoList(m_nCurAlarmPage);
		}
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_AlarmInfo_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageThemeMainKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_AlarmInfo_Main;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Set_Main;
		break;
	case EmKey::OK:
		if (m_nMemoryTcIndex == Menu_Tc_Sub_Power || m_nMemoryTcIndex == Menu_Tc_Sub_IntelligentScene)
		{
			m_nCurMenuPageID = Menu_None;
			m_nMemoryTcIndex = Menu_None;
		}
		else
		{
			m_nCurMenuPageID = Menu_Theme_Sub_Mode;
		}
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageThemeSubKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nThemePageIndex--;
		if (m_nThemePageIndex < 0)
			m_nThemePageIndex = 2;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nThemePageIndex);
		break;
	case EmKey::DOWN:
		m_nThemePageIndex++;
		if (m_nThemePageIndex > 2)
			m_nThemePageIndex = 0;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nThemePageIndex);
		break;
	case EmKey::OK:
		switch (m_nThemePageIndex)
		{
		case 0:
			m_nCurMenuPageID = Menu_Theme_Third_Color_Classic;
			m_nCurStyle = StyleEnum::Classic;
			break;
		case 1:
			m_nCurMenuPageID = Menu_None;
			m_nCurStyle = StyleEnum::Sport;
			break;
		case 2:
			m_nCurMenuPageID = Menu_Theme_Third_Color_Technology;
			m_nCurStyle = StyleEnum::Technology;
			break;
		}
		//m_pDisplayCtrl->mMainView->setProperty(DynamicPropertyType<int>("Common.StyleType"), m_nCurStyle);
		m_pDisplayCtrl->SwitchTheme(m_nCurStyle);
		SetTheme(m_nCurStyle);
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Theme_Main;
		break;
	default:
		break;
	}
}

void CMenuDisplay::SetTheme(int style)
{
	static int preStyle = -1;
	if (style != preStyle)
	{
		m_nClassBgIndex = (m_nClassBgIndex + 1) % 20;
		m_pDisplayCtrl->SwitchBg(m_nClassBgIndex);
		switch (style)
		{
		case 0:
			HmiIPC::SetThemeColor(style, m_nCurClassicColor);
			break;
		case 1:
			HmiIPC::SetThemeColor(style, 1);
			break;
		case 2:
			HmiIPC::SetThemeColor(style, m_nCurTechnologyColor);
			break;
		default:
			break;
		}
		preStyle = style;
	}
}

void CMenuDisplay::PageThemeSubColorClassicKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nThemeColorClassicPageIndex--;
		if (m_nThemeColorClassicPageIndex < 0)
			m_nThemeColorClassicPageIndex = 2;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nThemeColorClassicPageIndex);
		break;
	case EmKey::DOWN:
		m_nThemeColorClassicPageIndex++;
		if (m_nThemeColorClassicPageIndex > 2)
			m_nThemeColorClassicPageIndex = 0;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nThemeColorClassicPageIndex);
		break;
	case EmKey::OK:
		(m_nMemoryTcIndex == Menu_None) ? m_nCurMenuPageID = Menu_None : m_nCurMenuPageID = m_nMemoryTcIndex;
		m_nCurClassicColor = m_nThemeColorClassicPageIndex;
		//LOGWAR("m_nCurClassicColor = %d,m_nPreClassicColor = %d", m_nCurClassicColor, m_nPreClassicColor);
		if (m_nCurClassicColor != m_nPreClassicColor)
		{
			m_nClassBgIndex = (m_nClassBgIndex + 1) % 20;
			m_pDisplayCtrl->SwitchBg(m_nClassBgIndex);
			m_nPreClassicColor = m_nCurClassicColor;
		}
		HmiIPC::SetThemeColor(0, m_nCurClassicColor);
		m_pDisplayCtrl->mMainView->setProperty(DynamicPropertyType<int>("Common.ColorClassicState"), m_nCurClassicColor); 
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Theme_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageThemeSubColorTechnologyKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nThemeColorTechnologyPageIndex--;
		if (m_nThemeColorTechnologyPageIndex < 0)
			m_nThemeColorTechnologyPageIndex = 2;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nThemeColorTechnologyPageIndex);
		break;
	case EmKey::DOWN:
		m_nThemeColorTechnologyPageIndex++;
		if (m_nThemeColorTechnologyPageIndex > 2)
			m_nThemeColorTechnologyPageIndex = 0;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nThemeColorTechnologyPageIndex);
		break;
	case EmKey::OK:
		(m_nMemoryTcIndex == Menu_None) ? m_nCurMenuPageID = Menu_None : m_nCurMenuPageID = m_nMemoryTcIndex;
		m_nCurTechnologyColor = m_nThemeColorTechnologyPageIndex;
		HmiIPC::SetThemeColor(2, m_nCurTechnologyColor);
		m_pDisplayCtrl->mMainView->setProperty(DynamicPropertyType<int>("Common.ColorTechnoState"), m_nCurTechnologyColor);
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Theme_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageSetMainKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_Theme_Main;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_CallRecord_Main;
		break;
	case EmKey::OK:
		m_nCurMenuPageID = Menu_Set_Sub_Main;
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageSetSubMainKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nSettingMainPageIndex--;
		if (m_nSettingMainPageIndex < 0)
			m_nSettingMainPageIndex = 1;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nSettingMainPageIndex);
		break;
	case EmKey::DOWN:
		m_nSettingMainPageIndex++;
		if (m_nSettingMainPageIndex > 1)
			m_nSettingMainPageIndex = 0;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nSettingMainPageIndex);
		break;
	case EmKey::OK:
		if (m_nSettingMainPageIndex == 0)
			m_nCurMenuPageID = Menu_Set_Sub_VoicePlay;
		else if (m_nSettingMainPageIndex == 1)
			m_nCurMenuPageID = Menu_Set_Sub_VoiceSize;
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Set_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageSetSubVoicePlayKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nSettingVoicePlayPageIndex--;
		if (m_nSettingVoicePlayPageIndex < 0)
			m_nSettingVoicePlayPageIndex = 1;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nSettingVoicePlayPageIndex);
		break;
	case EmKey::DOWN:
		m_nSettingVoicePlayPageIndex++;
		if (m_nSettingVoicePlayPageIndex > 1)
			m_nSettingVoicePlayPageIndex = 0;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nSettingVoicePlayPageIndex);
		break;
	case EmKey::OK:
		m_nCurVoicePlay = m_nSettingVoicePlayPageIndex;
		(m_nMemoryTcIndex == Menu_None) ? m_nCurMenuPageID = Menu_None : m_nCurMenuPageID = m_nMemoryTcIndex;
		HmiIPC::SetVoicePlay(m_nCurVoicePlay, m_nCurVoiceSize);
		//LOGERR("m_nCurVoicePlay = %d,m_nCurVoiceSize = %d", m_nCurVoicePlay, m_nCurVoiceSize);
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Set_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageSetSubVoiceSizeKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nSettingVoiceSizePageIndex--;
		if (m_nSettingVoiceSizePageIndex < 0)
			m_nSettingVoiceSizePageIndex = 2;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nSettingVoiceSizePageIndex);
		break;
	case EmKey::DOWN:
		m_nSettingVoiceSizePageIndex++;
		if (m_nSettingVoiceSizePageIndex > 2)
			m_nSettingVoiceSizePageIndex = 0;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nSettingVoiceSizePageIndex);
		break;
	case EmKey::OK:
		m_nCurVoiceSize = m_nSettingVoiceSizePageIndex;
		(m_nMemoryTcIndex == Menu_None) ? m_nCurMenuPageID = Menu_None : m_nCurMenuPageID = m_nMemoryTcIndex;
		HmiIPC::SetVoicePlay(m_nCurVoicePlay, m_nCurVoiceSize);
		//LOGERR("m_nCurVoicePlay = %d,m_nCurVoiceSize = %d", m_nCurVoicePlay, m_nCurVoiceSize);
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Set_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageCallRecordMainKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_Set_Main;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Contact_Main;
		break;
	case EmKey::OK:
		//LOGERR("m_bSyncCallRecord = %d", m_bSyncCallRecord);
		if (m_bSyncCallRecord)
			m_nCurMenuPageID = Menu_CallRecord_Sub;
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageCallRecordSubKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCallRecordIndex--;
		if (m_nCallRecordIndex < 0)
		{
			m_nCurCallRecordPage--;
			if (m_nCurCallRecordPage < 1)//如果是第一页,把光标置为0和当前页置为1
			{
				m_nCurCallRecordPage = 1;
				m_nCallRecordIndex = 0;
			}
			else
			{
				//LOGERR("m_nCallRecordIndex = %d,m_nCurCallRecordPage = %d", m_nCallRecordIndex, m_nCurCallRecordPage);
				m_nCallRecordIndex = 3;
				SetCallRecordInfo(m_nCurCallRecordPage);
			}
		}
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nCallRecordIndex);
		break;
	case EmKey::DOWN:
		//LOGERR("m_nCurCallRecordPage = %d,m_nTotalCallRecordPage = %d,m_nEndPageCallRecordNum = %d,index = %d\n", m_nCurCallRecordPage, m_nTotalCallRecordPage, m_nEndPageCallRecordNum, m_nCallRecordIndex);
		if (m_nCurCallRecordPage < m_nTotalCallRecordPage)//如果当前页小于总的页数，说明当前页一定有5项，正常切光标
		{
			m_nCallRecordIndex++;
			if (m_nCallRecordIndex > 3)//如果切到最后一项，翻下一页，并把光标置为第一项
			{
				//LOGERR("m_nCurCallRecordPage =  %d, m_nTotalCallRecordPage = %d\n", m_nCurCallRecordPage, m_nTotalCallRecordPage);
				if (m_nCurCallRecordPage == m_nTotalCallRecordPage - 1)
				{
					HUHmiIPC::UpdateIVIBtList(HUHmiIPC::BtList_CallHistory, m_nCallRecordlistPage + 1);
					//LOGERR("################m_nCallRecordlistPage =  %d", m_nCallRecordlistPage + 1);
				}
				m_nCurCallRecordPage++;
				m_nCallRecordIndex = 0;
				SetCallRecordInfo(m_nCurCallRecordPage);
			}
		}
		else//如果当前页是最后一页，判断最后一页有多少项，切光标不能超过最后一项
		{
			if (m_nCallRecordIndex < m_nEndPageCallRecordNum - 1)
			{
				m_nCallRecordIndex++;
			}
		}
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nCallRecordIndex);
		break;
	case EmKey::OK:
		//LOGERR("SendCallRecordIndex&&&&&&&&&&&&&&&& = %d", (m_nCurCallRecordPage - 1) * 4 + m_nCallRecordIndex);
		HUHmiIPC::UpdateIVICallRecordIndex((m_nCurCallRecordPage - 1) * 4 + m_nCallRecordIndex);
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_CallRecord_Main;
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageContactMainKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nCurMenuPageID = Menu_CallRecord_Main;
		break;
	case EmKey::DOWN:
		m_nCurMenuPageID = Menu_Tc_Main;
		break;
	case EmKey::OK:
		//LOGERR("m_bSyncContact = %d", m_bSyncContact);
		if (m_bSyncContact)
			m_nCurMenuPageID = Menu_Contact_Sub;
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}
void CMenuDisplay::PageContactSubKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nContactIndex--;
		if (m_nContactIndex < 0)
		{
			m_nCurContactPage--;
			if (m_nCurContactPage < 1)//如果是第一页,把光标置为0和当前页置为1
			{
				m_nCurContactPage = 1;
				m_nContactIndex = 0;
			}
			else
			{
				//LOGERR("m_nContactIndex = %d,m_nCurContactPage = %d", m_nContactIndex, m_nCurContactPage);
				m_nContactIndex = 3;
				SetContactInfo(m_nCurContactPage);
			}
		}
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nContactIndex);
		break;
	case EmKey::DOWN:
		if (m_nCurContactPage < m_nTotalContactPage)//如果当前页小于总的页数，说明当前页一定有5项，正常切光标
		{
			m_nContactIndex++;
			if (m_nContactIndex > 3)//如果切到最后一项，翻下一页，并把光标置为第一项
			{
				if (m_nCurContactPage == m_nTotalContactPage - 1)
				{
					HUHmiIPC::UpdateIVIBtList(HUHmiIPC::BtList_Contact, m_nContactlistPage + 1);
					//LOGERR("################m_nContactlistPage =  %d", m_nContactlistPage + 1);
				}
				m_nCurContactPage++;
				m_nContactIndex = 0;
				SetContactInfo(m_nCurContactPage);
			}
		}
		else//如果当前页是最后一页，判断最后一页有多少项，切光标不能超过最后一项
		{
			//LOGERR("@@@@@@@@@@@@@@@@@@@@@m_nContactlistPage =  %d", m_nContactlistPage + 1);
			if (m_nContactIndex < m_nEndPageContactNum - 1)
			{
				m_nContactIndex++;
			}
		}
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nContactIndex);
		break;
	case EmKey::OK:
		//下发拨打联系人
		LOGERR("SendContactIndex&&&&&&&&&&&&&&&& = %d", (m_nCurContactPage - 1) * 4 + m_nContactIndex);
		HUHmiIPC::UpdateIVIContactIndex((m_nCurContactPage - 1) * 4 + m_nContactIndex);
		break;
	case EmKey::RETURN:
		m_nCurMenuPageID = Menu_Contact_Main;
		break;
	default:
		break;
	}
}

void CMenuDisplay::ContactList(bool state, int page,std::vector<std::string> list)
{
	m_bSyncContact = state;
	m_nContactlistPage = page;
	if (m_bSyncContact == 0 || m_nContactlistPage == 0)
	{
		mContactList.clear();
	}
	else
	{
		mContactList.insert(mContactList.end(), list.begin(), list.end());
	}
	m_nTotalContactNum = mContactList.size();
	if (m_nTotalContactNum > 0)
	{
		m_nTotalContactPage = (m_nTotalContactNum / 4.0) + 0.99;
		m_nEndPageContactNum = (m_nTotalContactNum - 1) % 4 + 1;
	}
	else
	{
		m_nTotalContactPage = 0;
		m_nEndPageContactNum = 0;
	}
}

void CMenuDisplay::SetContactInfo(int page)
{
	if (m_nCurMenuPageID = Menu_Contact_Sub)
	{
		m_nCurContactPage = page;
		if (page < m_nTotalContactPage)//不是最后一页
		{
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mContactList[(page - 1) * 4]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), (mContactList[(page - 1) * 4 + 1]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), (mContactList[(page - 1) * 4 + 2]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List3"), (mContactList[(page - 1) * 4 + 3]));
		}
		else//最后一页（前面做了限制，不可能超过最后一页）
		{
			//进来先全部清空
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List3"), "");
			switch (m_nEndPageContactNum)
			{
			case 1:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mContactList[(page - 1) * 4]));
				break;
			case 2:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mContactList[(page - 1) * 4]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), (mContactList[(page - 1) * 4 + 1]));
				break;
			case 3:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mContactList[(page - 1) * 4]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), (mContactList[(page - 1) * 4 + 1]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), (mContactList[(page - 1) * 4 + 2]));
				break;
			case 4:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mContactList[(page - 1) * 4]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), (mContactList[(page - 1) * 4 + 1]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), (mContactList[(page - 1) * 4 + 2]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List3"), (mContactList[(page - 1) * 4 + 3]));
				break;
			default:
				break;
			}
		}
	}
}

void CMenuDisplay::CallRecordList(bool state, int page, std::vector<StuCallRecord> list)
{
	m_bSyncCallRecord = state;
	m_nCallRecordlistPage = page;
	if (m_bSyncCallRecord == 0 || m_nCallRecordlistPage == 0)
	{
		mCallRecordList.clear();
	}
	else
	{
		mCallRecordList.insert(mCallRecordList.end(), list.begin(), list.end());
	}
	m_nTotalCallRecordNum = mCallRecordList.size();
	for (int i = 0; i < m_nTotalCallRecordNum; i++)
	{
		//LOGERR("CallRecordList[%d] = %s\n", i, mCallRecordList[i].name.c_str());
	}
	if (m_nTotalCallRecordNum > 0)
	{
		m_nTotalCallRecordPage = (m_nTotalCallRecordNum / 4.0) + 0.99;
		m_nEndPageCallRecordNum = (m_nTotalCallRecordNum - 1) % 4 + 1;
	}
	else
	{
		m_nCurCallRecordPage = 0;
		m_nTotalCallRecordPage = 0;
		m_nEndPageCallRecordNum = 0;
	}
}

void CMenuDisplay::SetCallRecordInfo(int page)
{
	//LOGERR("m_nCurCallRecordPage = %d,m_nTotalCallRecordPage = %d,m_nEndPageCallRecordNum = %d,m_nTotalCallRecordNum = %d\n", m_nCurCallRecordPage, m_nTotalCallRecordPage, m_nEndPageCallRecordNum, m_nTotalCallRecordNum);
	m_nCurCallRecordPage = page;
	if (page < m_nTotalCallRecordPage)//不是最后一页
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType"), (mCallRecordList[(page - 1) * 4].type));
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_1"), (mCallRecordList[(page - 1) * 4 + 1].type));
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_2"), (mCallRecordList[(page - 1) * 4 + 2].type));
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_3"), (mCallRecordList[(page - 1) * 4 + 3].type));
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mCallRecordList[(page - 1) * 4 + 0].name));
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), (mCallRecordList[(page - 1) * 4 + 1].name));
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), (mCallRecordList[(page - 1) * 4 + 2].name));
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List3"), (mCallRecordList[(page - 1) * 4 + 3].name));
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 0].amount) + ")");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 1].amount) + ")");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 2].amount) + ")");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List3_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 3].amount) + ")");
	}
	else//最后一页（前面做了限制，不可能超过最后一页）
	{
		//进来先全部清空
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType"), 5);
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_1"), 5);
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_2"), 5);
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_3"), 5);
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), "");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), "");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), "");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List3"), "");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0_1"), "");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1_1"), "");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2_1"), "");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List3_1"), "");
		switch (m_nEndPageCallRecordNum)
		{
		case 1:
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType"), (mCallRecordList[(page - 1) * 4].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mCallRecordList[(page - 1) * 4 + 0].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 0].amount) + ")");
			break;
		case 2:
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType"), (mCallRecordList[(page - 1) * 4].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_1"), (mCallRecordList[(page - 1) * 4 + 1].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mCallRecordList[(page - 1) * 4 + 0].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), (mCallRecordList[(page - 1) * 4 + 1].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 0].amount) + ")");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 1].amount) + ")");
			break;
		case 3:
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType"), (mCallRecordList[(page - 1) * 4].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_1"), (mCallRecordList[(page - 1) * 4 + 1].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_2"), (mCallRecordList[(page - 1) * 4 + 2].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mCallRecordList[(page - 1) * 4 + 0].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), (mCallRecordList[(page - 1) * 4 + 1].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), (mCallRecordList[(page - 1) * 4 + 2].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 0].amount) + ")");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 1].amount) + ")");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 2].amount) + ")");
			break;
		case 4:
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType"), (mCallRecordList[(page - 1) * 4].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_1"), (mCallRecordList[(page - 1) * 4 + 1].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_2"), (mCallRecordList[(page - 1) * 4 + 2].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.CallPhoneType_3"), (mCallRecordList[(page - 1) * 4 + 3].type));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), (mCallRecordList[(page - 1) * 4 + 0].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), (mCallRecordList[(page - 1) * 4 + 1].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), (mCallRecordList[(page - 1) * 4 + 2].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List3"), (mCallRecordList[(page - 1) * 4 + 3].name));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 0].amount) + ")");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 1].amount) + ")");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 2].amount) + ")");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List3_1"), "(" + to_string(mCallRecordList[(page - 1) * 4 + 3].amount) + ")");
			break;
		default:
			break;
		}
	}
}

void CMenuDisplay::PageMediaMusicPlayKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		break;
	case EmKey::DOWN:
		break;
	case EmKey::OK:
		//m_nCurMenuPageID = Menu_Media_List;
		m_pDisplayCtrl->SetMusicPlay(false);
		mMusicInfoList.clear();
		HUHmiIPC::UpdateIVIInfoList(HUHmiIPC::IVIList::IVI_Music_List,1);
		if (m_pDisplayCtrl->getCurMusicType() == 3)
		{
			stopTimer(m_MenuCloseToken);//先关掉5s定时器
			if (NULL != m_OneSecToken) removeTimerHandler(m_oRoot->getMessageDispatcher(), m_OneSecToken);
			m_OneSecToken = addTimerHandler(m_oRoot->getMessageDispatcher(),
				kanzi::chrono::milliseconds(1000),
				KZU_TIMER_MESSAGE_MODE_ONCE,
				[this](const TimerMessageArguments&){
				OpenMediaNoListPage();
			});
		}
		
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}

void CMenuDisplay::PageMediaRadioPlayKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		break;
	case EmKey::DOWN:
		break;
	case EmKey::OK:
		//m_nCurMenuPageID = Menu_Media_List; 
		m_pDisplayCtrl->SetRadioPlay(false);
		HUHmiIPC::UpdateIVIInfoList(HUHmiIPC::IVIList::IVI_Radio_LIst, 1);
		if (m_pDisplayCtrl->getCurRadioType() == 2)
		{
			stopTimer(m_MenuCloseToken);//先关掉5s定时器
			if (NULL != m_OneSecToken) removeTimerHandler(m_oRoot->getMessageDispatcher(), m_OneSecToken);
			m_OneSecToken = addTimerHandler(m_oRoot->getMessageDispatcher(),
				kanzi::chrono::milliseconds(1000),
				KZU_TIMER_MESSAGE_MODE_ONCE,
				[this](const TimerMessageArguments&){
				OpenMediaNoListPage();
			});
		}
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}

void CMenuDisplay::PageMediaListKeyMusic(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		m_nMusicListIndex--;
		//LOGERR("m_nMusicListIndex = %d,m_nCurMusicListPage = %d", m_nMusicListIndex, m_nCurMusicListPage);
		if (m_nMusicListIndex < 0)
		{
			m_nCurMusicListPage--;
			if (m_nCurMusicListPage < 1)//如果是第一页,把光标置为0和当前页置为1
			{
				m_nCurMusicListPage = 1;
				m_nMusicListIndex = 0;
			}
			else
			{
				//LOGERR("m_nMusicListIndex = %d,m_nCurMusicListPage = %d,m_nCurPlayIndex = %d", m_nMusicListIndex, m_nCurMusicListPage, m_nCurPlayIndex);
				m_nMusicListIndex = 4;
				SetMusicInfo(m_nCurMusicListPage);
				if (m_nCurMusicListPage == int((m_nCurPlayIndex / 5.0) + 0.99))
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), (m_nCurPlayIndex - 1) % 5);
				else
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), 20);//翻页了把播放按钮隐藏
			}
		}
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nMusicListIndex);
		break;
	case EmKey::DOWN:
		if (m_nCurMusicListPage < m_nTotalMusicListPage)//如果当前页小于总的页数，说明当前页一定有5项，正常切光标
		{
			m_nMusicListIndex++;
			if (m_nMusicListIndex > 4)//如果切到最后一项，翻下一页，并把光标置为第一项
			{
				if (m_nCurMusicListPage == m_nTotalMusicListPage - 1)
				{
					HUHmiIPC::UpdateIVIInfoList(HUHmiIPC::IVIList::IVI_Music_List, m_nMusiclistGroup + 1);
				}
				m_nCurMusicListPage++;
				m_nMusicListIndex = 0;
				SetMusicInfo(m_nCurMusicListPage);
				if (m_nCurMusicListPage == int((m_nCurPlayIndex / 5.0) + 0.99))
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), (m_nCurPlayIndex - 1) % 5);
				else
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), 20);//翻页了把播放按钮隐藏
			}
		}
		else//如果当前页是最后一页，判断最后一页有多少项，切光标不能超过最后一项
		{
			if (m_nMusicListIndex < m_nEndPageMusicListNum - 1)
			{
				m_nMusicListIndex++;
			}
		}
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nMusicListIndex);
		break;
	case EmKey::OK:
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), m_nMusicListIndex);
		//下发歌曲信息
		//LOGERR("m_nMusicListIndex = %d,m_nCurMusicListPage = %d,m_nCurPlayIndex = %d", m_nMusicListIndex, m_nCurMusicListPage, m_nCurPlayIndex);
		//LOGERR("SendMusicIndex = %d", (m_nCurMusicListPage - 1) * 5 + m_nMusicListIndex);
		HUHmiIPC::UpdateIVIInfoPlayIndex((m_nCurMusicListPage - 1) * 5 + m_nMusicListIndex);
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}

void CMenuDisplay::PageMediaListKeyRadio(EmKey key, EmKeyState keyState)
{

	switch (key)
	{
	case EmKey::UP:
		m_nRadioListIndex--;
		//LOGERR("m_nRadioListIndex = %d,m_nCurRadioListPage = %d", m_nRadioListIndex, m_nCurRadioListPage);
		if (m_nRadioListIndex < 0)
		{
			m_nCurRadioListPage--;
			if (m_nCurRadioListPage < 1)//如果是第一页,把光标置为0和当前页置为1
			{
				m_nCurRadioListPage = 1;
				m_nRadioListIndex = 0;
			}
			else
			{
				//LOGERR("m_nRadioListIndex = %d,m_nCurRadioListPage = %d,m_nCurRadioPlayIndex = %d", m_nRadioListIndex, m_nCurRadioListPage, m_nCurRadioPlayIndex);
				m_nRadioListIndex = 4;
				SetRadioInfo(m_nCurRadioListPage);
				if (m_nCurRadioListPage == int((m_nCurRadioPlayIndex / 5.0) + 0.99))
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), (m_nCurRadioPlayIndex - 1) % 5);
				else
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), 20);//翻页了把播放按钮隐藏
			}
		}
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nRadioListIndex);
		break;
	case EmKey::DOWN:
		if (m_nCurRadioListPage < m_nTotalRadioListPage)//如果当前页小于总的页数，说明当前页一定有5项，正常切光标
		{
			m_nRadioListIndex++;
			if (m_nRadioListIndex > 4)//如果切到最后一项，翻下一页，并把光标置为第一项
			{
				m_nCurRadioListPage++;
				m_nRadioListIndex = 0;
				SetRadioInfo(m_nCurRadioListPage);
				if (m_nCurRadioListPage == int((m_nCurRadioPlayIndex / 5.0) + 0.99))
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), (m_nCurRadioPlayIndex - 1) % 5);
				else
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), 20);//翻页了把播放按钮隐藏
			}
		}
		else//如果当前页是最后一页，判断最后一页有多少项，切光标不能超过最后一项
		{
			if (m_nRadioListIndex < m_nEndPageRadioListNum - 1)
			{
				m_nRadioListIndex++;
			}
		}
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nRadioListIndex);
		break;
	case EmKey::OK:
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), m_nRadioListIndex);
		//下发歌曲信息
		//LOGERR("m_nRadioListIndex = %d,m_nCurRadioListPage = %d,m_nCurRadioPlayIndex = %d", m_nRadioListIndex, m_nCurRadioListPage, m_nCurRadioPlayIndex);
		LOGERR("SendRadioIndex = %d", (m_nCurRadioListPage - 1) * 5 + m_nRadioListIndex);
		HUHmiIPC::UpdateIVIRadioPlayIndex((m_nCurRadioListPage - 1) * 5 + m_nRadioListIndex);
		break;
	case EmKey::RETURN:
		break;
	default:
		break;
	}
}

//auto func = [](const char* funcName, int line, int m_nMusicListIndex, int m_nTotalMusicListNum, int m_nTotalMusicListPage, int m_nCurMusicListPage)->void{
//		{
//			FILE* fp = fopen("/tmp/MsgService.txt", "a+");
//			if (fp)
//			{
//				fprintf(fp, "(%s,%d),m_nMusicListIndex = %d,m_nTotalMusicListNum = %d,m_nTotalMusicListPage = %d,m_nCurMusicListPage = %d\n", funcName, line, \
//					m_nMusicListIndex, m_nTotalMusicListNum, m_nTotalMusicListPage, m_nCurMusicListPage);
//				fflush(fp);
//				fclose(fp);
//			}
//		} while (0);
//};
void CMenuDisplay::MusicInfoList(int type, int curIndex, int page, std::vector<std::string> list)
{
	static int PreSourceType = -1;
	m_nMusiclistGroup = page;
	if (PreSourceType != type)
	{
		m_nMusicListType = type;
		PreSourceType = type;
		mMusicInfoList.clear();
	}
	m_nMediaType = 0; //0:音乐列表 1：电台列表
	if (m_nMusiclistGroup == 0)
	{
		mMusicInfoList.clear();
		m_nMusicListIndex = 0;
		m_nCurMusicListPage = 0;
		m_nTotalMusicListPage = 0;
		m_nEndPageMusicListNum = 0;
	}
	else
	{
		m_nCurMenuPageID = Menu_Media_List;
		ActiveLoadMenuPage(m_nCurMenuPageID);
		stopTimer(m_OneSecToken);//如果1s内收到列表，关闭定时器1s的定时器
		stopTimer(m_MenuCloseToken);
		m_MenuCloseToken = startTimer(5000, KZU_TIMER_MESSAGE_MODE_ONCE, std::bind(&CMenuDisplay::MenuCloseTimer, this));
		mMusicInfoList.insert(mMusicInfoList.end(), list.begin(), list.end());
		m_nTotalMusicListNum = mMusicInfoList.size();
		if (m_nTotalMusicListNum == 0)
		{
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ListState"), false);
			m_nMusicListIndex = 0;
			m_nCurMusicListPage = 0;
			m_nTotalMusicListPage = 0;
			m_nEndPageMusicListNum = 0;
		}
		else
		{
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ListState"), true);
			m_nCurPlayIndex = (curIndex  < 0) ? 0 : curIndex;
			if (m_nCurPlayIndex + 1 > m_nTotalMusicListNum)//如果当前播放的索引大于总共的歌曲数，则显示第一首
			{
				m_nCurPlayIndex = 0;//从0开始
			}
			m_nTotalMusicListPage = (m_nTotalMusicListNum / 5.0) + 0.99;
			m_nEndPageMusicListNum = (m_nTotalMusicListNum - 1) % 5 + 1;
			if (m_nMusiclistGroup == 1)
			{
				m_nMusicListIndex = m_nCurPlayIndex % 5;
				m_nCurMusicListPage = ((m_nCurPlayIndex + 1) / 5.0) + 0.99;
				SetMusicInfo(m_nCurMusicListPage);
			}
		}
	}
	
	if (m_nCurMenuPageID == Menu_Media_List)
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicOrRadio"), m_nMediaType);
	}
	
}
void CMenuDisplay::SetMusicInfo(int page)
{
	if (m_nCurMenuPageID == Menu_Media_List)
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nMusicListIndex);
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), m_nMusicListIndex);
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MediaType"), m_nMusicListType);
		if (m_nCurMusicListPage < m_nTotalMusicListPage)//不是最后一页
		{
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 1]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 2]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFour"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 3]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFive"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 4]));
		}
		else//最后一页（前面做了限制，不可能超过最后一页）
		{
			//进来先全部清空
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFour"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFive"), "");
			switch (m_nEndPageMusicListNum)
			{
			case 1:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5]));
				break;
			case 2:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 1]));
				break;
			case 3:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 1]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 2]));
				break;
			case 4:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 1]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 2]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFour"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 3]));
				break;
			case 5:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 1]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 2]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFour"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 3]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFive"), (mMusicInfoList[(m_nCurMusicListPage - 1) * 5 + 4]));
				break;
			default:
				break;
			}
		}
	}
}

void CMenuDisplay::RadioInfoList(int type, int curIndex)
{
	m_nMediaType = 1;
	m_nCurMenuPageID = Menu_Media_List;
	ActiveLoadMenuPage(m_nCurMenuPageID);
	stopTimer(m_OneSecToken);//如果1s内收到列表，关闭定时器1s的定时器
	stopTimer(m_MenuCloseToken);
	m_MenuCloseToken = startTimer(5000, KZU_TIMER_MESSAGE_MODE_ONCE, std::bind(&CMenuDisplay::MenuCloseTimer, this));
	if (m_nCurMenuPageID == Menu_Media_List)
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicOrRadio"), m_nMediaType);
	}

	m_nTotalRadioListNum = mRadioInfoList.size();
	m_nRadioListType = type;
	if (m_nTotalRadioListNum == 0)
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ListState"), false);
		m_nRadioListIndex = 0;
		m_nTotalRadioListPage = 0;
		m_nCurRadioListPage = 0;
		m_nEndPageRadioListNum = 0;
	}
	else
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ListState"), true);
		m_nCurRadioPlayIndex = (curIndex  < 0) ? 0 : curIndex;
		if (m_nCurRadioPlayIndex + 1 > m_nTotalRadioListNum)//如果当前播放的索引大于总共的歌曲数，则显示第一首
		{
			m_nCurRadioPlayIndex = 0;//从0开始
		}
		m_nRadioListIndex = m_nCurRadioPlayIndex % 5;
		m_nTotalRadioListPage = (m_nTotalRadioListNum / 5.0) + 0.99;
		m_nCurRadioListPage = ((m_nCurRadioPlayIndex + 1) / 5.0) + 0.99;
		m_nEndPageRadioListNum = (m_nTotalRadioListNum - 1) % 5 + 1;
	}
	SetRadioInfo(m_nCurRadioListPage);
}

void CMenuDisplay::SetRadioInfo(int page)
{
	std::string RadioType = "";
	switch (m_nRadioListType)
	{
	case 0:
		RadioType = "AM";
		break;
	case 1:
		RadioType = "FM";
		break;
	default:
		break;
	}
	if (m_nCurMenuPageID == Menu_Media_List)
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.SelectIndex"), m_nRadioListIndex);
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicPlayIndex"), m_nRadioListIndex);
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MediaType"), m_nRadioListType);
		if (m_nCurRadioListPage < m_nTotalRadioListPage)//不是最后一页
		{
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 1]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 2]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFour"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 3]));
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFive"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 4]));
		}
		else//最后一页（前面做了限制，不可能超过最后一页）
		{
			//进来先全部清空
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFour"), "");
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFive"), "");
			switch (m_nEndPageRadioListNum)
			{
			case 1:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5]));
				break;
			case 2:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 1]));
				break;
			case 3:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 1]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 2]));
				break;
			case 4:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 1]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 2]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFour"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 3]));
				break;
			case 5:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListOne"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListTwo"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 1]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListThree"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 2]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFour"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 3]));
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Media.ListFive"), RadioType + (mRadioInfoList[(m_nCurRadioListPage - 1) * 5 + 4]));
				break;
			default:
				break;
			}
		}
	}
}

void CMenuDisplay::PageMediaSourceKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::OK:
	{
		//LOGERR("m_nMediaSourceIndex = %d\n", m_nMediaSourceIndex);
		if ((m_nMediaSourceIndex == 1 && m_nBtDeviceState == false) || (m_nMediaSourceIndex == 2 && m_nUSBDeviceState == false))
		{
			return;
		}
		HUHmiIPC::UpdateIVIMediaSourceList(m_nMediaSourceIndex);
		static bool mSend = true;
		if (mSend == true && m_nMediaSourceIndex == 3)
		{
			mSend = false;
			m_MediaSourceToken = addTimerHandler(m_oRoot->getMessageDispatcher(),
				kanzi::chrono::milliseconds(1000),
				KZU_TIMER_MESSAGE_MODE_ONCE,
				[this](const TimerMessageArguments&){
				HUHmiIPC::UpdateIVIMediaSourceList(m_nMediaSourceIndex);
				if (NULL != m_MediaSourceToken) removeTimerHandler(m_oRoot->getMessageDispatcher(), m_MediaSourceToken);
			});
		}
		if (m_nMediaSourceIndex == 3 || m_nMediaSourceIndex == 6)
		{
			stopTimer(m_MenuCloseToken);//先关掉5s定时器
			if (NULL != m_OneSecToken) removeTimerHandler(m_oRoot->getMessageDispatcher(), m_OneSecToken);
			m_OneSecToken = addTimerHandler(m_oRoot->getMessageDispatcher(),
				kanzi::chrono::milliseconds(1000),
				KZU_TIMER_MESSAGE_MODE_ONCE,
				[this](const TimerMessageArguments&){
				IVIOpenMediaNoListPage();
				if (m_nMediaSourceIndex == 3)
				{
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicOrRadio"), 0);
				}
				else
				{
					mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicOrRadio"), 1);
				}
			});
		}
	}
		break;
	default:
		break;
	}
}

void CMenuDisplay::PageBTPhoneIncomingKey(EmKey key, EmKeyState keyState)
{
	switch (key)
	{
	case EmKey::UP:
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.State"), 0);
		break;
	case EmKey::DOWN:
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.State"), 1);
		break;
	case EmKey::OK:
		if (mMenuNode->getChild(0)->getProperty(DynamicPropertyType<int>("Common.State")) == 0)
			HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_Incoming, HUHmiIPC::BTPhoneButton::BTPhone_Answer, HUHmiIPC::BTPhoneControl::BTPhone_Open);
		else
			HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_Incoming, HUHmiIPC::BTPhoneButton::BTPhone_CallUp, HUHmiIPC::BTPhoneControl::BTPhone_Open);
		break;
	default:
		break;
	}
}

void CMenuDisplay::PageBTPhoneDialKey(EmKey key, EmKeyState keyState)
{
	bool DialSoundState = mMenuNode->getChild(0)->getProperty(DynamicPropertyType<bool>("Common.IsPressSound"));
	switch (key)
	{
	case EmKey::UP:
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.State"), 0);
		break;
	case EmKey::DOWN:
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.State"), 1);
		break;
	case EmKey::OK:
		if (mMenuNode->getChild(0)->getProperty(DynamicPropertyType<int>("Common.State")) == 0)
			HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_Dial, HUHmiIPC::BTPhoneButton::BTPhone_CallUp, HUHmiIPC::BTPhoneControl::BTPhone_Open);
		else
		{
			if (DialSoundState)
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressSound"), false);
				HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_Dial, HUHmiIPC::BTPhoneButton::BTPhone_Sound, HUHmiIPC::BTPhoneControl::BTPhone_Close);
			}
			else
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressSound"), true);
				HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_Dial, HUHmiIPC::BTPhoneButton::BTPhone_Sound, HUHmiIPC::BTPhoneControl::BTPhone_Open);
			}
		}
		break;
	default:
		break;
	}
}

void CMenuDisplay::PageBTPhoneInCallingKey(EmKey key, EmKeyState keyState)
{
	int CurInCallingIndex = mMenuNode->getChild(0)->getProperty(DynamicPropertyType<int>("Common.State"));
	bool InCallingSoundState = mMenuNode->getChild(0)->getProperty(DynamicPropertyType<bool>("Common.IsPressSound"));
	bool InCallingMoblieState = mMenuNode->getChild(0)->getProperty(DynamicPropertyType<bool>("Common.IsPressMobile"));
	switch (key)
	{
	case EmKey::UP:
		CurInCallingIndex--;
		if (CurInCallingIndex < 0)
			CurInCallingIndex = 0;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.State"), CurInCallingIndex);
		break;
	case EmKey::DOWN:
		CurInCallingIndex++;
		if (CurInCallingIndex > 2)
			CurInCallingIndex = 2;
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Common.State"), CurInCallingIndex);
		break;
	case EmKey::OK:
		switch (CurInCallingIndex)
		{
		case 0:
			HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_InCalling, HUHmiIPC::BTPhoneButton::BTPhone_CallUp, HUHmiIPC::BTPhoneControl::BTPhone_Open);
			break;
		case 1:
			if (InCallingSoundState)
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressSound"), false);
				HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_InCalling, HUHmiIPC::BTPhoneButton::BTPhone_Sound, HUHmiIPC::BTPhoneControl::BTPhone_Close);
			}
			else
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressSound"), true);
				HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_InCalling, HUHmiIPC::BTPhoneButton::BTPhone_Sound, HUHmiIPC::BTPhoneControl::BTPhone_Open);
			}
			break;
		case 2:
			if (InCallingMoblieState)
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressMobile"), false);
				HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_InCalling, HUHmiIPC::BTPhoneButton::BTPhone_Moblie, HUHmiIPC::BTPhoneControl::BTPhone_Open);
			}
			else
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.IsPressMobile"), true);
				HUHmiIPC::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage::BTPhone_InCalling, HUHmiIPC::BTPhoneButton::BTPhone_Moblie, HUHmiIPC::BTPhoneControl::BTPhone_Close);
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}


void CMenuDisplay::PageChargeSubscribeKey(EmKey key, EmKeyState keyState)
{
	m_nCurMenuPageID = m_nPerMenuPage;
}

void CMenuDisplay::PageChargingKey(EmKey key, EmKeyState keyState)
{
	m_nCurMenuPageID = m_nPerMenuPage;
}

void CMenuDisplay::PageChargeResultKey(EmKey key, EmKeyState keyState)
{
	m_nCurMenuPageID = m_nPerMenuPage;
}

void CMenuDisplay::CloseInstEnergyConsumPage()
{
	m_nCurMenuPageID = Menu_None;
	m_nMemoryTcIndex = Menu_None;
	ActiveLoadMenuPage(m_nCurMenuPageID);
}

void CMenuDisplay::OpenMusicPlayPage()
{
	m_nCurMenuPageID = Menu_Media_MusicPlay;
	ActiveLoadMenuPage(m_nCurMenuPageID);
}

void CMenuDisplay::CloseMusicPlayPage()
{
	//LOGERR("function %s", __func__);
	if (m_nCurMenuPageID == Menu_Media_MusicPlay || m_nCurMenuPageID == Menu_Media_List || m_nCurMenuPageID == Menu_Media_NoList)
	{
		m_nCurMenuPageID = m_nMemoryTcIndex;
		ActiveLoadMenuPage(m_nCurMenuPageID);
		if (m_nMemoryTcIndex != Menu_None)
		{
			m_bArrowsVisible = false;
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ArrowsVisible"), m_bArrowsVisible);
		}
	}
	
}

void CMenuDisplay::OpenRadioPlayPage()
{
	m_nCurMenuPageID = Menu_Media_RadioPlay;
	ActiveLoadMenuPage(m_nCurMenuPageID);
}
void CMenuDisplay::CloseRadioPlayPage()
{
	//LOGERR("function %s", __func__);
	if (m_nCurMenuPageID == Menu_Media_RadioPlay || m_nCurMenuPageID == Menu_Media_List || m_nCurMenuPageID == Menu_Media_NoList)
	{
		m_nCurMenuPageID = m_nMemoryTcIndex;
		ActiveLoadMenuPage(m_nCurMenuPageID);
		if (m_nMemoryTcIndex != Menu_None)
		{
			m_bArrowsVisible = false;
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ArrowsVisible"), m_bArrowsVisible);
		}
	}
}

void CMenuDisplay::OpenMediaNoListPage()
{
	m_nCurMenuPageID = Menu_Media_NoList;
	LOGERR("m_nCurMenuPageID =================== %d\n", m_nCurMenuPageID);
	ActiveLoadMenuPage(m_nCurMenuPageID);
	if (m_pDisplayCtrl->getCurMediaSourceType() == 0)
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicOrRadio"), 0);
	}
	else if (m_pDisplayCtrl->getCurMediaSourceType() == 1)
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MusicOrRadio"), 1);
	}
	stopTimer(m_MenuCloseToken);
	m_MenuCloseToken = startTimer(5000, KZU_TIMER_MESSAGE_MODE_ONCE, std::bind(&CMenuDisplay::MenuCloseTimer, this));
}

void CMenuDisplay::IVIOpenMediaNoListPage()
{
	m_nCurMenuPageID = Menu_Media_NoList;
	ActiveLoadMenuPage(m_nCurMenuPageID);
	stopTimer(m_MenuCloseToken);
	m_MenuCloseToken = startTimer(5000, KZU_TIMER_MESSAGE_MODE_ONCE, std::bind(&CMenuDisplay::MenuCloseTimer, this));
}

void CMenuDisplay::OpenChargePage(MenuPageEnum id)
{
	m_nCurMenuPageID = id;
	ActiveLoadMenuPage(m_nCurMenuPageID);
	stopTimer(m_MenuCloseToken);
}

void CMenuDisplay::OpenBTPhonePage(MenuPageEnum id)
{
	m_nCurMenuPageID = id;
	ActiveLoadMenuPage(m_nCurMenuPageID);
	m_bArrowsVisible = true;
}

void CMenuDisplay::CloseChargePage()
{
	//LOGERR("function %s", __func__);
	if (m_nCurMenuPageID == Menu_ChargeSubscribe || m_nCurMenuPageID == Menu_Charging || m_nCurMenuPageID == Menu_ChargeResult)
	{
		if (m_nCurMenuPageID != Menu_Media_MusicPlay && m_nCurMenuPageID != Menu_Media_RadioPlay && m_nCurMenuPageID != Menu_Media_List && m_nCurMenuPageID != Menu_Media_Source \
			&& m_nCurMenuPageID != Menu_Media_NoList)
		{
			m_nCurMenuPageID = m_nMemoryTcIndex;
			ActiveLoadMenuPage(m_nCurMenuPageID);
			if (m_nMemoryTcIndex != Menu_None)
			{
				m_bArrowsVisible = false;
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ArrowsVisible"), m_bArrowsVisible);
			}
		}
	}
}

void CMenuDisplay::AlarmInfoList(int AlarmPage)
{
	int num = 0;
	if (mAlarmInfoList.size() == 0)
	{
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), "");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), "");
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), "");
	}
	else
	{
		for (auto it = mAlarmInfoList.begin(); it != mAlarmInfoList.end(); it++)
		{
			if (num < (AlarmPage + 1) * 3)
			{
				if (num == AlarmPage * 3)
				{
					FirstAlarm = *it;
					SetWarnTxt(FirstAlarm, 0, true);
					if (++it != mAlarmInfoList.end())
					{
						SecondAlarm = *(it);
						SetWarnTxt(SecondAlarm, 1, true);
					}
					else//只有一条或者最后一页只有一条报警时，隐藏其他两条
					{
						SetWarnTxt(SecondAlarm, 1, false);
						SetWarnTxt(ThirdAlarm, 2, false);
						break;
					}
					if (++it != mAlarmInfoList.end())
					{
						ThirdAlarm = *(it);
						SetWarnTxt(ThirdAlarm, 2, true);
					}
					else//只有两条或者最后一页只有两条报警时，隐藏其他一条
					{
						SetWarnTxt(ThirdAlarm, 2, false);
						break;
					}
				}
			}
			else
			{
				break;
			}
			num++;
		}
	}
	
}

/*void CMenuDisplay::AlarmInfoList(int AlarmPage)
{
	int sum = mAlarmInfoList.size();
	int page = int((mAlarmInfoList.size() / 3.0) + 0.99);//向上取整
	int item = 0;
	if (mAlarmInfoList.size() > 0)
	{
		item = (mAlarmInfoList.size() - 1) % 3 + 1;//最后一页的列表数量
		if (page == AlarmPage)//如果是最后一页
		{
			switch (item)
			{
			case 1:
				SetWarnTxt(mAlarmInfoList[AlarmPage * 3], 0, true);
				SetWarnTxt(mAlarmInfoList[AlarmPage * 3], 1, false);
				SetWarnTxt(mAlarmInfoList[AlarmPage * 3], 2, false);
				break;
			case 2:
				SetWarnTxt(mAlarmInfoList[AlarmPage * 3], 0, true);
				SetWarnTxt(mAlarmInfoList[AlarmPage * 3 + 1], 1, true);
				SetWarnTxt(mAlarmInfoList[AlarmPage * 3], 2, false);
				break;
			case 3:
				SetWarnTxt(mAlarmInfoList[AlarmPage * 3], 0, true);
				SetWarnTxt(mAlarmInfoList[AlarmPage * 3 + 1], 1, true);
				SetWarnTxt(mAlarmInfoList[AlarmPage * 3 + 2], 2, true);
				break;
			default:
				break;
			}
		}
		else
		{
			SetWarnTxt(mAlarmInfoList[AlarmPage * 3], 0, true);
			SetWarnTxt(mAlarmInfoList[AlarmPage * 3 + 1], 1, true);
			SetWarnTxt(mAlarmInfoList[AlarmPage * 3 + 2], 2, true);
		}
	}
}*/

void CMenuDisplay::SetWarnTxt(EmPopWarnID CurWarn, int index, bool state)
{
	auto itWarn = mWarnListMap.find((EmPopWarnID)CurWarn);
	if (itWarn != mWarnListMap.end())
	{
		if (state)
		{
			switch (index)
			{
			case 0:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), itWarn->second);
				break;
			case 1:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), itWarn->second);
				break;
			case 2:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), itWarn->second);
				break;
			default:
				break;
			}
		}
		else
		{
			switch (index)//最后一页可能不足3条，需做隐藏处理
			{
			case 0:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List0"), "");
				break;
			case 1:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List1"), "");
				break;
			case 2:
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<string>("Common.List2"), "");
				break;
			default:
				break;
			}
		}
	}
}

void CMenuDisplay::setAlarmList()
{
	AlarmInfoList(m_nCurAlarmPage);
}

void CMenuDisplay::SetThemeColor(int theme, int color)
{
	switch (theme)
	{
	case 0:
		m_nCurClassicColor = color;
		m_pDisplayCtrl->mMainView->setProperty(DynamicPropertyType<int>("Common.ColorClassicState"), color);
		break;
	case 2:
		m_nCurTechnologyColor = color;
		m_pDisplayCtrl->mMainView->setProperty(DynamicPropertyType<int>("Common.ColorTechnoState"), color);
		break;
	default:
		break;
	}
}

void CMenuDisplay::UpdateIVIModeKey()
{
	if (m_nCurMenuPageID != Menu_Media_Source)
	{
		m_nCurMenuPageID = Menu_Media_Source;
		ActiveLoadMenuPage(m_nCurMenuPageID);
		LOGERR("UpdateIVIMediaCLose\n");
		HUHmiIPC::UpdateIVIMediaCLose(true);
		m_nMediaSourceIndex = -1;
		m_bArrowsVisible = true;
	}
	if (m_nCurMenuPageID == Menu_Media_Source)
	{
		m_nMediaSourceIndex++;
		if (m_nMediaSourceIndex > 6)
			m_nMediaSourceIndex = 0;
		stopTimer(m_MenuCloseToken);
		m_MenuCloseToken = startTimer(5000, KZU_TIMER_MESSAGE_MODE_ONCE, std::bind(&CMenuDisplay::MenuCloseTimer, this));
		switch (m_nMediaSourceIndex)
		{
		case 1:
			if (m_nBtDeviceState == false)
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MediaDeviceState"), 1);//图标置灰操作
			}
			else
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MediaDeviceState"), 0);
			}
			break;
		case 2:
			if (m_nUSBDeviceState == false)
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MediaDeviceState"), 1);
			}
			else
			{
				mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MediaDeviceState"), 0);
			}
			break;
		default:
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MediaDeviceState"), 0);
			break;
		}
		mMenuNode->getChild(0)->setProperty(DynamicPropertyType<int>("Media.MediaType"), m_nMediaSourceIndex);
	}
}

void CMenuDisplay::stop5sTimer()
{
	stopTimer(m_MenuCloseToken);
}

void CMenuDisplay::MemoryTcPage(int page)
{
	if (m_nCurMenuPageID != Menu_ChargeSubscribe && m_nCurMenuPageID != Menu_Charging && m_nCurMenuPageID != Menu_ChargeResult)
	{
		if (page == Menu_Tc_Sub_LongMileage || page == Menu_Tc_Sub_TirpMileage || page == Menu_Tc_Sub_EnergyFlow || page == Menu_Tc_Sub_MotorSpeed || \
			page == Menu_Tc_Sub_InstanEnergyCons || page == Menu_Tc_Sub_TireInfo || page == Menu_Tc_Sub_Power || page == Menu_Tc_Sub_IntelligentScene)
		{
			m_nMemoryTcIndex = page;
			m_nCurMenuPageID = m_nMemoryTcIndex;
			ActiveLoadMenuPage(m_nCurMenuPageID);
			m_bArrowsVisible = false;
			mMenuNode->getChild(0)->setProperty(DynamicPropertyType<bool>("Common.ArrowsVisible"), m_bArrowsVisible);
		}
	}
}