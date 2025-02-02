/*!
 ***********************************************************************
 \par Copyright
 \verbatim
  ________  _____           _____   _____           ____  ____   ____		
     /     /       /       /       /       /     /   /    /   \ /			
    /     /___    /       /___    /       /____ /   /    /____/ \___			
   /     /       /       /       /       /     /   /    /           \		
  /     /_____  /_____  /_____  /_____  /     / _ /_  _/_      _____/ 		
   																				
  Copyright (c) 2009 Telechips Inc.
  Luther Bldg, 7-20 Sincheon-dong, Songpa-gu, Seoul, Korea
 \endverbatim
 ***********************************************************************
 */
/*!
 ***********************************************************************
 *
 * \file
 *		TCC_VPU_D6.h
 * \date
 *		2015/05/20
 * \author
 *		AV algorithm group(AValgorithm@telechips.com) 
 * \brief
 *		main api
 * \version
 *		0.00(2015/05/20) : first beta release
 *
 ***********************************************************************
 */
#ifndef _TCC_VPU_D6_H_
#define _TCC_VPU_D6_H_

#include "TCCxxxx_VPU_CODEC_COMMON.h"

//#define USE_VPU_DISPLAY_MODE	//! use ring buffer

#define MAX_NUM_INSTANCE		4

#define RETCODE_ERR_SEQ_HEADER_NOT_FOUND	 31
#define RETCODE_ERR_STRIDE_ZERO_OR_ALIGN8	100
#define RETCODE_ERR_MIN_RESOLUTION			101
#define RETCODE_ERR_MAX_RESOLUTION			102
#define RETCODE_ERR_SEQ_INIT_HANGUP			103
#define RETCODE_ERR_CHROMA_FORMAT			104
#define RETCODE_H264ERR_PROFILE				110
#define RETCODE_VC1ERR_COMPLEX_PROFILE		120
#define RETCODE_H263ERR_ANNEX_D				130
#define RETCODE_H263ERR_ANNEX_EFG			131
#define RETCODE_H263ERR_UFEP				132
#define RETCODE_H263ERR_ANNEX_D_PLUSPTYPE	133
#define RETCODE_H263ERR_ANNEX_EF_PLUSPTYPE	134
#define RETCODE_H263ERR_ANNEX_NRS_PLUSPTYPE	135
#define RETCODE_H263ERR_ANNEX_PQ_MPPTYPE	136
#define RETCODE_H263ERR_UUI					137	//Unlimited Unrestricted Motion Vectors Indicator(UUI)
#define RETCODE_H263ERR_SSS					138	//Slice Structure Submode(Rectangular Slices or Arbitaray Slice Ordering)
#define RETCODE_H263ERR_PIC_SIZE			139
#define RETCODE_MPEG4ERR_OBMC_DISABLE		140
#define RETCODE_MPEG4ERR_SPRITE_ENABLE		141	//Sprite Coding Mode(include GMC)
#define RETCODE_MPEG4ERR_MP4ERR_SCALABILITY	142	//Scalable coding
#define RETCODE_MPEG4ERR_SPK_FORMAT			144
#define RETCODE_MPEG4ERR_SPK_VERSION		145
#define RETCODE_MPEG4ERR_SPK_RESOLUTION		146
#define RETCODE_MPEG4ERR_PACKEDPB			147
#define RETCODE_MPEG4ERR_PIC_SIZE			148
#define RETCODE_MPEG2ERR_CHROMA_FORMAT		150
#define RETCODE_MPEG2ERR_PROFILE			151
#define RETCODE_MPEG2ERR_SEQ_PIC_WIDTH_SPEC_OVER	152	// widht over 1920
#define RETCODE_MPEG2ERR_SEQ_PIC_HEIGHT_SPEC_OVER	153	// height over 1152

#define RETCODE_WRAP_AROUND					-10

