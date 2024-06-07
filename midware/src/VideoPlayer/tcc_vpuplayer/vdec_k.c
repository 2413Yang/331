/****************************************************************************
 *   FileName    : vdec_k.c
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
#include <stdint.h>

#define LOG_TAG	"VPU__DEC__K"
#include <utils/Log.h>

#include "vdec_k.h"
#include "TCCMemory.h"

#include <sys/mman.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <mach/tcc_vpu_ioctl.h>

#include <dlfcn.h>

#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>     // for close() declaration
#include <fcntl.h>		// for open()  declaration
/********************************************************************************************/
/* TEST and Debugging       =======================> Start              *********************/
/********************************************************************************************/
#define LOGD(x...)    ALOGD(x)
#define LOGE(x...)    ALOGE(x)
#define LOGI(x...)    ALOGI(x)
#define LOGW(x...)    ALOGW(x)

static int DEBUG_ON	= 0;
#define DPRINTF(msg...)	  ALOGE(msg);
#define DSTATUS(msg...)	  if (DEBUG_ON) { ALOGD(msg);}
#define DPRINTF_FRAME(msg...) if (DEBUG_ON) { ALOGD(msg);}
#define DISPLAY_BUFFER(msg...) if (DEBUG_ON) { ALOGD(msg);}
#define CLEAR_BUFFER(msg...) if (DEBUG_ON) { ALOGE(msg);}
#define SEQ_EXTRACTOR(msg...) if (DEBUG_ON) { ALOGW(msg);}

//#define VPU_IN_FRAME_DUMP
#ifdef VPU_IN_FRAME_DUMP
//#define VPU_ALL_FRAME_DUMP
#define TARGET_STORAGE "/data"
#define MAX_DUMP_LEN (10*1024*1024) //10MB
#define MAX_DUMP_CNT 10
#endif
//#define VPU_OUT_FRAME_DUMP // Change the output format into YUV420 seperated to play on PC.
//#define HEVC_OUT_FRAME_DUMP //HEVC Data save
//#define CHANGE_INPUT_STREAM // to change frame-data to test stream from RTP etc.
#ifdef CHANGE_INPUT_STREAM
#define STREAM_NAME "data/changed_stream.dat"
#endif

//#define ERROR_TEST
#ifdef ERROR_TEST
static int err_test = 0;
#endif
//#define DEBUG_TIME_LOG
#ifdef DEBUG_TIME_LOG
#include "time.h"
static unsigned int dec_time[30] = {0,};
static unsigned int time_cnt = 0;
static unsigned int total_dec_time = 0;
#endif
/********************************************************************************************/
/* TEST and Debugging       =======================> End                *********************/
/********************************************************************************************/
#define TCC_VPU_INPUT_BUF_SIZE 		(1024 * 1024)
//#define MAX_FRAME_BUFFER_COUNT		31 // move to vdec.h

static unsigned int total_opened_decoder = 0;
static unsigned int vpu_opened_count = 0;
#define VPU_MGR_NAME	"/dev/vpu_dev_mgr"
char *dec_devices[4] =
{
	"/dev/vpu_vdec",
	"/dev/vpu_vdec_ext",
	"/dev/vpu_vdec_ext2",
	"/dev/vpu_vdec_ext3"
};

#include <fcntl.h>         // O_RDWR
#include <sys/poll.h>

typedef int (*DEC_FUNC)( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

typedef struct _vdec_ {
	int vdec_instance_index;
	int renderered;
	unsigned int total_frm;
	
	unsigned char vdec_env_opened;
	unsigned char vdec_codec_opened;
	unsigned char prev_codec;

	int mgr_fd; // default -1
	int dec_fd; // default -1

	int codec_format;
#if defined(VPU_OUT_FRAME_DUMP) || defined(CHANGE_INPUT_STREAM)
	FILE *pFs;
	unsigned int is1st_dec;
	unsigned char* backup_data;
	unsigned int backup_len;
#endif

	int gsBitstreamBufSize;
	int gsIntermediateBufSize;
	int gsUserdataBufSize;
	int gsMaxBitstreamSize;
	int gsAdditionalFrameCount;

	codec_addr_t gsBitstreamBufAddr[3];  /////Working JW
	codec_addr_t gsIntermediateBufAddr[3];
	codec_addr_t gsUserdataBufAddr[3];

	dec_user_info_t gsUserInfo;
//	int gsAdditional_frame;

	codec_addr_t gsRegBaseVideoBusAddr;
	codec_addr_t gsRegBaseClkCtrlAddr;

	int gsBitWorkBufSize;
	int gsSliceBufSize;
	int gsSpsPpsSize;
	int gsFrameBufSize;

	codec_addr_t gsBitWorkBufAddr[3];
	codec_addr_t gsSliceBufAddr;
	codec_addr_t gsSpsPpsAddr;
	codec_addr_t gsFrameBufAddr[3];

	VDEC_INIT_t gsVpuDecInit_Info;
	VDEC_SEQ_HEADER_t gsVpuDecSeqHeader_Info;
	VDEC_SET_BUFFER_t gsVpuDecBuffer_Info;
	VDEC_DECODE_t gsVpuDecInOut_Info;
	VDEC_RINGBUF_GETINFO_t gsVpuDecBufStatus;
	VDEC_RINGBUF_SETBUF_t gsVpuDecBufFill;
	VDEC_RINGBUF_SETBUF_PTRONLY_t gsVpuDecUpdateWP;
	VDEC_GET_VERSION_t gsVpuDecVersion;

	int gsbHasSeqHeader;

	struct pollfd tcc_event[1];

	int bMaxfb_Mode;
	int mRealPicWidth;
	int mRealPicHeight;
	int extFunction;

	void* pExtDLLModule;
	DEC_FUNC gExtDecFunc;

	vdec_initial_info_t gsCommDecInfo;

	pts_ctrl gsPtsInfo;
	int gsextTRDelta;
	int gsextP_frame_cnt;
	int gsextReference_Flag;
	EXT_F_frame_time_t gsEXT_F_frame_time;
	
#ifdef DISPLAY_1ST_DECODED_IDX
	int mdisplayed_1st_IFrm;
#endif
#ifdef VPU_IN_FRAME_DUMP
	unsigned int lenDump;
	int IdxDump;
	int DelDump;
#endif
	int AvcSeq_status;
} _vdec_;

#define ALIGN_LEN (4*1024)

static int gsbHasSeqHeader = 0;
#define LEVEL_MAX		11
#define PROFILE_MAX		6

static const char *strProfile[VCODEC_MAX][PROFILE_MAX] =
{
	//STD_AVC
	{ "Base Profile", "Main Profile", "Extended Profile", "High Profile", "Reserved Profile", "Reserved Profile" },
	//STD_VC1
	{ "Simple Profile", "Main Profile", "Advance Profile", "Reserved Profile", "Reserved Profile", "Reserved Profile" },
	//STD_MPEG2
	{ "High Profile", "Spatial Scalable Profile", "SNR Scalable Profile", "Main Profile", "Simple Profile", "Reserved Profile" },
	//STD_MPEG4
	{ "Simple Profile", "Advanced Simple Profile", "Advanced Code Efficiency Profile", "Reserved Profile", "Reserved Profile", "Reserved Profile" },
	//STD_H263
	{ " ", " ", " ", " ", " ", " " },
	//STD_DIV3
	{ " ", " ", " ", " ", " ", " " },
	//STD_RV
	{ "Real video Version 8", "Real video Version 9", "Real video Version 10", " ", " ", " " },
	//STD_AVS
	{ "Jizhun Profile", " ", " ", " ", " ", " " },
	//STD_WMV78
	{ " ", " ", " ", " ", " ", " " },
	//STD_SORENSON263
	{ " ", " ", " ", " ", " ", " " },
	//STD_MJPG
	{ " ", " ", " ", " ", " ", " " },
	//STD_VP8
	{ " ", " ", " ", " ", " ", " " },
	//STD_THEORA
	{ " ", " ", " ", " ", " ", " " },
	//???
	{ " ", " ", " ", " ", " ", " " },
	//STD_MVC
	{ "Base Profile", "Main Profile", "Extended Profile", "High Profile", "Reserved Profile", "Reserved Profile" }
};

static const char *strLevel[VCODEC_MAX][LEVEL_MAX] =
{
	//STD_AVC
	{ "Level_1B", "Level_", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level" },
	//STD_VC1
	{ "Level_L0(LOW)", "Level_L1", "Level_L2(MEDIUM)", "Level_L3", "Level_L4(HIGH)", "Reserved Level", "Reserved Level", "Reserved Level" },
	//STD_MPEG2
	{ "High Level", "High 1440 Level", "Main Level", "Low Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level" },
	//STD_MPEG4
	{ "Level_L0", "Level_L1", "Level_L2", "Level_L3", "Level_L4", "Level_L5", "Level_L6", "Reserved Level" },
	//STD_H263
	{ "", "", "", "", "", "", "", "" },
	//STD_DIV3
	{ "", "", "", "", "", "", "", "" },
	//STD_RV
	{ "", "", "", "", "", "", "", "" },
	//STD_AVS
	{"2.0 Level", "4.0 Level", "4.2 Level", "6.0 Level", "6.2 Level", "", "", ""},
	//STD_WMV78
	{ "", "", "", "", "", "", "", "" },
	//STD_SORENSON263
	{ "", "", "", "", "", "", "", "" },
	//STD_MJPG
	{ "", "", "", "", "", "", "", "" },
	//STD_VP8
	{ "", "", "", "", "", "", "", "" },
	//STD_THEORA
	{ "", "", "", "", "", "", "", "" },
	//???
	{ "", "", "", "", "", "", "", "" },
	//STD_MVC
	{ "Level_1B", "Level_", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level" }	
};

static void vpu_env_close(_vdec_ * pVdec);

static void *vdec_malloc(unsigned int size)
{
	void * ptr = TCC_malloc(size);
	return ptr;
}

static void vdec_free(void * ptr )
{
	TCC_free(ptr);
}

static unsigned int cdk_sys_remain_memory_size( _vdec_ * pVdec )
{
	unsigned int sz_freeed_mem = pVdec->vdec_instance_index + VPU_DEC;
	if(ioctl(pVdec->dec_fd, VPU_GET_FREEMEM_SIZE, &sz_freeed_mem) < 0){
		LOGE("ioctl(0x%x) error[%s]!!", VPU_GET_FREEMEM_SIZE, strerror(errno));
	}

	return sz_freeed_mem;
}

static unsigned int cdk_sys_final_free_mem( _vdec_ * pVdec )
{
	unsigned int sz_freeed_mem = pVdec->vdec_instance_index + VPU_DEC;
	if(ioctl(pVdec->mgr_fd, VPU_GET_FREEMEM_SIZE, &sz_freeed_mem) < 0){
		LOGE("ioctl(0x%x) error[%s]!!", VPU_GET_FREEMEM_SIZE, strerror(errno));
	}

	return sz_freeed_mem;
}

static void *cdk_sys_malloc_physical_addr(unsigned int *remap_addr, int uiSize, Buffer_Type type, _vdec_ *pVdec)
{
	MEM_ALLOC_INFO_t alloc_mem;
	_vdec_ * pInst = pVdec;
	memset(&alloc_mem, 0x00, sizeof(MEM_ALLOC_INFO_t));
	
	alloc_mem.request_size = uiSize;
	alloc_mem.buffer_type = type;
	if(ioctl(pInst->dec_fd, V_DEC_ALLOC_MEMORY, &alloc_mem) < 0){
		LOGE("ioctl(0x%x) error[%s]!!  request(0x%x)/free(0x%x)", V_DEC_ALLOC_MEMORY, strerror(errno), uiSize, cdk_sys_remain_memory_size(pInst));
	}
	if(remap_addr != NULL)
		*remap_addr = (unsigned int)alloc_mem.kernel_remap_addr;

	return (void*)( alloc_mem.phy_addr );
}


static void *cdk_sys_malloc_virtual_addr(codec_addr_t pPtr, int uiSize, _vdec_ *pVdec)
{
	void *map_ptr = MAP_FAILED;

	map_ptr = (void *)mmap(NULL, uiSize, PROT_READ | PROT_WRITE, MAP_SHARED, pVdec->dec_fd, pPtr);
	if(MAP_FAILED == map_ptr)
	{
		LOGE("mmap failed. fd(%d), addr(0x%x), size(%d)", pVdec->dec_fd, pPtr, uiSize);
		return NULL;
	}
	
	return map_ptr;
}

static int cdk_sys_free_virtual_addr(void* pPtr, int uiSize)
{
	int ret = 0;

	if((ret = munmap((void*)pPtr, uiSize)) < 0)
	{
		LOGE("munmap failed. addr(0x%x), size(%d)", (unsigned int)pPtr, uiSize);
	}

	return ret;
}

void vpu_update_sizeinfo(uint format, unsigned int bps, unsigned int fps, unsigned int image_width, unsigned int image_height, void *pVdec)
{
	CONTENTS_INFO info;
	_vdec_ * pInst = pVdec;

	if(!pInst){
		ALOGE("vpu_update_sizeinfo :: Instance is null!!");
		return;
	}
	
	info.type = pInst->vdec_instance_index - VPU_DEC;

	info.isSWCodec = 0;

	if( format == STD_MJPG )
	{
		info.width = AVAILABLE_MAX_WIDTH;
		info.height = AVAILABLE_MAX_HEIGHT;
	}
	else
	{
		info.width = image_width;
		info.height = image_height;
	}

#ifndef MOVE_HW_OPERATION	
	info.isSWCodec = 1;
	info.width = AVAILABLE_MAX_WIDTH;
	info.height = AVAILABLE_MAX_HEIGHT;
#endif
	info.bitrate = bps;
	info.framerate = fps;	// fps/1000; -- bug?
	if(ioctl(pInst->mgr_fd, VPU_SET_CLK, &info) < 0){
		LOGE("ioctl(0x%x) error[%s]!!", VPU_SET_CLK, strerror(errno));
	}

	return;
}

static int vpu_check_for_video(unsigned char open_status, _vdec_ *pVdec)
{
	_vdec_ * pInst = pVdec;
	OPENED_sINFO sInfo;

	sInfo.type = DEC_WITH_ENC;
	if(open_status)
	{
		sInfo.opened_cnt = 1;		
		
		if(ioctl(pInst->mgr_fd, VPU_SET_MEM_ALLOC_MODE, &sInfo) < 0){
			LOGE("ioctl(0x%x) error[%s]!!", VPU_SET_MEM_ALLOC_MODE, strerror(errno));
		}
		if(ioctl(pInst->dec_fd, DEVICE_INITIALIZE, &(pInst->codec_format)) < 0){
			LOGE("ioctl(0x%x) error[%s]!!", DEVICE_INITIALIZE, strerror(errno));
		}
	}
	
	return 0;
}

static int vpu_env_open(unsigned int format, unsigned int bps, unsigned int fps, unsigned int image_width, unsigned int image_height, _vdec_ *pVdec)
{
	int vpu_reset = 0;
	_vdec_ * pInst = pVdec;
	DSTATUS("In  %s ",__func__);

	if((vpu_reset = vpu_check_for_video(1, pInst)) < 0)
		goto err;

	pInst->prev_codec = format;
	vpu_update_sizeinfo(format, bps, fps, image_width, image_height, pInst);

	pInst->vdec_env_opened = 1;
	pInst->renderered  = 0;
	DSTATUS("Out  %s ",__func__);

#if defined(VPU_OUT_FRAME_DUMP) || defined(CHANGE_INPUT_STREAM)
	pInst->pFs = NULL;
	pInst->is1st_dec = 1;
	pInst->backup_data = NULL;
	pInst->backup_len = 0;
#endif
#ifdef DEBUG_TIME_LOG	
	time_cnt = 0;
	total_dec_time = 0;
#endif
	pInst->gsAdditionalFrameCount = VPU_BUFF_COUNT;
	pInst->total_frm = 0;
	pInst->bMaxfb_Mode = 0;

	return 0;

err:	
	LOGE("vpu_env_open error");
	vpu_env_close(pInst);
	
	return -1;	
}


static void vpu_env_close(_vdec_ *pVdec)
{
	DSTATUS("In  %s ",__func__);

	_vdec_ * pInst = pVdec;

	pInst->vdec_env_opened = 0;
	vpu_check_for_video(0, pInst);

	if( 0 > ioctl(pInst->dec_fd, V_DEC_FREE_MEMORY, NULL))
	{
		LOGE("ioctl(0x%x) error[%s]!!", V_DEC_FREE_MEMORY, strerror(errno));
	}

#if defined(VPU_OUT_FRAME_DUMP) || defined(CHANGE_INPUT_STREAM)
	if(pInst->pFs){
		fclose(pInst->pFs);
		pInst->pFs = NULL;
	}

	if(pInst->backup_data){
		vdec_free((void *) pInst->backup_data);
		pInst->backup_data = NULL;
	}
#endif

	DSTATUS("Out  %s ",__func__);

}

static int vdec_cmd_process(int cmd, void* args, _vdec_ *pVdec)
{
	int ret;
	int success = 0;
	_vdec_ * pInst = pVdec;
	int retry_cnt = 10;
	int all_retry_cnt = 3;

	if((ret = ioctl(pInst->dec_fd, cmd, args)) < 0)
	{
		if( ret == -0x999 )
		{
			LOGE("VDEC[%d] Invalid command(0x%x) ", pInst->vdec_instance_index, cmd);
			return RETCODE_INVALID_COMMAND;
		}
		else
		{
			LOGE("VDEC[%d] ioctl err[%s] : cmd = 0x%x", pInst->vdec_instance_index, strerror(errno), cmd);
		}
	}

Retry:
	while (retry_cnt > 0) {
		memset(pInst->tcc_event, 0, sizeof(pInst->tcc_event));
		pInst->tcc_event[0].fd = pInst->dec_fd;
		pInst->tcc_event[0].events = POLLIN;
		
		ret = poll((struct pollfd *)&pInst->tcc_event, 1, 1000); // 1 sec
		if (ret < 0) {
			LOGE("VDEC[%d] -retry(%d:cmd(%d)) poll error '%s'", pInst->vdec_instance_index, retry_cnt, cmd, strerror(errno));
			retry_cnt--;
			continue;
		}else if (ret == 0) {
			LOGE("VDEC[%d] -retry(%d:cmd(%d)) poll timeout: %d'th frames, len %d", pInst->vdec_instance_index, retry_cnt, cmd, pInst->total_frm, pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize );
			retry_cnt--;
			continue;
		}else if (ret > 0) {
			if (pInst->tcc_event[0].revents & POLLERR) {
				LOGE("VDEC[%d]  poll POLLERR", pInst->vdec_instance_index);
				break;
			} else if (pInst->tcc_event[0].revents & POLLIN) {
				success = 1;
				break;
			}
		}
	}
	/* todo */

	switch(cmd)
	{
		case V_DEC_INIT:
			{			 
			 	VDEC_INIT_t* init_info = args;
				
				if(ioctl(pInst->dec_fd, V_DEC_INIT_RESULT, args) < 0){
					LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_DEC_INIT_RESULT, strerror(errno));
				}
				ret = init_info->result;
			}
			break;
			
		case V_DEC_SEQ_HEADER: 
			{			 
			 	VDEC_SEQ_HEADER_t* seq_info = args;
				
				if(ioctl(pInst->dec_fd, V_DEC_SEQ_HEADER_RESULT, args) < 0){
					LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_DEC_SEQ_HEADER_RESULT, strerror(errno));
				}
				ret = seq_info->result;
	#ifdef ERROR_TEST			
				err_test = 0;
	#endif
			}
			break;
			
		case V_DEC_DECODE:
			{			 
			 	VDEC_DECODE_t* decoded_info = args;
				
				if(ioctl(pInst->dec_fd, V_DEC_DECODE_RESULT, args) < 0){
					LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_DEC_DECODE_RESULT, strerror(errno));
				}
				ret = decoded_info->result;
			}
			break;
			
		case V_DEC_FLUSH_OUTPUT:			
			{			 
			 	VDEC_DECODE_t* decoded_info = args;
				
				if(ioctl(pInst->dec_fd, V_DEC_FLUSH_OUTPUT_RESULT, args) < 0){
					LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_DEC_FLUSH_OUTPUT_RESULT, strerror(errno));
				}
				ret = decoded_info->result;
			}
			break;
			
		case V_GET_RING_BUFFER_STATUS:
			{
				VDEC_RINGBUF_GETINFO_t* p_param = args;
				if(ioctl(pInst->dec_fd, V_GET_RING_BUFFER_STATUS_RESULT, args) < 0){
					LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_GET_RING_BUFFER_STATUS_RESULT, strerror(errno));
				}
				ret = p_param->result;
			}
			break;
		case V_FILL_RING_BUFFER_AUTO:
			{
				VDEC_RINGBUF_SETBUF_t* p_param = args;
				if(ioctl(pInst->dec_fd, V_FILL_RING_BUFFER_AUTO_RESULT, args) < 0){
					LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_FILL_RING_BUFFER_AUTO_RESULT, strerror(errno));
				}
				ret = p_param->result;
			}
			break;
		case V_DEC_UPDATE_RINGBUF_WP:
			{
				VDEC_RINGBUF_SETBUF_PTRONLY_t* p_param = args;
				if(ioctl(pInst->dec_fd, V_DEC_UPDATE_RINGBUF_WP_RESULT, args) < 0){
					LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_DEC_UPDATE_RINGBUF_WP_RESULT, strerror(errno));
				}
				ret = p_param->result;
			}
			break;
		case V_GET_INITIAL_INFO_FOR_STREAMING_MODE_ONLY:
			{
				VDEC_SEQ_HEADER_t* p_param = args;
				if(ioctl(pInst->dec_fd, V_GET_INITIAL_INFO_FOR_STREAMING_MODE_ONLY_RESULT, args) < 0){
					LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_GET_INITIAL_INFO_FOR_STREAMING_MODE_ONLY_RESULT, strerror(errno));
				}
				ret = p_param->result;
			}
			break;
		case V_GET_VPU_VERSION:
			{
				VDEC_GET_VERSION_t* p_param = args;
				if(ioctl(pInst->dec_fd, V_GET_VPU_VERSION_RESULT, args) < 0){
					LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_GET_VPU_VERSION_RESULT, strerror(errno));
				}
				ret = p_param->result;
			}
			break;

		case V_DEC_REG_FRAME_BUFFER:			
		case V_DEC_BUF_FLAG_CLEAR:
		case V_DEC_CLOSE:
		case V_DEC_GET_INFO:			
		case V_DEC_REG_FRAME_BUFFER2:
		default:
			if(ioctl(pInst->dec_fd, V_DEC_GENERAL_RESULT, &ret) < 0){
				LOGE("VDEC[%d] ioctl(0x%x) error[%s]!!", pInst->vdec_instance_index, V_DEC_GENERAL_RESULT, strerror(errno));
			}
			break;			
	}

	if((ret&0xf000) != 0x0000){ //If there is an invalid return, we skip it because this return means that vpu didn't process current command yet.
		all_retry_cnt--;
		if( all_retry_cnt > 0)
		{
			retry_cnt = 10;
			goto Retry;
		}
		else
		{
			LOGE("abnormal exception!!");
		}
	}

