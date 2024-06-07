

#include "AppMain.h"
#include "UpgradeInterface.h"

#include <iostream>
#include <fstream>
#include <fcntl.h>

#include <signal.h>
#include <errno.h>
#include <termio.h>
#include <unistd.h>
#include "DoShellCmd.hpp"
#include <dirent.h>
#include <hmi/MsgInterface.h>
#include "mylogCtrl.h"
#include "LogServiceApi.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

void SigExit(int sig)
{
	printf("===== ERROR SIG_TYPE: %d =====\n", sig);
#ifndef WIN32
	printf("===== ERROR NO: %d, %s =====\n", errno, strerror(errno));
	signal(SIGINT, SIG_IGN);
	kill(0, SIGINT);
#endif
	exit(-1);
}

std::mutex mLogMutex;

int main(int argc, char *argv[])
{

#ifdef SIGQUIT
	signal(SIGQUIT, SigExit);
#endif
	signal(SIGINT, SigExit);
	signal(SIGTERM, SigExit);
#ifdef SIGUP
	signal(SIGHUP, SigExit);
#endif
	signal(SIGSEGV, SigExit);
#ifdef SIGBUS
	signal(SIGBUS, SigExit);
#endif
#ifdef SIGKILL
	signal(SIGKILL, SigExit);
#endif
	signal(SIGFPE, SigExit);

    printf("[%s] build time: %s %s\n", argv[0], __DATE__, __TIME__);
    
    CApp app(argc, argv);
    app.Start(true);
    return EXIT_SUCCESS;
}



CApp::CApp(int argc, char *argv[])
    : oIpc("UpgradeService"), watchAppIpc("WatchService"),logAppIpc("LogService")
{
    strcpy(mARM_verion, "v00.00.00");
    strcpy(mMCU_version, "v0.0.0");
    mCheckOTAFile = false;
    mUpdatePath = updatePath_Last;
    mBurnMCU = nullptr;
    mDispCount = -1;
    mOTAProcessStep = OTA_Step_Init;
    mMsgPopFlag = EmSWState::close;
    mOTACheckPackTimes = 0;
	mVialdVersion = false;
}

CApp::~CApp()
{
    if(mBurnMCU)
    {
        delete mBurnMCU;
    }
}

void CApp::Init()
{
    upgrade::publisher::NotifyMcuIntoUpgradeMode(oIpc);
    upgrade::publisher::upgrade_ResponseIVICtrl(oIpc);
    upgrade::subscriber::upgrade_IVICtrl(oIpc, *this);
    
    upgrade::subscriber::Upgrade_clusterVersion(oIpc, *this);
    HmiIPC::publisher::SystemUpdateMessaage(oIpc);
    oIpc.start();

    WatchIpc::subscriber::LogLevelControl(watchAppIpc, *this);
	WatchIpc::subscriber::HeartbeatSignal(watchAppIpc, *this);

	WatchIpc::publisher::HeartbeatStart(watchAppIpc);
	WatchIpc::publisher::HeartbeatSignal_Resp(watchAppIpc);
	WatchIpc::publisher::HeartbeatStop(watchAppIpc);
	
    watchAppIpc.start();

	ZH::logService::publisher::LogRecord(logAppIpc);
	logAppIpc.start();

	WatchIpc::HeartbeatStart("UpgradeService");
}

