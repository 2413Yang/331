#include <fcntl.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "burnMCU.h"
#include "mylogCtrl.h"
#include "LogServiceApi.h"

#define LCHKBITS(LEN, BIT) ((uint8_t)((LEN >> BIT) & 0X0F))
#define LCHKSUM(LEN) ((uint8_t)(((~(LCHKBITS(LEN, 8) + LCHKBITS(LEN, 4) + LCHKBITS(LEN, 0))) + 1) & 0X0F))

burnMCU::burnMCU(const char *mcuFile) :
    mProgress(0),
    mBlockSize(0),
    mHandshakeDone(false),
    mHandshakeTimeoutCount(0),
    mFrameCache(""),
    mMCUFile(mcuFile),
    mMCUFileLength(0),
    mSendBuffer(4096)
{

    if (nullptr == mMCUFile)
    {
        i_fd = -1;
    }
    else
    {
        i_fd = open(mMCUFile, O_RDONLY);
        if ((i_totalLength = lseek(i_fd, 0, SEEK_END)) < 0)
        {
            LOG_SERROR("lseek file failure!");
            i_totalLength = 0;
        }
    }
    this->start();

    // Query block size from MCU.
    MCUFData data = {Upgrade : {data : {UPGRADE_PROTOCOL}}};
    std::string frame = frameEncode(FID_MCUUprgade, &data, sizeof(data.Upgrade));
    send(frame);
    LOG_RECORD("IP->MCU: Query upgrade protocol. \nframe=%s\n", getHexString(frame, " ").data());
}

burnMCU::~burnMCU()
{
    close(i_fd);
}

void burnMCU::handleEvent(int fd)
{
    std::string frame;
    char readCache[128] = {0};
    int n = read(fd, readCache, sizeof(readCache));
    if (n <= 0)
    {
        LOG_SERROR("device data read error [%d]. %s", n, strerror(errno));
        return;
    }

    mFrameCache += std::string(readCache, n);
    while (true)
    {
        if (getFrame(mFrameCache, frame))
        {
            frameHandler(frame);
        }
        else
        {
            break;
        }
    }
}

bool burnMCU::getFrameInHandshake(std::string &cacheFrame, std::string &frame)
{
    std::size_t cacheLen = cacheFrame.length();
    std::size_t headOffset = 0;
    std::size_t tailOffset = 0;
    std::size_t frameLen = 0;
    std::size_t lenCheckSum = 0;
    std::size_t offset = 0;
    std::string frameHeader = {0xFF, 0xAA};
    std::string frameTailer = {0x0A};
    const unsigned FRAME_MIN_SIZE = 9;
    const unsigned FRAME_LEN_OFFSET = 5;
    bool ret = false;

    frame = std::string("");
    if (std::string::npos != (headOffset = cacheFrame.find(frameHeader, 0)))
    {
        if (cacheLen >= (headOffset + FRAME_MIN_SIZE))
        {
            lenCheckSum = ((cacheFrame[headOffset + FRAME_LEN_OFFSET] & 0xF0) >> 4) & 0x0F;
            frameLen = ((cacheFrame[headOffset + FRAME_LEN_OFFSET] & 0x0F) << 8) |
                (cacheFrame[headOffset + FRAME_LEN_OFFSET + 1] & 0xFF);

            bool frameLenValid = (LCHKSUM(frameLen) == lenCheckSum);
            if (frameLenValid) {
                if (cacheLen >= (headOffset + frameLen))
                {
                    tailOffset = headOffset + frameLen - frameTailer.length();
                    offset = cacheFrame.find(frameTailer, tailOffset);
                    if (std::string::npos != offset)
                    {
                        frameLen = offset - headOffset + frameTailer.length();
                        frame.assign(cacheFrame, headOffset, frameLen);
                        cacheFrame.assign(cacheFrame, headOffset + frameLen, cacheLen);
                        ret = true;
                    }
                }
            }
        }
    }

    return ret;        
}

