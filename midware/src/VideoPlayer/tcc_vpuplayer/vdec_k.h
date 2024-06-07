/****************************************************************************
 *   FileName    : vdec_v2.h
 *   Description :
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved

This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information's accuracy, completeness, or performance.
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code.
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef _VDEC_V2_H_
#define _VDEC_V2_H_

#include <stdint.h>

#include <mach/TCC_VPU_CODEC.h>

#include <mach/tcc_video_private.h>
#include <mach/tcc_video_common.h>


#ifdef __cplusplus
extern "C" {
#endif

#if !defined(TCC_VPU_D6_INCLUDE)
#define SUPPORT_EXT_DECODER
#endif

//#define SUPPORT_ASPECT_RATIO

#define CDMX_PTS_MODE          		0   //! Presentation Timestamp (Display order)
#define CDMX_DTS_MODE           	1   //! Decode Timestamp (Decode order)

#define CVDEC_DISP_INFO_INIT		0
#define CVDEC_DISP_INFO_UPDATE		1
#define CVDEC_DISP_INFO_GET			2
#define CVDEC_DISP_INFO_RESET		3

#define TMP_SKIP_OPT_SKIP_INTERVAL  5

#define SEQ_HEADER_INIT_ERROR_COUNT 			20000 //sync with REPEAT_COUNT_FOR_THUMBNAIL in tcc_mediaffparser_node.cpp
#define DISPLAYING_FAIL_IN_THUMBNAIL_MODE_ERROR_COUNT 20
#define MAX_CONSECUTIVE_VPU_FAIL_COUNT 			20000
#define MAX_CONSECUTIVE_VPU_FAIL_COUNT_FOR_IDR 	20000
#define MAX_CONSECUTIVE_VPU_BUFFER_FULL_COUNT 	30
#define MAX_CONSECUTIVE_VPU_FAIL_TO_RESTORE_COUNT 20000

#define MPEG4_VOL_STARTCODE_MIN		0x00000120	// MPEG-4
#define MPEG4_VOL_STARTCODE_MAX		0x0000012F	// MPEG-4
#define MPEG4_VOP_STARTCODE			0x000001B6	// MPEG-4
#define MAX_SEQ_HEADER_ALLOC_SIZE   0x0007D000  // 500KB

#define K_VA 2

//#define MOVE_HW_OPERATION
#define MAX_FRAME_BUFFER_COUNT			31

#define AVC_IDR_PICTURE_SEARCH_MODE		0x001
#define AVC_NONIDR_PICTURE_SEARCH_MODE	0x201

#define VPU_ENV_INIT_ERROR		10000
#define VPU_NOT_ENOUGH_MEM		20000

//
#define AVAILABLE_MAX_WIDTH		1920
#define AVAILABLE_MAX_HEIGHT	1088
#define AVAILABLE_MAX_REGION	(AVAILABLE_MAX_WIDTH * AVAILABLE_MAX_HEIGHT)

#define AVAILABLE_HEVC_MAX_REGION   (AVAILABLE_HEVC_MAX_WIDTH * AVAILABLE_HEVC_MAX_HEIGHT)

#define AVAILABLE_VP9_MAX_WIDTH    4096
#define AVAILABLE_VP9_MAX_HEIGHT   2160
#define AVAILABLE_VP9_MAX_REGION   (AVAILABLE_VP9_MAX_WIDTH * AVAILABLE_VP9_MAX_HEIGHT)

#define AVAILABLE_MIN_WIDTH    16
#define AVAILABLE_MIN_HEIGHT   16

#define VDEC_INIT				0	//!< init
#define VDEC_DEC_SEQ_HEADER		1	//!< seq
#define VDEC_DECODE				2	//!< decode
#define VDEC_BUF_FLAG_CLEAR		3	//!< display buffer flag clear
#define VDEC_CLOSE				4	//!< close
#define VDEC_FINISH				5	//!< the end of decoding
#define VDEC_DEC_FLUSH_OUTPUT	6	//!< flush delayed output frame

#define VDEC_GET_RING_BUFFER_STATUS		7
#define VDEC_FILL_RING_BUFFER			8 //!< Fill the ring buffer

#define VDEC_UPDATE_WRITE_BUFFER_PTR						9
#define VDEC_GET_INITIAL_INFO_FOR_STREAMING_MODE_ONLY	10
#define VDEC_GET_INTERMEDIATE_BUF_INFO			11

#define VDEC_SW_RESET			12

#define VDEC_SKIP_FRAME_DISABLE		0	//!< Skip disable
#define VDEC_SKIP_FRAME_EXCEPT_I	1	//!< Skip except I(IDR) picture
#define VDEC_SKIP_FRAME_ONLY_B		2	//!< Skip B picture(skip if nal_ref_idc==0 in H.264)
#define VDEC_SKIP_FRAME_UNCOND		3	//!< Unconditionally Skip one picture

#define VCODEC_MAX		20

#define VPU_BUFF_COUNT  (VPU_BUFFER_MANAGE_COUNT)

#define REINIT_VPU_FOR_RESOLUTION_CHANGING // Should enable in order to work not in case of framebuffer_max mode!!
/******************* START :: To support adaptive or smooth streaming or something like those *******************************/
//#define SET_FRAMEBUFFER_INTO_MAX
#ifdef SET_FRAMEBUFFER_INTO_MAX
#define VPU_FRAMEBUFFER_MAX_WIDTH 1920
#define VPU_FRAMEBUFFER_MAX_HEIGHT 1080
#define ONLY_SUPPORT_STREAMING_CASE // In other word, will not support it in case of local playback. Instead of that, local playback will support to re-init vpu.
#endif
/******************* END :: To support adaptive or smooth streaming  ********************************************************/


