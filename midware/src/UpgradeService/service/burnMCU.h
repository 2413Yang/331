#ifndef _BURNMCU_H_
#define _BURNMCU_H_
#include <unistd.h>
#include "UartDevice.h"

#define ACK 0X5506F9
#define CAN 0X5518E7
#define NAK 0X5515EA
#define PGK_MAX_SZIE (4 * 1024 + 128)

#pragma pack(1)


// 帧结构体定义参考A301项目《[A301仪表]MCU与ARM通信协议_V0.8-202205013.xlsx》中的MCU升级通信协议。
// 《[仪表通用]MCU与ARM通信命令表2.5.docx》中的2.13.ARM 0x9C 命令：请求复位/升级

// ARM 0x9C 命令：请求复位/升级
typedef struct _mcuf_Upgrade
{
    uint8_t data[1]; // 0xA0 请求升级mcu软件, 0xA3 请求MCU升级的协议版本，根据不同的MCU程序空间，适配相应的升级协议。
} MCUF_Upgrade;

// 文件信息帧（首帧）的DATA域格式：
// 数据标志 TAG: 4字节，本项目中固定为'A','3','0','1'这4个字符。
// 协议版本 VER：1字节。 A301项目中为1。
// 数据总长度 TOTAL:  4字节，总共要发送数据的总字节数。接收方可以通过已接收数据的总字节数和TOTAL来计算发送和接收的进度。
// 帧最大长度 MAXLEN: 2字节，后续数据帧中LEN的最大值。接收方通过MAXLEN的数值，知道需要准备多少字节的缓存来接收一帧数据。
// 只有当最后的数据字节数不够MAXLEN时，LEN值才会小于MAXLEN。
// 最后一帧的LEN长度总是为0。
typedef struct _mcuf_FileInfo
{
    uint8_t tag[4];
    uint8_t ver;
    uint32_t total;
    uint16_t maxLen;
} MCUF_FileInfo;

// 文件数据帧的DATA域格式：
// 序号 IDX：  1字节，数据包的序号。从1开始编号。第一帧数据，序号为 IDX = 1,后续帧的序号依次循环累加（从1累加到255，超过255时，重新回到1）。
// 长度 LEN:   2字节，表示这一帧要发送的数据长度。例如 LEN = n 时，表示后面跟随n字节的数据。
// 保留 reserve： 5字节，保留位，使后面的data域8字节对齐。方便MCU烧录data数据。
// 数据 DATA： n字节，这一帧要发送的数据。占用字节数等于LEN域中的数值。
// 校验和 SUM: 2字节, IDX + LEN + DATA域中所有字节的CRC-16校验和。算法是MODBUS协议标准的CRC16 ，多项式：0x8005。
typedef struct _mcuf_FileData
{
    uint8_t idx;
    uint16_t len;
    uint8_t reserve[5];
    uint8_t data[1];
} MCUF_FileData;

typedef union _mcuf_data
{
    MCUF_Upgrade Upgrade;
    MCUF_FileInfo FileInfo;
    MCUF_FileData FileData;    
} MCUFData;

typedef struct _huf_head
{
    uint8_t soi[2];
    uint8_t cmd;
    uint8_t ver;
    uint8_t mark;
    uint8_t length[2];
} MCUFHead;

typedef struct _huf_frame_struct
{
    MCUFHead head;
    MCUFData info;
} MCUFrameStruct;
#pragma pack()

class burnMCU:public CUartDevice
{
public:
    enum {
        FID_MCUUprgade = 0x9C, //0x9c
        FID_MCUFileInfo = 0xE8, //0xE8
        FID_MCUFileData = 0xE9, //0xE9
    };

    enum {
        UPGRADE_REQUEST = 0xA0, //0xA0
        UPGRADE_PROTOCOL = 0xA3 //0xA3
    };

public:
    burnMCU(const char* mcuFile);
    ~burnMCU();
    
    //void startBurn(const char* mcuFile);
    int getProgressBarValue();
    std::string getErrorInfo();
    void registerDoneCB(std::function<void(void)> cb);

private:
    virtual void handleEvent(int fd);
    virtual bool getFrame(std::string &cache, std::string &frame);

    bool getFrameInProgress(std::string &cacheFrame, std::string &frame);
    bool getFrameInHandshake(std::string &cacheFrame, std::string &frame);
    void frameHandler(std::string frame);
    void frameHandlerInHandshake(std::string frame);
    void frameHandlerInProgress(std::string frame);
    void storeErrorInfo(std::string error);
    size_t takePackageSendToMCU(uint32_t blockIdx, size_t blockSize);
    
    // Util
    std::string frameEncode(int fid, MCUFData *pData, size_t dataLen);
    uint8_t frameCheckSum(const uint8_t *pData, size_t dataLen);
    bool frameCheckSumValid(std::string frame);
    void crc16Poly8005(uint8_t *s, size_t len, uint8_t *crcLow, uint8_t *crcHigh);
    uint16_t swap16(uint16_t data);
    uint32_t swap32(uint32_t data);
    std::string getHexString(std::string const &buffer, std::string const &delimiter);

private:
    int mProgress;
    size_t mBlockSize;
    bool mHandshakeDone;
    int mHandshakeTimeoutCount;
    std::string mFrameCache;    
    const char* mMCUFile;
    uint32_t mMCUFileLength;
    int i_fd;
    int i_totalLength;
    uint8_t ui_protocolType;
    uint8_t ui_dataPackage[PGK_MAX_SZIE];
    std::mutex mLock;
    std::string mErrorInfo;
    std::vector<uint8_t> mSendBuffer;
    std::function<void(void)> mDoneCB;
};

#endif//!_BURNMCU_H_