bool burnMCU::getFrameInProgress(std::string &cacheFrame, std::string &frame)
{
    bool ret = false;
    std::size_t Offset = 0;
    std::string strACK({(char)0X55, (char)0X06, (char)0XF9}, 3);
    std::string strCAN({(char)0X55, (char)0X18, (char)0XE7}, 3);
    std::string strNAK({(char)0X55, (char)0X15, (char)0XEA}, 3);
    if (((std::string::npos != (Offset = cacheFrame.find(strACK))) ||
        (std::string::npos != (Offset = cacheFrame.find(strCAN))) ||
        (std::string::npos != (Offset = cacheFrame.find(strNAK)))) &&
        (cacheFrame.length() >= (6 + Offset)))
    {
        frame = cacheFrame.substr(Offset, 6);
        cacheFrame = cacheFrame.substr(Offset + 6);
        ret = true;
    }
    else if (!cacheFrame.empty())
    {
        cacheFrame = ""; // cacheFrame.substr(Offset + 1);
        LOG_SERROR("error frame detected.");
    }

    return ret;
}

bool burnMCU::getFrame(std::string &cacheFrame, std::string &frame)
{
    bool ret = false;

    if (mBlockSize == 0)
    {
        ret = getFrameInHandshake(cacheFrame, frame);
    }
    else
    {
        ret = getFrameInProgress(cacheFrame, frame);
    }
    
    return ret;
}

int burnMCU::getProgressBarValue()
{
    if (!mHandshakeDone)
    {
        mHandshakeTimeoutCount++;
        if (mHandshakeTimeoutCount > 5)
        {
            // Retry handshake if timeout.
            mHandshakeTimeoutCount = 0;
            mBlockSize = 0;

            // Query block size from MCU.
            MCUFData data = {Upgrade : {data : {UPGRADE_PROTOCOL}}};
            std::string frame = frameEncode(FID_MCUUprgade, &data, sizeof(data.Upgrade));
            send(frame);
            LOG_RECORD("IP->MCU: Retry Query upgrade protocol. \nframe=%s\n", getHexString(frame, " ").data());
        }
    }

    return mProgress;
}

std::string burnMCU::getErrorInfo()
{
    std::unique_lock<std::mutex> guard(mLock);
    return mErrorInfo;
}
void burnMCU::storeErrorInfo(std::string error)
{
    std::unique_lock<std::mutex> guard(mLock);
    mErrorInfo = error;
}

void burnMCU::frameHandlerInHandshake(std::string frame)
{
    if (frameCheckSumValid(frame)) {
        MCUFrameStruct *pFrame = (MCUFrameStruct *)frame.data();
        if (pFrame->head.cmd == FID_MCUUprgade &&
            pFrame->info.Upgrade.data[0] == UPGRADE_PROTOCOL)
        {
            uint8_t type = pFrame->info.Upgrade.data[1];

            mBlockSize = (type == 8 || type == 16) ? 1024 : type == 32 ? 4096 : 1024;
            mFrameCache = "";

            // Request start upgrade.
            MCUFData data = {Upgrade : {data : {UPGRADE_REQUEST}}};
            std::string frame = frameEncode(FID_MCUUprgade, &data, sizeof(data.Upgrade));
            send(frame);
            LOG_RECORD("IP->MCU: Start. blockSize = %lu, \nframe=%s\n",
                (long unsigned int)(mBlockSize),
                getHexString(frame, " ").data());
        }
        else
        {
            LOG_SERROR("Frame protocol is unknow.\n");
        }
    }
    else
    {
        LOG_SERROR("Frame checksum is invalid.\n");
    }
}