#ifdef ERROR_TEST
	if (err_test++ == 1000)
		ret = 0xf000;
#endif

	if(!success
		|| ((ret&0xf000) != 0x0000) /* vpu can not start or finish its processing with unknown reason!! */
	)
	{	
		LOGE("VDEC[%d] command(0x%x) didn't work properly. maybe hangup(no return(0x%x))!!", pInst->vdec_instance_index, cmd, ret);

		if(ret != RETCODE_CODEC_EXIT && ret != RETCODE_MULTI_CODEC_EXIT_TIMEOUT){
//			ioctl(pInst->mgr_fd, VPU_HW_RESET, (void*)NULL);
		}

		return RETCODE_CODEC_EXIT;
	}

	return ret;
}

void vpu_set_additional_refframe_count(int count, void* pInst)
{
	_vdec_ *pInst_dec = (_vdec_ *)pInst;

	if(!pInst){
		ALOGE("vpu_set_additional_refframe_count :: Instance is null!!");
		return;
	}
	pInst_dec->gsAdditionalFrameCount = count;
	DSTATUS( "[VDEC-%d] gsAdditionalFrameCount %d", pInst_dec->vdec_instance_index, pInst_dec->gsAdditionalFrameCount );
}

static void save_input_stream(char* name, int size, _vdec_ * pVdec, int check_size)
{
#ifdef VPU_IN_FRAME_DUMP
	int i;
	_vdec_ * pInst = pVdec;
	unsigned char* ps = (unsigned char*)pInst->gsBitstreamBufAddr[VA];
	unsigned int len = size;
	unsigned char* pl = (unsigned char*)&len;
	char pName[256] = {0,};
	int oldDelete_file = 0;
	int newOpen_file = 0;
	unsigned int prev_LenDump = 0;

	if(check_size && MAX_DUMP_CNT > 1)
	{
		if( pInst->lenDump > MAX_DUMP_LEN )
		{
			newOpen_file = 1;
			pInst->IdxDump += 1;
			pInst->IdxDump = pInst->IdxDump%MAX_DUMP_CNT;
			prev_LenDump = pInst->lenDump;
			pInst->lenDump = 0;
			if(pInst->IdxDump == 0 && pInst->DelDump == 0)
			{
				pInst->DelDump = 1;
			}

			if( pInst->DelDump )
				oldDelete_file = 1;
		}
		sprintf(pName, "%s/%d_%d_%s", TARGET_STORAGE, pInst->vdec_instance_index, pInst->IdxDump, name);
		if(newOpen_file){
			ALOGD(" Stream-Dump : previous Dump length = %ld -> new file: %s", prev_LenDump, pName);
		}
	}
	else
	{
		sprintf(pName, "%s/%d_%s\n", TARGET_STORAGE, pInst->vdec_instance_index, name);
	}
	//ALOGD( "[VDEC - Stream] size: 0x%x 0x%x 0x%x 0x%x, stream: 0x%x 0x%x 0x%x 0x%x 0x%x ", pl[3], pl[2], pl[1], pl[0], ps[0], ps[1], ps[2], ps[3], ps[4]);
	
	if(1)
	{
		FILE *pFs;

		if(oldDelete_file){
			pFs = fopen(pName, "w+");
		}
		else{
			pFs = fopen(pName, "ab+");
		}
		if (!pFs) {
			LOGE("Cannot open %s [Err: %s]", pName, strerror(errno));
			return;
		}

		//save length of stream!!
		fwrite( &pl[3], 1, 1, pFs);
		fwrite( &pl[2], 1, 1, pFs);
		fwrite( &pl[1], 1, 1, pFs);
		fwrite( &pl[0], 1, 1, pFs);
		//save stream itself!!
		fwrite( ps, size, 1, pFs);
		fclose(pFs);

		if(check_size)
			pInst->lenDump += (size+4);
		return;
	}

	for(i=0; (i+10 <size) && (i+10 < 100); i += 10){
		DPRINTF_FRAME( "[VDEC - Stream] 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", ps[i], ps[i+1], ps[i+2], ps[i+3], ps[i+4], ps[i+5], ps[i+6], ps[i+7], ps[i+8], ps[i+9] );
	}
#endif
}


#ifdef VPU_OUT_FRAME_DUMP
static void save_decoded_frame(unsigned char* Y, unsigned char* U, unsigned char *V, int width, int height, _vdec_ *pVdec)
{
	FILE *pFs;

	if(width == 176 && height == 144)
	{
		pFs = fopen("/sdcard/frame1.yuv", "ab+");
		if (!pFs) {
			LOGE("Cannot open '/sdcard/frame1.yuv'");
			return;
		}

		fwrite( Y, width*height, 1, pFs);
		fwrite( U, width*height/4, 1, pFs);
		fwrite( V, width*height/4, 1, pFs);
		fclose(pFs);
	}
	else if(width == 320 && height == 240)
	{
		pFs = fopen("/sdcard/frame2.yuv", "ab+");
		if (!pFs) {
			LOGE("Cannot open '/sdcard/frame2.yuv'");
			return;
		}

		fwrite( Y, width*height, 1, pFs);
		fwrite( U, width*height/4, 1, pFs);
		fwrite( V, width*height/4, 1, pFs);
		fclose(pFs);
	}
	else if(width == 640 && height == 480)
	{
		pFs = fopen("/sdcard/frame3.yuv", "ab+");
		if (!pFs) {
			LOGE("Cannot open '/sdcard/frame3.yuv'");
			return;
		}

		fwrite( Y, width*height, 1, pFs);
		fwrite( U, width*height/4, 1, pFs);
		fwrite( V, width*height/4, 1, pFs);
		fclose(pFs);
	}
}	
#endif

#ifdef CHANGE_INPUT_STREAM
static void change_input_stream(unsigned char* out, int* len, int type, _vdec_ *pVdec)
{
	_vdec_ * pInst = pVdec;
	char length[4] = {0,};
	int log = 1;

	if(!pInst->pFs)
	{
		pInst->pFs = fopen(STREAM_NAME, "rb");
		if (!pInst->pFs) 
		{
			LOGE("Cannot open '%s'", STREAM_NAME);
			return;
		}
	}

	if(type == VDEC_DECODE && pInst->is1st_dec)
	{
		pInst->is1st_dec = 0;
		memcpy( out, pInst->backup_data, pInst->backup_len);
		*len = pInst->backup_len;
		if(log)
			LOGD("DEC => read[%d] :: %p %p %p %p %p %p %p %p %p %p", *len, out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9]);
		return;
	}

	//read stream-length
	fread( length, 4, 1, pInst->pFs); 
	len = (int*)length;
	//*len = (length[3]) | (length[2] << 8) | (length[1] << 16) | (length[0] << 24);

	if(log)
		LOGD("length(0x%x) => :: %p %p %p %p", *len, length[0], length[1], length[2], length[3]);
	
	//read stream
	fread( out, *len, 1, pInst->pFs);

	if(pInst->is1st_dec){
		if(pInst->backup_data == NULL){
			pInst->backup_data = TCC_malloc(*len);
		}
		else if( len > pInst->backup_len ){
			TCC_realloc(pInst->backup_data, *len);
		}
		memcpy(pInst->backup_data, out, *len);
		pInst->backup_len = *len;
	}

	if(log)
		LOGD("read[%d] :: %p %p %p %p %p %p %p %p %p %p", *len, out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9]);
}
#endif	

unsigned char *vpu_getBitstreamBufAddr(unsigned int index, void * pVdec)
{
	unsigned char *pBitstreamBufAddr = NULL;
	_vdec_ * pInst = pVdec;

	if(!pInst){
		ALOGE("vpu_getBitstreamBufAddr :: Instance is null!!");
		return NULL;
	}
	
	if (index == PA)
	{
		pBitstreamBufAddr = (unsigned char *)pInst->gsBitstreamBufAddr[PA];
	}
	else if (index == VA)
	{
		pBitstreamBufAddr = (unsigned char *)pInst->gsBitstreamBufAddr[VA];
	}
	else /* default : PA */
	{
		pBitstreamBufAddr = (unsigned char *)pInst->gsBitstreamBufAddr[PA];
	}

	return pBitstreamBufAddr;
}

unsigned char *vpu_getFrameBufVirtAddr(unsigned char *convert_addr, unsigned int base_index, void *pVdec)
{
	unsigned char *pBaseAddr; 
	unsigned char *pTargetBaseAddr = NULL;
	unsigned int szAddrGap = 0;
	_vdec_ * pInst = pVdec;

	if(!pInst){
		ALOGE("vpu_getFrameBufVirtAddr :: Instance is null!!");
		return NULL;
	}
	
	pTargetBaseAddr = (unsigned char*)pInst->gsFrameBufAddr[VA];
	
	if (base_index == K_VA)
	{
		pBaseAddr = (unsigned char*)pInst->gsFrameBufAddr[K_VA];
	}
	else /* default : PA */
	{
		pBaseAddr = (unsigned char*)pInst->gsFrameBufAddr[PA];
	}

	szAddrGap = convert_addr - pBaseAddr;
	
	return (pTargetBaseAddr+szAddrGap);
}

int vpu_getBitstreamBufSize(void *pVdec)
{
	_vdec_ * pInst = pVdec;

	if(!pInst){
		ALOGE("vpu_getBitstreamBufSize :: Instance is null!!");
		return 0;
	}
	
	return pInst->gsBitstreamBufSize;
}

static void set_dec_common_info(int w, int h, pic_crop_t* crop, int interlace, int mjpgFmt, _vdec_ *pVdec)
{
	_vdec_ * pInst = pVdec;

	pInst->gsCommDecInfo.m_iPicWidth = w;
	pInst->gsCommDecInfo.m_iPicHeight= h;

	if( pInst->gsCommDecInfo.m_iPicCrop.m_iCropRight == 0 && pInst->gsCommDecInfo.m_iPicCrop.m_iCropBottom == 0 )
	{
		pInst->gsCommDecInfo.m_iPicCrop.m_iCropLeft  	= crop->m_iCropLeft;
		pInst->gsCommDecInfo.m_iPicCrop.m_iCropTop 		= crop->m_iCropTop;
		pInst->gsCommDecInfo.m_iPicCrop.m_iCropRight 	= w - pInst->mRealPicWidth - crop->m_iCropLeft;
		pInst->gsCommDecInfo.m_iPicCrop.m_iCropBottom 	= h - pInst->mRealPicHeight - crop->m_iCropTop;
	}
	pInst->gsCommDecInfo.m_iInterlace = interlace;
	pInst->gsCommDecInfo.m_iMjpg_sourceFormat= mjpgFmt;

	DSTATUS("[COMM-%d] common-Info %d - %d - %d, %d - %d - %d, inter(%d)/mjpgFmt(%d) ", pInst->vdec_instance_index, pInst->gsCommDecInfo.m_iPicWidth, pInst->gsCommDecInfo.m_iPicCrop.m_iCropLeft, pInst->gsCommDecInfo.m_iPicCrop.m_iCropRight,
				pInst->gsCommDecInfo.m_iPicHeight, pInst->gsCommDecInfo.m_iPicCrop.m_iCropTop, pInst->gsCommDecInfo.m_iPicCrop.m_iCropBottom,
				pInst->gsCommDecInfo.m_iInterlace, pInst->gsCommDecInfo.m_iMjpg_sourceFormat);
}


