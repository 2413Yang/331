#ifndef HUFRAMEDECODER__H__
#define HUFRAMEDECODER__H__

#include <string>
#include <vector>
#include "HUFrame.h"

class CHUFrameDecoder : public CHUFrame
{
public:
    CHUFrameDecoder();
    virtual ~CHUFrameDecoder();
    std::string& operator()(std::string &frame);

protected:
    std::string mDecoded;
};

#endif /* HUFRAMEDECODER__H__ */