// <-- mm008: to report Error Reason
// The errors should be in sync with "frameworks\av\include\media\stagefright\OMXCodec.h"
#define ERR_PACKED_BITSTREAM    1 // PB detection: mm008
// -->

//#define DISPLAY_1ST_DECODED_IDX
#ifdef DISPLAY_1ST_DECODED_IDX
#define MAX_INDEX 33
#else
#define MAX_INDEX 32
#endif

#define EXT_FUNC_MEM_PROTECTION_WITH_INSTREAM 		0x0001  // for read protection for VPU memory!! specially WFD-SINK!!
#define EXT_FUNC_MEM_PROTECTION_ONLY 				0x0002  // for read protection for VPU memory!! specially WFD-SOURCE!!
#define EXT_FUNC_NO_BUFFER_DELAY					0x0004  // To ignore buffer-delay on VPU!!
#define EXT_FUNC_MAX_FRAMEBUFFER					0x0010  // To configure reference frame buffer into 1080p for smooth-streamming(adaptive-streamming).

typedef struct vdec_user_info_t
{
    unsigned int bitrate_mbps;          //!< video BitRate (Mbps)
    unsigned int frame_rate;            //!< video FrameRate (fps)
    unsigned int m_bStillJpeg;          //!< If this is set, jpeg is not M-JPEG but Still-image (ex. **.jpg)
    unsigned int jpg_ScaleRatio;        ////!< 0 ( Original Size )
                                        //!< 1 ( 1/2 Scaling Down )
                                        //!< 2 ( 1/4 Scaling Down )
                                        //!< 3 ( 1/8 Scaling Down )
    unsigned int extFunction;
} vdec_user_info_t;

typedef struct vdec_init_t
{
    int m_iBitstreamFormat;         	//!< bitstream format
    int m_iBitstreamBufSize;        	//!< bitstream buf size    : multiple of 1024
    int m_iPicWidth;                	//!< frame width from demuxer or etc
    int m_iPicHeight;               	//!< frame height from demuxer or etc
    unsigned int m_bEnableUserData;     //!< If this is set, VPU returns userdata.
    unsigned int m_bEnableVideoCache;   //!< video cache
    unsigned int m_bCbCrInterleaveMode; //!< 0 (chroma separate mode   : CbCr data is written in separate frame memories) \n
                                        //!< 1 (chroma interleave mode : CbCr data is interleaved in chroma memory)
    unsigned int m_bFilePlayEnable;     //!< 1:file play mode(default), 0:streaming mode
    unsigned int m_uiMaxResolution;     //!< 0:full-HD, 1:HD, 2:SD

    unsigned char *m_pExtraData;        //!< codec specific data of asf ( WMV78 Video Decoder only )
    int m_iFourCC;                  	//!< FourCC info.               ( WMV78 Video Decoder only )
    int m_iExtraDataLen;            	//!< codec specific data size   ( WMV78 Video Decoder only )
	unsigned int m_Reserved;
} vdec_init_t;

