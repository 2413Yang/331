#include "HUMsgService.h"

#define NUM(x) (x >= 0x30 && x<= 0x39)
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

void CHUMsgService::IVINaviInfo(int nStatus, int nLessMile, std::string sLessTime, int nDir, std::string sNextRoadMile, std::string sRoad, std::string sRoadNext, std::string mask)
{
    static struct d { 
        bool bValid = false; int nLessMile = 0; int nLessTimeHour = 0; int nLessTimeMinute = 0; 
        int nDir = -1; std::string sNextRoadMile = ""; std::string sRoad = ""; std::string sRoadNext = ""; 
    } dd;

    int size = mask.length();
    bool b = false;

    for (int i=0;i<size;i++)
    {
        if (mask[i] == '0') { continue; }
    
        switch (i) {
            case 0:
            {
                bool bNavi = (nStatus == 2 || nStatus == 3); // 导航中和模拟导航中使能
                if (dd.bValid != bNavi)
                {
                    dd.bValid = bNavi;
                    b = true;
                }
            }
                break;
            case 1: { if (dd.nLessMile != nLessMile) { dd.nLessMile = nLessMile; b = true; } break; }
            case 2:
            {
                int nHour = 0, nMinute = 0;
                SplitHourMinute(sLessTime, nHour, nMinute);
                if (dd.nLessTimeHour != nHour || dd.nLessTimeMinute != nMinute)
                {
                    dd.nLessTimeHour = nHour; dd.nLessTimeMinute = nMinute;
                    b = true;
                }
            }
                break;
            case 3: { if (dd.nDir != nDir) { dd.nDir = nDir; b = true; } break; }
            case 4:
            { 
                if (dd.sNextRoadMile != sNextRoadMile)
                {
                    dd.sNextRoadMile = sNextRoadMile;
                    b = true;
                }
            }
                break; 
            case 5: { if (dd.sRoad != sRoad) { dd.sRoad = sRoad; b = true; } break; }
            case 6: { if (dd.sRoadNext != sRoadNext) { dd.sRoadNext = sRoadNext; b = true; } break; }
            default: break;
        }
    }

    if (b)
    {
        LOGINF("%d-%d-%d-%d-%d-%s-%s-%s\n", dd.bValid, dd.nLessMile, dd.nLessTimeHour, dd.nLessTimeMinute, dd.nDir, dd.sNextRoadMile.data(), dd.sRoad.data(), dd.sRoadNext.data());
        HUHmiIPC::UpdateIVINaviInfo(dd.bValid, dd.nLessMile, dd.nLessTimeHour, dd.nLessTimeMinute, dd.nDir, dd.sNextRoadMile, dd.sRoad, dd.sRoadNext);
    }
}

void CHUMsgService::IVIMusicPlayInfo(int nSlideSync, int nStatus, int nMediaType, int nPlaySts, std::string sTitle, std::string sAlbum, std::string sSinger, std::string sFolder, int nPlayingIndex, int nTime, int nTimeTotal, bool bList, bool bHDD, bool bUSB, bool bBt, std::string mask)
{
    static struct d { 
        int nSlideSync; bool bValid = false; 
        bool bHDD = false; bool bUSB = false; bool bBt = false;
        int nPlaySts = -1; int nMediaType = -1; std::string sTitle = ""; std::string sAlbum = ""; std::string sSinger = ""; std::string sFolder = ""; int nPlayingIndex = -1; std::string sTime = "00:00"; std::string sTimeTotal = "00:00"; int nTimePercent = 0; 
    } dd;

    int size = mask.length();
    struct d d1 = dd;
    bool b = false;

    for (int i=0;i<size;i++)
    {
        if (mask[i] == '0') { continue; }

        switch (i) {
            case 0: // 三指滑动事件
            {
                LOGDBG("%s_%s_%d nSlideSync:%d->%d, bValid:%d\n", __FILE__, __func__, __LINE__, dd.nSlideSync, nSlideSync, dd.bValid);
                if (dd.nSlideSync != nSlideSync)
                {
                    dd.nSlideSync = nSlideSync;
                    b = (dd.nSlideSync == 0 || dd.nSlideSync == 1);
                }
            }
                break;
            case 1:
            {
                const bool bValid = (nStatus == 10 || nStatus == 47 || nStatus == 11 || nStatus == 12); // 10:HDD 47:BTAUDIO 11:USB 12:在线音乐
                if (dd.bValid != bValid)
                {
                    dd.bValid = bValid;
                    b = true;
                    if (dd.bValid) {
                        IVIRadioPlayInfo(0, nStatus, 0, 0, "", false, "01");

                        // 请求当前歌曲信息
                        IP_TO_HU_CMD(IPMediaCurInfo,0);
                    }
                }
            }
                break;
            case 2:
            {
                switch (nMediaType) {
                    // HDD
                    case 10:
                    {
                        if (dd.nMediaType != 0)
                        {
                            dd.nMediaType = 0; b = true;
                            IVIRadioPlayInfo(0, nStatus, 0, 0, "", false, "01");

                            // 请求当前歌曲信息
                            IP_TO_HU_CMD(IPMediaCurInfo,0);
                        }
                    }
                        break;
                    // BTAUDIO
                    case 47:
                    {
                        if (dd.nMediaType != 1)
                        {
                            dd.nMediaType = 1; b = true;

                            IVIRadioPlayInfo(0, nStatus, 0, 0, "", false, "01");

                            // 请求当前歌曲信息
                            IP_TO_HU_CMD(IPMediaCurInfo,0);
                        }
                    }
                        break;
                    // USB
                    case 11:
                    {
                        if (dd.nMediaType != 2)
                        {
                            dd.nMediaType = 2; b = true;

                            IVIRadioPlayInfo(0, nStatus, 0, 0, "", false, "01");

                            // 请求当前歌曲信息
                            IP_TO_HU_CMD(IPMediaCurInfo,0);
                        }
                    }
                        break;
                    // 在线音乐
                    case 12:
                    {
                        if (dd.nMediaType != 3)
                        {
                            dd.nMediaType = 3; b = true;

                            IVIRadioPlayInfo(0, nStatus, 0, 0, "", false, "01");

                            // 请求当前歌曲信息
                            IP_TO_HU_CMD(IPMediaCurInfo,0);
                        }
                    }
                        break;
                    default: break;
                }
            }
                break;
            case 3:
            {
                switch (nPlaySts)
                {
                    // 停止
                    case 1: { if (dd.nPlaySts != 2) { dd.nPlaySts = 2; b = true; } } break;
                    // 播放
                    case 2: { if (dd.nPlaySts != 0) { dd.nPlaySts = 0; b = true; } } break;
                    // 暂停
                    case 3: { if (dd.nPlaySts != 1) { dd.nPlaySts = 1; b = true; } } break;
                    default: break;
                }
            }
                break;
            case 4: { if (dd.sTitle != sTitle) { dd.sTitle = sTitle; b = true; } } break;
            case 5: { if (dd.sAlbum != sAlbum) { dd.sAlbum = sAlbum; b = true; } } break;
            case 6: { if (dd.sSinger != sSinger) { dd.sSinger = sSinger; b = true; } } break;
            case 7: { if (dd.sFolder != sFolder) { dd.sFolder = sFolder; b = true; } } break;
            case 8:
            {
                if (nPlayingIndex != dd.nPlayingIndex && nPlayingIndex >= 0)
                {
                    dd.nPlayingIndex = nPlayingIndex;
                    b = true;
                }
            }
                break;
            // 当前时间
            case 9:
            {
                char s[20];
                sprintf(s,"%02d:%02d",(nTime/1000)/60,(nTime/1000)%60);
                const std::string ss = s;

                if (dd.sTime != ss)
                {
                    dd.sTime = ss;
                    b = true;
                }

                const int nTimePercent = ((nTime<nTimeTotal && nTimeTotal>0) ? (nTime*100/nTimeTotal) : 0);
                if (dd.nTimePercent != nTimePercent)
                {
                    dd.nTimePercent = nTimePercent;
                    b = true;
                }
            }
                break;
            // 总时间
            case 10:
            {
                char s[20];
                sprintf(s,"%02d:%02d",(nTimeTotal/1000)/60,(nTimeTotal/1000)%60);
                const std::string ss = s;

                if (dd.sTimeTotal != s)
                {
                    dd.sTimeTotal = s;
                    b = true;
                }

                const int nTimePercent = ((nTime<nTimeTotal && nTimeTotal>0) ? (nTime*100/nTimeTotal) : 0);
                if (dd.nTimePercent != nTimePercent)
                {
                    dd.nTimePercent = nTimePercent;
                    b = true;
                }
            }
                break;
            case 11: // 进入列表,不发送播放界面给HMI
            {
                // 进入列表页面三指滑屏消失(不同步播放界面给HMI)
                if (bList)
                {
                    // 从播放页面进入列表页面,不需要发送取消播放界面消息给HMI
                    // IVIMusicPlayInfo(0, -1, -1, -1, "", "", "", "", 0, 0, 0, false, false, false, false, "1");
                    if (dd.nSlideSync != 0)
                    {
                        dd.nSlideSync = 0;
                        // b = true;
                    }
                    IVIRadioPlayInfo(0, -1, 0, 0, "", false, "1");
                }
            }
                break;
            case 12: // HDD
            {
                if (bHDD != dd.bHDD)
                {
                    LOGDBG("%s_%s_%d nMediaType:%d bHDD:%d->%d\n", __FILE__, __func__, __LINE__, dd.nMediaType, dd.bHDD, bHDD);
                    if (dd.bHDD != bHDD)
                    {
                        dd.bHDD = bHDD;
                        HUHmiIPC::UpdateIVIMusicDevice(ConverMusicType(MusicType_HDD), dd.bHDD);
                    }
                }
            }
                break;
            case 13: // USB
            {
                if (bUSB != dd.bUSB)
                {
                    LOGDBG("%s_%s_%d nMediaType:%d bUSB:%d->%d\n", __FILE__, __func__, __LINE__, dd.nMediaType, dd.bUSB, bUSB);
                    if (dd.bUSB != bUSB)
                    {
                        dd.bUSB = bUSB;
                        HUHmiIPC::UpdateIVIMusicDevice(ConverMusicType(MusicType_USB), dd.bUSB);
                    }
                }
            }
                break;
            case 14: // BT
            {
                if (bBt != dd.bBt)
                {
                    LOGDBG("%s_%s_%d nMediaType:%d bBt:%d->%d\n", __FILE__, __func__, __LINE__, dd.nMediaType, dd.bBt, bBt);
                    if (dd.bBt != bBt)
                    {
                        dd.bBt = bBt;
                        HUHmiIPC::UpdateIVIMusicDevice(ConverMusicType(MusicType_BT), dd.bBt);
                    }
                }
            }
                break;
            default:
                break;
        }
    }

    // if (d1 != dd)
    if (b && (dd.nSlideSync || (dd.nSlideSync != d1.nSlideSync)) && dd.bValid)
    {
        LOGINF("%d-%d-%d-%d-%s-%s-%s-%s-%d-%s-%s-%d\n",dd.nSlideSync,dd.bValid, dd.nPlaySts, dd.nMediaType, dd.sTitle.data(), dd.sAlbum.data(), dd.sSinger.data(), dd.sFolder.data(), dd.nPlayingIndex, dd.sTime.data(), dd.sTimeTotal.data(), dd.nTimePercent);
        HUHmiIPC::UpdateIVIMusicPlayInfo(dd.bValid && dd.nSlideSync, dd.nPlaySts, dd.nMediaType, dd.sTitle, dd.sAlbum, dd.sSinger, dd.sFolder, dd.nPlayingIndex, dd.sTime, dd.sTimeTotal, dd.nTimePercent);
    }
}

