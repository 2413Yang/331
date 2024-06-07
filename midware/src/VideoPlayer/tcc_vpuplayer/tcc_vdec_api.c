//********************************************************************************************
/**
 * @file        tcc_vdec_api.c
 * @brief		Decode H264 frame and display image onto screen through overlay driver.
 *
 * @author      Telechips Shenzhen Inc.
 * @date        2021/10/08
 */
//********************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>

// these micros will be defined by gcc cli parameter
//#define CONFIG_ARCH_TCC897X 1
//#define CONFIG_TCC_INVITE   1
#include <mach/tcc_overlay_ioctl.h>
#include <mach/vioc_global.h>

#include "vpu_codec.h"
#include "vpu_dec.h"
#include "tcc_vdec_api.h"

// >> For Debug
//#define   DEBUG_MODE
#ifdef  DEBUG_MODE

    #define DebugPrint( fmt, ... )  fprintf(stderr, "tcc_vdec_api.c(D):"fmt, ##__VA_ARGS__ )
    #define ErrorPrint( fmt, ... )  fprintf(stderr, "tcc_vdec_api.c(E):"fmt, ##__VA_ARGS__ )

#else

    #define DebugPrint( fmt, ... )
    #define ErrorPrint( fmt, ... )  fprintf(stderr, "tcc_vdec_api.c(E):"fmt, ##__VA_ARGS__ )

#endif
// << For Debug

// Overlay Devices
#define OVERLAY_DRIVER  		"/dev/overlay"

// LCDC IOCTL command for set layer priority
#define OVERLAY_GET_WMIXER_OVP  70
#define OVERLAY_SET_WMIXER_OVP  80

static int g_OverlayDrv = -1;
static overlay_config_t  g_OverlayConfig;
static overlay_config_t *g_pOverlayConfig;

// View size
static int g_ViewX, g_ViewY, g_ViewW, g_ViewH;
static float g_ViewRatio;
static int g_ImgLayout = 0;	// 0: center: 1: stretch
static int g_ImgFormat = VIOC_IMG_FMT_YUV420SEP;	//VIOC_IMG_FMT_YUV420IL0;

// Decoder State
static int g_DecoderState = -1;

// Setting OVP(OVerlay Priority)
//   Default OVP (HMI on Top):
//      TCC897x:    Image3, OVP=24 (image(RDMA) order: 0,1,2,3)
//   Video Layer On Top:
//      TCC897x:    Image3, OVP=00 (image(RDMA) order: 3,0,1,2)
#define MOVE_VIDEO_ON_TOP	0

static int g_SavedOVP;
static int g_NewOVP;

// Mutex
static pthread_mutex_t g_Mutex = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************************
        (0,0)                     
        +==============================================================+
        |                                                              |
        |   (g_ViewX, g_ViewY)                                         |
        |       +----+---------------------+----+                      |
        |       |    |                     |    |                      |
        |       |    |                     |    |                      |
        |       |    |        image        |    |g_ViewH               |
        |       |    |                     |    |                      |
        |       |    |                     |    |                      |
        |       +----+---------------------+----+                      |
        |                    g_ViewW                                   |
        |                                                              |
        +--------------------------------------------------------------+
                                 screen(LCD)
        Note:
        - view  : pos=(g_ViewX, g_ViewY) in screen(LCD), size=(g_ViewW, g_ViewH)
        - image : always at center of view, you can:
           	      1) keep image ratio
               	  2) stretch to full view
*******************************************************************************/
int tcc_vdec_init(uint32_t view_pos, uint32_t view_size, uint32_t img_layout, uint32_t img_format, int videoOnTop)
{
    pthread_mutex_lock(&g_Mutex);
    
    memset(&g_OverlayConfig, 0, sizeof(overlay_config_t));
    g_pOverlayConfig = &g_OverlayConfig;

    g_NewOVP = videoOnTop ? MOVE_VIDEO_ON_TOP : 24;
    // view position & size
    g_ViewX = view_pos >> 16;
    g_ViewY = view_pos & 0xFFFF;
    g_ViewW = view_size >> 16;
    g_ViewH = view_size & 0xFFFF;
	g_ImgLayout = img_layout;
	g_ImgFormat = img_format;

    g_ViewRatio = (float)g_ViewW/(float)g_ViewH;
    
	// VPU not oppened
    g_DecoderState = -1;
    
    // init overlay
    g_OverlayDrv = -1;
    g_pOverlayConfig->wmxX = g_ViewX;
    g_pOverlayConfig->wmxY = g_ViewY;
    g_pOverlayConfig->wmxW = g_ViewW;
    g_pOverlayConfig->wmxH = g_ViewH;
    g_pOverlayConfig->imgFormat = g_ImgFormat;

    pthread_mutex_unlock(&g_Mutex);
    return 0;
}