static void print_dec_initial_info( dec_init_t* pDecInit, dec_initial_info_t* pInitialInfo, _vdec_ *pVdec)
{
	unsigned int fRateInfoRes = pInitialInfo->m_uiFrameRateRes;
	unsigned int fRateInfoDiv = pInitialInfo->m_uiFrameRateDiv;
	_vdec_ * pInst = pVdec;
	int userDataEnable = 0;
	int profile = 0;
	int level =0;

	DSTATUS("[VDEC-%d]---------------VIDEO INITIAL INFO-----------------", pInst->vdec_instance_index);
	if (pDecInit->m_iBitstreamFormat == STD_MPEG4) {
		DSTATUS("[VDEC-%d] Data Partition Enable Flag [%1d]", pInst->vdec_instance_index , pInitialInfo->m_iM4vDataPartitionEnable);
		DSTATUS("[VDEC-%d] Reversible VLC Enable Flag [%1d]", pInst->vdec_instance_index , pInitialInfo->m_iM4vReversibleVlcEnable);
		if (pInitialInfo->m_iM4vShortVideoHeader) {			
			DSTATUS("[VDEC-%d] Short Video Header", pInst->vdec_instance_index);
			DSTATUS("[VDEC-%d] AnnexJ Enable Flag [%1d]", pInst->vdec_instance_index , pInitialInfo->m_iM4vH263AnnexJEnable);
		} else
			DSTATUS("[VDEC-%d] Not Short Video", pInst->vdec_instance_index);		
	}

	switch(pDecInit->m_iBitstreamFormat) {
	case STD_MPEG2:
		profile = (pInitialInfo->m_iProfile==0 || pInitialInfo->m_iProfile>5) ? 5 : (pInitialInfo->m_iProfile-1);
		level = pInitialInfo->m_iLevel==4 ? 0 : pInitialInfo->m_iLevel==6 ? 1 : pInitialInfo->m_iLevel==8 ? 2 : pInitialInfo->m_iLevel==10 ? 3 : 4;
		break;
	case STD_MPEG4:
		if (pInitialInfo->m_iLevel & 0x80) 
		{
			// if VOS Header 

			if (pInitialInfo->m_iLevel == 8 && pInitialInfo->m_iProfile == 0) {
				level = 0; profile = 0; // Simple, Level_L0
			} else {
				switch(pInitialInfo->m_iProfile) {
					case 0xB:	profile = 2; break;
					case 0xF:	if( (pInitialInfo->m_iLevel&8) == 0) 
									profile = 1; 
								else
									profile = 5;
								break;
					case 0x0:	profile = 0; break;
					default :	profile = 5; break;
				}
				level = pInitialInfo->m_iLevel;
			}
			
			DSTATUS("[VDEC-%d] VOS Header:%d, %d", pInst->vdec_instance_index , profile, level);
		} 
		else 
		{ 
			// Vol Header Only
			level = 7; // reserved level
			switch(pInitialInfo->m_iProfile) {
				case  0x1: profile = 0; break; // simple object
				case  0xC: profile = 2; break; // advanced coding efficiency object
				case 0x11: profile = 1; break; // advanced simple object
				default  : profile = 5; break; // reserved
			}
			DSTATUS("[VDEC-%d] VOL Header:%d, %d", pInst->vdec_instance_index , profile, level);
		}

		if( level > 7 )
			level = 0;
		break;
	case STD_VC1:
		profile = pInitialInfo->m_iProfile;
		level = pInitialInfo->m_iLevel;
		break;
	case STD_AVC:
	case STD_MVC:
		profile = (pInitialInfo->m_iProfile==66) ? 0 : (pInitialInfo->m_iProfile==77) ? 1 : (pInitialInfo->m_iProfile==88) ? 2 : (pInitialInfo->m_iProfile==100) ? 3 : 4;
		if(profile<3) {
			// BP, MP, EP
			level = (pInitialInfo->m_iLevel==11 && pInitialInfo->m_iAvcConstraintSetFlag[3] == 1) ? 0 /*1b*/ 
				: (pInitialInfo->m_iLevel>=10 && pInitialInfo->m_iLevel <= 51) ? 1 : 2;
		} else {
			// HP
			level = (pInitialInfo->m_iLevel==9) ? 0 : (pInitialInfo->m_iLevel>=10 && pInitialInfo->m_iLevel <= 51) ? 1 : 2;
		}
		
		break;
	case STD_RV:
		profile = pInitialInfo->m_iProfile - 8;
		level = pInitialInfo->m_iLevel;
		break;
	case STD_H263:
		profile = pInitialInfo->m_iProfile;
		level = pInitialInfo->m_iLevel;
		break;
	case STD_DIV3:
		profile = pInitialInfo->m_iProfile;
		level = pInitialInfo->m_iLevel;
		break;
	default: // STD_MJPG
		;
	}

	if( level >= LEVEL_MAX )
	{
		DSTATUS("[VDEC-%d] Invalid \"level\" value: %d", pInst->vdec_instance_index , level);
		level = 0;
	}
	if( profile >= PROFILE_MAX )
	{
		DSTATUS("[VDEC-%d] Invalid \"profile\" value: %d", pInst->vdec_instance_index , profile);
		profile = 0;
	}
	if( pDecInit->m_iBitstreamFormat >= VCODEC_MAX )
	{
		DSTATUS("[VDEC-%d] Invalid \"m_iBitstreamFormat\" value: %d", pInst->vdec_instance_index , pDecInit->m_iBitstreamFormat);
		pDecInit->m_iBitstreamFormat = 0;
	}

	// No Profile and Level information in WMV78
	if( 
		(pDecInit->m_iBitstreamFormat != STD_MJPG)
	)
	{
		DSTATUS("[VDEC-%d] %s\r", pInst->vdec_instance_index , strProfile[pDecInit->m_iBitstreamFormat][profile]);
		if (pDecInit->m_iBitstreamFormat != STD_RV) { // No level information in Rv.
			if ((pDecInit->m_iBitstreamFormat == STD_AVC || pDecInit->m_iBitstreamFormat == STD_MVC) && level != 0 && level != 2){
				DSTATUS("[VDEC-%d] %s%.1f\r", pInst->vdec_instance_index , strLevel[pDecInit->m_iBitstreamFormat][level], (float)pInitialInfo->m_iLevel/10);
			}
			else{
				DSTATUS("[VDEC-%d] %s\r", pInst->vdec_instance_index , strLevel[pDecInit->m_iBitstreamFormat][level]);
			}
		}
	}
	
	if(pDecInit->m_iBitstreamFormat == STD_AVC || pDecInit->m_iBitstreamFormat == STD_MVC) {
		DSTATUS("[VDEC-%d] frame_mbs_only_flag : %d", pInst->vdec_instance_index , pInitialInfo->m_iInterlace);
	} else if (pDecInit->m_iBitstreamFormat != STD_RV) {// No interlace information in Rv.
		if (pInitialInfo->m_iInterlace){
			DSTATUS("[VDEC-%d] %s", pInst->vdec_instance_index , "Interlaced Sequence");
		}
		else{
			DSTATUS("[VDEC-%d] %s", pInst->vdec_instance_index , "Progressive Sequence");
		}
	}

	if (pDecInit->m_iBitstreamFormat == STD_VC1) {
		if (pInitialInfo->m_iVc1Psf){
			DSTATUS("[VDEC-%d] %s", pInst->vdec_instance_index , "VC1 - Progressive Segmented Frame");
		}
		else{
			DSTATUS("[VDEC-%d] %s", pInst->vdec_instance_index , "VC1 - Not Progressive Segmented Frame");
		}
	}

	DSTATUS("[VDEC-%d] Aspect Ratio [%1d]", pInst->vdec_instance_index , pInitialInfo->m_iAspectRateInfo);
				
	switch (pDecInit->m_iBitstreamFormat) {
	case STD_AVC:
	case STD_MVC:
        	DSTATUS("[VDEC-%d] H.264 Profile:%d Level:%d FrameMbsOnlyFlag:%d", pInst->vdec_instance_index ,
			pInitialInfo->m_iProfile, pInitialInfo->m_iLevel, pInitialInfo->m_iInterlace);
		
		if(pInitialInfo->m_iAspectRateInfo) {
			int aspect_ratio_idc;
			int sar_width, sar_height;

			if( (pInitialInfo->m_iAspectRateInfo>>16)==0 ) {
				aspect_ratio_idc = (pInitialInfo->m_iAspectRateInfo & 0xFF);
				DSTATUS("[VDEC-%d] aspect_ratio_idc :%d", pInst->vdec_instance_index , aspect_ratio_idc);
			}
			else {
				sar_width  = (pInitialInfo->m_iAspectRateInfo >> 16);
				sar_height  = (pInitialInfo->m_iAspectRateInfo & 0xFFFF);
				DSTATUS("[VDEC-%d] sar_width  : %dsar_height : %d", pInst->vdec_instance_index , sar_width, sar_height);				
			}
		} else {
			LOGE("[VDEC-%d] Aspect Ratio is not present", pInst->vdec_instance_index);
		}

		break;
	case STD_VC1:
		if(pInitialInfo->m_iProfile == 0){
			DSTATUS("[VDEC-%d] VC1 Profile: Simple", pInst->vdec_instance_index);
		}
		else if(pInitialInfo->m_iProfile == 1){
			DSTATUS("[VDEC-%d] VC1 Profile: Main", pInst->vdec_instance_index);
		}
		else if(pInitialInfo->m_iProfile == 2){
			DSTATUS("[VDEC-%d] VC1 Profile: Advanced", pInst->vdec_instance_index);
		}
		
		DSTATUS("[VDEC-%d] Level: %d Interlace: %d PSF: %d", pInst->vdec_instance_index , 
			pInitialInfo->m_iLevel, pInitialInfo->m_iInterlace, pInitialInfo->m_iVc1Psf);

		if(pInitialInfo->m_iAspectRateInfo){
			DSTATUS("[VDEC-%d] Aspect Ratio [X, Y]:[%3d, %3d]", pInst->vdec_instance_index , (pInitialInfo->m_iAspectRateInfo>>8)&0xff,
					(pInitialInfo->m_iAspectRateInfo)&0xff);
		}
		else{
			DSTATUS("[VDEC-%d] Aspect Ratio is not present", pInst->vdec_instance_index);
		}


		break;
	case STD_MPEG2:
        	DSTATUS("[VDEC-%d] Mpeg2 Profile:%d Level:%d Progressive Sequence Flag:%d", pInst->vdec_instance_index ,
			pInitialInfo->m_iProfile, pInitialInfo->m_iLevel, pInitialInfo->m_iInterlace);
		// Profile: 3'b101: Simple, 3'b100: Main, 3'b011: SNR Scalable, 
		// 3'b10: Spatially Scalable, 3'b001: High
		// Level: 4'b1010: Low, 4'b1000: Main, 4'b0110: High 1440, 4'b0100: High
		if(pInitialInfo->m_iAspectRateInfo){
			DSTATUS("[VDEC-%d] Aspect Ratio Table index :%d", pInst->vdec_instance_index , pInitialInfo->m_iAspectRateInfo);
		}
		else{
			DSTATUS("[VDEC-%d] Aspect Ratio is not present", pInst->vdec_instance_index);
		}
        	break;

	case STD_MPEG4:
        	DSTATUS("[VDEC-%d] Mpeg4 Profile: %d Level: %d Interlaced: %d", pInst->vdec_instance_index ,
			pInitialInfo->m_iProfile, pInitialInfo->m_iLevel, pInitialInfo->m_iInterlace);
		// Profile: 8'b00000000: SP, 8'b00010001: ASP
		// Level: 4'b0000: L0, 4'b0001: L1, 4'b0010: L2, 4'b0011: L3, ...
		// SP: 1/2/3/4a/5/6, ASP: 0/1/2/3/4/5
		
		if(pInitialInfo->m_iAspectRateInfo){
			DSTATUS("[VDEC-%d] Aspect Ratio Table index :%d", pInst->vdec_instance_index , pInitialInfo->m_iAspectRateInfo);
		}
		else{
			DSTATUS("[VDEC-%d] Aspect Ratio is not present", pInst->vdec_instance_index);
		}
		break;
		
	case STD_H263:
        	DSTATUS("[VDEC-%d] H.263 ", pInst->vdec_instance_index);
		break;

	case STD_RV:
        	DSTATUS("[VDEC-%d] Real Video Version %d", pInst->vdec_instance_index ,	pInitialInfo->m_iProfile);
        	break;
   	}

	if (pDecInit->m_iBitstreamFormat == STD_RV) // RV has no user data
		userDataEnable = 0;


	DSTATUS("[VDEC-%d] Dec InitialInfo => m_iPicWidth: %u m_iPicHeight: %u frameRate: %.2f frRes: %u frDiv: %u", pInst->vdec_instance_index ,
		pInitialInfo->m_iPicWidth, pInitialInfo->m_iPicHeight, (double)fRateInfoRes/fRateInfoDiv, fRateInfoRes, fRateInfoDiv);

	DSTATUS("[VDEC-%d] ---------------------------------------------------", pInst->vdec_instance_index);
	
}

