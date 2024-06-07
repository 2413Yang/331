
#ifndef MENUDISPLAY__H__
#define MENUDISPLAY__H__

#include "DisplayCtrl.h"
#include "DataDefine.h"
#include "HUMsgDefine.h"
#include "Common.h"


class CMenuDisplay
{
public:
	CMenuDisplay(ScreenSharedPtr p, CDispCtrlScreenLayer *m_pDisplayCtrl, Hmi *_app);
	~CMenuDisplay();

	template<class T>
	void updateItemMsg(Node2DSharedPtr,string, T);
	std::vector<EmPopWarnID> mAlarmInfoList;
	std::vector<std::string> mMusicInfoList;
	std::vector<std::string> mRadioInfoList;
	
public:
	Node2DSharedPtr mMenuNode;
	void UpdateMenuKey(EmKey key, EmKeyState keyState);
	void UpdateIVIModeKey();
	void setCurStyle(int style) { m_nCurStyle = style; };
	int getCurStyle() { return m_nCurStyle; };
	void setCurClassicColor(int color) { m_nCurClassicColor = color; };
	void setCurTechnologyColor(int color) { m_nCurTechnologyColor = color; };
	void setCurVoicePlay(int index) { m_nCurVoicePlay = index; };
	void setCurVoiceSize(int index) { m_nCurVoiceSize = index; };
	void setCurMenuPageID(int id) { m_nCurMenuPageID = id; };
	int getCurMenuPageID() { return m_nCurMenuPageID; };
	void setInstEnergyConsumPage(bool state) { m_bInstEnergyConsumPage = state; };
	bool getInstEnergyConsumPage() { return m_bInstEnergyConsumPage; };
	void CloseInstEnergyConsumPage();
	void OpenMusicPlayPage();
	void CloseMusicPlayPage();
	void OpenRadioPlayPage();
	void CloseRadioPlayPage();
	void OpenMediaNoListPage();
	void IVIOpenMediaNoListPage();
	void OpenChargePage(MenuPageEnum);
	void OpenBTPhonePage(MenuPageEnum);
	void CloseChargePage();
	void ResetPageTimer();
	void setAlarmList();
	void MusicInfoList(int type, int curIndex, int page, std::vector<std::string> list);
	void RadioInfoList(int type, int curIndex);
	void SetThemeColor(int theme,int color);
	void ContactList(bool,int, std::vector<std::string>);
	void CallRecordList(bool, int, std::vector<StuCallRecord>);
	void PowerPagePriority();
	int GetMemoryTcIndex(){ return m_nMemoryTcIndex; };
	void stop5sTimer();
	void MemoryTcPage(int);
	void SyncMainInterface(int theme);
	void setArrowsVisible(int state){ m_bArrowsVisible = state; };
	void setBtDeviceState(bool state){ m_nBtDeviceState = state; };
	void setUSBDeviceState(bool state){ m_nUSBDeviceState = state; };
private:
	void PageNoneKey(EmKey key, EmKeyState keyState);
	void PageTcMainKey(EmKey key, EmKeyState keyState);
	void PageTcSubLongMileageKey(EmKey key, EmKeyState keyState);
	void PageTcSubTirpMileageKey(EmKey key, EmKeyState keyState);
	void PageTcSubEnergyFlowKey(EmKey key, EmKeyState keyState);
	void PageTcSubMotorSpeedKey(EmKey key, EmKeyState keyState);
	void PageTcSubInstanEnergyConsKey(EmKey key, EmKeyState keyState);
	void PageTcSubTireInfoKey(EmKey key, EmKeyState keyState);
	void PageTcSubPowerKey(EmKey key, EmKeyState keyState);
	void PageTcSubIntelligentSceneKey(EmKey key, EmKeyState keyState);
	void PageTcSubResetKey(EmKey key, EmKeyState keyState);
	void PageAlarmInfoMainKey(EmKey key, EmKeyState keyState);
	void PageAlarmInfoSubKey(EmKey key, EmKeyState keyState);
	void PageThemeMainKey(EmKey key, EmKeyState keyState);
	void PageThemeSubKey(EmKey key, EmKeyState keyState);
	void PageThemeSubColorClassicKey(EmKey key, EmKeyState keyState);
	void PageThemeSubColorTechnologyKey(EmKey key, EmKeyState keyState);
	void PageSetMainKey(EmKey key, EmKeyState keyState);
	void PageSetSubMainKey(EmKey key, EmKeyState keyState);
	void PageSetSubVoicePlayKey(EmKey key, EmKeyState keyState);
	void PageSetSubVoiceSizeKey(EmKey key, EmKeyState keyState);
	void PageCallRecordMainKey(EmKey key, EmKeyState keyState);
	void PageCallRecordSubKey(EmKey key, EmKeyState keyState);
	void PageContactMainKey(EmKey key, EmKeyState keyState);
	void PageContactSubKey(EmKey key, EmKeyState keyState);
	void PageMediaMusicPlayKey(EmKey key, EmKeyState keyState);
	void PageMediaRadioPlayKey(EmKey key, EmKeyState keyState);
	void PageMediaListKeyMusic(EmKey key, EmKeyState keyState);
	void PageMediaListKeyRadio(EmKey key, EmKeyState keyState);
	void PageMediaSourceKey(EmKey key, EmKeyState keyState);
	void PageBTPhoneIncomingKey(EmKey key, EmKeyState keyState);
	void PageBTPhoneDialKey(EmKey key, EmKeyState keyState);
	void PageBTPhoneInCallingKey(EmKey key, EmKeyState keyState);
	void PageChargeSubscribeKey(EmKey key, EmKeyState keyState);
	void PageChargingKey(EmKey key, EmKeyState keyState);
	void PageChargeResultKey(EmKey key, EmKeyState keyState);
private:
	void Init();
	void ActiveLoadMenuPage(int PageIndex); //按键切换页面
	void PassiveLoadMenuPage(int PageIndex); //消息驱动切换页面
	void MemoryMenuProperty(MenuPageEnum);
	kanzi::TimerSubscriptionToken startTimer(long milliseconds, KzuTimerMessageMode times, kanzi::TimerFunction callBack);
	void stopTimer(kanzi::TimerSubscriptionToken token);
	void MenuCloseTimer();
	void MainInterface(bool state);
	void AlarmInfoList(int AlarmPage);
	void SetWarnTxt(EmPopWarnID CurWarn, int index, bool state);
	void SetMusicInfo(int page);
	void SetRadioInfo(int page);
	void SetContactInfo(int page);
	void SetCallRecordInfo(int page);

