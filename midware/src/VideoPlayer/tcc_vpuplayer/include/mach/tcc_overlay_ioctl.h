/****************************************************************************
 *	 TCC Version 0.6
 *	 Copyright (c) telechips, Inc.
 *	 ALL RIGHTS RESERVED
 *
****************************************************************************/

#ifndef _OVERLAY_H
#define _OVERLAY_H

#ifndef ADDRESS_ALIGNED
#define ADDRESS_ALIGNED
#define ALIGN_BIT (0x8-1)
#define BIT_0 3
#define GET_ADDR_YUV42X_spY(Base_addr)  	(((((unsigned int)Base_addr) + ALIGN_BIT)>> BIT_0)<<BIT_0)
#define GET_ADDR_YUV42X_spU(Yaddr, x, y)	(((((unsigned int)Yaddr+(x*y)) + ALIGN_BIT)>> BIT_0)<<BIT_0)
#define GET_ADDR_YUV422_spV(Uaddr, x, y)	(((((unsigned int)Uaddr+(x*y/2)) + ALIGN_BIT) >> BIT_0)<<BIT_0)
#define GET_ADDR_YUV420_spV(Uaddr, x, y)	(((((unsigned int)Uaddr+(x*y/4)) + ALIGN_BIT) >> BIT_0)<<BIT_0)
#endif

#define CONFIG_OVERLAY_CROP
#define CONFIG_OVERLAY_SCALE
#define OVERLAY_GET_WMIXER_OVP      70
#define OVERLAY_SET_WMIXER_OVP      80

#define OVERLAY_PUSH_VIDEO_BUFFER	90
#define OVERLAY_SET_CONFIGURE		50
#define OVERLAY_SET_LAYER			51

typedef struct
{
	// RDMA configure
	unsigned int imgAddr;    	// image address (Y)
	unsigned int imgAddr1;		// image address (U)
	unsigned int imgAddr2;		// image address (V)
	unsigned int imgWidth;		// image width  (not including padding)
	unsigned int imgHeight;		// image height (not icnluding padding)
	unsigned int imgStride;		// image stride (offset?)
	unsigned int imgFormat;		// image format (VIOC_IMG_FMT_YUV420IL0,...)

	// Scaler configure
	unsigned int scDestW;		// scaled image size
	unsigned int scDestH;
	unsigned int scDispX;		// display out start postion
	unsigned int scDispY;
	unsigned int scDispW;		// display out image size
	unsigned int scDispH;

	// WMIX configure
	unsigned int wmxX;			// scaled image display start position in WMIX
	unsigned int wmxY;
	unsigned int wmxW;
	unsigned int wmxH;
} overlay_config_t;

#endif
