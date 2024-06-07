#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "HUMsgService.h"
#include "mylogCtrl.h"
#include "LogServiceApi.h"

const std::string CHUMsgService::SERVER_NAME_SELF = "HUMsgService";
const std::string CHUMsgService::UPGRADE_MOUNTPATH = "/run/ota";
const std::string CHUMsgService::UPGRADE_DIR = "/run/ota/";
const std::string CHUMsgService::ARCHIVE_PREFIX = "A301-IC";
const std::string CHUMsgService::ARCHIVE_SUFFIX = ".zip";

static CHUMsgService *pHUMsgServer = nullptr;
void SigExit(int sig)
{
    LOGDBG("===== ERROR SIG_TYPE: %d =====\n", sig);
#ifndef WIN32
    LOGDBG("===== ERROR NO: %d, %s =====\n", errno, strerror(errno));
    signal(SIGINT, SIG_IGN);
    kill(0, SIGINT);
#endif
    if (pHUMsgServer)
    {
        LOG_RECORD("==== ERROR SIG_TYPE:%d,NO:%d,%s ====", sig, errno, strerror(errno));
        delete pHUMsgServer;
    }
    exit(-1);
}

bool isNetCardReady()
{
    FILE *procpt;
    const std::string cmd = "ifconfig ncm0";
    std::string line = "";
    char lineBuf[33] = {0};

    procpt = popen(cmd.data(), "r");
    if (fgets(lineBuf, sizeof(lineBuf), procpt))
    {
        line = (char *)lineBuf;
        if (line.find("ncm0") == 0)
        {
            return true;
        }
    }

    return false;
}

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

	#if 0
    // Set stdout LOGDBG to log file for debug。
	fflush(stdout);
	setvbuf(stdout, NULL, _IONBF, 0);
	freopen("/opt/data/HUMsgService.log", "a", stdout);
	#endif

    LOGDBG("[%s] build time: %s %s\n", argv[0], __DATE__, __TIME__);
    
    // Wait netcard ready.
    do
    {
        if(ZH::BaseLib::getWorldTimeMS() > 20*1000)
        {
            break;
        }
        sleep(1);
    } while (true);
    
    do {
        system("fsg_ncm_device");
        sleep(2);
    } while(!isNetCardReady());

    pHUMsgServer = new CHUMsgService(argc, argv);
    pHUMsgServer->Start(true);

    return EXIT_SUCCESS;
}

CHUMsgService::CHUMsgService(int argc, char *argv[])
    : mSelfIpc(SERVER_NAME_SELF),
      mWatchIpc("WatchService"),
      mLogIpc("LogService"),
      mUpgradeIpc("UpgradeService"),
      mDevMsg("192.168.2.99", 10001),
      mDevFile("192.168.2.99", 10002),
      mUpgradeFilename(""),
      mUpgradeFileMD5(""),
      mHeartbeatTimeout(0),
      mSilence(false)
{
}

CHUMsgService::~CHUMsgService()
{
}

void CHUMsgService::Init()
{
    initCommunicDevice();
    initSelfIPC();
    initWathchIPC();
    initLogIPC();
    initUpgradeIPC();
    initFSM();
}

void CHUMsgService::UInit() {}

bool CHUMsgService::devMsgConnect()
{
    // Connect to file socket to receive file info/data.
    int retryCount = 0;
    while (!mDevMsg.isConnect() && retryCount < CONNECT_RETRY_THRESHOLD)
    {
        LOGDBG("%s:%d - Connect to message socket, try (%d).\n", __func__, __LINE__, retryCount + 1);
        retryCount++;
        mDevMsg.start();
        fflush(stdout);
        sleep(1);
    }

    return mDevMsg.isConnect();
}

bool CHUMsgService::devFileConnect()
{
    // Connect to file socket to receive file info/data.
    int retryCount = 0;
    while (!mDevFile.isConnect() && retryCount < CONNECT_RETRY_THRESHOLD)
    {
        LOGDBG("%s:%d - Connect to file socket, try (%d).\n", __func__, __LINE__, retryCount + 1);        
        retryCount++;
        mDevFile.start();
        fflush(stdout);
        sleep(1);
    }

    return mDevFile.isConnect();
}

void CHUMsgService::DoWork()
{
    devMsgConnect();
    devFileConnect();

    while (true)
    {        
        if (mSilence)
        {
            // Stop connect to HU in silence.
            if (mDevMsg.isConnect())
            {
                mDevMsg.stop();
            }

            if (mDevFile.isConnect())
            {
                mDevFile.stop();
            }

            sleep(2);
            LOGDBG(".");
            fflush(stdout);
            continue;
        }

        // IP Send heartbeat frame to HU every 800ms.
        HUFData data = {IPHeartbeat : {cmd : 0}};
        std::string frame = mFrameEncoder(CHUFrame::FID_IPHeartbeat, &data, sizeof(data.IPHeartbeat));
        mDevMsg.send(frame);
        usleep(800*1000);
        fflush(stdout);
        LOGDBG(">");

        mHeartbeatTimeout++;
        if (mHeartbeatTimeout > HEARTBEAT_TIMEOUT_THRESHOLD)
        {
            LOGDBG("\n*** %s:%d - HU heartbeat ack timeout, try to connect. ***\n", __func__, __LINE__);
            mHeartbeatTimeout = 0;
            mDevMsg.stop();
            mDevFile.stop();

            devMsgConnect();
            devFileConnect();
        }
    }
}