#define PS_SAVE_SIZE		0x080000
#define CODE_BUF_SIZE		(264*1024)
#define TEMP_BUF_SIZE		(512*1024)
#define WORK_BUF_SIZE		(80*1024)
#define PARA_BUF_SIZE		(10*1024)
#define SEC_AXI_BUF_SIZE	(128*1024)

#define SIZE_BIT_WORK	TEMP_BUF_SIZE + PARA_BUF_SIZE + CODE_BUF_SIZE + SEC_AXI_BUF_SIZE

#define WORK_CODE_PARA_BUF_SIZE		(SIZE_BIT_WORK + (WORK_BUF_SIZE * MAX_NUM_INSTANCE))

#define	LARGE_STREAM_BUF_SIZE	0x200000
#define	SLICE_SAVE_SIZE			0x180000
#define VP8_MB_SAVE_SIZE		0x100000


//------------------------------------------------------------------------------
// decode struct and definition
//------------------------------------------------------------------------------
//! represents rectangular window information in a frame 
typedef struct pic_crop_t
{
    unsigned int m_iCropLeft;
    unsigned int m_iCropTop;
    unsigned int m_iCropRight;
    unsigned int m_iCropBottom;
} pic_crop_t;

//-----------------------------------------------------
// data structure to get information necessary to 
// start decoding from the decoder (this is an output parameter)
//-----------------------------------------------------
typedef struct dec_initial_info_t     
{
	int m_iPicWidth;				//!< {(PicX+15)/16} * 16  (this width  will be used while allocating decoder frame buffers. picWidth  is a multiple of 16)
	int m_iPicHeight;				//!< {(PicY+15)/16} * 16  (this height will be used while allocating decoder frame buffers. picHeight is a multiple of 16)
	unsigned int m_uiFrameRateRes;	//!< decoded picture frame rate residual(number of time units of a clock operating at the frequency[m_iFrameRateDiv] Hz, frameRateInfo = m_uiFrameRateRes/m_uiFrameRateDiv
	unsigned int m_uiFrameRateDiv;	//!< decoded picture frame rate unit number in Hz				
	int m_iMinFrameBufferCount;		//!< the minimum number of frame buffers that are required for decoding. application must allocate at least this number of frame buffers.
	int m_iMinFrameBufferSize;		//!< minimum frame buffer size
	int	m_iNormalSliceSize;			//!< recommended size of to save slice. this value is determined as a quater of the memory size for one raw YUV image in KB unit.
	int	m_iWorstSliceSize;			//!< recommended size of to save slice in worst case. this value is determined as a half of the memory size for one raw YUV image in KB unit.

	//! H264/AVC only param
	pic_crop_t m_iAvcPicCrop;		//!< represents rectangular window information in a frame 
	int m_iAvcConstraintSetFlag[4];	//!< syntax element which is used to make level in H.264. used only in H.264 case. 
	int m_iAvcParamerSetFlag;		//!< These are H.264 SPS or PPS syntax element
									//!< [bit 0  ] direct_8x8_inference_flag in H.264 SPS
	int m_iFrameBufDelay;			//!< maximum display frame buffer delay to process reordering in case of H.264

	//! MPEG-4 only param
	int m_iM4vDataPartitionEnable;	//!< ( 0: disable   1: enable )
	int m_iM4vReversibleVlcEnable;	//!< ( 0: disable   1: enable )
	int m_iM4vShortVideoHeader;		//!< ( 0: disable   1: enable )
	int m_iM4vH263AnnexJEnable;		//!< ( 0: disable   1: enable )

	//! VC-1 only param
	int m_iVc1Psf;					//!< this is only available in VC1 and indicates the value of "Progressive Segmented Frame" 

	//! Additional Info
	int m_iProfile;					//!< profile of the decoded stream
	int m_iLevel;					//!< level of the decoded stream
	int m_iInterlace;				//!< when this value is 1, decoded stream will be decoded into both progressive or interlace frame. otherwise, all the frames will be progressive. In H.264, this is frame_mbs_only_flag.
	int m_iAspectRateInfo;			//!< aspect rate information. if this value is 0, then aspect ratio information is not present 
	int m_iReportErrorReason;		//!< reports the reason of 'RETCODE_CODEC_SPECOUT' or 'RETCODE_INVALID_STRIDE' error

	unsigned int m_Reserved[39];		//! Reserved.
} dec_initial_info_t;