int
vpu_dec_ready( dec_init_t* psVDecInit, _vdec_ *pVdec)
{
	//------------------------------------------------------------
	// [x] PS(SPS/PPS) buffer for each VPU decoder
	//------------------------------------------------------------
	_vdec_ * pInst = pVdec;
	if( psVDecInit->m_iBitstreamFormat == STD_AVC || psVDecInit->m_iBitstreamFormat == STD_MVC)
	{
		pInst->gsSpsPpsSize = PS_SAVE_SIZE;
		pInst->gsSpsPpsSize = ALIGNED_BUFF( pInst->gsSpsPpsSize, ALIGN_LEN );
		pInst->gsSpsPpsAddr = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->gsSpsPpsSize, BUFFER_PS, pInst );
		if( pInst->gsSpsPpsAddr == 0 ) 
		{
			DPRINTF( "[VDEC-%d] sps_pps_buf_addr malloc() failed ", pInst->vdec_instance_index);
			return -(VPU_NOT_ENOUGH_MEM);
		}
		DSTATUS("[VDEC-%d] sps_pps_buf_addr = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsSpsPpsAddr, pInst->gsSpsPpsSize );

		psVDecInit->m_pSpsPpsSaveBuffer = (unsigned char*)pInst->gsSpsPpsAddr;
		psVDecInit->m_iSpsPpsSaveBufferSize = pInst->gsSpsPpsSize;
	}

	//------------------------------------------------------------
	//! [x] bitstream buffer for each VPU decoder
	//------------------------------------------------------------

	if(psVDecInit->m_iBitstreamBufSize > LARGE_STREAM_BUF_SIZE)
		pInst->gsBitstreamBufSize = ALIGNED_BUFF( psVDecInit->m_iBitstreamBufSize, 64*1024 );
	else
		pInst->gsBitstreamBufSize = LARGE_STREAM_BUF_SIZE;
	pInst->gsBitstreamBufSize = ALIGNED_BUFF( pInst->gsBitstreamBufSize, ALIGN_LEN );
	pInst->gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitstreamBufAddr[K_VA], pInst->gsBitstreamBufSize, BUFFER_STREAM, pInst );
	
	if( pInst->gsBitstreamBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] bitstream_buf_addr[PA] malloc() failed ", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS( "[VDEC-%d] bitstream_buf_addr[PA] = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize );
	pInst->gsBitstreamBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize, pInst );
	if( pInst->gsBitstreamBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] bitstream_buf_addr[VA] malloc() failed ", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	memset( (void*)pInst->gsBitstreamBufAddr[VA], 0x00 , pInst->gsBitstreamBufSize);
	DSTATUS("[VDEC-%d] bitstream_buf_addr[VA] = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitstreamBufAddr[VA], pInst->gsBitstreamBufSize );

	psVDecInit->m_BitstreamBufAddr[PA]	= pInst->gsBitstreamBufAddr[PA];
	psVDecInit->m_BitstreamBufAddr[VA]	= pInst->gsBitstreamBufAddr[K_VA];	
	psVDecInit->m_iBitstreamBufSize 	= pInst->gsBitstreamBufSize;

	if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable == 0)
	{
#if 0 // intermediate buffer is not used.
		pInst->gsIntermediateBufSize = LARGE_STREAM_BUF_SIZE;
		pInst->gsIntermediateBufSize = ALIGNED_BUFF( pInst->gsIntermediateBufSize, ALIGN_LEN );
		pInst->gsIntermediateBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsIntermediateBufAddr[K_VA], pInst->gsIntermediateBufSize, BUFFER_STREAM, pInst );
		
		if( pInst->gsIntermediateBufAddr[PA] == 0 ) 
		{
			DPRINTF( "[VDEC-%d] gsIntermediateBufAddr[PA] malloc() failed ", pInst->vdec_instance_index);
			return -(VPU_NOT_ENOUGH_MEM);
		}
		DSTATUS( "[VDEC-%d] bitstream_buf_addr[PA] = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsIntermediateBufAddr[PA], pInst->gsIntermediateBufSize );
		pInst->gsIntermediateBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( pInst->gsIntermediateBufAddr[PA], pInst->gsIntermediateBufSize, pInst );
		if( pInst->gsIntermediateBufAddr[VA] == 0 ) 
		{
			DPRINTF( "[VDEC-%d] gsIntermediateBufAddr[VA] malloc() failed ", pInst->vdec_instance_index);
			return -(VPU_NOT_ENOUGH_MEM);
		}
		memset( (void*)pInst->gsIntermediateBufAddr[VA], 0x00 , pInst->gsIntermediateBufSize);
		DSTATUS("[VDEC-%d] gsIntermediateBufAddr[VA] = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsIntermediateBufAddr[VA], pInst->gsIntermediateBufSize );
#endif
	}
	else
	{
		pInst->gsIntermediateBufSize = 0;
		pInst->gsIntermediateBufAddr[PA] = 0;
		pInst->gsIntermediateBufAddr[VA] = 0;
		pInst->gsIntermediateBufAddr[K_VA] = 0;
	}

	/* Set the maximum size of input bitstream. */
//	gsMaxBitstreamSize = MAX_BITSTREAM_SIZE;
//	gsMaxBitstreamSize = ALIGNED_BUFF(gsMaxBitstreamSize, (4 * 1024));
//	if (gsMaxBitstreamSize > gsBitstreamBufSize)
//	{
		pInst->gsMaxBitstreamSize = pInst->gsBitstreamBufSize;
//	}

	//------------------------------------------------------------
	//! [x] user data buffer for each VPU decoder
	//------------------------------------------------------------
	if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
	{
		pInst->gsUserdataBufSize = 50 * 1024;
		pInst->gsUserdataBufSize = ALIGNED_BUFF( pInst->gsUserdataBufSize, ALIGN_LEN );
		pInst->gsUserdataBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsUserdataBufAddr[K_VA], pInst->gsUserdataBufSize, BUFFER_ELSE, pInst );
		if( pInst->gsUserdataBufAddr[PA] == 0 ) 
		{
			DPRINTF( "[VDEC-%d:Err%d] pInst->gsUserdataBufAddr physical alloc failed ", pInst->vdec_instance_index, -1 );
			return -(VPU_NOT_ENOUGH_MEM);
		}
		DSTATUS( "[VDEC-%d] pInst->gsUserdataBufAddr[PA] = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsUserdataBufAddr[PA], pInst->gsUserdataBufSize );
		pInst->gsUserdataBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( pInst->gsUserdataBufAddr[PA], pInst->gsUserdataBufSize, pInst );
		if( pInst->gsUserdataBufAddr[VA] == 0 ) 
		{
			DPRINTF( "[VDEC-%d:Err%d] pInst->gsUserdataBufAddr virtual alloc failed ", pInst->vdec_instance_index, -1 );
			return -(VPU_NOT_ENOUGH_MEM);
		}
		//memset( (void*)pInst->gsUserdataBufAddr[VA], 0 , gsUserdataBufSize);
		DSTATUS("[VDEC-%d] pInst->gsUserdataBufAddr[VA] = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsUserdataBufAddr[VA], pInst->gsUserdataBufSize );
	}
	
	//------------------------------------------------------------
	// [x] code buffer, work buffer and parameter buffer for VPU 
	//------------------------------------------------------------
	pInst->gsBitWorkBufSize = WORK_CODE_PARA_BUF_SIZE;
	pInst->gsBitWorkBufSize = ALIGNED_BUFF(pInst->gsBitWorkBufSize, ALIGN_LEN);
	pInst->gsBitWorkBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitWorkBufAddr[K_VA], pInst->gsBitWorkBufSize, BUFFER_WORK, pInst );
	if( pInst->gsBitWorkBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] bit_work_buf_addr[PA] malloc() failed ", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS("[VDEC-%d] bit_work_buf_addr[PA] = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitWorkBufAddr[PA], pInst->gsBitWorkBufSize );
	pInst->gsBitWorkBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( pInst->gsBitWorkBufAddr[PA], pInst->gsBitWorkBufSize, pInst );
	if( pInst->gsBitWorkBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] bit_work_buf_addr[VA] malloc() failed ", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS("[VDEC-%d] bit_work_buf_addr[VA] = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitWorkBufAddr[VA], pInst->gsBitWorkBufSize );

	psVDecInit->m_BitWorkAddr[PA] = pInst->gsBitWorkBufAddr[PA];
	psVDecInit->m_BitWorkAddr[VA] = pInst->gsBitWorkBufAddr[K_VA];

	psVDecInit->m_bCbCrInterleaveMode = psVDecInit->m_bCbCrInterleaveMode;
	if( psVDecInit->m_bCbCrInterleaveMode == 0 ){
		DSTATUS("[VDEC-%d] CbCrInterleaveMode OFF", pInst->vdec_instance_index);
	}
	else{
		DSTATUS("[VDEC-%d] CbCrInterleaveMode ON", pInst->vdec_instance_index);
	}

	if( psVDecInit->m_uiDecOptFlags&M4V_DEBLK_ENABLE ){
		DSTATUS( "[VDEC-%d] MPEG-4 Deblocking ON" , pInst->vdec_instance_index);
	}
	if( psVDecInit->m_uiDecOptFlags&M4V_GMC_FRAME_SKIP ){
		DSTATUS( "[VDEC-%d] MPEG-4 GMC Frame Skip" , pInst->vdec_instance_index);
	}

	return 0;
}

int
vpu_dec_seq_header( int iSize, int iIsThumbnail, _vdec_ *pVdec )
{
	int ret = 0;
	_vdec_ * pInst = pVdec;
	LOGI("[VDEC-%d] vpu_dec_seq_header in :: size(%d), JpegOnly(%d), format(%d)", pInst->vdec_instance_index, iSize ,pInst->gsUserInfo.m_bStillJpeg, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat);
	pInst->gsVpuDecSeqHeader_Info.stream_size = iSize;
	{
		unsigned char* ps = (unsigned char*)pInst->gsBitstreamBufAddr[VA];
		SEQ_EXTRACTOR( "[VDEC-%d Seq %d] " "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x " "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
							"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x " "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
							"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ", 
							pInst->vdec_instance_index, iSize,
							ps[0], ps[1], ps[2], ps[3], ps[4], ps[5], ps[6], ps[7], ps[8], ps[9], ps[10], ps[11], ps[12], ps[13], ps[14], ps[15],
							ps[16], ps[17], ps[18], ps[19], ps[20], ps[21], ps[22], ps[23], ps[24], ps[25], ps[26], ps[27], ps[28], ps[29], ps[30], ps[31],
							ps[32], ps[33], ps[34], ps[35], ps[36], ps[37], ps[38], ps[39], ps[40], ps[41], ps[42], ps[43], ps[44], ps[45], ps[46], ps[47],
							ps[48], ps[49], ps[50], ps[51], ps[52], ps[53], ps[54], ps[55], ps[56], ps[57], ps[58], ps[59], ps[60], ps[61], ps[62], ps[63],
							ps[64], ps[65], ps[66], ps[67], ps[68], ps[69], ps[70], ps[71], ps[72], ps[73], ps[74], ps[75], ps[76], ps[77], ps[78], ps[79]);
	}
	
	ret = vdec_cmd_process(V_DEC_SEQ_HEADER, &pInst->gsVpuDecSeqHeader_Info, pInst);
	if( ret != RETCODE_SUCCESS )
	{
		DPRINTF( "[VDEC-%d] VPU_DEC_SEQ_HEADER failed Error code is 0x%x. ErrorReason is %d", pInst->vdec_instance_index, ret, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iReportErrorReason);
        if(ret == RETCODE_CODEC_SPECOUT)
            DPRINTF("[VDEC-%d] NOT SUPPORTED CODEC. VPU SPEC OUT!!", pInst->vdec_instance_index);     // This is a very common error. Notice the detailed reason to users.

		pInst->gsCommDecInfo.m_iErrCode = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iReportErrorReason; // mm008
		return -ret;
	}

	print_dec_initial_info( &pInst->gsVpuDecInit_Info.gsVpuDecInit, &pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo, pInst );

	if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_AVC || pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MVC ){
		pInst->mRealPicWidth = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth - pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iAvcPicCrop.m_iCropLeft - pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iAvcPicCrop.m_iCropRight;
		pInst->mRealPicHeight = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight - pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iAvcPicCrop.m_iCropBottom - pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iAvcPicCrop.m_iCropTop;
	}
	else{
		pInst->mRealPicWidth = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth;
		pInst->mRealPicHeight = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight;
	}
#ifdef SET_FRAMEBUFFER_INTO_MAX
	if( (pInst->extFunction & EXT_FUNC_MAX_FRAMEBUFFER) != 0x0 )
	{
		pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth = AVAILABLE_MAX_WIDTH;
		pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight = AVAILABLE_MAX_HEIGHT;
		LOGI("[VDEC-%d]Set seq framebuffer into 1080p (<- %d x %d)", pInst->vdec_instance_index, pInst->mRealPicWidth, pInst->mRealPicHeight);
	}
	else
#endif
	{
		pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth = ((pInst->mRealPicWidth+15)>>4)<<4;
		pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight = pInst->mRealPicHeight;
	}

	set_dec_common_info(pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight,
						&pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iAvcPicCrop, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iInterlace,
						0,
						pInst );

	//------------------------------------------------------------
	// [x] slice buffer for VPU
	//------------------------------------------------------------
	if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_AVC || pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MVC )
	{
		pInst->gsSliceBufSize = SLICE_SAVE_SIZE;
		pInst->gsSliceBufSize = ALIGNED_BUFF( pInst->gsSliceBufSize, ALIGN_LEN );
		pInst->gsSliceBufAddr = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->gsSliceBufSize, BUFFER_SLICE, pInst );
		if( pInst->gsSliceBufAddr == 0 ) 
		{
			DPRINTF( "[VDEC-%d] slice_buf_addr malloc() failed ", pInst->vdec_instance_index);
			return -(VPU_NOT_ENOUGH_MEM);
		}
		DSTATUS("[VDEC-%d] slice_buf_addr = 0x%x, 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsSliceBufAddr, pInst->gsSliceBufSize );

		pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_AvcSliceSaveBufferAddr  = pInst->gsSliceBufAddr;
		pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iAvcSliceSaveBufferSize = pInst->gsSliceBufSize;
	}		
	else
	{
		pInst->gsSliceBufSize = 0;
		pInst->gsSliceBufAddr = 0;
	}




	//------------------------------------------------------------
	// [x] frame buffer for each VPU decoder
	//------------------------------------------------------------
	pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount;
	LOGD( "[VDEC-%d] FrameBufDelay %d, MinFrameBufferCount %d", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iFrameBufDelay, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount  );
	{
		int max_count;

		if(!iIsThumbnail)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + pInst->gsAdditionalFrameCount;
		max_count = cdk_sys_remain_memory_size(pInst) / pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;

		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > max_count)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = max_count;
		
		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > MAX_FRAME_BUFFER_COUNT)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = MAX_FRAME_BUFFER_COUNT;

		if(iIsThumbnail)				
		{
			if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < (pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount))
			{
				LOGE( "[VDEC-%d] Not enough memory for VPU frame buffer, Available[%d], Min[%d], Need[%d], min_size = %d", pInst->vdec_instance_index, max_count, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize);
				return -(VPU_NOT_ENOUGH_MEM);
			}
		}
		else
		{
			if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < (pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount+pInst->gsAdditionalFrameCount))
			{
				LOGE( "[VDEC-%d] Not enough memory for VPU frame buffer, Available[%d], Min[%d], Need[%d], min_size = %d", pInst->vdec_instance_index, max_count, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + pInst->gsAdditionalFrameCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize);
				return -(VPU_NOT_ENOUGH_MEM);
			}
#ifdef SET_FRAMEBUFFER_INTO_MAX
			if( (pInst->extFunction & EXT_FUNC_MAX_FRAMEBUFFER) != 0x0 )
			{
				if( pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < (max_count - 1) )
					pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = (max_count - 1);
				if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > MAX_FRAME_BUFFER_COUNT)
					pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = MAX_FRAME_BUFFER_COUNT;
			}