int tcc_vdec_open(void)
{
    int iRetValue;
	VPU_DEC_InitParam_T tInitParam;

    pthread_mutex_lock(&g_Mutex);

	// if already opened, close it
    if (g_DecoderState>=0) 
		VPU_DEC_Deinit();

	tInitParam.iWidth				= g_ViewW;
	tInitParam.iHeight 				= g_ViewH;
	tInitParam.iStreamBufferSize    = 0;
	tInitParam.eCodec 				= VPU_CODEC_Codec_H264;		//set just for h264
	tInitParam.eFormat 				= VPU_CODEC_Format_YUV420P;
	
	if((g_DecoderState = VPU_DEC_Init(&tInitParam)) < 0)
	{
		ErrorPrint( "tcc_vdec_open()->VPU_DEC_Init() failed!\n" );
		iRetValue = -1;
		goto opening_ret;
	}
	
    usleep(50000);		// must delay for a while -- don't know why

    // open overlay device
    if (g_OverlayDrv>=0)
		close(g_OverlayDrv);
	
    g_OverlayDrv = open(OVERLAY_DRIVER, O_RDWR);
    if ( g_OverlayDrv < 0 ) {
        ErrorPrint( "Error : Overlay Driver Open Fail\n" );
        iRetValue = -2;
        goto opening_ret;
    }

#if defined(MOVE_VIDEO_ON_TOP)
//	int l_newOVP = MOVE_VIDEO_ON_TOP;
	ioctl(g_OverlayDrv, OVERLAY_GET_WMIXER_OVP, &g_SavedOVP); //backup default_ovp
    if(g_NewOVP != g_SavedOVP)
	{
        ioctl(g_OverlayDrv, OVERLAY_SET_WMIXER_OVP, &g_NewOVP);	//set the new ovp value
    }
	//DebugPrint("Prvious OVP = %d \n", g_SavedOVP);
#endif
	iRetValue = 0;

opening_ret:
    pthread_mutex_unlock(&g_Mutex);
    return iRetValue;
}

int tcc_vdec_close(void)
{
    pthread_mutex_lock(&g_Mutex);
    
    DebugPrint( "tcc_vdec_close()!\n" );
    
#if defined(MOVE_VIDEO_ON_TOP)
    // disable the FB device to mix overlay layer
	ioctl(g_OverlayDrv, OVERLAY_SET_WMIXER_OVP, &g_SavedOVP);	//restore OVP
#endif
    
    if( g_OverlayDrv >= 0 ) {
        //ioctl( g_OverlayDrv, OVERLAY_SET_DISABLE, 0 );
        close(g_OverlayDrv);
    }
    g_OverlayDrv = -1;

	usleep(50000);
    
    if( g_DecoderState >= 0 ){
        VPU_DEC_Deinit();
    }
    g_DecoderState = -1;

    pthread_mutex_unlock(&g_Mutex);
    return 0;
}

int tcc_vdec_is_ready(void)
{
	int iRetValue=1;
#if 0
    pthread_mutex_lock(&g_Mutex);
    retv = tcc_vpudec_ready_to_recv();
    pthread_mutex_unlock(&g_Mutex);
#endif
    return iRetValue;
}