//! data structure for initializing Video unit
typedef struct dec_init_t 
{
	codec_addr_t m_BitWorkAddr[2];		//!< physical[0] and virtual[1] address of a working space of the decoder. This working buffer space consists of work buffer, code buffer, and parameter buffer.
	codec_addr_t m_RegBaseVirtualAddr;	//!< virtual address BIT_BASE

	//! Bitstream Info
	int m_iBitstreamFormat;				//!< bitstream format
	codec_addr_t m_BitstreamBufAddr[2];	//!< bitstream buf address : multiple of 4
	int m_iBitstreamBufSize;			//!< bitstream buf size	   : multiple of 1024
	int m_iPicWidth;					//!< frame width from demuxer or etc
	int m_iPicHeight;					//!< frame height from demuxer or etc

	//! Decoding Options
#define M4V_DEBLK_DISABLE		0	//!< (default)
#define M4V_DEBLK_ENABLE		1	//!< mpeg-4 deblocking
#define M4V_GMC_FILE_SKIP		(0<<1)	//!< (default) seq.init failure
#define M4V_GMC_FRAME_SKIP		(1<<1)	//!< frame skip without decoding
#define AVC_FIELD_DISPLAY		(1<<3)	//!< if only field is fed, display it
#define MVC_DEC_ENABLE			(1<<20)	//!< H.264 MVC enable
#define SEC_AXI_BUS_ENABLE_SRAM	(1<<21) //!< Use SRAM for sec. AXI bus

	unsigned int m_uiDecOptFlags;

	//! H264 only param
	unsigned char* m_pSpsPpsSaveBuffer;	//!< multiple of 4
	int m_iSpsPpsSaveBufferSize;		//!< multiple of 1024

	//! VPU Control 
	unsigned int m_bEnableUserData;		//!< If this is set, VPU returns userdata.
	unsigned int m_bCbCrInterleaveMode;	//!< 0 (chroma separate mode   : CbCr data is written in separate frame memories)
										//!< 1 (chroma interleave mode : CbCr data is interleaved in chroma memory)
	int m_iFilePlayEnable;				//!< enable file play mode. If this value is set to 0, streaming mode with ring buffer will be used

#define RESOLUTION_1080_HD	0			//1920x1088
#define RESOLUTION_720P_HD	1			//1280x720
#define RESOLUTION_625_SD	2			//720x576
	int m_iMaxResolution;				//!< maximum resolution limitation option

	//! Callback Func
	void* (*m_Memcpy ) ( void*, const void*, unsigned int );	//!< memcpy
	void  (*m_Memset ) ( void*, int, unsigned int );			//!< memset
	int   (*m_Interrupt ) ( void );								//!< hw interrupt (return value is always 0)
	unsigned int (*m_Ioremap ) ( unsigned int, unsigned int );
	void  (*m_Iounmap ) ( unsigned int );

	unsigned int m_Reserved[11];		//! Reserved.
} dec_init_t;

