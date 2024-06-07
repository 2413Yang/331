#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "LogServiceApi.h"
#include "UpgradeInterface.h"
#include "HUMsgService.h"

void CHUMsgService::HUUpgradeRequest(const fsm::args &args)
{    
    printf("%s:%d\n", __func__, __LINE__);

    // TODO: check whether enable upgrade in current status.
    bool enUpgrade = true;

    mFileByteReceived = 0;

    devFileConnect();

    enUpgrade |= mDevFile.isConnect();

    // Respond upgrade accept or not to HU.
    HUFData data = {IPUpgradeAccept : {cmd : (uint32_t)(enUpgrade ? 1 : 0)}};
    std::string frame = mFrameEncoder(CHUFrame::FID_IPUpgradeAccept, &data, sizeof(data.IPUpgradeAccept));
    mDevMsg.send(frame);
}

void CHUMsgService::HUFileInfo(const fsm::args &args)
{
    // printf("%s:%d\n", __func__, __LINE__);

    const HUF_HUFileInfo *pData = &((const HUFData *)args[0].data())->HUFileInfo;
    mUpgradeFileInfo = *pData;
    mUpgradeFilename = (char*)mUpgradeFileInfo.fileName;
    bool invalidName = (mUpgradeFilename.find(ARCHIVE_SUFFIX, mUpgradeFilename.length() - ARCHIVE_SUFFIX.length()) == std::string::npos);
    if (invalidName)
    {
        printf("%s:%d Invalid upgrade filename. %s\n", __func__, __LINE__, mUpgradeFilename.data());

        // Accept fail.
        HUFData data = {IPUpgradeStatus : {cmd : IPUpgradeStatus_AcceptFaile}};
        std::string frame = mFrameEncoder(CHUFrame::FID_IPUpgradeStatus, &data, sizeof(data.IPUpgradeStatus));
        mDevMsg.send(frame);
        return;
    }

    mUpgradeFilename = UPGRADE_DIR + ARCHIVE_PREFIX + "-"+ mUpgradeFilename;
    mUpgradeFileMD5 = getHexString((char *)mUpgradeFileInfo.fileMd5, "");
     
    printf("name=%s md5=%s, len=%lu, type=%lu\n",
        mUpgradeFilename.data(),
        mUpgradeFileMD5.data(),
        (long unsigned int)(mUpgradeFileInfo.fileType),
        (long unsigned int)(mUpgradeFileInfo.fileLength));

    // Init upgrade file path.
    if (access(UPGRADE_DIR.data(), F_OK) == -1)
    {
        mkdir(UPGRADE_DIR.data(), 0766);
        if (access(UPGRADE_DIR.data(), F_OK) == -1)
        {
            printf("%s:%d Upgrade dir is not exist. %s\n", __func__, __LINE__, UPGRADE_DIR.data());

            // Accept fail.
            HUFData data = {IPUpgradeStatus : {cmd : IPUpgradeStatus_AcceptFaile}};
            std::string frame = mFrameEncoder(CHUFrame::FID_IPUpgradeStatus, &data, sizeof(data.IPUpgradeStatus));
            mDevMsg.send(frame);
            return;
        }
    }

    // Check disk free.
    size_t freeBytes = getDiskFreeBytes(UPGRADE_MOUNTPATH);
    if (freeBytes < mUpgradeFileInfo.fileLength * 2)
    {
        printf("%s:%d Disk free space low.\n", __func__, __LINE__);

        // Accept fail.
        HUFData data = {IPUpgradeStatus : {cmd : IPUpgradeStatus_AcceptFaile}};
        std::string frame = mFrameEncoder(CHUFrame::FID_IPUpgradeStatus, &data, sizeof(data.IPUpgradeStatus));
        mDevMsg.send(frame);
        return;
    }

    // Create empty file, prepare to write.
    FILE *pFile = fopen(mUpgradeFilename.data(), "w+");
    fclose(pFile);
}

void CHUMsgService::HUFileData(const fsm::args &args)
{
    printf("%s:%d\n", __func__, __LINE__);

    size_t len = args[0].length();
    mFileByteReceived += len;
    // printf("len=%lu progress=%lu/%lu data=%s ...\n",
    //    (long unsigned int)(len),
    //    (long unsigned int)(mFileByteReceived), 
    //    (long unsigned int)(mUpgradeFileInfo.fileLength),
    //    getHexString(args[0].substr(0, 32), " ").data());

    FILE *pFile = fopen(mUpgradeFilename.data(), "ab");
    size_t bytes = fwrite(args[0].data(), sizeof(uint8_t), len, pFile);
    if (bytes != len)
    {
        printf("%s:%d Write file error\n", __func__, __LINE__);
    }
    fflush(pFile);
    fsync(fileno(pFile));
    fclose(pFile);

    if (mFileByteReceived >= mUpgradeFileInfo.fileLength)
    {
        const std::string md5 = getFileMD5(mUpgradeFilename.data());
        bool bInvalidMD5 = (md5 != mUpgradeFileMD5);
        if (bInvalidMD5)
        {
            printf("%s:%d Invlaid md5 checksum.\n", __func__, __LINE__);

            // Accept fail.
            HUFData data = {IPUpgradeStatus : {cmd : IPUpgradeStatus_AcceptFaile}};
            std::string frame = mFrameEncoder(CHUFrame::FID_IPUpgradeStatus, &data, sizeof(data.IPUpgradeStatus));
            mDevMsg.send(frame);
        }
        else
        {
            {
                // All file data are received.
                // IP Send FileEnd frame to HU for report all file data are received.
                HUFData data = {IPFileEnd : {cmd : 0}};
                std::string frame = mFrameEncoder(CHUFrame::FID_IPFileEnd, &data, sizeof(data.IPFileEnd));
                mDevMsg.send(frame);            
            }

            {
                // File receive done.
                HUFData data = {IPUpgradeStatus : {cmd : IPUpgradeStatus_AcceptDone}};
                std::string frame = mFrameEncoder(CHUFrame::FID_IPUpgradeStatus, &data, sizeof(data.IPUpgradeStatus));
                mDevMsg.send(frame);
            }

            // Notify Upgrade service that upgrade file is ready.
            const int UPGRADE_FILE_RECEIVE_DONE = 2;
            upgrade::upgrade_IVICtrl(UPGRADE_FILE_RECEIVE_DONE);

            {
                // Upgrade done.
                HUFData data = {IPUpgradeStatus : {cmd : IPUpgradeStatus_UpgradeDone}};
                std::string frame = mFrameEncoder(CHUFrame::FID_IPUpgradeStatus, &data, sizeof(data.IPUpgradeStatus));
                mDevMsg.send(frame);
            }

            // Goto silence.
            printf("\n*** %s:%d - Goto silence. ***\n", __func__, __LINE__);
            mSilence = true;
        }
    }
    else 
    {
        // Request next data.        
        // IP Send FileNext frame to HU for qeuest next file data.
        HUFData data = {IPFileNext : {cmd : 0}};
        std::string frame = mFrameEncoder(CHUFrame::FID_IPFileNext, &data, sizeof(data.IPFileNext));
        mDevMsg.send(frame);

    }
}
