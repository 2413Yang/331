#include "mlog.h"
#include "HUFrame.h"
#include "HUFrameDecoder.h"

CHUFrameDecoder::CHUFrameDecoder()
{
}

CHUFrameDecoder::~CHUFrameDecoder()
{
}

std::string& CHUFrameDecoder::operator()(std::string &frame)
{
    this->updateFrame(frame);

    HUFrameStruct *pFrame = (HUFrameStruct *)mFrame.data();
    mDecoded.assign(frame, sizeof(HUFHead), pFrame->head.len);
    return mDecoded;
}