typedef struct vdec_info_t
{
    int m_iPicWidth;                //!< {(PicX+15)/16} * 16  (this width  will be used while allocating decoder frame buffers. picWidth  is a multiple of 16)
    int m_iPicHeight;               //!< {(PicY+15)/16} * 16  (this height will be used while allocating decoder frame buffers. picHeight is a multiple of 16)
    int m_uiFrameRateRes;          //!< decoded picture frame rate residual(number of time units of a clock operating at the frequency[m_iFrameRateDiv] Hz, frameRateInfo(frame per second) = m_uiFrameRateRes/m_uiFrameRateDiv
    int m_uiFrameRateDiv;          //!< decoded picture frame rate unit number in Hz
    int m_iMinFrameBufferCount;     //!< the minimum number of frame buffers that are required for decoding. application must allocate at least this number of frame buffers.
    int m_iMinFrameBufferSize;      //!< minimum frame buffer size
    int m_iFrameBufferFormat;       //!< frame buffer format
    int m_iNormalSliceSize;         //!< recommended size of to save slice. this value is determined as a quater of the memory size for one raw YUV image in KB unit.
    int m_iWorstSliceSize;          //!< recommended size of to save slice in worst case. this value is determined as a half of the memory size for one raw YUV image in KB unit.

    // H264/AVC only param
    pic_crop_t m_iAvcPicCrop;           //!< represents rectangular window information in a frame
    int m_iAvcConstraintSetFlag[4]; //!< syntax element which is used to make level in H.264. used only in H.264 case.
    int m_iAvcParamerSetFlag;       //!< These are H.264 SPS or PPS syntax element
                                        //!< [bit 0  ] direct_8x8_inference_flag in H.264 SPS
    int m_iFrameBufDelay;           //!< maximum display frame buffer delay to process reordering in case of H.264

    // MPEG-4 only param
    int m_iM4vDataPartitionEnable;  //!< ( 0: disable   1: enable )
    int m_iM4vReversibleVlcEnable;  //!< ( 0: disable   1: enable )
    int m_iM4vShortVideoHeader;     //!< ( 0: disable   1: enable )
    int m_iM4vH263AnnexJEnable;     //!< ( 0: disable   1: enable )

    // VC-1 only param
    int m_iVc1Psf;                  //!< this is only available in VC1 and indicates the value of "Progressive Segmented Frame"

    //! Additional Info
    int m_iProfile;                 //!< profile of the decoded stream
    int m_iLevel;                   //!< level of the decoded stream
    int m_iInterlace;               //!< when this value is 1, decoded stream will be decoded into both progressive or interlace frame.
                                        //!< otherwise, all the frames will be progressive.
    int m_iAspectRateInfo;          //!< aspect rate information. if this value is 0, then aspect ratio information is not present
    int m_iReportErrorReason;       //!< reports reason of 'RETCODE_CODEC_SPECOUT' or 'RETCODE_INVALID_STRIDE' error

} vdec_info_t;

typedef struct vdec_input_t
{
    unsigned char* m_pInp[2];               //!< input data
    int m_iInpLen;                      //!< input data len
    unsigned char* m_UserDataAddr[2];       //!< Picture Layer User-data address
    int m_iUserDataBufferSize;          //!< Picture Layer User-data Size

    int m_iFrameSearchEnable;           //!< I-frame Search Mode. \n
                                            //!< If this option is enabled, the decoder skips the frame decoding until decoder meet IDR(and/or I)-picture for H.264 or I-frame.\n
                                            //!< If this option is enabled, m_iSkipFrameMode option is ignored.\n
                                            //!< 0 ( Disable ) \n
                                            //!< 1 ( Enable : search IDR-picture for H.264 or I-frame ) \n
                                            //!< 513 ( Enable : search I(or IDR)-picture for H.264 or I-frame )

    int m_iSkipFrameMode;               //!< Skip Frame Mode \n
                                            //!< 0 ( Skip disable ) \n
                                            //!< 1 ( Skip except I(IDR) picture ) \n
                                            //!< 2 ( Skip B picture : skip if nal_ref_idc==0 in H.264 ) \n
                                            //!< 3 ( Unconditionally Skip one picture )

    int m_iSkipFrameNum;                //!< Number of skip frames (for I-frame Search Mode or Skip Frame Mode) \n
                                            //!< When this number is 0, m_iSkipFrameMode option is disabled.

    int m_iIsThumbnail;                 //!< For checking size of memory while thumbnail extracting on sequence header init
} vdec_input_t;