void CApp::sendUpgradeInfo2Mcu(uint8_t cmd, uint8_t times)
{
	if(times > 5)
	{
		times = 5;
	}
	do
	{
		upgrade::NotifyMcuIntoUpgradeMode(cmd);
		usleep(100*1000);
	} while (--times);
}
void CApp::DoWork()
{
    sleep(4);//线休眠一段时间
    //先将上次升级的文件删除
    do
    {
        char cmdRmFile[128];
        snprintf(cmdRmFile, sizeof(cmdRmFile), "rm -rf %s* && sync", OTA_PATH);
        system(cmdRmFile);
    } while (0);
#if 0
    MCUUprgadeFlag flag = getMCUUpgradeFlag();
    if (flag == MCU_UFLAG_UPGRADEING)
    {
        if (mVialdVersion)
        {
            setMCUUpgradeFlag(MCU_UFLAG_DONE);
        }
        else
        {
            // 重新升级MCU
            char dstFile[128];
            char srcFile[128];
            snprintf(dstFile, sizeof(dstFile), "%s%s", OTA_PATH, MCU_FILE_NAME);
            snprintf(srcFile, sizeof(srcFile), "%s%s", cMCUBakFilePath.data(), MCU_FILE_NAME);
            bool copyRet = this->copyFile(dstFile, srcFile);
            if(copyRet)
            {
                mCheckOTAFile = true;
            }
        }
    }
#endif
    sendSystemUpdateMessaage(EmSWState::close, "", "", -1, __LINE__);
    mOTACtrlValue = 1;
    CDoShellCmd doShell;
    while (true)
    {
        if((mOTACtrlValue == 1) && (OTA_Step_Init != mOTAProcessStep))
        {
            if(OTA_Step_CheckMountPoint == mOTAProcessStep)
            {
                sendSystemUpdateMessaage(EmSWState::open, "仪表升级中", "等待中控传输文件...", -1, __LINE__);
                mOTAProcessStep = OTA_Step_CheckMountPointComplete;
                mOTACheckPackTimes = 0;
                mOTAWaitPackTimes = 0;
            }
            else
            {
                if(++mOTAWaitPackTimes >= 15*60)
                {
                    mOTAWaitPackTimes = 0;
                    mOTAProcessStep = OTA_Step_Init;
                    sendSystemUpdateMessaage(EmSWState::open, "文件传输超时", "请检查usb线连接", -1, __LINE__);
                    upgrade::upgrade_ResponseIVICtrl(4);//反馈给中控升级失败
					LOG_RECORD("文件传输超时,请检查usb线连接");
                    sleep(3);
                    sendSystemUpdateMessaage(EmSWState::close, "", "", -1, __LINE__);
                }
            }    
        }
        else if((mOTACtrlValue == 2) && (mOTAProcessStep == OTA_Step_CheckPack))
        {//OTA包处理过程
            sendSystemUpdateMessaage(EmSWState::open, "仪表升级中", "传输完成", -1, __LINE__);
            std::string fName = findOTAArchiveFromOTADir(OTA_PATH);
            LOG_SDEBUG("%s,%d fName = %s, mCheckOTAFile = %d\n", __func__, __LINE__, fName.c_str(), mCheckOTAFile);
            if(fName != "")
            {//解压ota升级包
                sendUpgradeInfo2Mcu(0x07);//正在解压升级包，请求关闭心跳包检查
                mOTAProcessStep = OTA_Step_Decompression;
                sendSystemUpdateMessaage(EmSWState::open, "仪表升级中", "开始解压升级包", -1, __LINE__);
                char cmd[128];
                if(std::string::npos != fName.find(cOTA_ArchiveSuffix[0]))
                {//tar包
                    snprintf(cmd, sizeof(cmd), "tar -zxf '%s%s' -C %s && sync", OTA_PATH, fName.c_str(), OTA_PATH);
                }
                else
                {//解压zip包
                    snprintf(cmd, sizeof(cmd), "unzip -qo '%s%s' -d %s && sync", OTA_PATH, fName.c_str(), OTA_PATH);
                }
                doShell(cmd);
                mOTAProcessStep = OTA_Step_DecompressionComplete;
                char otaFile[128];
                snprintf(otaFile, sizeof(otaFile), "%s%s", OTA_PATH, OS_FILE_NAME);
                bool bHaveFile = false;
                if(0 == access(otaFile, F_OK))
                {
                    bHaveFile = true;
					LOG_RECORD("find arm file");
                    #if 0
                    if(access("/opt/data/update/", F_OK) == -1)
                    {
                        char cmdStr[128];
                        snprintf(cmdStr, sizeof(cmdStr), "cp -rf %s /opt/data/updateTemp && sync", otaFile);
                        doShell(cmdStr);
                        snprintf(cmdStr, sizeof(cmdStr), "mv /opt/data/updateTemp /opt/data/update && sync");
                        doShell(cmdStr);
                    }
                    #endif
                }
                snprintf(otaFile, sizeof(otaFile), "%s%s", OTA_PATH, MCU_FILE_NAME);
                if(0 == access(otaFile, F_OK))
                {
                    bHaveFile = true;
					LOG_RECORD("find mcu file");
					char dstFile[128];
					snprintf(dstFile, sizeof(dstFile), "%s%s", cMCUBakFilePath.data(), MCU_FILE_NAME);
					this->copyFile(dstFile, otaFile);
                }
                if(!bHaveFile)
                {
                    //升级失败
                    upgrade::upgrade_ResponseIVICtrl(4);//反馈给中控升级失败
                    //remove(std::string(OTA_PATH + fName).c_str());
                    sendSystemUpdateMessaage(EmSWState::open, "仪表升级失败", "未找到升级包", -1, __LINE__);
                    sendUpgradeInfo2Mcu(0x08);
					LOG_RECORD("升级失败,未找到升级包");
                }
                else
                {
                    mCheckOTAFile = true;
                    upgrade::upgrade_ResponseIVICtrl(3);//反馈给中控切换u盘
                    LOG_SDEBUG("%s,%d fName = %s, mCheckOTAFile = %d\n", __func__, __LINE__, fName.c_str(), mCheckOTAFile);
                    sendSystemUpdateMessaage(EmSWState::open, "仪表升级中", "解压成功，开始校验", -1, __LINE__);
                }
            }
            else
            {
                //没找到ota包，下一个循环继续找
                if(++mOTACheckPackTimes >= 10)
                {//找十个周期（10秒）还没找到就不找了。
                    mOTACheckPackTimes = 0;
                    mOTAProcessStep = OTA_Step_CheckPackTimeOut;
                }
            }
        }
        else
        {
            if(mOTAProcessStep != OTA_Step_Init)
            {
                mOTAProcessStep = OTA_Step_Init;
                sendSystemUpdateMessaage(EmSWState::close, "", "", -1, __LINE__);
            }
        }
        CApp::updateType type = isHaveUpgradePackage();
        if(type == CApp::updateType_None)
        {
            if(mDispCount == 0xff)
            {
                if(mBurnMCU)
                {
                    delete mBurnMCU;
					mBurnMCU = nullptr;
                }
				sendUpgradeInfo2Mcu(0x08);
            }
            if((-1 != mDispCount) && (mMsgPopFlag == EmSWState::open))
            {
                sendSystemUpdateMessaage(EmSWState::close, "", "", 0, __LINE__);
                
            }
            mDispCount = -1;
        }
        
        do
        {
            if(CApp::updateType_OS == (int(type) & CApp::updateType_OS))
            {//升级os
                if(mUpdatePath >= updatePath_Last)
                {
                    break;
                }
                LOG_SDEBUG("%s,%d, mDispCount = %d\n", __func__,__LINE__, mDispCount);
                if(mDispCount == -1)
                {
					sendUpgradeInfo2Mcu(0x07);//正在升级校验，请求关闭心跳包检查
                    mDispCount = 10;
                    sendSystemUpdateMessaage(EmSWState::open, "ARM升级处理中", "找到arm升级包,开始校验", -1, __LINE__);
                    bool checkProject = checkProjectFile();
					if(!checkProject)
					{
						LOG_RECORD("project check failed!!!");
                        sendSystemUpdateMessaage(EmSWState::open, "ARM升级处理中", "ARM升级包防呆验证失败", -1, __LINE__);
                        sleep(3);
                        sendSystemUpdateMessaage(EmSWState::close, "", "", 0, __LINE__);
                        mDispCount = -1;
					}
					else
					{
						//校验md5
						bool md5CheckResult = checkOsMd5();
						LOG_SDEBUG("%s,%d, md5CheckResult = %d\n", __func__,__LINE__, md5CheckResult);
						if(md5CheckResult)
						{
							//sendSystemUpdateMessaage(EmSWState::open, "os", 0, 0, std::string("null"), __LINE__);
						}
						else
						{
							LOG_RECORD("MD5 check failed!!!");
							sendSystemUpdateMessaage(EmSWState::open, "ARM升级处理中", "ARM升级包校验失败", -1, __LINE__);
							sleep(3);
							sendSystemUpdateMessaage(EmSWState::close, "", "", 0, __LINE__);
							mDispCount = -1;
							LOG_SDEBUG("%s,%d, md5CheckResult = %d, mDispCount = %d\n", __func__,__LINE__, md5CheckResult, mDispCount);
						}
					}
					sendUpgradeInfo2Mcu(0x08);//升级校验完成，请求恢复心跳包检查
                }
                else if(0 == mDispCount)
                {
                    LOG_SDEBUG("%s,%d, type = %d\n", __func__,__LINE__, type);
                    #if 1
                    doShell("tc-write-misc && sync");//发送升级指令
                    #else
                    system("tc-write-misc");
                    sync();
                    sleep(1);
                    #endif
                    mDispCount = 0xff;
                    if(CApp::updateType_MCU == (int(type) & CApp::updateType_MCU))
                    {//如果要升级mcu，退出
                        upgrade::NotifyMcuIntoUpgradeMode(0XA0);
                        break;
                    }
                    else
                    {//开始升级，通知mcu重启
                        LOG_SDEBUG("%s,%d, NotifyMcuIntoUpgradeMode(0X06)\n", __func__,__LINE__);
                        sendUpgradeInfo2Mcu(0X06);
                    }
                }
				else if(0xff == mDispCount)
				{
					if(CApp::updateType_MCU != (int(type) & CApp::updateType_MCU))
					{
						sendUpgradeInfo2Mcu(0X06);
					}
				}
            }
        } while (0);
        do
        {
            if(CApp::updateType_MCU == (int(type) & CApp::updateType_MCU))
            {//升级mcu

                if(mDispCount == -1)
                {
                    mDispCount = 10;
                }
                else if(0 == mDispCount)
                {
                    mDispCount = 0xff;
                    const int REQUEST_MCU_UPGRADE = 0xA0;
                    upgrade::NotifyMcuIntoUpgradeMode(REQUEST_MCU_UPGRADE);
                }
                else if(0xff == mDispCount)
                {
                    // Set flag of start mcu upgrade
                    setMCUUpgradeFlag(MCU_UFLAG_UPGRADEING);
					
                    if(!mBurnMCU)
                    {
                        char mcuFullPath[128];
                        snprintf(mcuFullPath, sizeof(mcuFullPath), "%s%s", cUpdateFileInfoList[mUpdatePath].filePath, MCU_FILE_NAME);
                        const char* newPath = "/tmp/zh_arm_mcu.bin";
						bool copyResult = true;
						copyResult = this->copyFile(newPath, mcuFullPath);
                        if(copyResult)
                        {
                            mBurnMCU = new burnMCU(newPath);
                            mBurnMCU->registerDoneCB([&](void) { CApp::setMCUUpgradeFlag(CApp::MCU_UFLAG_DONE); });
                        }
                    }
                    else
                    {
                        int progress = mBurnMCU->getProgressBarValue();

						static std::string sErrStr = "";
						std::string errorStr = mBurnMCU->getErrorInfo();
						if(errorStr != sErrStr)
						{
							sErrStr = errorStr;
							if(errorStr != "")
							{
								LOG_RECORD("trans_mcu_err:%s,progress:%d", errorStr.data(), progress);
							}
						}
                        sendSystemUpdateMessaage(EmSWState::open, "MCU升级中", errorStr, progress, __LINE__);
                    }
                }
                LOG_SDEBUG("%s,%d, type = %d, mDispCount = %d\n", __func__,__LINE__, type, mDispCount);
            }
        } while (0);
        if((mDispCount > 0) && (mDispCount <= 10))
        {
            --mDispCount;
            char countdownStr[64];
            snprintf(countdownStr, sizeof(countdownStr), "倒计时%d秒", mDispCount);
            if(CApp::updateType_OS == type)
            {
                sendSystemUpdateMessaage(EmSWState::open, "ARM准备升级", countdownStr, -1, __LINE__);
            }
            else if(CApp::updateType_MCU == type)
            {
                sendSystemUpdateMessaage(EmSWState::open, "MCU准备升级", countdownStr, -1, __LINE__);
            }
            else if(CApp::updateType_All == type)
            {
                sendSystemUpdateMessaage(EmSWState::open, "ARM & MCU准备升级", countdownStr, -1, __LINE__);
            }
            else
            {
                mDispCount = -1;
                sendSystemUpdateMessaage(EmSWState::close, "", "", 0, __LINE__);
            }
        }
        usleep(1*1000*1000);
    }
}