void CHUMsgService::IVIRadioPlayInfo(int nSlideSync, int nType, int nFrq, int nBrand, std::string sName, bool bList, std::string mask)
{
    static struct d { int nSlideSync; bool bValid = false; int nType = -1; std::string sFrq = ""; std::string sName; } dd;

    int size = mask.length();
    bool b = false;
    struct d d1 = dd;

    for (int i=0;i<size;i++)
    {
        if (mask[i] == '0') { continue; }

        switch (i) {
            case 0:
            {
                LOGDBG("%s_%s_%d nSlideSync:%d->%d bValid:%d\n", __FILE__, __func__, __LINE__, dd.nSlideSync, nSlideSync, dd.bValid);
                if (dd.nSlideSync != nSlideSync)
                {
                    dd.nSlideSync = nSlideSync;
                    b = (dd.nSlideSync == 0 || dd.nSlideSync == 1);
                }
            }
                break;
            case 1:
            {
                const bool bType = (nType == 0 || nType == 1 || nType == 7); // 0:FM 1:AM 7:online
                if (dd.bValid != bType)
                {
                    dd.bValid = bType;
                    b = true;
                    if (dd.bValid) {
                        IVIMusicPlayInfo(0, nType, 0, 0, "", "", "", "", 0, 0, 0, false, false, false, false, "01");

                        // 请求电台信息
                        IP_TO_HU_CMD(IPRaidoInfo,0);
                    }
                }
                if (nType == 0 && dd.nType != 1) // FM
                {
                    dd.nType = 1; b = true;

                    IVIMusicPlayInfo(0, nType, 0, 0, "", "", "", "", 0, 0, 0, false, false, false, false, "01");

                    // 请求电台信息
                    IP_TO_HU_CMD(IPRaidoInfo,0);
                }
                else if (nType == 1 && dd.nType != 0) // AM
                {
                    dd.nType = 0; b = true;

                    IVIMusicPlayInfo(0, nType, 0, 0, "", "", "", "", 0, 0, 0, false, false, false, false, "01");

                    // 请求电台信息
                    IP_TO_HU_CMD(IPRaidoInfo,0);
                }
                else if (nType == 7 && dd.nType != 2) // online
                {
                    dd.nType = 2; b = true;

                    IVIMusicPlayInfo(0, nType, 0, 0, "", "", "", "", 0, 0, 0, false, false, false, false, "01");

                    // 请求电台信息
                    IP_TO_HU_CMD(IPRaidoInfo,0);
                }
            }
                break; 
            case 2:
            {
                char s[20];
                if (mask[3] == '1')
                {
                    if (nBrand==0 || nBrand==1 || nBrand==2) sprintf(s, "%.1f", (nFrq/100.0f));
                    else if (nBrand==3 || nBrand==4) sprintf(s, "%d", nFrq);
                }
                std::string ss = s;
                if (dd.sFrq != ss)
                {
                    dd.sFrq = ss;
                    b = true;
                } 

                if (mask[3] == '1')
                {
                    if (dd.nType != 1 && (nBrand==0 || nBrand==1 || nBrand==2)) // FM
                    {
                        b = true;
                        dd.nType = 1;
                        if (!dd.bValid) dd.bValid = true;
                    }
                    else if (dd.nType != 0 && (nBrand==3 || nBrand==4)) // AM
                    {
                        b = true;
                        dd.nType = 0;
                        if (!dd.bValid) dd.bValid = true;
                    }
                }
            }
                break;
            case 4:
            {
                if (dd.sName != sName)
                {
                    dd.sName = sName;
                    b = true;
                }
            }
                break;
            case 5: // 进入列表,不发送播放界面给HMI
            {
                // 进入列表页面三指滑屏消失(不同步播放界面给HMI)
                if (bList)
                {
                    // 从播放页面进入列表页面,不需要发送取消播放界面消息给HMI
                    if (dd.nSlideSync != 0)
                    {
                        dd.nSlideSync = 0;
                        // b = true;
                    }
                    IVIMusicPlayInfo(0, -1, -1, -1, "", "", "", "", 0, 0, 0, false, false, false, false, "1");
                }
            }
                break;
            default:
                break;
        }
    }

    if (b && (dd.nSlideSync || (dd.nSlideSync != d1.nSlideSync)) && dd.bValid)
    {
        LOGINF("%s %d-%d-%d-%s-%s\n", __func__, dd.nSlideSync, dd.bValid, dd.nType, dd.sFrq.data(), dd.sName.data());
        HUHmiIPC::UpdateIVIRadioPlayInfo(dd.bValid && dd.nSlideSync, dd.nType, dd.sFrq, dd.sName);
    }
}

