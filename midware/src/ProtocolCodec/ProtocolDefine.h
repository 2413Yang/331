#ifndef PROTOCOL_DEFINE_H_
#define PROTOCOL_DEFINE_H_

#include <stdarg.h>
#include <map>
#include <vector>
#include <initializer_list>
#include <string>
#include "IdDefine.h"
#pragma pack(1)

const char PRTOCOL_HEAD_H = 0xFF;  //帧头标识
const char PRTOCOL_HEAD_L = 0xAA;
const char FRAME_END_FLAG = 0x0A; // 协议帧的结束标识

const unsigned int VAILD_FRAME_MIN_LEN = 9; // 有效帧最小长度

const int LEN_INDEX_HIGH = 5; // 帧长度高字节和低字节的索引
const int LEN_INDEX_LOW = 6; 

unsigned char CalcCheckLength(unsigned char ucFrameLenH, unsigned char ucFrameLenL);
unsigned char CalcCheckSum(const char *pData, int iLen);

#define FRAME_MAX_SIZE 1024 // 帧的最大大小
#define PROTOCOL_BASE_LEN_BYTE 12
#define PROTOCOL_BASE_LEN_BIT 80
#define MAJOR_ID_OFFSET 56 // 主ID和次ID在帧中的偏移量
#define MINOR_ID_OFFSET 80
#define IVI_MAJOR_ID_OFFSET 48 // IVI主ID在帧中的偏移量
#define INVALID_ID 0xFFFFFFFFu // 无效ID的标识
class CProtocolFrame
{
public:
    CProtocolFrame();
    virtual unsigned int getMinorID(void);
    virtual unsigned int getMajorID(void);
    void updateFrame(std::string &frame);
    void updateFrame(unsigned char *frame, int size);

protected:
    unsigned char *getFrameBuf();
    void lockedFrameDef();
    void addFrameDef(unsigned int ID, std::initializer_list<int> cfg);
    int getBit(int offset);
    void setBit(int offset, int bit);
    unsigned int reorder(unsigned int tmp, int nbit);
    std::map<unsigned int, std::vector<int>> mFrameCfg;

private:
    bool bLock;
    unsigned char mFrame[FRAME_MAX_SIZE];
};

#pragma pack()
#endif // PROTOCOL_DEFINE_H_