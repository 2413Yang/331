#include "mlog.h"
#include "HUFrame.h"
#include "HUFrameEncoder.h"

CHUFrameEncoder::CHUFrameEncoder()
{
}

CHUFrameEncoder::~CHUFrameEncoder()
{
}

std::string CHUFrameEncoder::operator()(int fid, HUFData *pData, size_t dataLen)
{
    std::vector<uint8_t> frameHeader = {0xA5, 0xFF, 0x5A, 0xFF};
    std::vector<uint8_t> frameTailer = {0x5A, 0xFF, 0xA5, 0xFF};
    std::vector<uint8_t> buffer(sizeof(HUFHead) + dataLen);
    HUFrameStruct *pFrame = (HUFrameStruct *)buffer.data();

    pFrame->head = {
        tag : {frameHeader[0], frameHeader[1], frameHeader[2], frameHeader[3]},
        type : ((uint32_t)fid & 0xff00) >> 8,
        sub_type : ((uint32_t)fid & 0x00ff),
        sub_type_1 : 0,
        len : (uint32_t)dataLen
    };
    memcpy(&pFrame->data, pData, dataLen);
    buffer.insert(buffer.end(), frameTailer.begin(), frameTailer.end());

    mFrame.clear();
    mFrame.assign(buffer.begin(), buffer.end());

    return mFrame;
}