#endif
		}
	}

	{
		pInst->gsFrameBufSize = pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount * pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;
		LOGD( "[VDEC-%d] FrameBufferCount %d [min %d], min_size = %d ", pInst->vdec_instance_index, pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize);
	}

	pInst->gsFrameBufSize = ALIGNED_BUFF( pInst->gsFrameBufSize, ALIGN_LEN );
	pInst->gsFrameBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsFrameBufAddr[K_VA], pInst->gsFrameBufSize, BUFFER_FRAMEBUFFER, pInst );
	if( pInst->gsFrameBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] frame_buf_addr[PA] malloc() failed ", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}	


	DSTATUS( "[VDEC-%d] MinFrameBufferSize %d bytes ", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize );

	DSTATUS( "[VDEC-%d] frame_buf_addr[PA] = 0x%x, 0x%x , index = %d ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsFrameBufAddr[PA], pInst->gsFrameBufSize, pInst->vdec_instance_index );
	pInst->gsFrameBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( pInst->gsFrameBufAddr[PA], pInst->gsFrameBufSize, pInst );
	if( pInst->gsFrameBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] frame_buf_addr[VA] malloc() failed ", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS("[VDEC-%d] frame_buf_addr[VA] = 0x%x, frame_buf_addr[K_VA] = 0x%x ", pInst->vdec_instance_index, (codec_addr_t)pInst->gsFrameBufAddr[VA], pInst->gsFrameBufAddr[K_VA] );
	pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_FrameBufferStartAddr[PA] = pInst->gsFrameBufAddr[PA];
	pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_FrameBufferStartAddr[VA] = pInst->gsFrameBufAddr[K_VA];

#if 0//def VPU_PERFORMANCE_UP
	{
		 unsigned int regAddr = ((unsigned int)gsRegisterBase + 0x10000); //0xB0910000

		 //VCACHE_CTRL
		 *(volatile unsigned int *)(regAddr+0x00)	= (1<<0);			  //CACHEON

		 //VCACHE_REG
		 *(volatile unsigned int *)(regAddr+0x04)	= (3<<0);			//WR0|RD0
		 *(volatile unsigned int *)(regAddr+0x024)	= gsFrameBufAddr[PA];//VIDEO_PHY_ADDR;							//VCACHE_R0MIN
		 *(volatile unsigned int *)(regAddr+0x028)	= VIDEO_PHY_ADDR+ VIDEO_MEM_SIZE;	  //VCACHE_R0MAX
		 *(volatile unsigned int *)(regAddr+0x02C)	= 0; //VCACHE_R1MIN
		 *(volatile unsigned int *)(regAddr+0x030)	= 0; //VCACHE_R1MAX
		 *(volatile unsigned int *)(regAddr+0x034)	= 0; //VCACHE_R2MIN
		 *(volatile unsigned int *)(regAddr+0x038)	= 0; //VCACHE_R2MAX
		 *(volatile unsigned int *)(regAddr+0x03C)	= 0; //VCACHE_R3MIN
		 *(volatile unsigned int *)(regAddr+0x040)	= 0; //VCACHE_R3MAX
   }
#endif

	ret = vdec_cmd_process(V_DEC_REG_FRAME_BUFFER, &pInst->gsVpuDecBuffer_Info, pInst);

	if( ret != RETCODE_SUCCESS )
	{
		DPRINTF( "[VDEC-%d] DEC_REG_FRAME_BUFFER failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
		return -ret;
	}
	DSTATUS("[VDEC-%d] TCC_VPU_DEC VPU_DEC_REG_FRAME_BUFFER OK!", pInst->vdec_instance_index);
	return ret;
}

int
vdec_vpu( int iOpCode, int* pHandle, void* pParam1, void* pParam2, void * pParam3 )
{
	int ret = 0;
	_vdec_ *pInst = (_vdec_ *)pParam3;

	if(!pInst){
		ALOGE("vdec_vpu(OP:%d) :: Instance is null!!", iOpCode);
		return -RETCODE_NOT_INITIALIZED;
	}
	
	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !pInst->vdec_codec_opened)
		return -RETCODE_NOT_INITIALIZED;

#ifdef DEBUG_TIME_LOG
	clock_t start, end;
	start = clock();
#endif

	if( iOpCode == VDEC_INIT )
	{
		vdec_init_t* p_init_param = (vdec_init_t*)pParam1;

		vdec_user_info_t* p_init_user_param = (vdec_user_info_t*)pParam2;

		pInst->gsUserInfo.bitrate_mbps = p_init_user_param->bitrate_mbps;
		pInst->gsUserInfo.frame_rate   = p_init_user_param->frame_rate;
		pInst->gsUserInfo.m_bStillJpeg  = p_init_user_param->m_bStillJpeg;	
		pInst->gsUserInfo.jpg_ScaleRatio  = p_init_user_param->jpg_ScaleRatio;	

		pInst->codec_format = p_init_param->m_iBitstreamFormat;
		if(vpu_env_open(p_init_param->m_iBitstreamFormat, p_init_user_param->bitrate_mbps, p_init_user_param->frame_rate, p_init_param->m_iPicWidth, p_init_param->m_iPicHeight, pInst ) < 0)
			return -VPU_ENV_INIT_ERROR;	

		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_RegBaseVirtualAddr = (unsigned int)NULL;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat		= p_init_param->m_iBitstreamFormat;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicWidth			= p_init_param->m_iPicWidth;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicHeight			= p_init_param->m_iPicHeight;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData		= p_init_param->m_bEnableUserData;

		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode = p_init_param->m_bCbCrInterleaveMode;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_Memcpy				= NULL; // No need to set!!
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_Memset				= NULL; // No need to set!!
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_Interrupt			= NULL; // No need to set!!

#ifdef VPU_PERFORMANCE_UP
		//gsVpuDecInit.m_bEnableVideoCache	= 1;
	#if defined(TCC_93XX_INCLUDE)
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_ENABLE_TCC93XX;		// use secAXI SRAM0 128K
	#elif defined(TCC_88XX_INCLUDE)
		if((pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicHeight * pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicWidth) > (1280*720))
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_ENABLE_TCC88XX;	// use secAXI SRAM0 80K
		else
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_DISABLE;

		if(pInst->gsUserInfo.m_bStillJpeg)
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_ENABLE_TCC88XX;	// use secAXI SRAM0 80K
	#endif
#else
	#if defined(TCC_VPU_C5_INCLUDE)
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_DISABLE;
	#endif
#endif

		pInst->extFunction = p_init_user_param->extFunction;
#ifdef SET_FRAMEBUFFER_INTO_MAX
		if( (pInst->extFunction & EXT_FUNC_MAX_FRAMEBUFFER) != 0x0 )
		{
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags |= (1<<16);
			DSTATUS("[VDEC-%d]Set framebuffer into 1080p", pInst->vdec_instance_index);
		}
#endif

		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable		= p_init_param->m_bFilePlayEnable;
		pInst->gsbHasSeqHeader = 0;//p_init_param->m_bHasSeqHeader; 

		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamBufSize = p_init_param->m_iBitstreamBufSize;
		ret = vpu_dec_ready( &pInst->gsVpuDecInit_Info.gsVpuDecInit, pInst );
		if( ret != RETCODE_SUCCESS )
		{
			return ret;
		}

		if( (pInst->extFunction & EXT_FUNC_NO_BUFFER_DELAY) != 0x0 )
		{
			LOGI("[VDEC_K] : No BufferDelay Mode....");
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags |= (1<<2);
		}
		DSTATUS("[VDEC-%d]workbuff 0x%x/0x%x, Reg: 0x%x, format : %d, Stream(0x%x/0x%x, %d), Res: %d x %d", pInst->vdec_instance_index, 
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitWorkAddr[PA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitWorkAddr[VA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_RegBaseVirtualAddr,
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[PA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[VA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamBufSize,
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicWidth, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicHeight);
		DSTATUS("[VDEC-%d]optFlag 0x%x, avcBuff: 0x%x- %d, Userdata(%d), VCache: %d, Inter: %d, PlayEn: %d, MaxRes: %d", pInst->vdec_instance_index, 
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags, (unsigned int)pInst->gsVpuDecInit_Info.gsVpuDecInit.m_pSpsPpsSaveBuffer, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iSpsPpsSaveBufferSize,
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData, 
					0,
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode,
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iMaxResolution);
	
		DSTATUS("[VDEC-%d]Format : %d, Stream(0x%x, %d)", pInst->vdec_instance_index, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[PA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamBufSize);
		ret = vdec_cmd_process(V_DEC_INIT, &pInst->gsVpuDecInit_Info, pInst);
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VPU_DEC_INIT failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
			return -ret;
		}

		if( (pInst->extFunction & (EXT_FUNC_MEM_PROTECTION_WITH_INSTREAM | EXT_FUNC_MEM_PROTECTION_ONLY)) == 0x0 )
		{
			pInst->gsVpuDecVersion.pszVersion = (char*)(pInst->gsBitstreamBufAddr[K_VA] + (pInst->gsBitstreamBufSize - 100));
			pInst->gsVpuDecVersion.pszBuildData = (char*)(pInst->gsBitstreamBufAddr[K_VA] + (pInst->gsBitstreamBufSize - 50));

			ret = vdec_cmd_process(V_GET_VPU_VERSION, &pInst->gsVpuDecVersion, pInst);
			if( ret != RETCODE_SUCCESS )
			{
				//If this operation returns fail, it doesn't mean that there's a problem in vpu
				//so do not return error to host.
				DPRINTF( "[VDEC-%d] V_GET_VPU_VERSION failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
			}
			else
			{
				int vpu_closed[VPU_MAX];

				if( 0 <= ioctl(pInst->mgr_fd, VPU_CHECK_CODEC_STATUS, &vpu_closed) ) {
					LOGI("[VDEC-%d] Multi-instance status : %d/%d/%d/%d/%d", pInst->vdec_instance_index, vpu_closed[VPU_DEC], vpu_closed[VPU_DEC_EXT], vpu_closed[VPU_DEC_EXT2], vpu_closed[VPU_DEC_EXT3], vpu_closed[VPU_ENC]);
				}
				LOGI( "[VDEC-%d] V_GET_VPU_VERSION OK. Version is %s, and it's built at %s ", pInst->vdec_instance_index,
							(char*)(pInst->gsBitstreamBufAddr[VA] + (pInst->gsBitstreamBufSize - 100)),
							(char*)(pInst->gsBitstreamBufAddr[VA] + (pInst->gsBitstreamBufSize - 50)));
			}
		}
		
		pInst->vdec_codec_opened = 1;
#ifdef DISPLAY_1ST_DECODED_IDX
		pInst->mdisplayed_1st_IFrm = 0;
#endif
		LOGI( "[VDEC-%d] VPU_DEC_INIT OK( has seq = %d) ", pInst->vdec_instance_index, pInst->gsbHasSeqHeader );

	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{		
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
		int seq_stream_size = (p_input_param->m_iInpLen > pInst->gsMaxBitstreamSize) ? pInst->gsMaxBitstreamSize : p_input_param->m_iInpLen;
		unsigned int iIsThumbnail = p_input_param->m_iIsThumbnail;

		if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable)
		{
			if (    ((codec_addr_t)p_input_param->m_pInp[PA] == pInst->gsBitstreamBufAddr[PA])
			     && ((codec_addr_t)p_input_param->m_pInp[VA] == pInst->gsBitstreamBufAddr[VA]) )
			{
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
			}
			else
			{
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];		
				memcpy( (void*)pInst->gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], seq_stream_size);
#ifdef CHANGE_INPUT_STREAM
				change_input_stream((unsigned char *)pInst->gsBitstreamBufAddr[VA], &seq_stream_size, iOpCode, pInst);
#endif
			}
/*
			if( (pInst->extFunction & (EXT_FUNC_MEM_PROTECTION_WITH_INSTREAM | EXT_FUNC_MEM_PROTECTION_ONLY)) == 0x0 )
			{
				unsigned char* ps = (unsigned char*)pInst->gsBitstreamBufAddr[VA];
				DSTATUS( "[VDEC-%d Seq %d] " "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x " "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
									"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x " "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
									"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ", 
									pInst->vdec_instance_index, seq_stream_size,
									ps[0], ps[1], ps[2], ps[3], ps[4], ps[5], ps[6], ps[7], ps[8], ps[9], ps[10], ps[11], ps[12], ps[13], ps[14], ps[15],
									ps[16], ps[17], ps[18], ps[19], ps[20], ps[21], ps[22], ps[23], ps[24], ps[25], ps[26], ps[27], ps[28], ps[29], ps[30], ps[31],
									ps[32], ps[33], ps[34], ps[35], ps[36], ps[37], ps[38], ps[39], ps[40], ps[41], ps[42], ps[43], ps[44], ps[45], ps[46], ps[47],
									ps[48], ps[49], ps[50], ps[51], ps[52], ps[53], ps[54], ps[55], ps[56], ps[57], ps[58], ps[59], ps[60], ps[61], ps[62], ps[63],
									ps[64], ps[65], ps[66], ps[67], ps[68], ps[69], ps[70], ps[71], ps[72], ps[73], ps[74], ps[75], ps[76], ps[77], ps[78], ps[79]);
			}
*/
		}
		else
		{
			seq_stream_size = 1;
		}

		DSTATUS( "[VDEC-%d] VDEC_DEC_SEQ_HEADER start  :: len = %d / %d ", pInst->vdec_instance_index, seq_stream_size, p_input_param->m_iInpLen);
		ret = vpu_dec_seq_header(seq_stream_size, iIsThumbnail, pInst);
		if( ret != RETCODE_SUCCESS )
		{
			p_output_param->m_pInitialInfo = &pInst->gsCommDecInfo; // mm008
			return ret;
		}
#ifdef VPU_ALL_FRAME_DUMP
		save_input_stream("seqHeader_stream.bin", seq_stream_size, pInst, 0);
#endif
		pInst->gsbHasSeqHeader = 1;
		p_output_param->m_pInitialInfo = &pInst->gsCommDecInfo;
		//check the maximum/minimum video resolution limitation
		if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_MJPG )
		{
			vdec_info_t * pVdecInfo = (vdec_info_t *)&pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;
			int max_width, max_height;
			int min_width, min_height;
		 
			max_width  	= ((AVAILABLE_MAX_WIDTH+15)&0xFFF0);
			max_height 	= ((AVAILABLE_MAX_HEIGHT+15)&0xFFF0);
			min_width 	= AVAILABLE_MIN_WIDTH;
			min_height 	= AVAILABLE_MIN_HEIGHT;
			
			if(    (pVdecInfo->m_iPicWidth > max_width)
				|| ((pVdecInfo->m_iPicWidth * pVdecInfo->m_iPicHeight) > AVAILABLE_MAX_REGION)
				|| (pVdecInfo->m_iPicWidth < min_width)
				|| (pVdecInfo->m_iPicHeight < min_height) )
			{
				ret = 0 - RETCODE_INVALID_STRIDE;
				DPRINTF( "[VDEC-%d] VDEC_DEC_SEQ_HEADER - don't support the resolution %dx%d  ", pInst->vdec_instance_index, 
									pVdecInfo->m_iPicWidth, pVdecInfo->m_iPicHeight);
				return ret;
			}
		}

#ifdef DISPLAY_1ST_DECODED_IDX
		if( iIsThumbnail )
			pInst->mdisplayed_1st_IFrm = 1;
#endif		
		LOGI( "[VDEC-%d] VDEC_DEC_SEQ_HEADER - Success mem_free = 0x%x ", pInst->vdec_instance_index, cdk_sys_final_free_mem(pInst) );
		DSTATUS( "[VDEC-%d] =======================================================", pInst->vdec_instance_index );		
	}
	else if( iOpCode == VDEC_DECODE )
	{
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;

		#ifdef PRINT_VPU_INPUT_STREAM
		{
			int kkk;
			unsigned char* p_input = p_input_param->m_pInp[VA];
			int input_size = p_input_param->m_iInpLen;
			printf("FS = %7d :", input_size);
			for( kkk = 0; kkk < PRINT_BYTES; kkk++ )
				printf("%02X ", p_input[kkk] );
			printf("");
		}
		#endif

		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = (p_input_param->m_iInpLen > pInst->gsMaxBitstreamSize) ? pInst->gsMaxBitstreamSize : p_input_param->m_iInpLen;

		if( (pInst->extFunction & EXT_FUNC_MEM_PROTECTION_WITH_INSTREAM) != 0x0 )
		{
			//LOGI("Usable Input Addr : 0x%x", p_input_param->m_pInp[PA]);
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = (codec_addr_t)p_input_param->m_pInp[PA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = (codec_addr_t)p_input_param->m_pInp[VA];
		}
		else if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable )
		{
			if (    ((codec_addr_t)p_input_param->m_pInp[PA] == pInst->gsBitstreamBufAddr[PA])
				 && ((codec_addr_t)p_input_param->m_pInp[VA] == pInst->gsBitstreamBufAddr[VA]) )
			{
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
			}
			else
			{
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
				memcpy( (void*)pInst->gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize);
#ifdef CHANGE_INPUT_STREAM
				change_input_stream((unsigned char *)pInst->gsBitstreamBufAddr[VA], (&pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize), iOpCode, pInst);
#endif
			}
		}
		else
		{
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = 1;
		}

		if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
		{		
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[PA] = pInst->gsUserdataBufAddr[PA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[VA] = pInst->gsUserdataBufAddr[K_VA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iUserDataBufferSize = pInst->gsUserdataBufSize;
		}
		
//		gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[PA] = (codec_addr_t)p_input_param->m_UserDataAddr[PA];
//		gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[VA] = (codec_addr_t)p_input_param->m_UserDataAddr[VA];
//		gsVpuDecInOut_Info.gsVpuDecInput.m_iUserDataBufferSize = p_input_param->m_iUserDataBufferSize;
		
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode = p_input_param->m_iSkipFrameMode;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable = p_input_param->m_iFrameSearchEnable;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = 0;
		if( pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode > 0 || pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable > 0 )
		{
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = p_input_param->m_iSkipFrameNum;
		}


	#ifdef VPU_ALL_FRAME_DUMP
		save_input_stream("all_stream.bin", pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize, pInst, 1);
	#endif

		// Start decoding a frame.
		ret = vdec_cmd_process(V_DEC_DECODE, &pInst->gsVpuDecInOut_Info, pInst);
		pInst->total_frm++;
//		if(gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iOutputStatus != VPU_DEC_OUTPUT_SUCCESS)
//			LOGD("systemtime:: ## decoded frame but no-output");
//		else
//			LOGD("systemtime:: ## decoded frame");

		if(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_BUF_FULL
			|| (pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iWidth <= 64 || pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iHeight <= 64)
			|| (ret == RETCODE_CODEC_EXIT)
		)
		{
			if(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_BUF_FULL){
				ALOGE("Buffer full");
			}
			else{
				ALOGE("Strange resolution");
			}
			ALOGE("Dec In 0x%x - 0x%x, %d, 0x%x - 0x%x, %d, flsg: %d / %d / %d  \n", pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA], pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA], pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize,
								pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[PA], pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[VA], pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iUserDataBufferSize,
								pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable, pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode, pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum);

			ALOGE("%d - %d - %d, %d - %d - %d \n", pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iWidth, pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_CropInfo.m_iCropLeft, 
								pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_CropInfo.m_iCropRight, pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iHeight, 
								pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_CropInfo.m_iCropTop, pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_CropInfo.m_iCropBottom);

			ALOGE("@@ Dec Out[%d] !! PicType[%d], OutIdx[%d/%d], OutStatus[%d/%d] \n", ret, pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iPicType,
								pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDispOutIdx, pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodedIdx,
								pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iOutputStatus, pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodingStatus);
			ALOGE("DispOutIdx : %d \n", pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDispOutIdx);

			if(ret == RETCODE_CODEC_EXIT) {
				save_input_stream("error_codec_exit.bin", pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize, pInst, 0);
			}
			else if(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_BUF_FULL){
				save_input_stream("error_buffer_full.bin", pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize, pInst, 0);
			}
			else {
				save_input_stream("error_strange_res.bin", pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize, pInst, 0);
			}
		}

//		if( ret == VPU_DEC_FINISH )
//			return ERR_END_OF_FILE;
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VPU_DEC_DECODE failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
			return -ret;
		}

		if( pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iPicType == 0 ){
#ifdef DISPLAY_1ST_DECODED_IDX
			if( pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iInterlacedFrame )
				pInst->mdisplayed_1st_IFrm = 1;

			if( pInst->mdisplayed_1st_IFrm == 0 && pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0){
				DSTATUS( "[VDEC-%d] mdisplayed_1st_IFrm (%d)", pInst->vdec_instance_index, pInst->total_frm);
				pInst->mdisplayed_1st_IFrm = 1;
				pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iOutputStatus = VPU_DEC_OUTPUT_SUCCESS;
				pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDispOutIdx = MAX_INDEX-1;
				pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[PA][COMP_Y] = pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pCurrOut[PA][COMP_Y];
				pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[PA][COMP_U] = pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pCurrOut[PA][COMP_U];
				pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[PA][COMP_V] = pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pCurrOut[PA][COMP_V];
				pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][COMP_Y] = pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pCurrOut[VA][COMP_Y];
				pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][COMP_U] = pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pCurrOut[VA][COMP_U];
				pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][COMP_V] = pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pCurrOut[VA][COMP_V];
			}
#endif
			DSTATUS( "[VDEC-%d] I-Frame (%d)", pInst->vdec_instance_index, pInst->total_frm);
		}

		memcpy((void*)p_output_param, (void*)&pInst->gsVpuDecInOut_Info.gsVpuDecOutput, sizeof(dec_output_t ) );
		p_output_param->m_pInitialInfo = &pInst->gsCommDecInfo;

		{
			p_output_param->m_pDispOut[VA][COMP_Y] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][COMP_Y], K_VA, pInst);
			p_output_param->m_pDispOut[VA][COMP_U] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][COMP_U], K_VA, pInst);
			p_output_param->m_pDispOut[VA][COMP_V] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][COMP_V], K_VA, pInst);
		}

		p_output_param->m_pCurrOut[VA][COMP_Y] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][COMP_Y], K_VA, pInst);
		p_output_param->m_pCurrOut[VA][COMP_U] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][COMP_U], K_VA, pInst);
		p_output_param->m_pCurrOut[VA][COMP_V] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][COMP_V], K_VA, pInst);

		if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
		{	
			unsigned int addr_gap = 0;

			addr_gap = pInst->gsUserdataBufAddr[K_VA] - pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_UserDataAddress[VA];
			p_output_param->m_DecOutInfo.m_UserDataAddress[VA] = pInst->gsUserdataBufAddr[VA] + addr_gap;
		}

		if(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS){
//			LOGE("Displayed addr 0x%x", p_output_param->m_pDispOut[PA][0]);
#ifdef VPU_OUT_FRAME_DUMP
			save_decoded_frame((unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[PA][0], PA, pInst),
								(unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[PA][1], PA, pInst),
								(unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[PA][2], PA, pInst),
								pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight, pInst);
#endif
		}

		DISPLAY_BUFFER("[VDEC-%d] Display idx = %d", pInst->vdec_instance_index, pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDispOutIdx);
	}
	else if( iOpCode == VDEC_GET_RING_BUFFER_STATUS )
	{
		vdec_ring_buffer_out_t* p_out_param = (vdec_ring_buffer_out_t*)pParam2;

		ret = vdec_cmd_process(V_GET_RING_BUFFER_STATUS, &pInst->gsVpuDecBufStatus, pInst); // get the available space in the ring buffer
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] GET_RING_BUFFER_STATUS failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
			return -ret;
		}
		p_out_param->m_ulAvailableSpaceInRingBuffer = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ulAvailableSpaceInRingBuffer;
		p_out_param->m_ptrReadAddr_PA = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrReadAddr_PA;
		p_out_param->m_ptrWriteAddr_PA = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrWriteAddr_PA;