void burnMCU::frameHandlerInProgress(std::string frame)
{
    storeErrorInfo("");

    if (frame.length() == 6)
    {
        const uint8_t idx = frame[3] & 0xFF;
        const uint32_t frameID = ((frame[0] & 0xFF) << 16) | ((frame[1] & 0xFF) << 8) | (frame[2] & 0xFF);
        const uint32_t errorCode = (frame[4] & 0xFF);

        switch (frameID)
        {
        case ACK:
        {
            // Send next file data frame.
            takePackageSendToMCU(idx + 1, mBlockSize);
            if (0x04 == errorCode)
            {
                storeErrorInfo("接收帧完成，但超过5s，未收到上位机指令， 请求发送下一帧");
            }
            break;
        }
        case CAN:
        {
			LOG_RECORD("%s CAN errorCode;%d\n", __func__, errorCode);
            switch (errorCode)
            {
            case 0x01:
                storeErrorInfo("信息包序号不匹配");
                break;
            case 0x02:
                storeErrorInfo("客户ID信息不匹配");
                break;
            case 0x03:
                storeErrorInfo("FLASH格式化失败");
                break;
            case 0x04:
                storeErrorInfo("FLASH写入失败");
                break;
            case 0x05:
                storeErrorInfo("公司标识符不匹配");
                break;
            };
            break;
        }
        case NAK:
        {
			LOG_RECORD("%s NAK errorCode;%d\n", __func__, errorCode);
            if (idx == 0)
            {
                mHandshakeDone = true;

                // Send file information frame.
                MCUFData data = {
                    FileInfo : {
                        tag : {'A', '3', '0', '1'},
                        ver : 1,
                        total : swap32((uint32_t)i_totalLength),
                        maxLen : swap16(mBlockSize)
                    }
                };
                std::string frame = frameEncode(FID_MCUFileInfo, &data, sizeof(data.FileInfo));
                send(frame);
                LOG_RECORD("IP->MCU: File info. \nframe=%s\n", getHexString(frame, " ").data());
            }
            else
            {
                // Resend file data frame.
                takePackageSendToMCU(idx, mBlockSize);
            }

            // Set error message by error code.
            switch (errorCode)
            {
            case 0x01:
                storeErrorInfo("头码出错");
                break;
            case 0x02:
                storeErrorInfo("信息包校验出错");
                break;
            case 0x03:
                storeErrorInfo("校验和错");
                break;
            case 0x04:
                storeErrorInfo("接收帧5s超时出错,请求重发帧");
                break;
            }
            break;
        }
        default:
            LOG_SERROR("Incorrect protocol frame occurred during MCU upgrade.");
        }
    }
}

void burnMCU::frameHandler(std::string frame)
{
    printf("IP<-MCU: len=%d, frame=%s\n", frame.length(), getHexString(frame, " ").data());
    
    if (mBlockSize == 0)
    {
        frameHandlerInHandshake(frame);
    }
    else
    {
        frameHandlerInProgress(frame);
    }
}


size_t burnMCU::takePackageSendToMCU(uint32_t blockIdx, size_t blockSize)
{
    #define OFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

    MCUFData *pData = (MCUFData *)ui_dataPackage;
    size_t readBytes = 0;
    size_t crcDataLen = 0;

    if (lseek(i_fd, (blockIdx - 1) * blockSize, SEEK_SET) >= 0)
    {
        readBytes = read(i_fd, &pData->FileData.data, blockSize);
    }

    pData->FileData.idx = blockIdx;
    pData->FileData.len = swap16((uint16_t)readBytes);
    crcDataLen = OFFSETOF(MCUF_FileData, data) + readBytes;
    crc16Poly8005((uint8_t*)pData,
                crcDataLen,
                &(pData->FileData.data[readBytes + 1]),
                &(pData->FileData.data[readBytes]));

    const int CRC_SIZE = sizeof(uint16_t);
    const size_t dataLen = crcDataLen + CRC_SIZE;
    std::string frame = frameEncode(FID_MCUFileData, pData, dataLen);
    send(frame);

    printf("IP->MCU: File Data. readBytes=%d, crcDataLen=%d, dataLen=%d, frameLen=%d, crcHi=0x%x crcLow=0x%2x.\nframe=%s ...\n",
        readBytes,
        crcDataLen,
        dataLen,
        frame.length(),
        pData->FileData.data[readBytes],
        pData->FileData.data[readBytes + 1],
        getHexString(frame.substr(0, 32), " ").data());

    mProgress = (readBytes == blockSize ? ((blockIdx * blockSize * 100) / i_totalLength) : 100);
    
    const bool sendComplete = (readBytes == 0);
    if (sendComplete && mDoneCB)
    {
        mDoneCB();
    }

    return readBytes;
}