void CApp::sendSystemUpdateMessaage(EmSWState sts, std::string title, std::string content, int progress, int debugID)
{
    LOG_SDEBUG("%s, debugID = %d, sts = %d, title = %s, content = %s, progress = %d\n", __func__, debugID, sts, title.c_str(), content.c_str(), progress);
    mMsgPopFlag = sts;
    HmiIPC::SystemUpdateMessaage(sts, title, content, progress);
}

void CApp::UInit()
{
    
}

bool CApp::copyFile(const char* dstFile, const char* srcFile)
{
	char buf[256];
	bool copyResult = true;
	if(access(srcFile, R_OK) == -1)
	{
		return false;
	}
	int srcFileFD = open(srcFile, O_RDONLY);
    int copyFileFD = open(dstFile, O_CREAT | O_WRONLY);
	if((srcFileFD <= 0)  || (copyFileFD <= 0))
	{
		LOG_SDEBUG("%s, srcFileFD = %d, copyFileFD = %d\n", __func__, srcFileFD, copyFileFD);
		return false;
	}
	LOG_SDEBUG("######### start copy\n");
	while (true)
	{
		int size = read(srcFileFD, buf, sizeof(buf));
		if(size > 0)
		{
			write(copyFileFD, buf, size);
		}
		else 
		{
			if(size < 0)
			{
				copyResult = false;
			}
			break;
		}
	}
	close(srcFileFD);
	fsync(copyFileFD);
	close(copyFileFD);
	LOG_SDEBUG("####### end copy\n");
	return copyResult;
}