//		LOGE("[VDEC] [AVAIL: %8d] [RP: 0x%08X / WP: 0x%08X]"
//			  , pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ulAvailableSpaceInRingBuffer
//			  , pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrReadAddr_PA
//			  , pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrWriteAddr_PA
//			  );
	}
	else if( iOpCode == VDEC_FILL_RING_BUFFER )
	{
		vdec_ring_buffer_set_t* p_set_param = (vdec_ring_buffer_set_t*)pParam1;

		memcpy((void*)pInst->gsIntermediateBufAddr[VA],(void*)p_set_param->m_pbyBuffer, p_set_param->m_uiBufferSize);
		pInst->gsVpuDecBufFill.gsVpuDecRingFeed.m_iOnePacketBufferSize = p_set_param->m_uiBufferSize;
		pInst->gsVpuDecBufFill.gsVpuDecRingFeed.m_OnePacketBufferAddr = pInst->gsIntermediateBufAddr[K_VA];

		ret = vdec_cmd_process(V_FILL_RING_BUFFER_AUTO, &pInst->gsVpuDecBufFill, pInst);  // fille the Ring Buffer 

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] FILL_RING_BUFFER_AUTO failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_GET_INTERMEDIATE_BUF_INFO )
	{
		*(unsigned int*)pParam1 = pInst->gsIntermediateBufAddr[VA];
		*(unsigned int*)pParam2 = pInst->gsIntermediateBufSize;
		return 0;
	}
	else if( iOpCode == VDEC_UPDATE_WRITE_BUFFER_PTR )
	{
		pInst->gsVpuDecUpdateWP.iCopiedSize = (int)pParam1;
		pInst->gsVpuDecUpdateWP.iFlushBuf = (int)pParam2;

		ret = vdec_cmd_process(V_DEC_UPDATE_RINGBUF_WP, &pInst->gsVpuDecUpdateWP,  pInst);  // fille the Ring Buffer 

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VDEC_UPDATE_WRITE_BUFFER_PTR failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_BUF_FLAG_CLEAR )
	{
		int idx_display = *(int*)pParam1;
		CLEAR_BUFFER("[VDEC-%d] ************* cleared idx = %d", pInst->vdec_instance_index, idx_display);
#ifdef DISPLAY_1ST_DECODED_IDX
		if( idx_display == MAX_INDEX-1 )
			return RETCODE_SUCCESS;
#endif
		ret = vdec_cmd_process(V_DEC_BUF_FLAG_CLEAR, &idx_display, pInst);
		
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VPU_DEC_BUF_FLAG_CLEAR failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_DEC_FLUSH_OUTPUT)
	{
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;

		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = 0;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable = 0;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = 0;

		ret = vdec_cmd_process(V_DEC_FLUSH_OUTPUT, &pInst->gsVpuDecInOut_Info, pInst);

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VDEC_DEC_FLUSH_OUTPUT failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
			return -ret;
		}

		memcpy((void*)p_output_param, (void*)&pInst->gsVpuDecInOut_Info.gsVpuDecOutput, sizeof(dec_output_t ) );
		p_output_param->m_pInitialInfo = &pInst->gsCommDecInfo;

		{
			p_output_param->m_pDispOut[VA][COMP_Y] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][COMP_Y], K_VA, pInst);
			p_output_param->m_pDispOut[VA][COMP_U] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][COMP_U], K_VA, pInst);
			p_output_param->m_pDispOut[VA][COMP_V] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][COMP_V], K_VA, pInst);
		}

		p_output_param->m_pCurrOut[VA][COMP_Y] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][COMP_Y], K_VA, pInst);
		p_output_param->m_pCurrOut[VA][COMP_U] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][COMP_U], K_VA, pInst);
		p_output_param->m_pCurrOut[VA][COMP_V] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][COMP_V], K_VA, pInst);

		if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
		{
			unsigned int addr_gap = 0;

			addr_gap = pInst->gsUserdataBufAddr[K_VA] - pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_UserDataAddress[VA];
			p_output_param->m_DecOutInfo.m_UserDataAddress[VA] = pInst->gsUserdataBufAddr[VA] + addr_gap;
		}
	}
	else if( iOpCode == VDEC_SW_RESET)
	{
		ret = vdec_cmd_process(V_DEC_SWRESET, NULL, pInst);

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] V_DEC_SWRESET failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_CLOSE )
	{
		if(!pInst->vdec_codec_opened && !pInst->vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;

		if(pInst->vdec_codec_opened)
		{		
			ret = vdec_cmd_process(V_DEC_CLOSE, &pInst->gsVpuDecInOut_Info, pInst);
			if( ret != RETCODE_SUCCESS )
			{
				DPRINTF( "[VDEC-%d] VPU_DEC_CLOSE failed Error code is 0x%x ", pInst->vdec_instance_index, ret );
				ret = -ret;
			}
			
			pInst->vdec_codec_opened = 0;
		}

		if(!pInst->vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;
				
		if( pInst->gsBitstreamBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( (void*)pInst->gsBitstreamBufAddr[VA], pInst->gsBitstreamBufSize)  >= 0)
			{
				pInst->gsBitstreamBufAddr[VA] = 0;
			}
		}
		
		if( pInst->gsUserdataBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( (void*)pInst->gsUserdataBufAddr[VA], pInst->gsUserdataBufSize )  >= 0)
			{
				pInst->gsUserdataBufAddr[VA] = 0;
			}
			
		}

		if( pInst->gsBitWorkBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( (void*)pInst->gsBitWorkBufAddr[VA], pInst->gsBitWorkBufSize )  >= 0)
			{
				pInst->gsBitWorkBufAddr[VA] = 0;
			}
			
		}

		if( pInst->gsFrameBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( (void*)pInst->gsFrameBufAddr[VA], pInst->gsFrameBufSize )  >= 0)
			{
				pInst->gsFrameBufAddr[VA] = 0;
			}
		}
		vpu_env_close(pInst);
	}
	else
	{
		DPRINTF( "[VDEC-%d] Invalid Operation!!", pInst->vdec_instance_index );
		return -ret;
	}

#if 0
#ifdef DEBUG_TIME_LOG
	end = clock();

	if( iOpCode == VDEC_INIT ){
		LOGD("[VDEC-%d] VDEC_INIT_TIME %d ms", pInst->vdec_instance_index, (end-start)*1000/CLOCKS_PER_SEC);
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER){
		LOGD("[VDEC-%d] VDEC_SEQ_TIME %d ms", pInst->vdec_instance_index, (end-start)*1000/CLOCKS_PER_SEC);
	}
	else if( iOpCode == VDEC_DECODE )
	{
		dec_time[time_cnt] = (end-start)*1000/CLOCKS_PER_SEC;
		total_dec_time += dec_time[time_cnt];
		if(time_cnt != 0 && time_cnt % 29 == 0)
		{
			LOGD("[VDEC-%d] VDEC_DEC_TIME %.1f ms: %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d",
				pInst->vdec_instance_index, total_dec_time/(float)pInst->total_frm, 
				dec_time[0], dec_time[1], dec_time[2], dec_time[3], dec_time[4], dec_time[5], dec_time[6], dec_time[7], dec_time[8], dec_time[9], 
				dec_time[10], dec_time[11], dec_time[12], dec_time[13], dec_time[14], dec_time[15], dec_time[16], dec_time[17], dec_time[18], dec_time[19], 
				dec_time[20], dec_time[21], dec_time[22], dec_time[23], dec_time[24], dec_time[25], dec_time[26], dec_time[27], dec_time[28], dec_time[29]);
			time_cnt = 0;
		}
		else{
			time_cnt++;
		}
	}
#endif
#endif
	return ret;
}


const float MPEG2_Aspect_Ratio[]={
	0.0,        // forbidden
	1.0,        // 1 : 1 Square
	4.0/3.0,	// 3 / 4 = 1.33
	16.0/9.0,   // 9 / 16 = 1.77
	221.0/1.0   // 1/2,21 ??
};

const float MPEG4_Aspect_Ratio[]={
	0,          // forbidden
	1,          // 1 : 1 Square
	12.0/11.0,  // 12:11
	10.0/11.0,  // 10:11
	16.0/11.0,  // 16:11
	40.0/33.0   // 40:33
};

const float AVC_Aspect_Ratio[]={
	0.0,        // unspecified
	1.0,        // 1 : 1 Square
	12.0/11.0,  // 12:11
	10.0/11.0,  // 10:11
	16.0/11.0,  // 16:11
	40.0/33.0,  // 40:33
	24.0/11.0,  // 24:11
	20.0/11.0,  // 20:11
	32.0/11.0,  // 32:11
	80.0/33.0,  // 80:33
	18.0/11.0,  // 18:11
	15.0/11.0,  // 15:11
	64.0/33.0,  // 64:33
   160.0/99.0,  // 160:99
     4.0/3.0,
     3.0/2.0,
     2.0/1.0,
};


float vdec_getAspectRatio(void *pvInstance, int iBitstreamFormat)
{
	_vdec_ *pVdec = (_vdec_ *)pvInstance;
	float PAR = 0.0;
	
	int AspectRatio_Idx = pVdec->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iAspectRateInfo;

   	switch(iBitstreamFormat)
   	{
   		case STD_AVC:
		case STD_MVC:

			break;
		case STD_VC1:
		
			break;
		case STD_MPEG2:
			
			break;
		case STD_MPEG4:
			if(AspectRatio_Idx>= 0 && AspectRatio_Idx < sizeof(MPEG4_Aspect_Ratio)/sizeof(float) )
			{
				PAR = MPEG4_Aspect_Ratio[AspectRatio_Idx];
			}
			else if(AspectRatio_Idx & 0xF == 0xF) // extended PAR
			{
				int Aspect_width, Aspect_height;
				
				Aspect_width = (AspectRatio_Idx & 0xFF0) >> 4;
             	Aspect_height = (AspectRatio_Idx & 0xFF000) >> 12;

				if(Aspect_height && Aspect_width > 0 && Aspect_height > 0)
					PAR = (float)Aspect_width / (float)(Aspect_height);
			}

			break;
		case STD_RV:
		case STD_MJPG:
		default:
			break;
	}

	//PAR= ((int)(PAR * pow(10.0, 1)))/pow(10.0, 1);

	return PAR;
}

void vdec_get_realResolution(void *pInst, int *width, int *height)
{
	_vdec_ * pVdec = (_vdec_ *) pInst;

	if(!pInst){
		ALOGE("vdec_get_realResolution :: Instance is null!!");
		return;
	}

	*width = pVdec->mRealPicWidth;
	*height = pVdec->mRealPicHeight;
	DSTATUS( "[VDEC-%d] Real-Resolution: %d x %d", pVdec->vdec_instance_index, *width, *height);
}

