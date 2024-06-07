#ifndef __VPU_DEC_H__
#define __VPU_DEC_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tag_VPU_DEC_InitParam_T
{
	int iWidth;
	int iHeight;
	int iStreamBufferSize;

	VPU_CODEC_Codec_E eCodec;
	VPU_CODEC_Format_E eFormat;
} VPU_DEC_InitParam_T;

typedef struct tag_VPU_DEC_InputParam_T
{
	int iBufferSize;
	
	unsigned char *pucBuffer;

	signed long long llTimeStamp;
} VPU_DEC_InputParam_T;

typedef struct tag_VPU_DEC_OutputParam_T
{
	int iWidth;
	int iHeight;
	int iStride;

	int iDispFrameIdx;

	signed long long llTimeStamp;
	
	unsigned char *pucBufferVir[3];
	unsigned char *pucBufferPhy[3];

	VPU_CODEC_BOOL_E bFrameOut;

	VPU_CODEC_Format_E eFormat;
	VPU_CODEC_PictureType_E ePictureType;

	//added for crop info
	unsigned int crop_left;
	unsigned int crop_top;
	unsigned int crop_right;
	unsigned int crop_bottom;
} VPU_DEC_OutputParam_T;

int VPU_DEC_Init(VPU_DEC_InitParam_T *ptInitParam);
int VPU_DEC_Deinit(void);

int VPU_DEC_Decode(VPU_DEC_InputParam_T *ptInputParam, VPU_DEC_OutputParam_T *ptOutputParam);

int VPU_DEC_ReleaseDecodedFrame(int iDispFrameIdx);


#ifdef __cplusplus
}
#endif

#endif	// __VPU_DEC_H__