typedef struct dec_input_t 
{
	codec_addr_t m_BitstreamDataAddr[2];//!< bitstream data address
	int m_iBitstreamDataSize;			//!< bitstream data size
	codec_addr_t m_UserDataAddr[2];		//!< Picture Layer User-data address
	int m_iUserDataBufferSize;			//!< Picture Layer User-data Size

	int m_iFrameSearchEnable;			//!< I-frame Search Mode
										//!< If this option is enabled, then decoder performs skipping frame decoding until decoder meet IDR(and/or I)-picture for H.264 or I-frame.
										//!< If this option is enabled, m_iSkipFrameMode option is ignored.
										//!< 0x000 ( Disable )
										//!< 0x001 ( Enable : search IDR-picture for H.264 or I-frame ) 
										//!< 0x201 ( Enable : search I(or IDR)-picture for H.264 or I-frame ) 

	int m_iSkipFrameMode;				//!< Skip Frame Mode
										//!< 0 ( Skip disable )
										//!< 1 ( Skip except I(IDR) picture )
										//!< 2 ( Skip B picture : skip if nal_ref_idc==0 in H.264 )
										//!< 3 ( Unconditionally Skip one picture )

	int m_iSkipFrameNum;				//!< Number of skip frames (for I-frame Search Mode or Skip Frame Mode)
										//!< When this number is 0, m_iSkipFrameMode option is disabled.
	unsigned int m_Reserved[23];		//! Reserved.
} dec_input_t;

typedef struct dec_buffer_t
{
	codec_addr_t m_FrameBufferStartAddr[2];	//!< physical[0] and virtual[1] address of a frame buffer of the decoder.
	int m_iFrameBufferCount;				//!< allocated frame buffer count
	codec_addr_t m_AvcSliceSaveBufferAddr;	//!< start address and size of slice save buffer which the decoder can save slice RBSP : multiple of 4
	int m_iAvcSliceSaveBufferSize;			//!< multiple of 1024
	codec_addr_t m_Vp8MbDataSaveBufferAddr;	//!< start address and size of mb save buffer which the decoder can save mb data : multiple of 4
	int m_iVp8MbDataSaveBufferSize;			//!< multiple of 1024
	unsigned int m_Reserved[25];			//! Reserved.
} dec_buffer_t;

typedef struct dec_buffer2_t
{
	codec_addr_t	m_addrFrameBuffer[2][32];		//!< physical[0] and virtual[1] address of a frame buffer of the decoder.
	unsigned long	m_ulFrameBufferCount;			//!< allocated frame buffer count
	codec_addr_t	m_addrAvcSliceSaveBuffer;		//!< start address and size of slice save buffer which the decoder can save slice RBSP : multiple of 4
	unsigned long	m_ulAvcSliceSaveBufferSize;		//!< multiple of 1024
	codec_addr_t	m_addrVp8MbDataSaveBuffer;		//!< start address and size of mb save buffer which the VP8 decoder can save mb data : multiple of 4
	unsigned long	m_ulVp8MbDataSaveBufferSize;	//!< multiple of 1024
} dec_buffer2_t;

typedef struct dec_ring_buffer_setting_in_t
{
	codec_addr_t m_OnePacketBufferAddr;
	unsigned int m_iOnePacketBufferSize;
} dec_ring_buffer_setting_in_t;

typedef struct dec_ring_buffer_status_out_t
{
	unsigned long m_ulAvailableSpaceInRingBuffer;
	codec_addr_t m_ptrReadAddr_PA;
	codec_addr_t m_ptrWriteAddr_PA;
} dec_ring_buffer_status_out_t;

// MVC specific picture information
typedef struct MvcPictureInfo_t
{
    int m_iViewIdxDisplay;
    int m_iViewIdxDecoded;
} MvcPictureInfo_t;

// AVC specific SEI information (frame packing arrangement SEI)
typedef struct MvcAvcFpaSei_t
{
    unsigned int m_iExist;
    unsigned int m_iFrame_Packing_Arrangement_Id;
    unsigned int m_iFrame_Packing_Arrangement_Cancel_Flag; 
    unsigned int m_iQuincunx_Sampling_Flag;
    unsigned int m_iSpatial_Flipping_Flag;
    unsigned int m_iFrame0_Flipped_Flag;
    unsigned int m_iField_Views_Flag;
    unsigned int m_iCurrent_Frame_Is_Frame0_Flag;
    unsigned int m_iFrame0_Self_Contained_Flag;
    unsigned int m_iFrame1_Self_Contained_Flag;
    unsigned int m_iFrame_Packing_Arrangement_Extension_Flag;
    unsigned int m_iFrame_Packing_Arrangement_Type;
    unsigned int m_iContent_Interpretation_Type;
    unsigned int m_iFrame0_Grid_Position_X;
    unsigned int m_iFrame0_Grid_Position_Y;
    unsigned int m_iFrame1_Grid_Position_X;
    unsigned int m_iFrame1_Grid_Position_Y;
    unsigned int m_iFrame_Packing_Arrangement_Repetition_Period;
} MvcAvcFpaSei_t;