void CApp::LogLevelControl(std::string name, LOG_LEV_EN logLevel)
{
    if(name == "UpgradeService")
    {
        setPrintLevel(logLevel);
    }
}
void CApp::HeartbeatSignal(std::string name, int count)
{
	if(name == "UpgradeService")
	{
		int countResp = count + 1;
		WatchIpc::HeartbeatSignal_Resp("UpgradeService", countResp);
	}
}

CApp::updateType CApp::isHaveUpgradePackage()
{
    int retType = updateType::updateType_None;
    for(auto item : CApp::cUpdateFileInfoList)
    {
        if(!mCheckOTAFile)
        {
            if(item.enUpdatePath == CApp::updatePath_OTA)
            {
                continue;
            }
        }
        std::string checkFile = std::string(item.filePath) + std::string(MCU_FILE_NAME);
        if(0 == access(checkFile.c_str(), F_OK))
        {
            mUpdatePath = item.enUpdatePath;
            retType |= updateType::updateType_MCU;
            checkFile = std::string(item.filePath) + std::string(OS_FILE_NAME);
            if(0 == access(checkFile.c_str(), F_OK))
            {
                retType |= updateType::updateType_OS;
            }
            break;
        }
        //
        checkFile = std::string(item.filePath) + std::string(OS_FILE_NAME);
        if(0 == access(checkFile.c_str(), F_OK))
        {
            mUpdatePath = item.enUpdatePath;
            retType |= updateType::updateType_OS;
            checkFile = std::string(item.filePath) + std::string(MCU_FILE_NAME);
            if(0 == access(checkFile.c_str(), F_OK))
            {
                retType |= updateType::updateType_MCU;
            }
            break;
        }
    }
    //LOG_SDEBUG("%s, retType = %d\n", __func__, retType);
    return static_cast<CApp::updateType>(retType);
}
std::string CApp::findOTAArchiveFromOTADir(const char* dir)
{
    LOG_SDEBUG("%s, dir = %s\n", __func__, dir);
    std::string  arFileName = "";
    DIR *directory_pointer;
    struct dirent *entry;
    directory_pointer=opendir(dir);
    while((entry=readdir(directory_pointer))!=NULL)
    {
        if(entry->d_type & DT_REG) //查找文件
        {
            std::string fileName = std::string(entry->d_name);
            if(fileName.find(CApp::cOTA_ArchivePrefix) != std::string::npos)
            {
                if((fileName.find(cOTA_ArchiveSuffix[0]) != std::string::npos) || 
                    (fileName.find(cOTA_ArchiveSuffix[1]) != std::string::npos))
                {
                    arFileName.assign(entry->d_name);
                    LOG_RECORD("find file: %s", arFileName.c_str());		
                    break;
                }
            }
        }
    }
    closedir(directory_pointer);
    return arFileName;
}