#define COMP_Y 0
#define COMP_U 1
#define COMP_V 2

typedef struct vdec_initial_info_t
{
    int m_iPicWidth;                //!< {(PicX+15)/16} * 16  (this width  will be used while allocating decoder frame buffers. picWidth  is a multiple of 16)
    int m_iPicHeight;               //!< {(PicY+15)/16} * 16  (this height will be used while allocating decoder frame buffers. picHeight is a multiple of 16)
    pic_crop_t m_iPicCrop;              //!< represents rectangular window information in a frame

    int m_iInterlace;               //!< when this value is 1, decoded stream will be decoded into both progressive or interlace frame. otherwise, all the frames will be progressive. In H.264, this is frame_mbs_only_flag.
    int m_iMjpg_sourceFormat;       //!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )(only for TCC891x/88xx/93XX)

    int m_iFrameBufferFormat;       // HEVC out Bit Depth info(0 : 8 Bit, 6 : 10 Bit)
    int m_iSourceFormat;            // HEVC Source Bit Depth info(0 : 8 Bit, 6 : 10 Bit)
    int m_iErrCode;                 // mm008
} vdec_initial_info_t;

typedef struct vdec_output_t
{
	dec_output_info_t m_DecOutInfo;
	unsigned char* m_pDispOut[2][3];	//!< physical[0] and virtual[1] display  address of Y, Cb, Cr component
	unsigned char* m_pCurrOut[2][3];	//!< physical[0] and virtual[1] current  address of Y, Cb, Cr component
	unsigned char* m_pPrevOut[2][3];	//!< physical[0] and virtual[1] previous address of Y, Cb, Cr component
	vdec_initial_info_t* m_pInitialInfo;

    //hevc user data
    //uint32_t m_uiUserData;
    //uint32_t m_uiBitDepthLuma;
} vdec_output_t;

typedef struct vdec_ring_buffer_set_t
{
    unsigned char *m_pbyBuffer;
    uint32_t m_uiBufferSize;
} vdec_ring_buffer_set_t;

typedef struct vdec_ring_buffer_out_t
{
    unsigned int m_ulAvailableSpaceInRingBuffer;
    unsigned int m_ptrReadAddr_PA;
    unsigned int m_ptrWriteAddr_PA;
} vdec_ring_buffer_out_t;

#define EXT_V_DECODER_TR_TEST
#define TIMESTAMP_CORRECTION
#ifdef TIMESTAMP_CORRECTION
typedef struct pts_ctrl
{
//  int m_iLatestPTS;
    signed long long m_lLatestPTS;//[usec]
    int m_iPTSInterval;
    int m_iRamainingDuration;
} pts_ctrl;
#endif

#ifdef EXT_V_DECODER_TR_TEST
typedef struct EXT_F_frame_t
{
    int Current_TR;
    int Previous_TR;
    int Current_time_stamp;
    int Previous_time_stamp;
} EXT_F_frame_t;

typedef struct EXT_F_frame_time_t
{
    EXT_F_frame_t  ref_frame;
    EXT_F_frame_t  frame_P1;
    EXT_F_frame_t  frame_P2;
} EXT_F_frame_time_t;
#endif

typedef struct dec_disp_info_ctrl_t
{
    int       m_iTimeStampType;                     //! TS(Timestamp) type (0: Presentation TS(default), 1:Decode TS)
    int       m_iStdType;                           //! STD type
    int       m_iFmtType;                           //! Formater Type

    int       m_iUsedIdxPTS;                        //! total number of decoded index for PTS
    int       m_iPrevIdx;                           //! Index of previous frame
    int       m_iRegIdxPTS[MAX_INDEX];              //! decoded index for PTS
    void      *m_pRegInfoPTS[MAX_INDEX];			//! side information of the decoded index for PTS

    int       m_iDecodeIdxDTS;                      //! stored DTS index of decoded frame
    int       m_iDispIdxDTS;                        //! display DTS index of DTS array
//  int       m_iDTS[MAX_INDEX];                	//! Decode Timestamp (decoding order)
    signed long long m_lDTS[MAX_INDEX];//[usec]     //! Decode Timestamp (decoding order)

    int       m_iReserved;
} dec_disp_info_ctrl_t;