typedef struct Vp8DecScaleInfo_t
{
    unsigned int m_iHscaleFactor;
    unsigned int m_iVscaleFactor;
    unsigned int m_iPicWidth;
    unsigned int m_iPicHeight;
} Vp8DecScaleInfo_t;

typedef struct Vp8DecPicInfo_t
{
    unsigned int m_iShowFrame;
    unsigned int m_iVersionNumber;
    unsigned int m_iRefIdxLast;
    unsigned int m_iRefIdxAltr;
    unsigned int m_iRefIdxGold;
} Vp8DecPicInfo_t;

//-----------------------------------------------------
// data structure to get resulting information from 
// VPU after decoding a frame
//-----------------------------------------------------

typedef struct dec_output_info_t
{
	int m_iPicType;					//!< ( 0- I picture,  1- P picture,  2- B picture )

	int m_iDispOutIdx;				//!< index of output frame buffer
	int m_iDecodedIdx;				//!< index of decoded frame buffer

	int m_iOutputStatus;
	int m_iDecodingStatus;

	int m_iInterlacedFrame;			//!< Interlaced Frame 
	int m_iNumOfErrMBs;				//!< number of error macroblocks

	//! VC-1 Only
	int m_iVc1HScaleFlag;			//!< Flag for reduced resolution output in horizontal direction (in case of VC1 )
	int m_iVc1VScaleFlag;		    //!< Flag for reduced resolution output in vertical direction (in case of VC1 )

	//! MPEG-2 Only
	int m_iM2vProgressiveFrame;		//!< indicate progressive frame in picture coding extention in MPEG2
	int m_iM2vProgressiveSequence;
	int m_iM2vFieldSequence;		//!< indicate field sequence in picture coding extention in MPEG2  
	int m_iM2vAspectRatio;
	int m_iM2vFrameRate;

	//! More Info
	int m_iPictureStructure;		//!< indicates that the decodec picture is progressive or interlaced picture 
	int m_iTopFieldFirst;			//!< if this value is 1, top field is first decoded and then bottom field is decoded.  
	int m_iRepeatFirstField;		//!< repeat first field. this value is used during display process. 

	//! VP8 Only
	Vp8DecScaleInfo_t m_Vp8ScaleInfo;
	Vp8DecPicInfo_t m_Vp8PicInfo;

	//! MVC Only
	MvcPictureInfo_t m_MvcPicInfo; 
	MvcAvcFpaSei_t m_MvcAvcFpaSei;

	int m_iHeight;					//!< Height of input bitstream. In some cases, this value can be different from the height of previous frames.
	int m_iWidth;					//!< Width of input bitstream. In some cases, this value can be different from the height of previous frames.
	pic_crop_t m_CropInfo;			//!< Cropping information.

	//! User Data Buffer Address
	codec_addr_t m_UserDataAddress[2];	//!< If contents have picture-layer user-data, return it.

	int m_iConsumedBytes;			//!< consumed bytes (only for file play mode, m_iFilePlayEnable=1)

	int m_iInvalidDispCount;		//!< counter of consecutive display index error

	unsigned int m_Reserved[36];		//! Reserved. 

} dec_output_info_t;

typedef struct dec_output_t 
{
	dec_output_info_t m_DecOutInfo;
	unsigned char* m_pDispOut[2][3]; //! physical[0] and virtual[1] display  address of Y, Cb, Cr component
	unsigned char* m_pCurrOut[2][3]; //! physical[0] and virtual[1] current  address of Y, Cb, Cr component
	unsigned char* m_pPrevOut[2][3]; //! physical[0] and virtual[1] previous address of Y, Cb, Cr component
} dec_output_t;