void CApp::upgrade_IVICtrl(int value)
{//1-请求切换U盘；2-升级包传输完毕；3-升级包传输错误
    mOTACtrlValue = value;
    if(mOTACtrlValue == 1)
    {
        mOTAProcessStep = OTA_Step_CheckMountPoint;
        upgrade::upgrade_ResponseIVICtrl(1);
    }
    else if(mOTACtrlValue == 2)
    {
        mOTAProcessStep = OTA_Step_CheckPack;
    }
    LOG_RECORD("%s,%d value = %d, mOTAProcessStep = %d\n", __func__, __LINE__, value, mOTAProcessStep);
}
void CApp::Upgrade_clusterVersion(std::string armVer, std::string mcuVer)
{
	mVialdVersion = true;
	if(armVer == "" || mcuVer == "")
	{
		return;
	}
    if(armVer.length() <sizeof(mARM_verion))
    {//arm version
        strncpy(mARM_verion, armVer.c_str(), sizeof(mARM_verion));
    }
    
    if(mcuVer.length() <sizeof(mMCU_version))
    {//mcu version
        strncpy(mMCU_version, mcuVer.c_str(), sizeof(mMCU_version));
    }
	
}
bool CApp::checkProjectFile()
{
	LOG_SDEBUG("%s, \n", __func__);
    bool ret = false;
	//防呆文件校验
	char filePath[128];
	snprintf(filePath, sizeof(filePath), "%s%s%s", cUpdateFileInfoList[mUpdatePath].filePath, OS_FILE_NAME, cFoolProofFile);
	if( 0 == access(filePath, F_OK))
	{
		FILE* fp = fopen(filePath, "r");
		if(fp)
		{
			char buf[128];
			int len = fread(buf, 1, sizeof(buf), fp);
			if(len >= strlen(PROJECT_CHECK))
			{
				std::string readBuf(buf, len);
				if(readBuf.find(PROJECT_CHECK) != std::string::npos)
				{
					ret = true;
				}
			}
			fclose(fp);
		}
	}
	return ret;
}
bool CApp::checkOsMd5()
{
    LOG_SDEBUG("%s, \n", __func__);
    bool ret = false;
    //校验md5
	char md5FilePath[128];
    std::vector<std::string> mMd5TextContent;
    snprintf(md5FilePath, sizeof(md5FilePath), "%s%s%s", cUpdateFileInfoList[mUpdatePath].filePath, OS_FILE_NAME, cOSFileMd5TextName);
    //std::string md5FilePath = OTA_PATH + cOSFileMd5TextName;
    if( 0 == access(md5FilePath, F_OK))
    {
        mMd5TextContent.clear();
        std::ifstream in(md5FilePath);
        std::string line;
        while (getline(in, line))
        {
            mMd5TextContent.push_back(line);
        }
    }
    else
    {
        //os校验md5失败，未检测到md5文件
        return ret;
    }
    if(mMd5TextContent.empty())
    {
        //校验失败
        return ret;
    }
    else
    {
        auto splitStr = [](std::string str, char pattCh)->std::pair<std::string, std::string>{
            std::pair<std::string, std::string> result;
            int pattCh_startIndex, pattCh_endIndex;
            pattCh_startIndex = str.find(pattCh);
            pattCh_endIndex = str.find_last_of(pattCh);
            result.first = str.substr(0, pattCh_startIndex); //md5sum
            result.second = str.substr(pattCh_endIndex + 1);    //fimename
            return result;
        };
        auto checkFileMd5Func = [](std::string filePath)->std::string{
            CDoShellCmd doShell;
            std::string cmd = "md5sum " + filePath + " | awk -F \" \" '{print $1}'";
            return doShell(cmd);
        };
        bool checkResult_Ok = true;
        for(size_t i = 0; i < mMd5TextContent.size(); i++)
        {
            std::pair<std::string, std::string> result = splitStr(mMd5TextContent[i], ' ');
            std::string fileFullPath = cUpdateFileInfoList[mUpdatePath].filePath + std::string(OS_FILE_NAME) + result.second;
            if(access(fileFullPath.c_str(), F_OK) != 0)
            {
                checkResult_Ok = false;
                LOG_SDEBUG("file(%s) is not exist", fileFullPath.c_str());
                break;
            }
            std::string checkResult = checkFileMd5Func(fileFullPath);
            if(checkResult.size() == 0)
            {
                checkResult_Ok = false;
                break;
            }
            else
            {
                checkResult.erase(checkResult.find_last_not_of(" \t")); // 去掉尾部空格
                if(checkResult != result.first)
                {
                    checkResult_Ok = false;
                    LOG_SDEBUG("file(%s) Md5 is not match, checkResult = %s, md5TextMd5Value = %s", fileFullPath.c_str(), checkResult.c_str(), result.first.c_str());
                    //break;
                }
            }
        }
        if(false == checkResult_Ok)
        {
            LOG_SDEBUG("%s,%d, checkResult_Failed\n", __func__,__LINE__);
            //校验失败
            ret = false;
        }
        else
        {
            ret = true;
        }
    }
    return ret;
}

