#include "videoPlayer.h"

videoPlayer::videoPlayer(uint32_t pos, uint32_t viewSize, int videoOnTop)
{
    this->strFileName[0] = '\0';
    this->pTempBuffer = nullptr;
    this->lastPlayTick = getSysTimeMS() - c_MinFrameIntervalMs;
    this->videoFile = nullptr;
    vpuPlayer_init(pos, viewSize, videoOnTop);
}
videoPlayer::videoPlayer(const char* fileName, uint32_t pos, uint32_t viewSize, int videoOnTop)
{
    strncpy(this->strFileName, fileName, sizeof(this->strFileName));
    this->videoFile = fopen(fileName, "r");
    pTempBuffer = (uint8_t*)malloc(254*1024);
    vpuPlayer_init(pos, viewSize, videoOnTop);
}

videoPlayer::~videoPlayer()
{
    if(this->videoFile)
    {
        fclose(this->videoFile);
    }
    if(this->pTempBuffer)
    {
        free(this->pTempBuffer);
    }
    tcc_vdec_close();
}

#include <time.h>
uint32_t videoPlayer::getSysTimeMS()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	uint32_t currentTime = (uint32_t) (t.tv_nsec / (1000 * 1000) + ((uint64_t)t.tv_sec  * 1000));
	return currentTime;
}

int videoPlayer::vpuPlayer_init(uint32_t pos, uint32_t viewSize, int videoOnTop)
{
    if((this->iViewX + this->iViewW) > this->iScreenW)
    {
        this->iViewW = this->iScreenW - this->iViewX;
    }
    if((this->iViewY + this->iViewH) > this->iScreenH)
    {
        this->iViewH = this->iScreenH - this->iViewY;
    }
    int ret = 0;
    ret = tcc_vdec_init(pos, viewSize, 1, VIOC_IMG_FMT_YUV420SEP, videoOnTop);
    if(ret)
    {
        return ret;
    }
	ret = tcc_vdec_open();
    if(ret)
    {
        return ret;
    }
    return ret;
}

void videoPlayer::videoPlayerResetFrame()
{
	if(this->videoFile)
	{
		fseek(this->videoFile, 0, SEEK_SET);
	}
}
void videoPlayer::setFileName(const char* fileName)
{
	if(this->videoFile)
	{
		fclose(videoFile);
		videoFile = fopen(fileName, "r");
	}
}
int videoPlayer::videoPlayerNextFrame()
{
    if(nullptr == this->videoFile)
    {
        return - 1;
    }
    if(nullptr == this->pTempBuffer)
    {
        return -1;
    }
#if 0
    uint32_t nowTick = getSysTimeMS();
    if((nowTick - this->lastPlayTick) < c_MinFrameIntervalMs)
    {
        return -1;
    }
    this->lastPlayTick = nowTick;
#endif
    //
    int ret = 0;
    typedef union 
    {
        int frameHead[2];
        struct head
        {
            int iFrameSize;
            int iPadSize;
        }stHeadInfo;
        
    }FrameHeadInfo;

    FrameHeadInfo headInfo;
    
    // get one frame from input file
    if ( (ret=fread(&headInfo, sizeof(int), 2, this->videoFile)) <=0)
    {
		return -1;
	}
    if ((ret=fread(this->pTempBuffer, sizeof(unsigned char), headInfo.stHeadInfo.iFrameSize, this->videoFile))<=0) 
    {
		return -1;
	}
    if (headInfo.stHeadInfo.iPadSize)
        fseek(this->videoFile, headInfo.stHeadInfo.iPadSize, SEEK_CUR); // skip padding bytes
        
    // decode the frame		
    if((ret=tcc_vdec_process((unsigned char*)this->pTempBuffer, (int)headInfo.stHeadInfo.iFrameSize)) < 0) {
        printf("VPU_DEC_Decode() error!!\n");
        return ret;
    }
    return ret;
}
int videoPlayer::videoPlayerNextFrame(const uint8_t* frameData, int size)
{
    if(nullptr == frameData)
    {
        return -1;
    }
#if 0
    uint32_t nowTick = getSysTimeMS();
    if((nowTick - this->lastPlayTick) < c_MinFrameIntervalMs)
    {
        return -1;
    }
    this->lastPlayTick = nowTick;
#endif
    int ret = tcc_vdec_process((uint8_t*)frameData, size);
    if(ret < 0)
    {
        printf("VPU_DEC_Decode() error!!\n");
    }
    return ret;
}