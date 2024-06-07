#ifndef __VPU_CODEC_H__
#define __VPU_CODEC_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tag_VPU_CODEC_BOOL_E
{
	VPU_CODEC_FALSE, 
	VPU_CODEC_TRUE
} VPU_CODEC_BOOL_E;

typedef enum tag_VPU_CODEC_Codec_E
{
	VPU_CODEC_Codec_H264, 
	VPU_CODEC_Codec_VC1,		
	VPU_CODEC_Codec_MPEG2,
	VPU_CODEC_Codec_MPEG4, 
	VPU_CODEC_Codec_H263
} VPU_CODEC_Codec_E;

typedef enum tag_VPU_CODEC_PictureType_E
{
	VPU_CODEC_PictureType_I,	// I-Frame
	VPU_CODEC_PictureType_P, 	// P-Frame
	VPU_CODEC_PictureType_B,	// B-Frame
	VPU_CODEC_PictureType_O		// Other types
} VPU_CODEC_PictureType_E;

typedef enum tag_VPU_CODEC_Format_E
{
	VPU_CODEC_Format_YUV420P, 	/* YUV420 planar : Y field + U field + V field */
	VPU_CODEC_Format_YVU420P, 	/* YVU420 planar : Y field + V field + U field */
	VPU_CODEC_Format_YUV420I, 	/* YUV420 interleaved : Y field + UV field. */
	VPU_CODEC_Format_YUV422P, 	/* only for In/Output format in MJPEG case. but, can't be assigned this format forcingly on external request. */
	VPU_CODEC_Format_YUV422V, 	/* only for In/Output format in MJPEG case. but, can't be assigned this format forcingly on external request. */
	VPU_CODEC_Format_YUV224P, 	/* only for In/Output format in MJPEG case. but, can't be assigned this format forcingly on external request. */
	VPU_CODEC_Format_YUV444P, 	/* only for In/Output format in MJPEG case. but, can't be assigned this format forcingly on external request. */
	VPU_CODEC_Format_YUV400P	/* only for In/Output format in MJPEG case. but, can't be assigned this format forcingly on external request. */
} VPU_CODEC_Format_E;

typedef enum tag_VPU_CODEC_AddrType_E
{
	VPU_CODEC_AddrType_Virtaul, 
	VPU_CODEC_AddrType_Physical
} VPU_CODEC_AddrType_E;

#ifdef __cplusplus
}
#endif
#endif	// __VPU_CODEC_H__