#define RV(t) ((t == RadioType_AM) || (t == RadioType_FM) || (t == RadioType_OL) || (t == RadioType_POL) || (t == RadioType_MIX))
int CMD(RadioType c) \
{ \
    if (c == RadioType_FM) return 0;  \
    else if (c == RadioType_AM) return 1; \
    else if (c == RadioType_OL) return 7; \
    else return 0xFF; \
}
int CMD_HMI(RadioType c) \
{ \
    if (c == RadioType_AM) return 0; \
    else if (c == RadioType_FM) return 1;  \
    else if (c == RadioType_OL) return 2; \
    else return 0xFF; \
}
void CHUMsgService::IVIRadioList(RadioType eType, std::vector<RadioCell> &list, int nCount, int nReqStart, int nReqCount, int nPlayIndex, int nPlayingIndex, int nReqHUCount, bool bMode, std::string mask)
{
    struct d { 
        RadioType eType = RadioType_NONE; std::vector<RadioCell> list; int nCount = 0; int nReqStart = 0; int nReqCount = 0; int nPlayIndex = -1; int nPlayingIndex = -1;
        d(const RadioType eType) { this->eType = eType; }
        d() {}
    };
    static RadioType esType = RadioType_NONE;
    static std::map<RadioType,struct d> m = {
        {RadioType_AM,d(RadioType_AM)}, {RadioType_FM,d(RadioType_FM)},
        {RadioType_OL,d(RadioType_OL)}, {RadioType_POL,d(RadioType_POL)},
        {RadioType_MIX,d(RadioType_MIX)}
    };

    // if (eType!=RadioType_NONE && !RV(esType)) { LOGDBG("%s_%s_%d RadioType(%d) err\n", __FILE__, __func__, __LINE__, esType); return; }

    int size = mask.length();

    for (int i=0; i<size; i++)
    {
        if (mask[i] == '0') continue;
        // if (esType!=RadioType_NONE && !RV(esType)) { LOGDBG("%s_%s_%d err esType:%d eType:%d mask:%s\n", __FILE__, __func__, __LINE__, esType, eType, mask.data()); break; }

        switch (i)
        {
            case 0:
            {
                if ((esType != eType) && RV(eType))
                {
                    LOGDBG("%s_%s_%d esType:%d->%d mask:%s\n", __FILE__, __func__, __LINE__, esType, eType, mask.data());
                    esType = eType;
                    if (m[esType].list.size() > 0) m[esType].list.clear();

                    // HUHmiIPC::UpdateIVIRadioList(CMD(esType), m[esType].nPlayingIndex, m_vecStrNull);

                    if (CMD(esType) != 0xff) IP_TO_HU_CMD(IPRaidoChannelCount, CMD(esType));
                }
            }
                break;
            case 1: // 列表:HU->IP
            {
                int len = list.size();
                LOGDBG("%s_%s_%d:%d _list:%d[%d-%d]\n", __FILE__, __func__, __LINE__, esType, len, (len>0?list[0].index:-1), (len>0?list[len-1].index:-1));

                for (auto j=list.begin(); j!=list.end(); j++)
                {
                    if (j->index < 0) continue;

                    auto k = m[esType].list.begin();
                    for (; k!=m[esType].list.end(); k++)
                    {
                        if (j->index == k->index) break;
                        if (j->index < k->index)
                        {
                            m[esType].list.insert(k,*j);
                            break;
                        }
                    }

                    if (m[esType].list.empty() || k==m[esType].list.end())
                        m[esType].list.push_back(*j);
                }
                len = m[esType].list.size();
                LOGDBG("%s_%s_%d:%d list_:%d[%d-%d]\n", __FILE__, __func__, __LINE__, esType, len, (len>0?m[esType].list[0].index:-1), (len>0?m[esType].list[len-1].index:-1));

                goto update_radio_list;
            }
                break;
            case 2: // 列表总数
            {
                if (esType == RadioType_FM || esType == RadioType_AM || esType == RadioType_OL || esType == RadioType_MIX)
                {
                    if ((nCount != m[esType].nCount || (int)m[esType].list.size() != nCount) && nCount>0)
                    {
                        m[esType].nCount = nCount;
                        LOGDBG("%s_%s_%d:%d nCount:%d,list.size:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nCount, m[esType].list.size());

                        // 请求电台(0:本地电台 1:在线电台 2:混合电台)
                        if (m[esType].list.size() > 0) m[esType].list.clear();
                        if (CMD(esType) != 0xff) IP_TO_HU_SRC_START_COUNT(IPRaidoChannelInfo, CMD(esType), 0, 200);
                    }
                }
                else
                {
                    LOGDBG("%s_%s_%d nCount:%d err(not exist)\n", __FILE__, __func__, __LINE__, esType);
                }
            }
                break;
            case 3: // HMI请求列表开始索引
            {
                if (nReqStart >= 0 && nReqStart != m[esType].nReqStart)
                {
                    m[esType].nReqStart = nReqStart;
                }
            }
                break;
            case 4: // HMI请求列表数量
            {
                if (nReqCount > 0 && nReqCount != m[esType].nReqCount)
                {
                    m[esType].nReqCount = nReqCount;
                }

update_radio_list:
                if (m[esType].nCount == 0) {
                    if (CMD(esType) != 0xff) IP_TO_HU_CMD(IPRaidoChannelCount, CMD(esType));
                }
                else if (m[esType].nReqStart >= 0 && m[esType].nReqCount > 0)
                {
                    int k=0;
                    int len = m[esType].list.size();
                    int lenLeft = 0;
                    for(; k<len; k++) { if ((int)m[esType].list[k].index >= m[esType].nReqStart) break; }
                    lenLeft = MIN(m[esType].nReqCount,m[esType].nCount-k);
                    LOGDBG("%s_%s_%d k:%d len:%d(index:%d-%d) nReqStart:%d nReqCount:%d nCount:%d lenLeft:%d m[%d].nPlayingIndex:%d\n", __FILE__, __func__, __LINE__, k, len, (len>0?m[esType].list[0].index:-1), (len>0?m[esType].list[len-1].index:-1), m[esType].nReqStart, m[esType].nReqCount, m[esType].nCount, lenLeft, esType, m[esType].nPlayingIndex);
                    if ((len-k) >= lenLeft) // MW缓存足够,直接返回给HMI
                    {
                        std::vector<std::string> listOut; listOut.clear();
                        for (; k<lenLeft; k++)
                        {
                            char c[128]; 
                            if (esType == RadioType_FM)
                                sprintf(c, "%.1f", (m[esType].list[k].frq/100.0f));
                            else if (esType == RadioType_AM)
                                sprintf(c, "%d", m[esType].list[k].frq);
                            else // RadioType_OL
                                memcpy(c, m[esType].list[k].name, sizeof(m[esType].list[k].name)/sizeof(m[esType].list[k].name[0]));

                            listOut.push_back(c);
                        }
                        IVIRadioPlayInfo(0, -1, 0, 0, "", true, "000001");
                        m[esType].nReqCount = -1;

                        LOGDBG("%s_%s_%d send2HMI m[%d].nPlayingIndex:%d %d\n", __FILE__, __func__, __LINE__, esType, m[esType].nPlayingIndex, listOut.size());
                        HUHmiIPC::UpdateIVIRadioList(CMD_HMI(esType), m[esType].nPlayingIndex, listOut);
                    }
                    else
                    {
                        if (len > 0 && len < m[esType].nReqCount)
                        {
                            if (CMD(esType) != 0xff) IP_TO_HU_SRC_START_COUNT(IPRaidoChannelInfo, CMD(esType), m[esType].list[len-1].index, MIN(m[esType].nReqCount,m[esType].nCount));
                        }
                    }
                } else {
                    LOGDBG("%s_%s_%d %d nCount:%d nReqStart:%d nReqCount:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nCount, m[esType].nReqStart, m[esType].nReqCount);
                }
            }
                break;
            case 5: // HMI->MW 列表界面播放
            {
                if (nPlayIndex >= 0 && nPlayIndex != m[esType].nPlayIndex)
                {
                    m[esType].nPlayIndex = nPlayIndex;

                    for (auto itor = m[esType].list.begin(); itor != m[esType].list.end(); itor++)
                    {
                        if ((int)itor->index == m[esType].nPlayIndex)
                        {
                            if (esType == RadioType_AM || esType == RadioType_FM) // 播放本地电台
                            {
                                LOGDBG("%s_%s_%d m[%d].nPlayIndex:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nPlayIndex);
                                HUFData d = { IPRaidoPlayback : { index : itor->index, frequence: itor->frq, band: itor->band } };
                                std::string f = mFrameEncoder(CHUFrame::FID_IPRaidoPlayback, &d, sizeof(d.IPRaidoPlayback));
                                IP_TO_HU_PRINT(IPRaidoPlayback, f);
                                mDevMsg.send(f);
                            }
                            else if (esType == RadioType_OL) // 播放在线电台
                            {
                                LOGDBG("%s_%s_%d m[%d].nPlayIndex:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nPlayIndex);
                                HUFData d = { IPRaidoOnlinePlayback : { } };
                                d.IPRaidoOnlinePlayback.radioType = itor->type;
                                d.IPRaidoOnlinePlayback.index = itor->index;
                                memcpy(d.IPRaidoOnlinePlayback.dataid, itor->id, sizeof(d.IPRaidoOnlinePlayback.dataid)/sizeof(d.IPRaidoOnlinePlayback.dataid[0]));
                                memcpy(d.IPRaidoOnlinePlayback.radioName, itor->name, sizeof(d.IPRaidoOnlinePlayback.radioName)/sizeof(d.IPRaidoOnlinePlayback.radioName[0]));
                                memcpy(d.IPRaidoOnlinePlayback.radioDesc, itor->dscr, sizeof(d.IPRaidoOnlinePlayback.radioDesc)/sizeof(d.IPRaidoOnlinePlayback.radioDesc[0]));
                                std::string f = mFrameEncoder(CHUFrame::FID_IPRaidoOnlinePlayback, &d, sizeof(d.IPRaidoOnlinePlayback));
                                IP_TO_HU_PRINT(IPRaidoOnlinePlayback, f);
                                mDevMsg.send(f);
                            }
                            break;
                        }
                    }
                }
            }
                break;
            case 6: // 当前播放索引
            {
                if (nPlayingIndex >= 0 && nPlayingIndex != m[esType].nPlayingIndex)
                {
                    m[esType].nPlayingIndex = nPlayingIndex;
                }
            }
                break;
            case 7: // 向HU请求列表
            {
                (void)nReqHUCount;
                if (CMD(esType) && m[esType].list.size() <= 0) IP_TO_HU_CMD(IPRaidoChannelCount, CMD(esType));
            }
                break;
            case 8: // HMI切换音源
            {
                LOGDBG("%s_%s_%d eType:%d esType:%d bMode:%d nCount:%d\n", __FILE__, __func__, __LINE__, eType, esType, bMode, m[eType].nCount);

                if (eType == esType) {
                    m[eType].nReqStart = nReqStart;
                    m[eType].nReqCount = nReqCount;

                    if (m[eType].nCount > 0)
                    {
                        goto update_radio_list;
                    }
                    else
                    {
                        if (CMD(eType) != 0xff) IP_TO_HU_CMD(IPRaidoChannelCount, CMD(eType));
                    }
                }
                else if (bMode && RV(eType))
                {
                    m[eType].nCount = 0;
                    m[eType].nReqStart = nReqStart;
                    m[eType].nReqCount = nReqCount;
                    if (m[eType].list.size() > 0) m[eType].list.clear();
                }
            }
                break;
            default:
                break;
        }
    }
}

void CHUMsgService::IVIPhoneInfo(int nStatus, std::string sName, std::string sTelNum, int nTelMedia, int nMute, std::string sTime, std::string mask)
{
    static struct d { int nStatus = -1; std::string sName = ""; std::string sTelNum = ""; int nTelMedia = -1; int nMute = -1; std::string sTime = ""; } dd;

    int size = mask.length();
    bool b = false;

    for (int i=0;i<size;i++)
    {
        if (mask[i] == '0') { continue; }

        switch (i)
        {
            case 0: // 电话状态
            {
                const int n = dd.nStatus; 
                switch (nStatus)
                {
                    case -1: { if (dd.nStatus != -1) { dd.nStatus = -1; b = true; } } break;
                    case  0: { if (dd.nStatus !=  0) { dd.nStatus =  0; b = true; } } break;
                    case  2: { if (dd.nStatus !=  1) { dd.nStatus =  1; b = true; } } break;
                    case  3: { if (dd.nStatus !=  2) { dd.nStatus =  2; b = true; } } break;
                    case  4: { if (dd.nStatus !=  3) { dd.nStatus =  3; b = true; } } break;
                    case  5: { if (dd.nStatus !=  4) { dd.nStatus =  4; b = true; } } break;
                    case  6: { if (dd.nStatus !=  5) { dd.nStatus =  5; b = true; } } break;
                    case  7: { if (dd.nStatus !=  6) { dd.nStatus =  6; b = true; } } break;
                    default: break;
                }

                // 电话状态变化,三指滑屏状态消失
                if (n != dd.nStatus)
                {
                    IVIRadioPlayInfo(0, -1, 0, -1, "", false, "1");
                    IVIMusicPlayInfo(0, -1, -1, -1, "", "", "", "", 0, 0, 0, false, false, false, false, "1");
                }
            }
                break;
            case 1: // 联系人名字
                if (dd.sName != sName) { dd.sName = sName; b = true; }
                break;
            case 2: // 联系人电话号码
                if (dd.sTelNum != sTelNum) { dd.sTelNum = sTelNum; b = true; }
                break;
            case 3: // 通话媒介
            {
                if (nTelMedia == 0 && dd.nTelMedia != 1) { dd.nTelMedia = 1; b = true; }
                else if (nTelMedia == 1 && dd.nTelMedia != 0) { dd.nTelMedia = 0; b = true; }
            }
                break;
            case 4:
            {
                if (nMute == 0 && dd.nMute != 1) { dd.nMute = 1; b = true; }
                else if (nMute == 1 && dd.nMute != 0) { dd.nMute = 0; b = true; }
            }
                break;
            // case 5:
            // {
            //     if (dd.sTime != sTime) { dd.sTime = sTime; b = true; }
            // }
            //     break;
            default:
                break;
        }
    }

    if (b)
    {
        LOGINF("%d-%s-%s-%d-%d-%s\n", dd.nStatus, dd.sName.data(), dd.sTelNum.data(), dd.nTelMedia, dd.nMute, dd.sTime.data());
        HUHmiIPC::UpdateIVIPhoneInfo(dd.nStatus, dd.sName, dd.sTelNum, dd.nTelMedia, dd.nMute, dd.sTime);
    }
}

void CHUMsgService::IVIBtInfo(int nStatus, std::string sPhoneName, std::string mask)
{
    static struct d { int nStatus = -1; std::string sPhoneName = ""; } dd;

    int size = mask.length();
    bool b = false;

    for (int i=0;i<size;i++)
    {
        if (mask[i] == '0') { continue; }

        switch (i)
        {
            case 0: // 蓝牙状态
            {
                if (dd.nStatus != nStatus) LOGDBG("%s_%s_%d nStatus:%d->%d\n", __FILE__, __func__, __LINE__, dd.nStatus, nStatus);
                switch (nStatus)
                {
                    case 0: // 蓝牙未连接
                    { if (dd.nStatus != 0) { dd.nStatus = 0; b = true; } }
                        break;
                    case 1: // 蓝牙连接,联系人未同步完成
                    { if (dd.nStatus != 1) { dd.nStatus = 1; b = true; } }
                        break;
                    case 2: // 蓝牙连接,联系人同步完成
                    { if (dd.nStatus != 2) { dd.nStatus = 2; b = true; } }
                        break;
                    default:
                        break;
                }

                if (b)
                {
                    CallHistory c;
                    IVIContactList(0, m_vecContactNull, 0, 0, c, dd.nStatus, "000001");
                    IVICallHistory(0, m_vecCallHNull, 0, 0, dd.nStatus, "00001");
                }
            }
                break;
            case 1: // 电话名字
            {
                if (dd.sPhoneName != sPhoneName)
                {
                    dd.sPhoneName = sPhoneName;
                    b = true;
                }
            }
                break;
            default:
                break;
        }
    }

    if (b)
    {
        LOGINF("%d-%s\n", dd.nStatus, dd.sPhoneName.data());
        HUHmiIPC::UpdateIVIBtInfo(dd.nStatus, dd.sPhoneName);
    }
}

#define MV(e) ((e == MusicType_HDD) || (e == MusicType_USB) || (e == MusicType_OL) || (e == MusicType_BT) || (e == MusicType_NONE))
#define MLV(e) ((e == MusicType_HDD) || (e == MusicType_USB) || (e == MusicType_OL))
void CHUMsgService::IVIMusicList(MusicType eType, std::vector<MusicCell> &list, int nCount, int nReqPage, int nPlayIndex, int nPlayingIndex, bool bMode, std::string mask)
{
    struct d { 
        MusicType eType = MusicType_NONE; std::vector<MusicCell> list; int nCount = 0; int nPages = 0; int nReqPage = 0; int nPage = 0; int nPageCount = 200; int nPlayIndex = -1; int nPlayingIndex = -1;
        d(const MusicType eType) { this->eType = eType; }
        d() {}
    };
    static MusicType esType = MusicType_NONE;
    static MusicType eTypeMode = MusicType_NONE;
    static int nIgnModeOL = 0;
    static std::map<MusicType,struct d> m = {
        {MusicType_HDD,d(MusicType_HDD)}, {MusicType_USB,d(MusicType_USB)},
        {MusicType_OL,d(MusicType_OL)}, {MusicType_BT,d(MusicType_BT)}
    };

    if ( !MV(esType) ) { LOGDBG("%s_%s_%d esType:%d mv:%d mask:%s err\n", __FILE__, __func__, __LINE__, esType, MV(esType), mask.data()); return; }

    int size = mask.length();

    for (int i=0; i<size; i++)
    {
        if (mask[i] == '0') { continue; }
        if ( !MV(esType) ) { LOGDBG("%s_%s_%d err esType:%d eType:%d mask:%s\n", __FILE__, __func__, __LINE__, esType, eType, mask.data()); break; }

        switch (i)
        {
            case 0:
            {
                LOGDBG("%s_%s_%d eType:%d eTypeMode:%d mask:%s\n", __FILE__, __func__, __LINE__, eType, eTypeMode, mask.data());
                if ((esType != eType) && MV(eType))
                {
                    LOGDBG("%s_%s_%d esType:%d->%d\n", __FILE__, __func__, __LINE__, esType, eType);
                    esType = eType;

                    if (eTypeMode != esType) eTypeMode = MusicType_NONE;
                }

                if (MLV(eType) && MLV(esType) && (eTypeMode == MusicType_NONE || eTypeMode == esType))
                {
                    IP_TO_HU_CMD(IPPlayListCount, (esType==MusicType_OL?13:esType));
                }
            }
                break;
            case 1: // 列表:HU->IP
            {
                int len = list.size();
                LOGDBG("%s_%s_%d:%d _list:%d[%d-%d]\n", __FILE__, __func__, __LINE__, esType, len, (len>0?list[0].nIndex:-1), (len>0?list[len-1].nIndex:-1));

                for (auto j=list.begin(); j!=list.end(); j++)
                {
                    if ((j->sName=="") || (j->nIndex<0)) continue;

                    auto k = m[esType].list.begin();
                    for (; k!=m[esType].list.end(); k++)
                    {
                        if (j->nIndex == k->nIndex) break;
                        if (j->nIndex < k->nIndex)
                        {
                            m[esType].list.insert(k,*j);
                            break;
                        }
                    }

                    if (m[esType].list.empty() || k==m[esType].list.end())
                        m[esType].list.push_back(*j);
                }
                len = m[esType].list.size();
                LOGDBG("%s_%s_%d:%d list_:%d[%d-%d],nReqPage:%d\n", __FILE__, __func__, __LINE__, esType, len, (len>0?m[esType].list[0].nIndex:-1), (len>0?m[esType].list[len-1].nIndex:-1),m[esType].nReqPage);

                if (m[esType].nReqPage > 0)
                {
                    goto update_list;
                }
            }
                break;
            case 2: // 列表总数
            {
                // HMI切换在线音乐源,前两次源变化请求到的是上个音乐源列表,故前两次列表数量不进行任何处理(请求列表)
                if (eTypeMode != MusicType_OL && nIgnModeOL > 0) nIgnModeOL = 0;

                if (eTypeMode == MusicType_OL && (nIgnModeOL--) > 0) {
                    LOGDBG("%s_%s_%d:%d err MusicType_OL nCount:%d,nIgnModeOL:%d\n", __FILE__, __func__, __LINE__, esType, nCount, nIgnModeOL);
                }
                else if (nCount>0 && nCount != m[esType].nCount && esType != MusicType_BT)
                {
                    m[esType].nCount = nCount;
                    m[esType].nPages = (m[esType].nCount/m[esType].nPageCount + (m[esType].nCount%m[esType].nPageCount > 0 ? 1 : 0));
                    LOGDBG("%s_%s_%d:%d nCount:%d,nPageCount:%d,nPages:%d,eTypeMode:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nCount, m[esType].nPageCount, m[esType].nPages, eTypeMode);

                    if (eTypeMode == MusicType_NONE) m[esType].nReqPage = 0;
                    m[esType].nPage = 0;

                    if (m[esType].list.size() > 0) m[esType].list.clear();
                    IP_TO_HU_SRC_START_COUNT(IPMediaInfo, esType, 0, MIN(m[esType].nCount,m[esType].nPageCount));
                }
            }
                break;
            case 3: // HMI通过页码请求列表
            {
                if (nReqPage > 0 && nReqPage!=m[esType].nReqPage && nReqPage<=m[esType].nPages && esType != MusicType_BT)
                {
                    LOGDBG("%s_%s_%d m[%d].nReqPage:%d->%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nReqPage, nReqPage);
                    m[esType].nReqPage = nReqPage;
                }

            update_list:
                if (m[esType].nCount <= 0)
                { 
                    // if (m[esType].nPageCount > 0) IP_TO_HU_SRC_START_COUNT(IPMediaInfo, esType, 0, m[esType].nPageCount);
                    IP_TO_HU_CMD(IPPlayListCount, (esType==MusicType_OL?13:esType));
                }
                else if (i != 3 || nReqPage <= m[esType].nPages)
                {
                    int len = m[esType].list.size();
                    int nStart = MIN((m[esType].nReqPage-1) * m[esType].nPageCount, m[esType].nCount-1);
                    LOGDBG("%s_%s_%d:%d len:%d(index:%d-%d) nReqPage:%d nCount:%d nStart:%d m[%d].nPlayingIndex:%d\n", __FILE__, __func__, __LINE__, esType, len, (len>0?m[esType].list[0].nIndex:-1), (len>0?m[esType].list[len-1].nIndex:-1), m[esType].nReqPage, m[esType].nCount, nStart, esType, m[esType].nPlayingIndex);
                    if ((nStart+m[esType].nPageCount) <= (int)m[esType].list.size() || ((int)m[esType].list.size() == m[esType].nCount)) // MW缓存足够或最后一页,直接返回给HMI
                    {
                        std::vector<std::string> listOut; listOut.clear();
                        for (int k=0; (k<m[esType].nPageCount && (nStart+k)<m[esType].nCount); k++) { listOut.push_back(m[esType].list[nStart+k].sName); }
                        IVIMusicPlayInfo(0, -1, -1, -1, "", "", "", "", 0, 0, 0, true, false, false, false, "000000000001"); // 发送列表消息前,先取消播放界面,并禁止发送播放界面消息

                        if (m[esType].nPlayingIndex >= 0 && listOut.size() > 0)
                        {
                            LOGDBG("%s_%s_%d send2HMI nReqPage:%d nPage:%d m[%d].nPlayingIndex:%d listOut:%d eTypeMode:%d\n", __FILE__, __func__, __LINE__, m[esType].nReqPage, m[esType].nPage, esType, m[esType].nPlayingIndex, listOut.size(), eTypeMode);
                            if (m[esType].nReqPage>0 && m[esType].nReqPage!=m[esType].nPage) m[esType].nPage = m[esType].nReqPage;
                            if (eTypeMode != MusicType_NONE) eTypeMode = MusicType_NONE;
                            HUHmiIPC::UpdateIVIMusicPlayList(ConverMusicType(esType), m[esType].nPlayingIndex, m[esType].nPage, listOut);
                        }
                        else
                        {
                            LOGDBG("%s_%s_%d m[%d].nPlayingIndex:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nPlayingIndex);
                        }
                    }
                    else
                    {
                        if (len > 0) IP_TO_HU_SRC_START_COUNT(IPMediaInfo, esType, m[esType].list[len-1].nIndex+1, MIN(m[esType].nPageCount, m[esType].nCount-len));
                    }
                }
                else
                {
                    LOGDBG("%s_%s_%d error nReqPage request m[%d].nPlayingIndex:%d nReqPage:%d nPages:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nPlayingIndex, nReqPage, m[esType].nPages);
                }
            }
                break;
            case 4: // 播放歌曲(根据索引)
            {
                if (nPlayIndex != m[esType].nPlayIndex && nPlayIndex>=0 && esType != MusicType_BT)
                {
                    m[esType].nPlayIndex = nPlayIndex;
                    LOGDBG("%s_%s_%d m[%d].nPlayIndex:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nPlayIndex);
                }
                if (m[esType].nPlayIndex >= 0 && m[esType].nPlayIndex<m[esType].nCount)
                {
                    for (auto itor=m[esType].list.begin(); itor!=m[esType].list.end(); itor++)
                    {
                        if (m[esType].nPlayIndex == (int)itor->nIndex)
                        {
                            LOGDBG("%s_%s_%d m[%d].nPlayIndex:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nPlayIndex);
                            HUFData d = { IPMediaPlayback : { index : itor->nIndex, type: itor->nType, id: itor->nId } };
                            memcpy(d.IPMediaPlayback.title, itor->title, sizeof(d.IPMediaPlayback.title)/sizeof(d.IPMediaPlayback.title[0]));
                            memcpy(d.IPMediaPlayback.album, itor->album, sizeof(d.IPMediaPlayback.album)/sizeof(d.IPMediaPlayback.album[0]));
                            memcpy(d.IPMediaPlayback.artist, itor->artist, sizeof(d.IPMediaPlayback.artist)/sizeof(d.IPMediaPlayback.artist[0]));
                            memcpy(d.IPMediaPlayback.name, itor->name, sizeof(d.IPMediaPlayback.name)/sizeof(d.IPMediaPlayback.name[0]));
                            std::string f = mFrameEncoder(CHUFrame::FID_IPMediaPlayback, &d, sizeof(d.IPMediaPlayback));
                            IP_TO_HU_PRINT(IPMediaPlayback, f);
                            mDevMsg.send(f);
                            break;
                        }
                    }
                }
            }
                break;
            case 5: // 当前播放歌曲索引
            {
                if (nPlayingIndex >= 0 && m[esType].nPlayingIndex != nPlayingIndex && esType != MusicType_BT)
                {
                    m[esType].nPlayingIndex = nPlayingIndex;
                    LOGDBG("%s_%s_%d m[%d].nPlayingIndex:%d\n", __FILE__, __func__, __LINE__, esType, m[esType].nPlayingIndex);
                }
            }
                break;
            case 6: // HMI切换音源
            {
                LOGDBG("%s_%s_%d eType:%d,esType:%d,bMode:%d\n", __FILE__, __func__, __LINE__, eType, esType, bMode);
                if (eType == esType) {
                    LOGDBG("%s_%s_%d eType==esType==%d\n", __FILE__, __func__, __LINE__, eType);
                    m[eType].nReqPage = 1;
                    eTypeMode = eType;
                    goto update_list;
                }
                else if (bMode && MLV(eType))
                {
                    LOGDBG("%s_%s_%d mode eType:%d\n", __FILE__, __func__, __LINE__, eType);
                    m[eType].nCount = 0;
                    m[eType].nPages = 0;
                    m[eType].nReqPage = 1;
                    m[eType].nPage = 0;
                    eTypeMode = eType;
                    if (m[eType].list.size() > 0) m[eType].list.clear();
                    if (eType == MusicType_OL) nIgnModeOL = 2;
                }
            }
                break;
            default:
                break;
        }
    }
}

void CHUMsgService::IVIContactList(int nCount, std::vector<ContactCell> &list, int nReqPage, int nReqCallIndex, CallHistory &chReq, int nBtSts, std::string mask)
{
    static struct {
        int nCount = 0;
        int nPages = 0;
        int nReqPage = 1;
        int nPage = 0; // 同步给HMI的页码,有效页码从1开始
        int nPageCount = 100; // 每页数量
        std::vector<ContactCell> list;
        int nBtSts = -1;
    } dd;

    int size = mask.length();

    for (int i=0; i<size; i++)
    {
        if (mask[i] == '0') continue;
        // 蓝牙未连接前,不处理任何通讯录列表消息
        if (dd.nBtSts!=2 && i!=5) { LOGDBG("%s_%s_%d nBtSts:%d,mask:%s\n", __FILE__, __func__, __LINE__, dd.nBtSts, mask.data()); continue; }

        switch (i)
        {
            case 0: // 通讯录联系人数量
            {
                if (nCount > 0 && nCount != dd.nCount)
                {
                    LOGDBG("%s_%s_%d nCount:%d->%d\n", __FILE__, __func__, __LINE__, dd.nCount, nCount);
                    dd.nCount = nCount;
                    dd.nPages = (dd.nCount/dd.nPageCount + (dd.nCount%dd.nPageCount > 0 ? 1 : 0));
                }
                if (dd.nCount > 0)
                {
                    if (dd.list.size() > 0)
                    {
                        dd.list.clear();

                        // 先清空HMI
                        std::vector<std::string> listOut; listOut.clear();
                        HUHmiIPC::UpdateIVIContactList(true, 0, listOut);
                    }
                    dd.nReqPage = 1;
                    dd.nPage = 0;

                    LOGDBG("%s_%s_%d mw->hu request contact list[0->%d]\n", __FILE__, __func__, __LINE__, MIN(dd.nCount,dd.nPageCount));
                    IP_TO_HU_START_COUNT(IPContactInfo, 0, MIN(dd.nCount,dd.nPageCount));
                }
            }
                break;
            case 1:  // 通讯录联系人列表
            {
                const bool bAuto = (dd.list.size() <= 0); // 第一页主动发送给HMI
                int nAddCount = 0;

                // 加入通讯录列表(按照index递增排序)
                for (auto j=list.begin(); j!=list.end(); j++)
                {
                    if (j->index < 0) continue;

                    auto k = dd.list.begin();
                    for (; k!=dd.list.end(); k++)
                    {
                        if (j->index == k->index) break;
                        if (j->index < k->index)
                        {
                            dd.list.insert(k,*j);
                            break;
                        }
                    }

                    if (dd.list.empty() || k==dd.list.end())
                    {
                        nAddCount++;
                        dd.list.push_back(*j);
                    }
                }

                LOGDBG("%s_%s_%d nAddCount:%d,bAuto:%d,nReqPage:%d,nPage:%d\n", __FILE__, __func__, __LINE__, nAddCount, bAuto, dd.nReqPage, dd.nPage);

                if (bAuto || dd.nReqPage!=dd.nPage || dd.nPage==0) {
                    goto update_contact_list;
                }
            }
                break;
            case 2: // hmi通过页码请求通讯录
            {
                LOGDBG("%s_%s_%d dd.nReqPage:%d,nReqPage:%d,dd.nCount:%d,dd.nPageCount:%d,nPages:%d\n", __FILE__, __func__, __LINE__, dd.nReqPage, nReqPage, dd.nCount, dd.nPageCount, dd.nPages);
                if (nReqPage>0 && nReqPage!=dd.nReqPage && nReqPage<=dd.nPages)
                {
                    LOGDBG("%s_%s_%d dd.nReqPage:%d,nReqPage:%d\n", __FILE__, __func__, __LINE__, dd.nReqPage, nReqPage);
                    dd.nReqPage = nReqPage;

update_contact_list:
                    if (dd.nCount <= 0)
                    {
                        if (dd.nPageCount > 0) IP_TO_HU_CMD(IPContactCount,0);
                    }
                    else
                    {
                        int len = dd.list.size();
                        int nStart = MIN((dd.nReqPage-1)*dd.nPageCount,dd.nCount-1);

                        LOGDBG("%s_%s_%d len:%d(index:%d-%d) dd.nReqPage:%d nCount:%d nStart:%d\n", __FILE__, __func__, __LINE__, len, (len>0?dd.list[0].index:-1), (len>0?dd.list[len-1].index:-1), dd.nReqPage, dd.nCount, nStart);
                        if ((nStart+dd.nPageCount)<=(int)dd.list.size() || ((int)dd.list.size()==dd.nCount)) // MW缓存足够或最后一页,直接返回给HMI
                        {
                            std::vector<std::string> listOut; listOut.clear();
                            for (int k=0; (k<dd.nPageCount&&(nStart+k)<dd.nCount); k++) { listOut.push_back((char *)dd.list[nStart+k].name); }

                            if (listOut.size() > 0)
                            {
                                LOGDBG("%s_%s_%d send2HMI nReqPage:%d,nPage:%d listOut:%d\n", __FILE__, __func__, __LINE__, dd.nReqPage, dd.nPage, listOut.size());
                                if (dd.nReqPage>0 && dd.nReqPage!=dd.nPage) dd.nPage = dd.nReqPage;
                                HUHmiIPC::UpdateIVIContactList(true, dd.nPage, listOut);
                            }
                        }
                        else
                        {
                            LOGDBG("%s_%s_%d mw->hu request contact list[%d->%d]\n", __FILE__, __func__, __LINE__, dd.list[len-1].index+1, MIN(dd.nPageCount,dd.nCount-(int)dd.list.size()));
                            IP_TO_HU_START_COUNT(IPContactInfo, dd.list[len-1].index+1, MIN(dd.nPageCount,(dd.nCount-(int)dd.list.size())));
                        }
                    }
                }
            }
                break;
            case 3: // 拨打电话(通讯录)
            {
                // HMI发偏移量,MW计算索引值
                // const int nPageStartIndex = (dd.nPage-1)*dd.nPageCount;
                // if (nReqCallIndex >= 0 && dd.nPage>0 && dd.nPageCount>0 && nPageStartIndex>0 && nPageStartIndex<dd.nCount)
                // {
                //     LOGDBG("%s_%s_%d nReqCallIndex:%d nPage:%d nPageCount:%d nCount:%d nPageStartIndex:%d\n", __FILE__, __func__, __LINE__, nReqCallIndex, dd.nPage, dd.nPageCount, dd.nCount, nPageStartIndex);

                //     if ((nReqCallIndex+nPageStartIndex) < (int)dd.list.size() && dd.list[nReqCallIndex+nPageStartIndex].nums.size() > 0)
                //     {
                //         HUFData d = { IPPhoneCall : { index : dd.list[nReqCallIndex+nPageStartIndex].index } };
                //         memcpy(d.IPPhoneCall.name, dd.list[nReqCallIndex+nPageStartIndex].name, sizeof(d.IPPhoneCall.name)/sizeof(d.IPPhoneCall.name[0]));
                //         strcpy((char *)d.IPPhoneCall.number, dd.list[nReqCallIndex+nPageStartIndex].nums[0].c_str());
                //         std::string f = mFrameEncoder(CHUFrame::FID_IPPhoneCall, &d, sizeof(d.IPPhoneCall));
                //         mDevMsg.send(f);
                //         LOGDBG("%s_%s_%d nReqCallIndex:%d %s %s\n", __FILE__, __func__, __LINE__, nReqCallIndex, dd.list[nReqCallIndex+nPageStartIndex].name, dd.list[nReqCallIndex+nPageStartIndex].nums[0].c_str());
                //     }
                // }

                // HMI计算最终的索引值
                if (nReqCallIndex>=0 && nReqCallIndex<(int)dd.list.size() && dd.list[nReqCallIndex].nums.size() > 0)
                {
                    HUFData d = { IPPhoneCall : { index : dd.list[nReqCallIndex].index } };
                    memcpy(d.IPPhoneCall.name, dd.list[nReqCallIndex].name, sizeof(d.IPPhoneCall.name)/sizeof(d.IPPhoneCall.name[0]));
                    strcpy((char *)d.IPPhoneCall.number, dd.list[nReqCallIndex].nums[0].c_str());
                    std::string f = mFrameEncoder(CHUFrame::FID_IPPhoneCall, &d, sizeof(d.IPPhoneCall));
                    IP_TO_HU_PRINT(IPPhoneCall, f);
                    mDevMsg.send(f);
                    LOGDBG("%s_%s_%d nReqCallIndex:%d %s %s\n", __FILE__, __func__, __LINE__, nReqCallIndex, dd.list[nReqCallIndex].name, dd.list[nReqCallIndex].nums[0].c_str());
                }
            }
                break;
            case 4: // 拨打电话(通话记录)
            {
                const std::string sName = std::string((char *)chReq.name);
                bool bBreak = false;
                if (sName != "" && chReq.number != "")
                {
                    for (auto itor=dd.list.begin(); itor!=dd.list.end(); itor++)
                    {
                        const std::string s = std::string((char *)itor->name);
                        if (s != sName) continue;

                        for (auto j=itor->nums.begin(); j!=itor->nums.end(); j++)
                        {
                            if (chReq.number != *j) continue;
                            LOGDBG("%s_%s_%d index:%d name:%s,number:%s\n", __FILE__, __func__, __LINE__, itor->index, sName.data(), (*j).data());

                            CallHistory c;
                            IVIContactList(0, m_vecContactNull, 0, itor->index, c, -1, "0001");
                            bBreak = true;
                            break;
                        }

                        if (bBreak) break;
                    }
                }
            }
                break;
            case 5: // 蓝牙状态
            {
                if (nBtSts != dd.nBtSts)
                {
                    LOGDBG("%s_%s_%d nBtSts:%d->%d\n", __FILE__, __func__, __LINE__, dd.nBtSts, nBtSts);
                    switch (nBtSts)
                    {
                        case 0: // 蓝牙未连接
                        case 1: // 蓝牙已连接未同步列表
                        {
                            if (dd.list.size() > 0)
                            {
                                std::vector<std::string> listOut; listOut.clear();
                                HUHmiIPC::UpdateIVIContactList(false, 0, listOut);
                            }

                            dd.nCount = 0;
                            dd.list.clear();
                            dd.nBtSts = nBtSts;
                        }
                            break;
                        case 2: // 蓝牙已连接已同步列表
                        {
                            dd.nBtSts = nBtSts;

                            if (dd.nCount == 0)
                            {
                                IP_TO_HU_CMD(IPContactCount,0);
                            }
                            else if (dd.list.size() == 0)
                            {
                                IP_TO_HU_START_COUNT(IPContactInfo, 0, MIN(dd.nCount,dd.nPageCount));
                            }
                            else if (dd.nReqPage != dd.nPage)
                            {
                                goto update_contact_list;
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
            }
                break;
            default:
                break;
        }
    }
}

void CHUMsgService::IVICallHistory(int nCount, std::vector<CallHistory> &list, int nReqPage, int nReqCallIndex, int nBtSts, std::string mask)
{
    static struct {
        int nCount = 0;
        int nPages = 0;
        int nReqPage = 1;
        int nPage = 0; // 同步给HMI的页码,有效页码从1开始
        int nPageCount = 100; // 每页数量
        std::vector<CallHistory> list;
        int nBtSts = -1;
    } dd;

    int size = mask.length();

    for (int i=0; i<size; i++)
    {
        if (mask[i] == '0') continue;
        // 蓝牙未连接前,不处理任何通话记录列表消息
        if (dd.nBtSts!=2 && i!=4) { LOGDBG("%s_%s_%d nBtSts:%d,mask:%s\n", __FILE__, __func__, __LINE__, dd.nBtSts, mask.data()); continue; }

        switch (i)
        {
            case 0: // 通话记录数量
            {
                if (nCount > 0 && nCount != dd.nCount)
                {
                    LOGDBG("%s_%s_%d nCount:%d->%d\n", __FILE__, __func__, __LINE__, dd.nCount, nCount);
                    dd.nCount = nCount;
                    dd.nPages = (dd.nCount/dd.nPageCount + (dd.nCount%dd.nPageCount > 0 ? 1 : 0));
                }
                if (dd.nCount > 0)
                {
                    if (dd.list.size() > 0)
                    {
                        dd.list.clear();

                        // 先清空HMI
                        std::vector<StuCallRecord> listOut; listOut.clear();
                        HUHmiIPC::UpdateIVICallRecordList(true, 0, listOut);
                    }
                    dd.nReqPage = 1;
                    dd.nPage = 0;

                    LOGDBG("%s_%s_%d mw->hu request call history list[0->%d]\n", __FILE__, __func__, __LINE__, MIN(dd.nCount,dd.nPageCount));
                    IP_TO_HU_START_COUNT(IPCallHistory, 0, MIN(dd.nCount,dd.nPageCount));
                }
            }
                break;
            case 1:  // 通话记录列表
            {
                const bool bAuto = (dd.list.size() <= 0);
                int nAddCount = 0;

                // 加入通话记录列表(按照index递增排序)
                for (auto j=list.begin(); j!=list.end(); j++)
                {
                    if (j->index < 0) continue;

                    auto k = dd.list.begin();
                    for (; k!=dd.list.end(); k++)
                    {
                        if (j->index == k->index) break;
                        if (j->index < k->index)
                        {
                            dd.list.insert(k,*j);
                            break;
                        }
                    }

                    if (dd.list.empty() || k==dd.list.end())
                    {
                        nAddCount++;
                        dd.list.push_back(*j);
                    }
                }

                LOGDBG("%s_%s_%d nAddCount:%d,bAuto:%d,nReqPage:%d,nPage:%d\n", __FILE__, __func__, __LINE__, nAddCount, bAuto, dd.nReqPage, dd.nPage);

                if (bAuto || dd.nReqPage!=dd.nPage || dd.nPage==0) {
                    goto update_call_history_list;
                }
            }
                break;
            case 2: // hmi通过页码请求通话记录
            {
                LOGDBG("%s_%s_%d dd.nReqPage:%d,nReqPage:%d,dd.nCount:%d,dd.nPageCount:%d,nPages:%d\n", __FILE__, __func__, __LINE__, dd.nReqPage, nReqPage, dd.nCount, dd.nPageCount, dd.nPages);
                if (nReqPage>0 && nReqPage!=dd.nReqPage && nReqPage<=dd.nPages)
                {
                    LOGDBG("%s_%s_%d dd.nReqPage:%d,nReqPage:%d\n", __FILE__, __func__, __LINE__, dd.nReqPage, nReqPage);
                    dd.nReqPage = nReqPage;

update_call_history_list:
                    if (dd.nCount <= 0) { 
                        if (dd.nPageCount > 0) IP_TO_HU_CMD(IPCallContactCount,0);
                    }
                    else
                    {
                        int len = dd.list.size();
                        int nStart = MIN((dd.nReqPage-1)*dd.nPageCount,dd.nCount-1);

                        LOGDBG("%s_%s_%d len:%d(index:%d-%d) dd.nReqPage:%d nCount:%d nStart:%d\n", __FILE__, __func__, __LINE__, len, (len>0?dd.list[0].index:-1), (len>0?dd.list[len-1].index:-1), dd.nReqPage, dd.nCount, nStart);
                        if ((nStart+dd.nPageCount)<=(int)dd.list.size() || ((int)dd.list.size()==dd.nCount)) // MW缓存足够或最后一页,直接返回给HMI
                        {
                            std::vector<StuCallRecord> listOut; listOut.clear();
                            for (int k=0; (k<dd.nPageCount&&(nStart+k)<dd.nCount); k++)
                            {
                                StuCallRecord c;
                                c.type = (int)dd.list[nStart+k].type;
                                c.name = std::string((char *)dd.list[nStart+k].name);
                                c.amount = (int)dd.list[nStart+k].count;
                                listOut.push_back(c);
                            }

                            if (listOut.size() > 0)
                            {
                                LOGDBG("%s_%s_%d send2HMI nReqPage:%d,nPage:%d listOut:%d\n", __FILE__, __func__, __LINE__, dd.nReqPage, dd.nPage, listOut.size());
                                if (dd.nReqPage>0 && dd.nReqPage!=dd.nPage) dd.nPage = dd.nReqPage;
                                HUHmiIPC::UpdateIVICallRecordList(true, dd.nPage, listOut);
                            }
                        }
                        else
                        {
                            LOGDBG("%s_%s_%d mw->hu request call history list[%d->%d]\n", __FILE__, __func__, __LINE__, dd.list[len-1].index+1, MIN(dd.nPageCount,dd.nCount-(int)dd.list.size()));
                            IP_TO_HU_START_COUNT(IPCallHistory, dd.list[len-1].index+1, MIN(dd.nPageCount,dd.nCount-(int)dd.list.size()));
                        }
                    }
                }
            }
                break;
            case 3: // 拨打电话
            {
                if (nReqCallIndex>=0 && nReqCallIndex<(int)dd.list.size())
                {
                    LOGDBG("%s_%s_%d nReqCallIndex:%d,name:%s,number:%s\n", __FILE__, __func__, __LINE__, nReqCallIndex, dd.list[nReqCallIndex].name, dd.list[nReqCallIndex].number.data());
                    // IVIContactList(0, m_vecContactNull, 0, 0, dd.list[nReqCallIndex], -1, "00001");
                    
                    // 通话记录列表直接拨打电话
                    HUFData d = { IPPhoneCall : { index : dd.list[nReqCallIndex].index } };
                    memcpy(d.IPPhoneCall.name, dd.list[nReqCallIndex].name, sizeof(d.IPPhoneCall.name)/sizeof(d.IPPhoneCall.name[0]));
                    strcpy((char *)d.IPPhoneCall.number, dd.list[nReqCallIndex].number.c_str());
                    std::string f = mFrameEncoder(CHUFrame::FID_IPPhoneCall, &d, sizeof(d.IPPhoneCall));
                    IP_TO_HU_PRINT(IPPhoneCall, f);
                    mDevMsg.send(f);
                }
            }
                break;
            case 4: // 蓝牙状态
            {
                if (nBtSts != dd.nBtSts)
                {
                    LOGDBG("%s_%s_%d nBtSts:%d->%d\n", __FILE__, __func__, __LINE__, dd.nBtSts, nBtSts);
                    switch (nBtSts)
                    {
                        case 0: // 蓝牙未连接
                        case 1: // 蓝牙已连接未同步列表
                        {
                            if (dd.list.size() > 0)
                            {
                                std::vector<StuCallRecord> listOut; listOut.clear();
                                HUHmiIPC::UpdateIVICallRecordList(false, 0, listOut);
                            }

                            dd.nCount = 0;
                            dd.list.clear();
                            dd.nBtSts = nBtSts;
                        }
                            break;
                        case 2: // 蓝牙已连接已同步列表
                        {
                            dd.nBtSts = nBtSts;

                            if (dd.nCount == 0)
                            {
                                IP_TO_HU_CMD(IPCallContactCount,0);
                            }
                            else if (dd.list.size() == 0)
                            {
                                IP_TO_HU_START_COUNT(IPCallHistory, 0, MIN(dd.nCount,dd.nPageCount));
                            }
                            else if (dd.nReqPage != dd.nPage)
                            {
                                goto update_call_history_list;
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
            }
                break;
            default:
                break;
        }
    }
}

void CHUMsgService::SplitHourMinute(const std::string &s, int &h, int &m)
{
    // 从"hh小时mm分钟"中分离出hh和mm。例如，s="60小时50分钟"，结果h=60,m=50

    int nH = s.find("小时");
    int nM = s.find("分钟");

    if (nH <= 0 || nM <= 0) return;

    std::string sH = s.substr(0,nH);
    std::string sM = s.substr(nH + std::string("小时").length(), nM-nH-std::string("小时").length());

    int len = sH.length();
    int nRst = 0;
    int nCount = 1;

    for (int i=len-1; i>=0; i--)
    {
        if ( NUM(sH[i]) ) { nRst += ((sH[i]-0x30) * nCount); nCount *= 10; }
        else { nRst = 0; break; }
    }
    if (nRst > 0) h = nRst;

    len = sM.length();
    nRst = 0;
    nCount = 1;

    for (int i=len-1; i>=0; i--)
    {
        if ( NUM(sM[i]) ) { nRst += ((sM[i]-0x30) * nCount); nCount *= 10; }
        else { nRst = 0; break; }
    }
    if (nRst > 0) m = nRst;
}

void CHUMsgService::SplitMile(const std::string &s,int &m)
{
    // 从xxxm或xxxkm或xxx.yykm中分离出xxx。例如，s="1000m"，结果m=1000

    if (s.find("m") <= 0) { LOGINF("return s:%s",s.data()); return; }

    int nNumInt = 0, nNumDec = 0, nCount = 1;
    bool bDec = false;
    for (int i=s.length()-1; i>=0; i--) {
        if (s[i] == 0x6D || s[i] == 0x6B) continue; // 0x6D:m 0x6B:k
        if (s[i] == 0x2E) // .
        {
            bDec = true;
            nNumDec = nNumInt;
            nCount = 1;
            nNumInt = 0;
            continue;
        }

        if (s[i] >= 0x30 && s[i] <= 0x39) {
            nNumInt += ((s[i]-0x30) * nCount);
            nCount *= 10;
        }
        else
        {
            nNumInt = 0;
            nNumDec = 0;
            break;
        }
    }

    if (nNumInt > 0 || nNumDec > 0)
    {
        if (bDec) m = (nNumInt*1000 + nNumDec);
        else m = nNumInt;
    }
}

// HMI->MW(IP)->HU(IVI)：请求列表信息
void CHUMsgService::UpdateIVIInfoList(HUHmiIPC::IVIList eList, int nPage)
{
    LOGDBG("%s_%s line:%d eList::%d nPage:%d\n", __FILE__, __func__, __LINE__, eList, nPage);

    switch (eList)
    {
        case HUHmiIPC::IVI_Music_List:
        {
            std::vector<MusicCell> list;
            IVIMusicList(MusicType_NONE, list, 0, nPage, 0, 0, false, "0001");
        }
            break;
        case HUHmiIPC::IVI_Radio_LIst:
        {
            // IP_TO_HU_START_COUNT(IPRaidoChannelInfo,0,10);
            IVIRadioList(RadioType_NONE, m_vecRadioNull, 0, 0, 100, 0, 0, 0, false, "00011");
        }
            break;
        default:
            break;
    }
}

void CHUMsgService::UpdateIVIInfoPlayIndex(int index)
{
    LOGDBG("%s_%s_%d index:%d\n", __FILE__, __func__, __LINE__, index);
    std::vector<MusicCell> list;
    IVIMusicList(MusicType_NONE, list, 0, 0, index, 0, false, "00001");
    IVIMusicPlayInfo(2, -1, -1, -1, "", "", "", "", 0, 0, 0, false, false, false, false, "1");
    IVIRadioPlayInfo(2, -1, 0, 0, "", false, "1");
}

void CHUMsgService::UpdateIVIBtList(HUHmiIPC::BtList eListType, int nPage)
{
    LOGDBG("%s_%s_%d eListType:%d,nPage:%d\n", __FILE__, __func__, __LINE__, eListType, nPage);

    if (nPage > 0)
    {
        switch (eListType)
        {
            case HUHmiIPC::BtList_Contact:
            {
                CallHistory c;
                IVIContactList(0,m_vecContactNull,nPage,0,c,-1,"001");
            }
                break;
            case HUHmiIPC::BtList_CallHistory:
            {
                IVICallHistory(0,m_vecCallHNull,nPage,0,-1,"001");
            }
                break;
            default:
                break;
        }
    }
}

void CHUMsgService::UpdateIVIBtPhoneControl(HUHmiIPC::BTPhonePage ePage, HUHmiIPC::BTPhoneButton eCmd, HUHmiIPC::BTPhoneControl eV)
{
    LOGDBG("%s_%s line:%d ePage::%d, eCmd:%d, eV:%d\n", __FILE__, __func__, __LINE__, ePage, eCmd, eV);

    if (eCmd == HUHmiIPC::BTPhone_Answer && ePage == HUHmiIPC::BTPhone_Incoming) // 接听: 来电
    {
        // 1、接听 CALL_STATE_INCOMING or 接听 CALL_STATE_WAITING 状态的电话, 并保持当前的通话
        // 2、接听 CALL_STATE_WAITING 状态的电话, 并挂断当前的通话
        IP_TO_HU_CMD(IPPhoneAnswer,1);
    }
    else if (eCmd == HUHmiIPC::BTPhone_CallUp && (ePage == HUHmiIPC::BTPhone_Incoming || ePage == HUHmiIPC::BTPhone_Dial || ePage == HUHmiIPC::BTPhone_InCalling)) // 挂断: 来电;去电;通话中
    {
        // 1：挂断当前处于 CALL_STATE_ACTIVE 状态的电话，当有来电处于 CALL_STATE_WAITING 状态时,与pickUpWaitingCallAndHangUpCurrentCall()具有相同的功能
        // 2：挂断处于 CALL_STATE_INCOMING 状态的来电
        // 3：挂断处于 CALL_STATE_WAITING 
        int n = 0;
        if (HUHmiIPC::BTPhone_InCalling == ePage) n = 1;
        else if (HUHmiIPC::BTPhone_Incoming == ePage) n = 2;
        else if (HUHmiIPC::BTPhone_Dial == ePage) n = 3;

        if (n > 0) IP_TO_HU_CMD(IPPhoneHangup, n);
    }
    else if (eCmd == HUHmiIPC::BTPhone_Sound && (ePage == HUHmiIPC::BTPhone_InCalling)) // 静音: 通话中
    {
        // 1 静音
        // 2 解除静音
        IP_TO_HU_CMD(IPPhoneMute, (eV == HUHmiIPC::BTPhone_Open?1:2));
    }
    else if (eCmd == HUHmiIPC::BTPhone_Moblie && (ePage == HUHmiIPC::BTPhone_InCalling)) // 车机通话/手机通话: 通话中
    {
        // 0 车机通话
        // 1 手机通话
        IP_TO_HU_CMD(IPPhoneHandfree, eV);
    }
}

void CHUMsgService::UpdateIVIContactIndex(int nIndex)
{
    LOGDBG("%s_%s_%d nIndex::%d\n", __FILE__, __func__, __LINE__, nIndex);

    CallHistory c;
    IVIContactList(0, m_vecContactNull, 0, nIndex, c, -1, "0001");
}

void CHUMsgService::UpdateIVICallRecordIndex(int nIndex)
{
    LOGDBG("%s_%s_%d nIndex::%d\n", __FILE__, __func__, __LINE__, nIndex);
    IVICallHistory(0, m_vecCallHNull, 0, nIndex, -1, "0001");
}

void CHUMsgService::UpdateIVIRadioPlayIndex(int nIndex)
{
    LOGDBG("%s_%s_%d nIndex::%d\n", __FILE__, __func__, __LINE__, nIndex);
    IVIRadioList(RadioType_NONE, m_vecRadioNull, 0, 0, 0, nIndex, 0, 0, false, "000001");
    IVIRadioPlayInfo(1, -1, 0, 0, "", false, "1");
    IVIMusicPlayInfo(2, -1, -1, -1, "", "", "", "", 0, 0, 0, false, false, false, false, "1");
}

void CHUMsgService::UpdateIVIMediaSourceList(int nFromHMI)
{
    const int nToHU = ConverMediaSource(nFromHMI);

    if (nToHU != -1)
    {
        LOGDBG("%s_%s_%d nFromHMI:%d nToHU:%d\n", __FILE__, __func__, __LINE__, nFromHMI, nToHU);

        HUFData d = { IPMediaSource : { sourceType : (uint32_t)(nToHU) } };
        std::string f = mFrameEncoder(CHUFrame::FID_IPMediaSource, &d, sizeof(d.IPMediaSource));
        IP_TO_HU_PRINT(IPMediaSource, f);
        mDevMsg.send(f);

        if (nToHU == MusicType_HDD || nToHU == MusicType_USB || nToHU == MusicType_OL)
        {
            std::vector<MusicCell> list;
            IVIMusicList((MusicType)nToHU, list, 0, 0, 0, 0, true, "0000001");
        }
        else if (nToHU == MusicType_BT)
        {
            IVIMusicPlayInfo(2, -1, -1, -1, "", "", "", "", 0, 0, 0, false, false, false, false, "1");
            IVIRadioPlayInfo(2, -1, 0, 0, "", false, "1");
        }
        else if (nToHU == RadioType_FM || nToHU == RadioType_AM || nToHU == RadioType_OL)
        {
            IVIRadioList((RadioType)nToHU, m_vecRadioNull, 0, 0, 100, 0, 0, 0, true, "000000001");
        }
    }
}

void CHUMsgService::UpdateIVIMediaCLose(bool bClose)
{
    LOGDBG("%s_%s_%d bClose:%d\n", __FILE__, __func__, __LINE__, bClose);

    if (bClose)
    {
        IVIRadioPlayInfo(0, -1, 0, 0, "", false, "1");
        IVIMusicPlayInfo(0, -1, -1, -1, "", "", "", "", 0, 0, 0, false, false, false, false, "1");
    }
}

void CHUMsgService::IVIIPStatus(std::string sVer)
{

    // 0x11 0x01
    HUFData dd = { IPStatus : { count:0, screenMode:0 } };
    memcpy(dd.IPStatus.version, sVer.data(), MIN(sVer.length(), sizeof(dd.IPStatus.version)/sizeof(dd.IPStatus.version[0])));
    std::string ff = mFrameEncoder(CHUFrame::FID_IPStatus, &dd, sizeof(dd.IPStatus));
    mDevMsg.send(ff);
    IP_TO_HU_PRINT(IPStatus,ff);
}

int CHUMsgService::ConverMusicType(MusicType eType)
{
    int nMediaType = -1;

    switch (eType) {
        case MusicType_HDD:
            nMediaType = 0;
            break;
        case MusicType_USB:
            nMediaType = 2;
            break;
        case MusicType_OL:
            nMediaType = 3;
            break;
        case MusicType_BT:
            nMediaType = 1;
            break;
        default:
            break;
    }

    return nMediaType;
}

int CHUMsgService::ConverMediaSource(int nHMI)
{
    int nRet = -1;

    switch (nHMI)
    {
        case 0: // 本地音乐
        {
            nRet = MusicType_HDD;
        }
            break;
        case 1: // 蓝牙音乐
        {
            nRet = MusicType_BT;
        }
            break;
        case 2: // USB音乐
        {
            nRet = MusicType_USB;
        }
            break;
        case 3: // 在线音乐
        {
            nRet = MusicType_OL;
        }
            break;
        case 4: // FM
        {
            nRet = RadioType_FM;
        }
            break;
        case 5: // AM
        {
            nRet = RadioType_AM;
        }
            break;
        case 6: // 在线电台
        {
            nRet = RadioType_OL;
        }
            break;
        default:
            break;
    }

    return nRet;
}