int vdec_seqHeader_extract(const unsigned char	*pbyStreamData, unsigned int lStreamDataSize, unsigned char **ppbySeqHeaderData, unsigned int *plSeqHeaderSize, int codec_type, unsigned int isTCCExtractor, void *pInst)
{
	long i = 0;
	_vdec_ * pVdec = pInst;

	if(!pInst){
		ALOGE("vdec_seqHeader_extract :: Instance is null!!");
		return 0;
	}

	SEQ_EXTRACTOR("[VDEC-%d] sequence header: 0x%x - %d bytes, frame: 0x%x - %d bytes", pVdec->vdec_instance_index, (unsigned int)plSeqHeaderSize, *plSeqHeaderSize,
								(unsigned int)pbyStreamData, lStreamDataSize);

	if( *plSeqHeaderSize == 0 )
		pVdec->AvcSeq_status = 0;

	{
		unsigned char* ps = (unsigned char*)pbyStreamData;
		SEQ_EXTRACTOR( "[VDEC-%d In %d] " "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x " "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
							"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x " "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
							"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ", 
							pVdec->vdec_instance_index, lStreamDataSize,
							ps[0], ps[1], ps[2], ps[3], ps[4], ps[5], ps[6], ps[7], ps[8], ps[9], ps[10], ps[11], ps[12], ps[13], ps[14], ps[15],
							ps[16], ps[17], ps[18], ps[19], ps[20], ps[21], ps[22], ps[23], ps[24], ps[25], ps[26], ps[27], ps[28], ps[29], ps[30], ps[31],
							ps[32], ps[33], ps[34], ps[35], ps[36], ps[37], ps[38], ps[39], ps[40], ps[41], ps[42], ps[43], ps[44], ps[45], ps[46], ps[47],
							ps[48], ps[49], ps[50], ps[51], ps[52], ps[53], ps[54], ps[55], ps[56], ps[57], ps[58], ps[59], ps[60], ps[61], ps[62], ps[63],
							ps[64], ps[65], ps[66], ps[67], ps[68], ps[69], ps[70], ps[71], ps[72], ps[73], ps[74], ps[75], ps[76], ps[77], ps[78], ps[79]);
	}

	if( codec_type == STD_AVC || codec_type == STD_MVC )
	{
		long l_seq_start_pos = 0, l_seq_end_pos = 0, l_seq_length = 0; // Start Position, End Position, Length of the sequence header
		long l_sps_found = 0;
		long l_pps_found = 0;

		unsigned long ul_read_word_buff;	   	    	            //4 byte temporary buffer
		unsigned long ul_masking_word_seq          = 0x0FFFFFFF;    //Masking Value for finding H.264 sequence header
		unsigned long ul_masking_word_sync         = 0x0FFFFFFF;    //Masking Value for finding sync word of H.264
		unsigned long ul_h264_result_word_seq_SPS  = 0x07010000;    //Masking result should be this value in case of SPS. SPS Sequence header of H.264 must start by "00 00 01 x7"
		unsigned long ul_h264_result_word_seq_PPS  = 0x08010000;    //Masking result should be this value in case of PPS. PPS Sequence header of H.264 must start by "00 00 01 x8"
		unsigned long ul_h264_result_word_sync     = 0x01010000;    //Masking result should be this value. Sequence header of H.264 must start by "00 00 01 x1"
		unsigned long ul_h264_result_word_sync2    = 0x05010000;    //Masking result should be this value. Sequence header of H.264 must start by "00 00 01 x5"
		unsigned int SPS_FOUND = 0x1;
		unsigned int PPS_FOUND = 0x2;
		unsigned int SEQ_ALL_FOUND = 0x1|0x2;

		if ( lStreamDataSize < 4 )
			return 0; // there's no Seq. header in this frame. we need the next frame.

		if ( (pVdec->AvcSeq_status & SEQ_ALL_FOUND) != SEQ_ALL_FOUND )
		{
			if( (pVdec->AvcSeq_status & SPS_FOUND) != SPS_FOUND )
			{
				SEQ_EXTRACTOR("Check SPS frame!!");
				// find the SPS of H.264
				ul_read_word_buff = 0;
				ul_read_word_buff |= (pbyStreamData[0] << 8);
				ul_read_word_buff |= (pbyStreamData[1] << 16);
				ul_read_word_buff |= (pbyStreamData[2] << 24);

				for ( i = 0; i < lStreamDataSize-4; i++ )
				{
					ul_read_word_buff = ul_read_word_buff >> 8;
					ul_read_word_buff &= 0x00FFFFFF;
					ul_read_word_buff |= (pbyStreamData[i+3] << 24);

					if ( (ul_read_word_buff & ul_masking_word_seq) == ul_h264_result_word_seq_SPS )
					{
						// SPS Sequence Header has been detected
						SEQ_EXTRACTOR("SPS frame is dectected!!");
						l_seq_start_pos = i;          // save the start position of the sequence header
						l_sps_found = 1;

						break;
					}

					// Continue to find the sps in next loop
				}
			}

			if ( (pVdec->AvcSeq_status & PPS_FOUND) != PPS_FOUND )
			{
				// Now, let's start to find the PPS of the Seq. header.
				if(l_sps_found)
					i = i + 4;
				else
					i = 0;
				SEQ_EXTRACTOR("Check PPS frame i = %d !!", i);
				ul_read_word_buff = 0;
				ul_read_word_buff |= (pbyStreamData[i] << 8);
				ul_read_word_buff |= (pbyStreamData[i+1] << 16);
				ul_read_word_buff |= (pbyStreamData[i+2] << 24);

				for (  ; i < lStreamDataSize - 4; i++ )
				{
					ul_read_word_buff = ul_read_word_buff >> 8;
					ul_read_word_buff &= 0x00FFFFFF;
					ul_read_word_buff |= (pbyStreamData[i+3] << 24);

					if ( (ul_read_word_buff & ul_masking_word_seq) == ul_h264_result_word_seq_PPS )
					{
						// PPS has been detected.
						SEQ_EXTRACTOR("PPS frame is dectected!!");
						l_pps_found = 1;
						break;
					}

					// Continue to find the pps in next loop
				}
			}
		}

		if ( l_pps_found == 1 || ((pVdec->AvcSeq_status & SEQ_ALL_FOUND) == SEQ_ALL_FOUND) )
		{
			// Now, let's start to find the next sync word to find the end position of Seq. Header
			SEQ_EXTRACTOR("Check Sync frame!!");
			if ( l_pps_found > 0 )
				i = i + 4;     // we already find the sps, pps in previous frame
			else
				i = 0;
			ul_read_word_buff = 0;
			ul_read_word_buff |= (pbyStreamData[i] << 8);
			ul_read_word_buff |= (pbyStreamData[i+1] << 16);
			ul_read_word_buff |= (pbyStreamData[i+2] << 24);

			for ( ; i < lStreamDataSize - 4; i++ )
			{
				ul_read_word_buff = ul_read_word_buff >> 8;
				ul_read_word_buff &= 0x00FFFFFF;
				ul_read_word_buff |= (pbyStreamData[i+3] << 24);

				if ( ((ul_read_word_buff & ul_masking_word_sync) == ul_h264_result_word_sync) || // 00 00 01 x1
					 ((ul_read_word_buff & ul_masking_word_sync) == ul_h264_result_word_sync2))  // 00 00 01 x5
				{
					long l_cnt_zeros = 0;       // to count extra zeros ahead of "00 00 01"

					// next sync-word has been found.
					l_seq_end_pos = i - 1;      // save the end position of the sequence header (00 00 01 case)

					// any zeros can be added ahead of "00 00 01" sync word by H.264 specification. Count the number of these leading zeros.
					while (1)
					{
						l_cnt_zeros++;

						if(i >= l_cnt_zeros) //ZzaU :: to prevent segmentation fault.
						{
							if ( pbyStreamData[i-l_cnt_zeros] == 0 )
							{
								l_seq_end_pos = l_seq_end_pos -1;    // decrease the end position of Seq. Header by 1.
							}
							else
								break;
						}
						else
							break;
					}

					if ( *plSeqHeaderSize > 0 && (pVdec->AvcSeq_status & SEQ_ALL_FOUND) == SEQ_ALL_FOUND )
					{
						if ( isTCCExtractor ) {
							// we already find the sps, pps in previous frame
							l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;

							if ( l_seq_length > 0 )
							{
								if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE ) // check the maximum threshold
									return 0;

								*ppbySeqHeaderData = TCC_realloc(*ppbySeqHeaderData , *plSeqHeaderSize + l_seq_length );     // allocation memory for sequence header array (must free this at the CLOSE step)
								SEQ_EXTRACTOR("Sync frame is copyed %d!!", l_seq_length);
								memcpy( (unsigned char*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
								*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
							}
						}
						return 1;

					}
					else
					{
						// calculate the length of the sequence header
						l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;

						if ( l_seq_length > 0 )
						{
							if ( *plSeqHeaderSize > 0 )
							{
								SEQ_EXTRACTOR("PPS or PPS+Sync frame is copyed %d!!", l_seq_length);
								*ppbySeqHeaderData = TCC_realloc(*ppbySeqHeaderData , *plSeqHeaderSize + l_seq_length );     // allocation memory for sequence header array (must free this at the CLOSE step)
								memcpy( (unsigned char*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
								*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
							}
							else
							{
								SEQ_EXTRACTOR("SPS+PPS+Sync frame is copyed %d!!", l_seq_length);
								*ppbySeqHeaderData = TCC_malloc( l_seq_length );     // allocation memory for sequence header array (must free this at the CLOSE step)
								memcpy( (unsigned char*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
								*plSeqHeaderSize = l_seq_length;
							}

							if( l_sps_found == 1 )
								pVdec->AvcSeq_status |= SPS_FOUND;
							if ( l_pps_found == 1 )
								pVdec->AvcSeq_status |= PPS_FOUND;

							return 1;  // We've found the sequence header successfully
						}
					}
				}

				// Continue to find the sync-word in next loop
			}
		}

		if ( ( l_sps_found == 1  ||  l_pps_found == 1 ) )
		{
			// we found sps and pps, but we couldn't find the next sync word yet
			l_seq_end_pos = lStreamDataSize - 1;
			l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;        // calculate the length of the sequence header

			if ( *plSeqHeaderSize > 0 )
			{
				// we already saved the sps in previous frame
				if ( l_seq_length > 0 )
				{
					if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE )     // check the maximum threshold
						return 0;

					*ppbySeqHeaderData = TCC_realloc(*ppbySeqHeaderData , *plSeqHeaderSize + l_seq_length );     // allocate memory for sequence header array (must free this at the CLOSE step)
					SEQ_EXTRACTOR("PPS frame is copyed %d!!", l_seq_length);
					memcpy( (unsigned char*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
					*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
				}
			}
			else
			{
				*ppbySeqHeaderData = TCC_malloc( l_seq_length );           // allocate memory for sequence header array (must free this at the CLOSE step)
				SEQ_EXTRACTOR("SPS or SPS+PPS frame is copyed %d!!", l_seq_length);
				memcpy( (unsigned char*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
				*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
			}

			if( l_sps_found == 1 )
				pVdec->AvcSeq_status |= SPS_FOUND;
			if ( l_pps_found == 1 )
				pVdec->AvcSeq_status |= PPS_FOUND;
			
		}
	}
	else
	{
		unsigned long syncword = 0xFFFFFFFF;
		int	start_pos = -1;
		int end_pos = -1;
		int i;

		syncword <<= 8;
		syncword |= pbyStreamData[0];
		syncword <<= 8;
		syncword |= pbyStreamData[1];
		syncword <<= 8;
		syncword |= pbyStreamData[2];

		for(i = 3; i < lStreamDataSize; i++) {
			syncword <<= 8;
			syncword |= pbyStreamData[i];

			if( (syncword >> 8) == 1 ) {	// 0x 000001??
				if( syncword >= MPEG4_VOL_STARTCODE_MIN &&
					syncword <= MPEG4_VOL_STARTCODE_MAX )
					start_pos = i-3;
				//else if( start_pos >= 0 || *plSeqHeaderSize > 0 ) {
				else if( start_pos >= 0 && *plSeqHeaderSize > 0 ) {
					if ( syncword == MPEG4_VOP_STARTCODE )
					{
						end_pos = i-3;
						break;
					}
				}
			}
		}

		if (start_pos >= 0 && end_pos == -1) {
			//end_pos = lStreamDataSize - start_pos;
			end_pos = lStreamDataSize;
		}

		if( start_pos >= 0 ) {
			if( end_pos >= 0 ) {
				*plSeqHeaderSize = end_pos-start_pos;
				*ppbySeqHeaderData = TCC_malloc( *plSeqHeaderSize );     // allocate memory for sequence header array
				memcpy(*ppbySeqHeaderData, pbyStreamData + start_pos, *plSeqHeaderSize);
				return 1;
			}
			else {
				*plSeqHeaderSize = lStreamDataSize - start_pos;
				*ppbySeqHeaderData = TCC_malloc( *plSeqHeaderSize );     // allocate memory for sequence header array
				memcpy(*ppbySeqHeaderData, pbyStreamData + start_pos, *plSeqHeaderSize);
				return 0;
			}
		}
		else if( *plSeqHeaderSize > 0 ) {
			if( end_pos < 0 )
				end_pos = lStreamDataSize;

			if ( *plSeqHeaderSize + end_pos > MAX_SEQ_HEADER_ALLOC_SIZE ) // check the maximum threshold
				return 0;

			*ppbySeqHeaderData = TCC_realloc(*ppbySeqHeaderData , *plSeqHeaderSize + end_pos);     // re-allocate memory for sequence header array
			memcpy(*ppbySeqHeaderData + *plSeqHeaderSize, pbyStreamData, end_pos);
			*plSeqHeaderSize += end_pos;
			return 1;
		}
	}

	return 0; // We couldn't find the complete sequence header yet. We need to search the next frame data.
}

char* vdec_print_pic_type( int iVideoType, int iPicType, int iPictureStructure )
{
	switch ( iVideoType )
	{
	case STD_MPEG2 :
		if( iPicType == PIC_TYPE_I )
			return "I :";
		else if( iPicType == PIC_TYPE_P )
			return "P :";
		else if( iPicType == PIC_TYPE_B )
			return "B :";
		else
			return "D :"; //D_TYPE
		break;

	case STD_MPEG4 :
		if( iPicType == PIC_TYPE_I )
			return "I :";
		else if( iPicType == PIC_TYPE_P )
			return "P :";
		else if( iPicType == PIC_TYPE_B )
			return "B :";
		else if( iPicType == PIC_TYPE_B_PB ) //MPEG-4 Packed PB-frame
			return "pB:";
		else
			return "S :"; //S_TYPE
		break;

	case STD_VC1 :
		if( iPictureStructure == 3)
		{
			// FIELD_INTERLACED
			if( (iPicType>>3) == PIC_TYPE_I )
				return "TF_I   :";	//TOP_FIELD = I
			else if( (iPicType>>3) == PIC_TYPE_P )
				return "TF_P   :";	//TOP_FIELD = P
			else if( (iPicType>>3) == 2 )
				return "TF_BI  :";	//TOP_FIELD = BI_TYPE
			else if( (iPicType>>3) == 3 )
				return "TF_B   :";	//TOP_FIELD = B_TYPE
			else if( (iPicType>>3) == 4 )
				return "TF_SKIP:";	//TOP_FIELD = SKIP_TYPE
			else
				return "TF_FORBIDDEN :"; //TOP_FIELD = FORBIDDEN

			if( (iPicType&0x7) == PIC_TYPE_I )
				return "BF_I   :";	//BOTTOM_FIELD = I
			else if( (iPicType&0x7) == PIC_TYPE_P )
				return "BF_P   :";	//BOTTOM_FIELD = P
			else if( (iPicType&0x7) == 2 )
				return "BF_BI  :";	//BOTTOM_FIELD = BI_TYPE
			else if( (iPicType&0x7) == 3 )
				return "BF_B   :";	//BOTTOM_FIELD = B_TYPE
			else if( (iPicType&0x7) == 4 )
				return "BF_SKIP:";	//BOTTOM_FIELD = SKIP_TYPE
			else
				return "BF_FORBIDDEN :"; //BOTTOM_FIELD = FORBIDDEN
		}
		else
		{
			iPicType = iPicType>>3;
			if( iPicType == PIC_TYPE_I )
				return "I   :";
			else if( iPicType == PIC_TYPE_P )
				return "P   :";
			else if( iPicType == 2 )
				return "BI  :";
			else if( iPicType == 3 )
				return "B   :";
			else if( iPicType == 4 )
				return "SKIP:";
			else
				return "FORBIDDEN :"; //FORBIDDEN
		}
		break;
	default:
		if( iPicType == PIC_TYPE_I )
			return "I :";
		else if( iPicType == PIC_TYPE_P )
			return "P :";
		else if( iPicType == PIC_TYPE_B )
			return "B :";
		else if( iPicType == PIC_TYPE_IDR )
			return "IDR :";
		else
			return "U :"; //Unknown
	}
}

void vdec_disp_pic_info (int Opcode, void* pParam1, void *pParam2, void *pParam3, int fps, int bPlayDirection, void *pInst)
{
	int i;
	dec_disp_info_ctrl_t  *pInfoCtrl = (dec_disp_info_ctrl_t*)pParam1;
	dec_disp_info_t 	  *pInfo = (dec_disp_info_t *)pParam2;
	dec_disp_info_input_t *pInfoInput = (dec_disp_info_input_t*)pParam3;
	_vdec_ * pVdec = (_vdec_ *) pInst;

	if(!pInst){
		ALOGE("vdec_disp_pic_info :: Instance is null!!");
		return;
	}

	switch( Opcode )
	{
	case CVDEC_DISP_INFO_INIT:	//init.
			pInfoCtrl->m_iStdType = pInfoInput->m_iStdType;
			pInfoCtrl->m_iFmtType = pInfoInput->m_iFmtType;
			pInfoCtrl->m_iTimeStampType = pInfoInput->m_iTimeStampType;

		#if 0 //def TIMESTAMP_CORRECTION
			if( pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MPG
			|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_TS
			|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MKV )
			{
				pVdec->gsPtsInfo.m_iLatestPTS = 0;
				pVdec->gsPtsInfo.m_iRamainingDuration = 0;

				if( fps != 0 )
				{
					pVdec->gsPtsInfo.m_iPTSInterval = (((1000 * 1000) << 10) / fps) >> 10;
				}
			}
		#endif

	case CVDEC_DISP_INFO_RESET: //reset
			for( i=0 ; i<MAX_INDEX ; i++ )
			{
				pInfoCtrl->m_iRegIdxPTS[i] = -1;	//unused
				pInfoCtrl->m_pRegInfoPTS[i] = (void*)&pInfo[i];
			}
			pInfoCtrl->m_iUsedIdxPTS = 0;
			pInfoCtrl->m_iPrevIdx = -1;
		#if 0
			if( pInfoCtrl->m_iTimeStampType == CDMX_DTS_MODE )	//Decode Timestamp (Decode order)
			{
				pInfoCtrl->m_iDecodeIdxDTS = 0;
				pInfoCtrl->m_iDispIdxDTS = 0;
				for( i=0 ; i<MAX_INDEX ; i++ )
				{
					pInfoCtrl->m_iDTS[i] = 0;
				}
			}
		#endif
			memset(&pVdec->gsEXT_F_frame_time, 0, sizeof(EXT_F_frame_time_t));
			pVdec->gsextReference_Flag = 1;
			pVdec->gsextP_frame_cnt = 0;

		#if 0 //def TIMESTAMP_CORRECTION
			if( pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MPG
				|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_TS
				|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MKV )
			{
				pVdec->gsPtsInfo.m_iLatestPTS = 0;
				pVdec->gsPtsInfo.m_iRamainingDuration = 0;
			}
		#endif
		break;

	case CVDEC_DISP_INFO_UPDATE: //update
		{
			int iDecodedIdx;
			int usedIdx, startIdx, regIdx;
			dec_disp_info_t * pdec_disp_info;

			iDecodedIdx = pInfoInput->m_iFrameIdx;

			//In case that frame rate is changed...
		#if 0 //def TIMESTAMP_CORRECTION
			if( pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MPG )
			{
				if(pInfoInput->m_iFrameRate)
				{
					int new_fps = ((pInfoInput->m_iFrameRate & 0xffff) * 1000) / (((pInfoInput->m_iFrameRate >> 16) + 1)&0xffff);
					if(new_fps != 0)
					{
						pVdec->gsPtsInfo.m_iPTSInterval = (((1000 * 1000) << 10) / new_fps) >> 10;
					}
					else if(fps != 0)
					{
						pVdec->gsPtsInfo.m_iPTSInterval = ((1000 << 10) / fps) >> 10;
					}

					//LOGD("CVDEC_DISP_INFO_UPDATE m_iPTSInterval %d m_iFrameRate %d input FrameRate %x ",omx_private->gsMPEG2PtsInfo.m_iPTSInterval , omx_private->cdmx_info.m_sVideoInfo.m_iFrameRate,pInfoInput->m_iFrameRate);
				}
			}
		#endif
			//Presentation Timestamp (Display order)
			{
				//sort
				usedIdx=0;
				startIdx = -1;
				for( i=0 ; i<MAX_INDEX ; i++ )
				{
					if( pInfoCtrl->m_iRegIdxPTS[i] > -1 )
					{
						if( startIdx == -1 )
						{
							startIdx = i;
						}
						usedIdx++;
					}
				}

				if( usedIdx > 0 )
				{
					regIdx = 0;
					for( i=startIdx ; i<MAX_INDEX ; i++ )
					{
						if( pInfoCtrl->m_iRegIdxPTS[i] > -1 )
						{
							if( i != regIdx )
							{
								void * pswap;
								int iswap;

								iswap = pInfoCtrl->m_iRegIdxPTS[regIdx];
								pswap = pInfoCtrl->m_pRegInfoPTS[regIdx];

								pInfoCtrl->m_iRegIdxPTS[regIdx] = pInfoCtrl->m_iRegIdxPTS[i];
								pInfoCtrl->m_pRegInfoPTS[regIdx] = pInfoCtrl->m_pRegInfoPTS[i];

								pInfoCtrl->m_iRegIdxPTS[i] = iswap;
								pInfoCtrl->m_pRegInfoPTS[i] = pswap;
							}
							regIdx++;
							if( regIdx == usedIdx )
								break;
						}
					}
				}

				//save the side info.
				pInfoCtrl->m_iRegIdxPTS[usedIdx] = iDecodedIdx;
				pdec_disp_info = (dec_disp_info_t*)pInfoCtrl->m_pRegInfoPTS[usedIdx];

				if(pInfoCtrl->m_iPrevIdx != -1)
				{
					dec_disp_info_t * pdec_disp_info_prev = (dec_disp_info_t*)pInfoCtrl->m_pRegInfoPTS[pInfoCtrl->m_iPrevIdx];
					if(pInfo->m_lTimeStamp == pdec_disp_info_prev->m_lTimeStamp)
						pdec_disp_info->m_lTimeStamp = -1;
					else
						pdec_disp_info->m_lTimeStamp = pInfo->m_lTimeStamp;
				}
				else
				{
					pdec_disp_info->m_lTimeStamp = pInfo->m_lTimeStamp;
				}
				pInfoCtrl->m_iPrevIdx = usedIdx;

				pdec_disp_info->m_iFrameType = pInfo->m_iFrameType;
				pdec_disp_info->m_iPicStructure = pInfo->m_iPicStructure;
				pdec_disp_info->m_lextTimeStamp = pInfo->m_lextTimeStamp;
				pdec_disp_info->m_iM2vFieldSequence = pInfo->m_iM2vFieldSequence;
				pdec_disp_info->m_iFrameDuration = pInfo->m_iFrameDuration;
				pdec_disp_info->m_iFrameSize = pInfo->m_iFrameSize;
				pdec_disp_info->m_bIsMvcDependent = pInfo->m_bIsMvcDependent;
				pdec_disp_info->m_iNumMBError = pInfo->m_iNumMBError;
				pdec_disp_info->m_iPicWidth = pInfo->m_iPicWidth;
				pdec_disp_info->m_iPicHeight = pInfo->m_iPicHeight;

				pInfoCtrl->m_iUsedIdxPTS = usedIdx + 1;
				if( pInfoCtrl->m_iUsedIdxPTS > (MAX_INDEX-1) )
				{
					DSTATUS( "[CDK_CORE] _disp_pic_info index failed" );
					for( i=0 ; i<MAX_INDEX ; i++ )
					{
						pInfoCtrl->m_iRegIdxPTS[i] = -1;
					}
				}
			}
		#if 0
			if( pInfoCtrl->m_iTimeStampType == CDMX_DTS_MODE )	//Decode Timestamp (Decode order)
			{
				if( iDecodedIdx >= 0 || ( iDecodedIdx == -2 && pInfoCtrl->m_iStdType  == STD_MPEG4  ) )
				{
					pInfoCtrl->m_iDTS[pInfoCtrl->m_iDecodeIdxDTS] = pInfo->m_lTimeStamp;
					pInfoCtrl->m_iDecodeIdxDTS = ( pInfoCtrl->m_iDecodeIdxDTS + 1 ) & (MAX_INDEX-1);
				}
			}
		#endif
		}
		break;
	case CVDEC_DISP_INFO_GET:	//display
		{
			dec_disp_info_t **pInfo = (dec_disp_info_t **)pParam2;
			int dispOutIdx = pInfoInput->m_iFrameIdx;

			//Presentation Timestamp (Display order)
			{
				*pInfo = 0;

				for( i=0; i<pInfoCtrl->m_iUsedIdxPTS ; i++ )
				{
					if( dispOutIdx == pInfoCtrl->m_iRegIdxPTS[i] )
					{
						*pInfo = (dec_disp_info_t*)pInfoCtrl->m_pRegInfoPTS[i];

					 #if 0 //def TIMESTAMP_CORRECTION
						if( (pInfoCtrl->m_iFmtType  == CONTAINER_TYPE_MPG
							|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_TS
							|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MKV)
							&& (bPlayDirection))
						{
							static long prev_pts;
							if( (*pInfo)->m_bIsMvcDependent ) {
								(*pInfo)->m_iTimeStamp = pVdec->gsPtsInfo.m_iLatestPTS;
							} 
							else {
								if( (*pInfo)->m_iTimeStamp == -1 )
									(*pInfo)->m_iTimeStamp = pVdec->gsPtsInfo.m_iLatestPTS + ((pVdec->gsPtsInfo.m_iPTSInterval * pVdec->gsPtsInfo.m_iRamainingDuration) >> 1);
							}
							prev_pts = (*pInfo)->m_iTimeStamp;
							pVdec->gsPtsInfo.m_iLatestPTS = (*pInfo)->m_iTimeStamp;
							pVdec->gsPtsInfo.m_iRamainingDuration = (*pInfo)->m_iFrameDuration;
						}
					#endif

						pInfoCtrl->m_iRegIdxPTS[i] = -1; //unused
						pInfoCtrl->m_iUsedIdxPTS--;
						break;
					}
				}

				if( *pInfo ) {
					void *p_temp;
					for(; i < (MAX_INDEX-1); i++) 
					{
						if( pInfoCtrl->m_iRegIdxPTS[i+1] <= -1 )
							break; 

						pInfoCtrl->m_iRegIdxPTS[i ] = pInfoCtrl->m_iRegIdxPTS[i+1];
						pInfoCtrl->m_iRegIdxPTS[i+1] = -1;

						p_temp = pInfoCtrl->m_pRegInfoPTS[i+1];
						pInfoCtrl->m_pRegInfoPTS[i+1] = pInfoCtrl->m_pRegInfoPTS[i];
						pInfoCtrl->m_pRegInfoPTS[i] = p_temp;
					}
				}
			}
		#if 0
			if( pInfoCtrl->m_iTimeStampType == CDMX_DTS_MODE )	//Decode Timestamp (Decode order)
			{
				if( *pInfo != 0 )
				{
					(*pInfo)->m_iTimeStamp =
					(*pInfo)->m_iextTimeStamp = pInfoCtrl->m_iDTS[pInfoCtrl->m_iDispIdxDTS];
					pInfoCtrl->m_iDispIdxDTS = ( pInfoCtrl->m_iDispIdxDTS + 1 ) & (MAX_INDEX-1);
				}
			}
		#endif
		}
		break;
	}

	return;
}

pts_ctrl * vdec_get_PtsInfo(void *pInst)
{
	_vdec_ * pVdec = (_vdec_ *) pInst;

	if(!pInst){
		ALOGE("vdec_get_PtsInfo :: Instance is null!!");
		return NULL;
	}

	return &pVdec->gsPtsInfo;

}

void * vdec_alloc_instance(int codec_format, int refer_instance)
{
	_vdec_ *pInst = NULL;
	int fd, nInstance;
	char *mgr_name;
	INSTANCE_INFO iInst_info;

	pInst = (_vdec_*)TCC_malloc(sizeof(_vdec_));
	if( pInst )
	{
		memset(pInst, 0x00, sizeof(_vdec_));

		pInst->mgr_fd = -1;
		pInst->dec_fd = -1;
		pInst->codec_format = codec_format;

		mgr_name = VPU_MGR_NAME;

		pInst->mgr_fd = open(mgr_name, O_RDWR);
		if(pInst->mgr_fd < 0)
		{
			LOGE("%s open error[%s]!!", mgr_name, strerror(errno));
			goto MGR_OPEN_ERR;
		}

		iInst_info.type = VPU_DEC;
		iInst_info.nInstance = refer_instance;
		if(ioctl(pInst->mgr_fd, VPU_GET_INSTANCE_IDX, &iInst_info) < 0){
			LOGE("%s ioctl(0x%x) error[%s]!!", mgr_name, VPU_GET_INSTANCE_IDX, strerror(errno));
		}
		if( iInst_info.nInstance < 0 )
		{
			goto INST_GET_ERR;
		}

		pInst->vdec_instance_index = iInst_info.nInstance;
		pInst->dec_fd = open(dec_devices[pInst->vdec_instance_index], O_RDWR);
		if(pInst->dec_fd < 0)
		{
			LOGE("%s open error[%s]", dec_devices[pInst->vdec_instance_index], strerror(errno));
			goto DEC_OPEN_ERR;
		}

		total_opened_decoder++;
		LOGI("@@@@@@@@@@@@@@@@@@@@@@@@@@@@[%d] alloc Instance[%d] = %s", refer_instance, pInst->vdec_instance_index, dec_devices[pInst->vdec_instance_index]);

		{
			vpu_opened_count++;
			LOGI("[VDEC-%d] %d/%d :: vdec_alloc_instance total %d", pInst->vdec_instance_index, vpu_opened_count, total_opened_decoder);
		}
		
	}

	return pInst;

DEC_OPEN_ERR:
	iInst_info.type = VPU_DEC;
	iInst_info.nInstance = pInst->vdec_instance_index;
	if( ioctl(pInst->mgr_fd, VPU_CLEAR_INSTANCE_IDX, &iInst_info) < 0){
		LOGE("%s ioctl(0x%x) error[%s]!!", mgr_name, VPU_CLEAR_INSTANCE_IDX, strerror(errno));
	}
INST_GET_ERR:
	if(close(pInst->mgr_fd) < 0){
		LOGE("%s close error[%s]", mgr_name, strerror(errno));
	}
MGR_OPEN_ERR:
	TCC_free(pInst);
	return NULL;
	
}

void vdec_release_instance(void * pInst, int codec_format)
{
	if(pInst)
	{
		_vdec_ * pVdec = (_vdec_ *)pInst;
		int used_instance = pVdec->vdec_instance_index;
		char *mgr_name;
		INSTANCE_INFO iInst_info;

		mgr_name = VPU_MGR_NAME;

		iInst_info.type = VPU_DEC;
		iInst_info.nInstance = used_instance;
		if( ioctl(pVdec->mgr_fd, VPU_CLEAR_INSTANCE_IDX, &iInst_info) < 0){
			LOGE("%s ioctl(0x%x) error[%s]!!", mgr_name, VPU_CLEAR_INSTANCE_IDX, strerror(errno));
		}

		if(pVdec->dec_fd)
		{
			if(close(pVdec->dec_fd) < 0)
			{
				LOGE("%s close error", dec_devices[pVdec->vdec_instance_index]);
			}
			pVdec->dec_fd = -1;
		}

		if(pVdec->mgr_fd)
		{
			if(close(pVdec->mgr_fd) < 0){
				LOGE("%s close error[%s]", mgr_name, strerror(errno));
			}
			pVdec->mgr_fd = -1;
		}

		TCC_free(pInst);
		pInst = NULL;

		if(total_opened_decoder > 0)
			total_opened_decoder--;

		LOGI("############################ <==== free Instance[%d] = %s", used_instance, dec_devices[used_instance]);

		{
			if(vpu_opened_count > 0)
				vpu_opened_count--;
			LOGI("[VDEC-%d] %d/%d :: vdec_release_instance total %d", used_instance, vpu_opened_count, total_opened_decoder);
		}
	}
}

int vdec_get_instance_index(void * pInst)
{
	_vdec_ * pVdec = (_vdec_ *) pInst;

	if(!pInst){
		ALOGE("vdec_get_instance_index :: Instance is null!!");
		return -1;
	}

	return pVdec->vdec_instance_index;
}

void vdec_set_rendered_index(void * pInst)
{
	if(!pInst){
		ALOGE("vdec_set_rendered_index :: Instance is null!!");
		return;
	}
	
	if(pInst)
	{
		VDEC_RENDERED_BUFFER_t fb_info;
		_vdec_ * pVdec = (_vdec_ *) pInst;

		if(pVdec->renderered == 1)
			return;

		pVdec->renderered = 1;
		fb_info.start_addr_phy = pVdec->gsFrameBufAddr[PA];
		fb_info.size = pVdec->gsFrameBufSize;

		if( 0 > ioctl(pVdec->mgr_fd, VPU_SET_RENDERED_FRAMEBUFFER, &fb_info)){
			LOGE("%s ioctl(0x%x) error[%s]!!", VPU_MGR_NAME, VPU_SET_RENDERED_FRAMEBUFFER, strerror(errno));
		}
	}	
}

int vdec_is_rendered_index(void * pInst)
{
	_vdec_ * pVdec = (_vdec_ *) pInst;
	
	if(!pInst){
		ALOGE("vdec_is_rendered_index :: Instance is null!!");
		return 0;
	}

	return pVdec->renderered;
}

int vdec_is_reused_buffer(void * pInst)
{
	if(!pInst){
		ALOGE("vdec_is_reused_buffer :: Instance is null!!");
		return 0;
	}

	
	VDEC_RENDERED_BUFFER_t fb_info;
	_vdec_ * pVdec = (_vdec_ *) pInst;

	if( 0 > ioctl(pVdec->mgr_fd, VPU_GET_RENDERED_FRAMEBUFFER, &fb_info)){
		LOGE("%s ioctl(0x%x) error[%s]!!", VPU_MGR_NAME, VPU_GET_RENDERED_FRAMEBUFFER, strerror(errno));
	}

	if(	( (fb_info.start_addr_phy >= pVdec->gsFrameBufAddr[PA]) && (fb_info.start_addr_phy < (pVdec->gsFrameBufAddr[PA] + pVdec->gsFrameBufSize)))
		|| ( ((fb_info.start_addr_phy + fb_info.size ) > pVdec->gsFrameBufAddr[PA] ) && ((fb_info.start_addr_phy + fb_info.size ) <= (pVdec->gsFrameBufAddr[PA] + pVdec->gsFrameBufSize)))
		|| ( (fb_info.start_addr_phy <= pVdec->gsFrameBufAddr[PA]) && ((fb_info.start_addr_phy + fb_info.size ) >= (pVdec->gsFrameBufAddr[PA] + pVdec->gsFrameBufSize)))
		){
			ALOGI(" Reused frame-buffer!! render(0x%x-0x%x), current(0x%x-0x%x)", fb_info.start_addr_phy, (fb_info.start_addr_phy + fb_info.size ), pVdec->gsFrameBufAddr[PA], (pVdec->gsFrameBufAddr[PA] + pVdec->gsFrameBufSize));
			return 1;
	}
	else{
		ALOGI(" No Reused frame-buffer!! render(0x%x-0x%x), current(0x%x-0x%x)", fb_info.start_addr_phy, (fb_info.start_addr_phy + fb_info.size ), pVdec->gsFrameBufAddr[PA], (pVdec->gsFrameBufAddr[PA] + pVdec->gsFrameBufSize));
	}

	return 0;
}