typedef struct dec_disp_info_t
{
	int       m_iFrameType;								   //! Frame Type

//  int       m_iTimeStamp;                                //! Time Stamp
//  int       m_iextTimeStamp;                             //! TR(Ext)
    signed long long m_lTimeStamp;   //[usec]                     //! Time Stamp
    signed long long m_lextTimeStamp;//[usec]                     //! TR(Ext)


    int       m_iPicStructure;                             //! PictureStructure
    int       m_iM2vFieldSequence;                         //! Field sequence(MPEG2)
    int       m_iFrameDuration;                            //! MPEG2 Frame Duration

    int       m_iFrameSize;                                //! Frame size

    int       m_iTopFieldFirst;                            //! Top Field First
    int       m_iIsProgressive;                            //! Interlace information :: 0:interlace, 1:progressive

    int       m_bIsMvcDependent;                           //! MVC Dependent frame
    int       m_iNumMBError;
    int       m_iPicWidth;
    int       m_iPicHeight;
} dec_disp_info_t;

typedef struct dec_disp_info_input_t
{
	int       m_iFrameIdx;								   //! Display frame buffer index for CVDEC_DISP_INFO_UPDATE command
    //! Decoded frame buffer index for CVDEC_DISP_INFO_GET command
	int       m_iStdType;								   //! STD type for CVDEC_DISP_INFO_INIT
	int       m_iTimeStampType;							   //! TS(Timestamp) type (0: Presentation TS(default), 1:Decode TS) for CVDEC_DISP_INFO_INIT
	int       m_iFmtType;								   //! Formater Type specification
	int       m_iFrameRate;
} dec_disp_info_input_t;

typedef struct mpeg2_pts_ctrl
{
    signed long long m_lLatestPTS;//[usec]
    int       m_iPTSInterval;
    int       m_iRamainingDuration;
} mpeg2_pts_ctrl;

void vpu_set_additional_refframe_count(int count, void* pVdec);
void vpu_update_sizeinfo(unsigned int format, unsigned int bps, unsigned int fps, unsigned int image_width, unsigned int image_height, void* pVdec);
unsigned char *vpu_getBitstreamBufAddr(unsigned int index, void* pVdec);
int vpu_getBitstreamBufSize(void *pVdec);
unsigned char *vpu_getFrameBufVirtAddr(unsigned char *convert_addr, unsigned int base_index, void* pVdec);
int vdec_vpu( int iOpCode, int* pHandle, void* pParam1, void* pParam2, void* pParam3 );

typedef int (cdk_func_t) ( int iOpCode, int* pHandle, void* pParam1, void* pParam2, void* pParam3);

static cdk_func_t* gspfVDecList[VCODEC_MAX] =
{
	 vdec_vpu 	//STD_AVC
	,vdec_vpu 	//STD_VC1
	,vdec_vpu 	//STD_MPEG2
	,vdec_vpu 	//STD_MPEG4
};

float vdec_getAspectRatio(void *pvInstance, int iBitstreamFormat);
void vdec_get_realResolution(void *pInst, int *width, int *height);
int vdec_seqHeader_extract(const unsigned char	*pbyStreamData, unsigned int lStreamDataSize, unsigned char **ppbySeqHeaderData, unsigned int *plSeqHeaderSize, int codec_type, unsigned int isTCCExtractor, void *pInst);
char* vdec_print_pic_type( int iVideoType, int iPicType, int iPictureStructure );
void vdec_disp_pic_info (int Opcode, void* pParam1, void *pParam2, void *pParam3, int fps, int bPlayDirection, void *pInst);
pts_ctrl * vdec_get_PtsInfo(void *pInst);
void* vdec_alloc_instance(int codec_format, int refer_instance);
void vdec_release_instance(void * pInst, int codec_format);
void vdec_set_rendered_index(void * pInst);
int vdec_get_instance_index(void * pInst);
int vdec_is_rendered_index(void * pInst);
int vdec_is_reused_buffer(void * pInst);


#ifdef __cplusplus
}
#endif
#endif //_VDEC_V2_H_