	void SetTheme(int style);

private:
	ScreenSharedPtr m_oRoot;
	CDispCtrlScreenLayer *m_pDisplayCtrl;


private:
	std::map < int, std::string> mMenuPageMap;
	std::map<EmPopWarnID, std::string> mWarnListMap;
	std::vector<std::string> mContactList;
	std::vector<StuCallRecord> mCallRecordList;

	EmPopWarnID FirstAlarm, SecondAlarm, ThirdAlarm;
	int m_nPerMenuPage;
	int m_nCurMenuPageID;//当前页面ID
	int m_nPerPassiveMenuPage;
	int m_nMemoryTcIndex;
	int m_nTcResetPageIndex;
	int m_nThemePageIndex;
	int m_nThemeColorClassicPageIndex;
	int m_nThemeColorTechnologyPageIndex;
	int m_nSettingMainPageIndex;
	int m_nSettingVoicePlayPageIndex;
	int m_nSettingVoiceSizePageIndex;
	bool m_bArrowsVisible;
	int m_nCurStyle;
	int m_nCurClassicColor;
	int m_nPreClassicColor;
	int m_nCurTechnologyColor;
	int m_nCurVoicePlay;
	int m_nCurVoiceSize;
	int m_nClassBgIndex;
	int m_nTotalAlarmPage;
	int m_nCurAlarmPage;
	int m_nMusicListType;//音乐类型
	int m_nMusicListIndex;//光标索引
	int m_nCurMusicListPage;//当前页
	int m_nCurPlayIndex;//当前播放索引
	int m_nTotalMusicListPage;//总共页数
	int m_nTotalMusicListNum;//总共数量
	int m_nEndPageMusicListNum; //最后一页的项数
	int m_nMusiclistGroup;//以100条消息为一组

	int m_nRadioListType;//电台类型
	int m_nRadioListIndex;//光标索引
	int m_nCurRadioListPage;//当前页
	int m_nCurRadioPlayIndex;//当前播放索引
	int m_nTotalRadioListPage;//总共页数
	int m_nTotalRadioListNum;//总共数量
	int m_nEndPageRadioListNum; //最后一页的项数

	int m_nMediaType;
	bool m_nBtDeviceState;
	bool m_nUSBDeviceState;

	int m_nContactIndex;//光标索引
	int m_nCurContactPage;//当前页
	int m_nTotalContactPage;//总共页数
	int m_nTotalContactNum;//总共数量
	int m_nEndPageContactNum; //最后一页的项数
	bool m_bSyncContact;
	int m_nContactlistPage;//以100条消息为一页

	int m_nCallRecordIndex;//光标索引
	int m_nCurCallRecordPage;//当前页
	int m_nTotalCallRecordPage;//总共页数
	int m_nTotalCallRecordNum;//总共数量
	int m_nEndPageCallRecordNum; //最后一页的项数
	bool m_bSyncCallRecord;
	int m_nCallRecordlistPage;//以100条消息为一页

	int m_nMediaSourceIndex;

	bool m_bInstEnergyConsumPage;

	kanzi::TimerSubscriptionToken m_MenuCloseToken;
	kanzi::TimerSubscriptionToken m_MediaSourceToken;
	kanzi::TimerSubscriptionToken m_OneSecToken;
};

#endif /*MENUDISPLAY__H__*/