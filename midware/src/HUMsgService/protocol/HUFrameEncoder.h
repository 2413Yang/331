#ifndef HUFRAMEENCODER__H__
#define HUFRAMEENCODER__H__

#include <string>
#include "HUFrame.h"

class CHUFrameEncoder : public CHUFrame
{
public:
    CHUFrameEncoder();
    virtual ~CHUFrameEncoder();
    virtual std::string operator()(int fid, HUFData *pData, size_t dataLen);    
};

#endif /* HUFRAMEENCODER__H__ */