uint8_t burnMCU::frameCheckSum(const uint8_t *pData, size_t dataLen)
{
    uint8_t checkSum = 0;
    for (size_t i = 0; i < dataLen; i++)
    {
        checkSum += pData[i];
    }

#ifdef _QNX_TARGET_
    return ((~checkSum) + 1);
#endif
#ifdef _LINUX_TARGET_
    return checkSum;
#endif
}

bool burnMCU::frameCheckSumValid(std::string frame)
{
    const size_t len = frame.length();
    const uint8_t *pFrame = (uint8_t *)frame.data();
    const uint8_t checkSum = frameCheckSum(pFrame + 2, len - 4) & 0xFF;
    return checkSum == pFrame[len - 2];
}

std::string burnMCU::frameEncode(int fid, MCUFData *pData, size_t dataLen)
{
    const size_t FRAME_MIN_SIZE = 9;
    uint8_t *frameBuf = mSendBuffer.data();
    size_t frameLen = FRAME_MIN_SIZE + dataLen;

    frameBuf[0] = 0xFF;
    frameBuf[1] = 0xAA;
    frameBuf[2] = fid;
    frameBuf[3] = 0;
    frameBuf[4] = 0;
    frameBuf[5] = (LCHKSUM(frameLen) << 4) | (frameLen & 0xFF00) >> 8;
    frameBuf[6] = frameLen & 0xFF;

    memcpy(&frameBuf[7], pData, dataLen);
    frameBuf[frameLen - 2] = frameCheckSum(frameBuf + 2, frameLen - 4) & 0xFF;
    frameBuf[frameLen - 1] = 0x0A;
    return std::string((char *)frameBuf, frameLen);
}

std::string burnMCU::getHexString(std::string const &buffer, std::string const &delimiter)
{
    std::stringstream ss;
    std::string space = "";
    for (size_t i = 0; i < buffer.length(); i++)
    {
        ss << space << std::hex << std::setw(2) << std::setfill('0') << int(buffer[i]);
        space = delimiter;
    }
    return ss.str();
}

uint32_t burnMCU::swap32(uint32_t data)
{
    uint8_t *p = (uint8_t *)&data;
    uint8_t tmp = p[0];
    p[0] = p[3];
    p[3] = tmp;
    tmp = p[1];
    p[1] = p[2];
    p[2] = tmp;

    return data;
}

uint16_t burnMCU::swap16(uint16_t data)
{
    uint8_t *p = (uint8_t *)&data;
    uint8_t tmp = p[0];
    p[0] = p[1];
    p[1] = tmp;

    return data;
}

// CRC-16/MODBUS
// 多项式公式: x^16 + x^15 + x^2 + 1
//
// 宽度  多项式  初始值  结果异或值  输入反转  输出反转
// 16    8005   FFFF   0000        true     true
void burnMCU::crc16Poly8005(uint8_t *s, size_t len, uint8_t *crcLow, uint8_t *crcHigh)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;
    while (len--)
    {
        crc ^= *s++;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x01)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    *crcLow = (uint8_t)(crc & 0x00FF);
    *crcHigh = (uint8_t)((crc & 0xFF00) >> 8);
}

void burnMCU::registerDoneCB(std::function<void(void)> cb)
{
    mDoneCB = cb;
}