bool CApp::setMCUUpgradeFlag(MCUUprgadeFlag flag)
{
    bool ret = false;
#if 0
    // Init upgrade file path.
    if (access(cMCUBakFilePath.data(), F_OK) == -1)
    {
        mkdir(cMCUBakFilePath.data(), 0766);
        if (access(cMCUBakFilePath.data(), F_OK) == -1)
        {
            LOG_RECORD("create MCU Backup directory\n");
            return ret;
        }
    }

    FILE* fp = fopen(cMCUBKRecFilename.data(), "r+");
    if (fp)
    {
        fseek(fp, 0, SEEK_SET);
        fprintf(fp, "%d\n", flag);
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);

        ret = true;
    }
    else
    {
        LOG_RECORD("create MCU Upgarde status file\n");
    }
#endif
    return ret;    
}

CApp::MCUUprgadeFlag CApp::getMCUUpgradeFlag()
{
    MCUUprgadeFlag ret = MCU_UFLAG_NONE;

    FILE* fp = fopen(cMCUBKRecFilename.data(), "r+");
    if (fp)
    {
        int val;
        fscanf(fp, "%d", &val);
        fclose(fp);

        ret = MCUUprgadeFlag(val);
    }

    return ret;    
}

const char* CApp::cOSFileMd5TextName = "md5.txt";
const char* CApp::cOTA_ArchivePrefix = "A301-IC";

const char* CApp::cOTA_ArchiveSuffix[2] = {".tar", ".zip"};

const CApp::updateFileInfo CApp::cUpdateFileInfoList[CApp::updatePath_Last] = {
    {CApp::updatePath_SDA1,     SD1_PATH},
    {CApp::updatePath_SDA2,     SD2_PATH},
    {CApp::updatePath_OTA,      OTA_PATH},
};

const std::string CApp::cMCUBakFilePath = "/opt/data/MCU_Bak/";
const std::string CApp::cMCUBKRecFilename = "/opt/data/MCU_Bak/mcu_upgradeSts.rec"; //升级mcu的记录文件,0=无升级文件，1=升级中，2=升级完成
const char* CApp::cFoolProofFile = "project.txt";