/*!
 ***********************************************************************
 * \brief
 *		TCC_VPU_DEC		: main api function of libvpudec
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: libvpudec's handle
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or info parameter
 * \return
 *		If successful, TCC_VPU_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
codec_result_t
TCC_VPU_DEC( int Op, codec_handle_t* pHandle, void* pParam1, void* pParam2 );


typedef struct enc_rc_init_t 
{
	////////AVC Deblocking Filter Related/////////
	int m_iDeblkDisable;//!< 0 : Enable, 1 : Disable, 2 Disable at slice boundary
	int m_iDeblkAlpha;//!< deblk_filter_offset_alpha (-6 ~ 6)
	int m_iDeblkBeta;//!< deblk_filter_offset_beta (-6 ~ 6)
	int m_iDeblkChQpOffset;//!< chroma_qp_offset (-12 ~ 12)

	////////AVC Performance Related/////////
	int m_iAvcFastEncoding;//!< 0 : Normal, 1 : 16x16 block only
	int m_iConstrainedIntra;//!< constained_intra_pred_flag

	////////////Common RC Related////////////
	int m_iPicQpY;//!< intra_pic_qp_y (0 ~ 51)
	int m_iVbvBufferSize;//!< Reference decoder buffer size in bits(0 : ignore)
	int m_iSearchRange;//!< 0 : 128x64, 1 : 64x32, 2 : 32x16, 3 : 16x16(Always 0 for H263)
	int m_iPVMDisable;//!< 0 : Enable PMV, 1 : Disable PMV
	int m_iWeightIntraCost;//!< Intra Weight when decide INTRA/Inter
	int m_iRCIntervalMode;//!< 0 : Normal, 1 : Frame Mode, 2 : Slice Mode, 3 : MB-Num Mode
	int m_iRCIntervalMBNum;//!< This field is used when m_iRCIntervalMode is 3

	///////////Error Resilience Related//////////
	int m_iIntraMBRefresh;//!< 0 : none, 1~ : MBNum - 1

	///////////Slice Mode Related//////////////
	int m_iSliceMode;//!< 0 : frame mode, 1 : Slice mode
	int m_iSliceSizeMode;//!< 0 : the number of bit, 1 : the number of MB
	int m_iSliceSize;//!< the number of bit or MB in one Slice

	///////////Encoded Video Quality Related//////////////
	int m_iEncQualityLevel;//!< Encoded Video Quality level control (H.264 only)
						   //!< 1(Lowest Quality) ~ 35(Highest Quality), default is 11.

}enc_rc_init_t;

typedef struct enc_init_t 
{
	codec_addr_t m_BitWorkAddr[2];		//!< physical[0] and virtual[1] address of a working space of the decoder. This working buffer space consists of work buffer, code buffer, and parameter buffer.
	codec_addr_t m_RegBaseVirtualAddr;	//!< virtual address BIT_BASE

	//! Encoding Info
	int m_iBitstreamFormat;
	int m_iPicWidth;					//!< multiple of 16
	int m_iPicHeight;					//!< multiple of 16
	int m_iFrameRate;
	int m_iTargetKbps;					//!< Target bit rate in Kbps. if 0, there will be no rate control, 
										//!< and pictures will be encoded with a quantization parameter equal to quantParam
	int m_iKeyInterval;					//!< max 32767
	int m_iMjpg_sourceFormat;			//!< input YUV format for mjpeg encoding

	int m_iUseSpecificRcOption;		//!< 0 : Use default setting, 1 : Use parameters in m_stRcInit
	enc_rc_init_t m_stRcInit;

	//! VPU Control 
	unsigned int m_bEnableVideoCache;
	unsigned int m_bCbCrInterleaveMode;	//!< 0 (chroma separate mode   : CbCr data is written in separate frame memories)
										//!< 1 (chroma interleave mode : CbCr data is interleaved in chroma memory)

	codec_addr_t m_BitstreamBufferAddr; //!< physical address : multiple of 4
	codec_addr_t m_BitstreamBufferAddr_VA; //!< virtual address : multiple of 4
	int m_iBitstreamBufferSize;			//!< multiple of 1024
	
#define SEC_AXI_BUS_ENABLE_SRAM	    (1<<21)	//!< Use SRAM for sec. AXI bus
#define ENHANCED_RC_MODEL_ENABLE	(1<<10)

	unsigned int m_uiEncOptFlags;

	//! Callback Func
	void* (*m_Memcpy ) ( void*, const void*, unsigned int );	//!< memcpy
	void  (*m_Memset ) ( void*, int, unsigned int );			//!< memset
	int   (*m_Interrupt ) ( void );								//!< hw interrupt (return value is always 0)

	unsigned int m_Reserved[11];	//! Reserved.
} enc_init_t;

typedef struct enc_initial_info_t 
{
	int m_iMinFrameBufferCount;
	int m_iMinFrameBufferSize;		//!< minimum frame buffer size
} enc_initial_info_t;

typedef struct enc_input_t 
{
	codec_addr_t m_PicYAddr;
	codec_addr_t m_PicCbAddr;
	codec_addr_t m_PicCrAddr;
	
	int m_iForceIPicture;
	int m_iSkipPicture;
	int m_iQuantParam;

	codec_addr_t m_BitstreamBufferAddr; //!< physical address
	int m_iBitstreamBufferSize;

	int m_iChangeRcParamFlag;	//!< RC(Rate control) parameter changing mode
								//!<	bit[0]: 0-disable, 1-enable
								//!<	bit[1]: 0-disable, 1-enable(change a bitrate)
								//!<	bit[2]: 0-disable, 1-enable(change a framerate)
								//!<	bit[3]: 0-disable, 1-enable(change a Keyframe interval)
								//!<	bit[4]: 0-disable, 1-enable(change a I-frame Qp)
								//!<	bit[5]: 0-disable, 1-enable(RC automatic skip disable)
	int m_iChangeTargetKbps;	//!< Target bit rate in Kbps
	int m_iChangeFrameRate;		//!< Target frame rate
	int	m_iChangeKeyInterval;	//!< Keyframe interval(max 32767)
	
	int m_iReportSliceInfoEnable;		//!< Report slice information mode (0: disable, 1: enable)
	codec_addr_t m_SliceInfoAddr[2];	//!< Slice information buffer(last MB number and slice size in bits)

	int m_iReportMVInfoEnable;		//!< Report MV information mode (0: disable, 1: enable)
	codec_addr_t m_MVInfoAddr[2];	//!< MV information buffer(group of word which is formed as X and Y value of MV)
} enc_input_t;

typedef struct enc_buffer_t
{
	codec_addr_t m_FrameBufferStartAddr[2];
} enc_buffer_t;

typedef struct enc_output_t 
{
	codec_addr_t m_BitstreamOut[2];
	int m_iBitstreamOutSize;
	int m_iPicType;

	int m_iSliceInfoNum;			//!< Number of encoded slices
	int m_iSliceInfoSize;			//!< Size in bytes of the encoded slices information(end position)
	codec_addr_t m_SliceInfoAddr;	//!< Address of slices information(last MB number and slice size in bits)

	int m_iMVInfoSize;			//!< Size in bytes of the encoded MV information
	codec_addr_t m_MVInfoAddr;	//!< Address of slices information

	int m_Reserved;
} enc_output_t;

typedef struct enc_header_t
{
	int m_iHeaderType;				//!< [in] header type : MPEG-4(VOL/VOS/VIS), H264(SPS/PPS)
	int m_iHeaderSize;				//!< [out]
	codec_addr_t m_HeaderAddr;		//!< [out] physical address
	codec_addr_t m_HeaderAddr_VA;	//!< [out] virtual address
} enc_header_t;

#endif	// _TCC_VPU_D6_H_