//static buffer_cnt = 0;
int tcc_vdec_process(unsigned        char *pucBuffer, int nSize)
{
    int iRetValue = 0;
	unsigned int scaled_w, scaled_h;
	VPU_DEC_InputParam_T  tInputParam;
	VPU_DEC_OutputParam_T tOutputParam;
	
    pthread_mutex_lock(&g_Mutex);

    if ( g_DecoderState == -1 ) {
        ErrorPrint( "decoder is not opened...\n" );
		goto decodig_ret;
    }

	tInputParam.pucBuffer = pucBuffer;
	tInputParam.iBufferSize = nSize;
	tInputParam.llTimeStamp = 0;
	
	if((iRetValue = VPU_DEC_Decode(&tInputParam, &tOutputParam)) < 0) {
		ErrorPrint("VPU_DEC_Decode() error!!\n");
		goto decodig_ret;
	}
	
	if(tOutputParam.bFrameOut == VPU_CODEC_FALSE) {
		DebugPrint("VPU_DEC_Decode() suceed, but no frame out!\n");
		iRetValue = 0;
		goto decodig_ret;
	}

    if ( g_OverlayDrv >= 0 ) {
		// config RDMA
        g_pOverlayConfig->imgAddr   = (unsigned int)tOutputParam.pucBufferPhy[0];	// Y address;
        g_pOverlayConfig->imgAddr1  = (unsigned int)tOutputParam.pucBufferPhy[1];	// U address
        g_pOverlayConfig->imgAddr2  = (unsigned int)tOutputParam.pucBufferPhy[2];	// V address
        g_pOverlayConfig->imgFormat = (unsigned int)tOutputParam.eFormat;
        g_pOverlayConfig->imgWidth  = (unsigned int)tOutputParam.iWidth;	// this is cropped width
        g_pOverlayConfig->imgHeight = (unsigned int)tOutputParam.iHeight;	// this is cropped height
		g_pOverlayConfig->imgStride = (unsigned int)tOutputParam.iStride;
		
		DebugPrint("tOutputParam: image(%d,%d), fmt=%d, crop(%d,%d,%d,%d), stride=%d, pic=%d, FrameOut=%d, idx=%d\n", 
				tOutputParam.iWidth, tOutputParam.iHeight, tOutputParam.eFormat,
				tOutputParam.crop_top, tOutputParam.crop_left,
				tOutputParam.crop_right, tOutputParam.crop_bottom, 
				tOutputParam.iStride, tOutputParam.ePictureType, 
				tOutputParam.bFrameOut, tOutputParam.iDispFrameIdx);
		
		// config SCALER
		if (g_ImgLayout == 0) {
			// image align to view cnter
	        float ratio = (float)tOutputParam.iWidth/(float)tOutputParam.iHeight;
	        if (ratio >= g_ViewRatio) {
	            // video ratio >= view ratio, scale to LCD width
	            scaled_w = g_ViewW;
	            scaled_h = (int)((float)g_ViewW / ratio);
	        }
	        else {
	            // scale to view height
	            scaled_w = (int)((float)g_ViewH * ratio);
	            scaled_h = g_ViewH;
	        }

	        g_pOverlayConfig->wmxX = g_ViewX + (g_ViewW-scaled_w)/2;
	        g_pOverlayConfig->wmxY = g_ViewY + (g_ViewH-scaled_h)/2;
		}
		else {
			// stretch the image to view size
			scaled_w = g_ViewW;
			scaled_h = g_ViewH;
			g_pOverlayConfig->wmxX = g_ViewX;
			g_pOverlayConfig->wmxY = g_ViewY;
		}
        g_pOverlayConfig->scDestW = scaled_w;
        g_pOverlayConfig->scDestH = scaled_h;
	    g_pOverlayConfig->scDispW = scaled_w;
        g_pOverlayConfig->scDispH = scaled_h;
		g_pOverlayConfig->scDispX = 0;
		g_pOverlayConfig->scDispY = 0;
		DebugPrint("scaled pos:(%d,%d), size:(%d,%d) \n", g_pOverlayConfig->wmxX, g_pOverlayConfig->wmxY, scaled_w, scaled_h);
	    // push the image, if needed, the driver will configure the overlay
	    ioctl( g_OverlayDrv, OVERLAY_PUSH_VIDEO_BUFFER, g_pOverlayConfig );
		//DebugPrint( "buffer_cnt=%d\n", buffer_cnt++);
    }
    else {
        DebugPrint( "Decode but Overlay Driver is not opened\n" );
		iRetValue = 0;
    }
	
	VPU_DEC_ReleaseDecodedFrame(tOutputParam.iDispFrameIdx);

decodig_ret:
    pthread_mutex_unlock(&g_Mutex);
    return iRetValue;
}

