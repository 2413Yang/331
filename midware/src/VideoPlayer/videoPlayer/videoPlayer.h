#ifndef _VIDEOPLAYER_H_
#define _VIDEOPLAYER_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "videoPlay/tcc_vdec_api.h"

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN	(260)
#endif

#define DEFAULT_VIEW_POSTION_X  (0)
#define DEFAULT_VIEW_POSTION_Y  (0)
#define DEFAULT_VIEW_SIZE_W     (1920)
#define DEFAULT_VIEW_SIZE_H     (720)

class videoPlayer
{
private:
    const int  iScreenW = DEFAULT_VIEW_SIZE_W, iScreenH = DEFAULT_VIEW_SIZE_H;		// FB device size
    const int   c_MinFrameIntervalMs = 25;   //解码最小帧间隔，防止调用vpu解码器频率太高导致解码出错
	int  iViewX, iViewY;			// view position
	int  iViewW, iViewH;			// view size
	char strFileName[MAX_PATH_LEN];	// directory/file name
	int  iPlayType;					// 0: embedded video; 1: directory
	int  iDispDelay;				// unit: ms
	int  iDispMode;					// 0: center, 1: stretch	
	int  iImageFmt;					// video format (ex: VIOC_IMG_FMT_YUV420IL0, see vioc_global.h)
    FILE*   videoFile;
    uint32_t    lastPlayTick;       //最后一次解码播放一帧的时间
    uint8_t*    pTempBuffer;
private:
    int vpuPlayer_init(uint32_t pos, uint32_t viewSize, int videoOnTop);

    
    
public:
    videoPlayer() = delete;
    videoPlayer(uint32_t pos, uint32_t viewSize, int videoOnTop = 0);
    videoPlayer(const char* fileName, uint32_t pos = ((DEFAULT_VIEW_POSTION_X << 16) | DEFAULT_VIEW_POSTION_Y), uint32_t viewSize = ((DEFAULT_VIEW_SIZE_W << 16) | DEFAULT_VIEW_SIZE_H), int videoOnTop = 1);
    ~videoPlayer();

    static uint32_t getSysTimeMS();

    int videoPlayerNextFrame();
    int videoPlayerNextFrame(const uint8_t* frameData, int size);
	void videoPlayerResetFrame();
	void setFileName(const char* fileName);
};



#endif