#include "ProtocolDefine.h"
#include "mlog.h"
#include <string.h>

unsigned char CalcCheckLength(unsigned char ucFrameLenH, unsigned char ucFrameLenL)
{
    unsigned char ucLenCheckCalc = ((~((ucFrameLenL & 0x0F) + ((ucFrameLenL >> 4) & 0x0F) + (ucFrameLenH & 0x0F))) + 1) & 0x0F;
    return ucLenCheckCalc;
}

unsigned char CalcCheckSum(const char *pData, int iLen)
{
    unsigned char ucCheckSumCalc = 0;
    for (int i = 0; i < iLen; i++)
    {
        ucCheckSumCalc += pData[i];
        //LOGDBG("pTmp[%d] = 0x%x ucCheckSumCalc = 0x%x", i, pTmp[i], ucCheckSumCalc);
    }

#ifdef _QNX_TARGET_
    return ((~ucCheckSumCalc) + 1);
#endif
#ifdef _LINUX_TARGET_
    return ucCheckSumCalc;
#endif
}

CProtocolFrame::CProtocolFrame()
{
    bLock = false;
    memset(mFrame, 0, FRAME_MAX_SIZE);
}

void CProtocolFrame::updateFrame(std::string &frame)
{
    memset(mFrame, 0, FRAME_MAX_SIZE);
    if (frame.length() < FRAME_MAX_SIZE)
        memcpy(mFrame, frame.data(), frame.length());

    // PRFHEX(mFrame, frame.length(), "updateFrame");
}

void CProtocolFrame::updateFrame(unsigned char *frame, int size)
{
    if (size < FRAME_MAX_SIZE)
        memcpy(mFrame, frame, size);
}

unsigned int CProtocolFrame::getMinorID(void)
{
    return ((mFrame[7] << 8) & 0xFF00) | (mFrame[8] & 0xFF);
}

unsigned int CProtocolFrame::getMajorID(void)
{
    return (((mFrame[2] << 16) & 0xFF0000) | ((mFrame[3] << 8) & 0xFF00) | (mFrame[4] & 0xFF));
}

void CProtocolFrame::lockedFrameDef()
{
    bLock = true;
}

void CProtocolFrame::addFrameDef(unsigned int ID, std::initializer_list<int> cfg)
{
    if (mFrameCfg.find(ID) == mFrameCfg.end() && !bLock)
        mFrameCfg[ID].insert(mFrameCfg[ID].end(), cfg.begin(), cfg.end());
}

unsigned char *CProtocolFrame::getFrameBuf()
{
    return mFrame;
}

int CProtocolFrame::getBit(int offset)
{
    unsigned int targ = mFrame[offset / 8] & 0xFF;
    // printf("ID: %04X, getBit: %02X  offset: %d\n", getID(), targ, offset);
    return (targ >> offset % 8) & 0x01;
}

#pragma pack(1)
struct StuBit
{
    char bit0 : 1;
    char bit1 : 1;
    char bit2 : 1;
    char bit3 : 1;
    char bit4 : 1;
    char bit5 : 1;
    char bit6 : 1;
    char bit7 : 1;
};
#pragma pack()
// int CProtocolFrame::getBit(int offset)
// {
//     StuBit *pBit = (StuBit *)&mFrame[offset / 8];
//     switch (offset % 8)
//     {
//     case 0:
//         return pBit->bit0;
//     case 1:
//         return pBit->bit1;
//     case 2:
//         return pBit->bit2;
//     case 3:
//         return pBit->bit3;
//     case 4:
//         return pBit->bit4;
//     case 5:
//         return pBit->bit5;
//     case 6:
//         return pBit->bit6;
//     case 7:
//         return pBit->bit7;
//     }
//     return 0;
// }

void CProtocolFrame::setBit(int offset, int bit)
{
    StuBit *pBit = (StuBit *)&mFrame[offset / 8];
    switch (offset % 8)
    {
    case 0:
        pBit->bit0 = bit;
        break;
    case 1:
        pBit->bit1 = bit;
        break;
    case 2:
        pBit->bit2 = bit;
        break;
    case 3:
        pBit->bit3 = bit;
        break;
    case 4:
        pBit->bit4 = bit;
        break;
    case 5:
        pBit->bit5 = bit;
        break;
    case 6:
        pBit->bit6 = bit;
        break;
    case 7:
        pBit->bit7 = bit;
        break;
    }
}

unsigned int CProtocolFrame::reorder(unsigned int tmp, int nbit)
{
    unsigned int result = 0;
    int modulus = 0, remainder = 0;
    modulus = nbit % 8;
    remainder = nbit / 8;
    if ((modulus > 0) && (remainder > 0))
        remainder += 1;
    switch (remainder)
    {
    case 2:
    {
        result |= (tmp >> 8) & 0xFF;
        result |= ((tmp & 0xFF) << 8);
    }
    break;
    case 3:
    {
        result |= (tmp >> 16) & 0xFF;
        result |= (((tmp >> 8) & 0xFF) << 8);
        result |= (((tmp & 0xFF) << 16) & 0xFF0000);
    }
    break;
    case 4:
    {
        result |= (tmp >> 24) & 0xFF;
        result |= (((tmp >> 16) & 0xFF) << 8);
        result |= ((((tmp >> 8) & 0xFF) << 16) & 0xFF0000);
        result |= (((tmp & 0xFF) << 24) & 0xFF000000);
    }
    break;
    default:
        result = tmp;
        break;
    }

    return result;
}
