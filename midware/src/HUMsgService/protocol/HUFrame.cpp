#include "HUFrame.h"
#include "mlog.h"
#include <string.h>

CHUFrame::CHUFrame() : mFrame("")
{
}

void CHUFrame::updateFrame(std::string &frame)
{
    mFrame = frame;
}

unsigned int CHUFrame::getID(void)
{
    if (mFrame.length() > sizeof(HUFHead)) {
        HUFrameStruct *pf = (HUFrameStruct*)mFrame.data();
        return HUFID(pf->head.type, pf->head.sub_type);        
    }

    return 0;
}