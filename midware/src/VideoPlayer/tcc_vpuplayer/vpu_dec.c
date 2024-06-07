#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mach/vioc_global.h>
#include <vpu_codec.h>
#include "vdec_k.h"
#include "vpu_dec.h"

typedef enum tag_VPU_DEC_ContainerType_E
{
	VPU_DEC_ContainerType_None = 0,
	VPU_DEC_ContainerType_MKV,
	VPU_DEC_ContainerType_MP4,
	VPU_DEC_ContainerType_AVI,
	VPU_DEC_ContainerType_MPG,
	VPU_DEC_ContainerType_TS,
	VPU_DEC_ContainerType_ASF,
	VPU_DEC_ContainerType_RMFF,
	VPU_DEC_ContainerType_FLV = 10
} VPU_DEC_ContainerType_E;

typedef struct tag_VPU_DEC_Instance_T
{
	int iCodec;
	int iFrameRate;
	int iAdditionalFrameCount;
	int iSearchOrSkip;

	VPU_CODEC_BOOL_E bDecoderInit;
	VPU_CODEC_BOOL_E bSequenceHeaderDone;
	VPU_CODEC_BOOL_E bFirstFrameOut;

	void *pvDecoder;

	VPU_DEC_ContainerType_E eContainerType;

	vdec_init_t tInitParam;
	vdec_user_info_t tUserInfoParam;
	vdec_input_t tInputParam;
	vdec_output_t tOutputParam;

	dec_disp_info_ctrl_t tDispInfoCtrl;
	dec_disp_info_input_t tDispInfoIn;
	dec_disp_info_t tDispInfo[MAX_INDEX];

	cdk_func_t *pfnCdk;
} VPU_DEC_Instance_T;

static VPU_DEC_Instance_T *G_ptInstance = NULL;

int VPU_DEC_Init(VPU_DEC_InitParam_T *ptInitParam)
{
	VPU_DEC_Instance_T *ptInstance = G_ptInstance;

	if(ptInstance != NULL)
		return -1;

	ptInstance = G_ptInstance = (VPU_DEC_Instance_T *)calloc(1, sizeof(VPU_DEC_Instance_T));
		
	switch(ptInitParam->eCodec)
	{
	case VPU_CODEC_Codec_H264:
		ptInstance->iCodec = STD_AVC;
		break;
	case VPU_CODEC_Codec_H263:
		ptInstance->iCodec = STD_H263;
		break;
	case VPU_CODEC_Codec_MPEG4:
		ptInstance->iCodec = STD_MPEG4;
		break;
	default:
		printf("Unknown codec!!\n");
		goto error;
	}
//
	ptInstance->iFrameRate 				= 30;
	ptInstance->iAdditionalFrameCount 	= VPU_BUFF_COUNT;
	ptInstance->iSearchOrSkip 			= 0;
	ptInstance->eContainerType 			= VPU_DEC_ContainerType_None;

	ptInstance->pfnCdk = gspfVDecList[ptInstance->iCodec];

	ptInstance->pvDecoder = (void *)vdec_alloc_instance(ptInstance->iCodec, 0);

	memset(&ptInstance->tInitParam, 0, sizeof(vdec_init_t));

	ptInstance->tInitParam.m_iBitstreamFormat 		= ptInstance->iCodec;
	ptInstance->tInitParam.m_iPicWidth 				= ptInitParam->iWidth;
	ptInstance->tInitParam.m_iPicHeight 			= ptInitParam->iHeight;
	ptInstance->tInitParam.m_iBitstreamBufSize 		= ptInitParam->iStreamBufferSize;
	ptInstance->tInitParam.m_bEnableVideoCache 		= 0;
	ptInstance->tInitParam.m_bFilePlayEnable 		= 1;

	if(ptInitParam->eFormat == VPU_CODEC_Format_YUV420P)
	{
		ptInstance->tInitParam.m_bCbCrInterleaveMode = 0;
	}
	else
	{
		ptInstance->tInitParam.m_bCbCrInterleaveMode = 1;
	}

	ptInstance->tUserInfoParam.bitrate_mbps 	= 10;
	ptInstance->tUserInfoParam.frame_rate 		= 30;
	ptInstance->tUserInfoParam.m_bStillJpeg 	= 0;
	ptInstance->tUserInfoParam.jpg_ScaleRatio 	= 0;
	ptInstance->tUserInfoParam.extFunction 		= 0x00;

	if(ptInstance->pfnCdk(VDEC_INIT, NULL, &ptInstance->tInitParam, &ptInstance->tUserInfoParam, ptInstance->pvDecoder) != RETCODE_SUCCESS)
	{
		printf("Decoder initializing failed!!\n");
		goto error;
	}

	memset(&ptInstance->tDispInfoIn, 0, sizeof(dec_disp_info_input_t));

	ptInstance->tDispInfoIn.m_iStdType = ptInstance->tInitParam.m_iBitstreamFormat;
	ptInstance->tDispInfoIn.m_iFmtType = ptInstance->eContainerType;

	if(ptInstance->eContainerType == VPU_DEC_ContainerType_AVI || ptInstance->eContainerType == VPU_DEC_ContainerType_MP4)
	{
		ptInstance->tDispInfoIn.m_iTimeStampType = CDMX_PTS_MODE;
	}
	else
	{
		ptInstance->tDispInfoIn.m_iTimeStampType = CDMX_DTS_MODE;
	}

	vdec_disp_pic_info(CVDEC_DISP_INFO_INIT, &ptInstance->tDispInfoCtrl, ptInstance->tDispInfo, &ptInstance->tDispInfoIn, ptInstance->iFrameRate, 1, ptInstance->pvDecoder);

	ptInstance->bDecoderInit = VPU_CODEC_TRUE;

	return 0;

error:

	VPU_DEC_Deinit();

	return -1;
}

int VPU_DEC_Deinit(void)
{
	VPU_DEC_Instance_T *ptInstance = G_ptInstance;

	if(ptInstance == NULL)
		return -1;

	if(ptInstance->bDecoderInit == VPU_CODEC_TRUE)
	{
		ptInstance->pfnCdk(VDEC_CLOSE, NULL, NULL, NULL, ptInstance->pvDecoder);
	}

	if(ptInstance->pvDecoder != NULL)
	{
		vdec_release_instance(ptInstance->pvDecoder, ptInstance->iCodec);
	}

	free(ptInstance);

	G_ptInstance = NULL;

	return 0;
}

int VPU_DEC_Decode(VPU_DEC_InputParam_T *ptInputParam, VPU_DEC_OutputParam_T *ptOutputParam)
{
	int iRetValue = RETCODE_SUCCESS;

	dec_disp_info_t tDispInfo;

	VPU_DEC_Instance_T *ptInstance = G_ptInstance;

	if(ptInstance == NULL)
		return -RETCODE_FAILURE;

	ptInstance->tInputParam.m_pInp[PA] 	= ptInstance->tInputParam.m_pInp[VA] = ptInputParam->pucBuffer;
	ptInstance->tInputParam.m_iInpLen 	= ptInputParam->iBufferSize;

	if(ptInstance->bSequenceHeaderDone == VPU_CODEC_FALSE)
	{
		ptInstance->iAdditionalFrameCount = VPU_BUFF_COUNT;

		vpu_set_additional_refframe_count(ptInstance->iAdditionalFrameCount - 1, ptInstance->pvDecoder);

		if((iRetValue = ptInstance->pfnCdk(VDEC_DEC_SEQ_HEADER, NULL, &ptInstance->tInputParam, &ptInstance->tOutputParam, ptInstance->pvDecoder)) != RETCODE_SUCCESS)
		{
			printf("Sequence header error!! - %d\n", iRetValue);
			return iRetValue;
		}

		ptInstance->iSearchOrSkip = 1;

		ptInstance->bSequenceHeaderDone = VPU_CODEC_TRUE;
	}

	switch(ptInstance->iSearchOrSkip)
	{
	case 0:
		{
			ptInstance->tInputParam.m_iSkipFrameNum 		= 0;
			ptInstance->tInputParam.m_iFrameSearchEnable 	= 0;		// I-frame (IDR-picture for H.264)
			ptInstance->tInputParam.m_iSkipFrameMode 		= VDEC_SKIP_FRAME_DISABLE;
		}
		break;
	case 1:
		{
			ptInstance->tInputParam.m_iSkipFrameNum 		= 1;
			//ptInstance->tInputParam.m_iFrameSearchEnable 	= 0x001;	// I-frame (IDR-picture for H.264)
			ptInstance->tInputParam.m_iFrameSearchEnable 	= 0x201;	// I-frame (I-slice for H.264) : Non IDR-picture
			ptInstance->tInputParam.m_iSkipFrameMode 		= VDEC_SKIP_FRAME_DISABLE;

			if(ptInstance->tInputParam.m_iFrameSearchEnable == 0x001)
			{
				printf("[VPU_DEC] I-frame Search Mode(IDR-picture for H.264) Enable!!!\n");
			}
			else if(ptInstance->tInputParam.m_iFrameSearchEnable == 0x201)
			{
				printf("[VPU_DEC] I-frame Search Mode(I-slice for H.264) Enable!!!\n");
			}
		}
		break;
	case 2:
		{
			ptInstance->tInputParam.m_iSkipFrameNum 		= 1;
			ptInstance->tInputParam.m_iFrameSearchEnable 	= 0;
			ptInstance->tInputParam.m_iSkipFrameMode 		= VDEC_SKIP_FRAME_ONLY_B;

			printf("[VPU_DEC] B-frame Skip Mode Enable!!!\n");
		}
		break;
	default:
		{
		}
		break;
	}

	if((iRetValue = ptInstance->pfnCdk(VDEC_DECODE, NULL, &ptInstance->tInputParam, &ptInstance->tOutputParam, ptInstance->pvDecoder)) != RETCODE_SUCCESS)
	{
		printf("[VPU_DEC] VDEC_DECODE error! - %d\n", iRetValue);
		return iRetValue;
	}

	if(ptInstance->tOutputParam.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS && ptInstance->tOutputParam.m_DecOutInfo.m_iDecodedIdx >= 0)
	{
		tDispInfo.m_lTimeStamp 			= ptInputParam->llTimeStamp;
		tDispInfo.m_iFrameType 			= ptInstance->tOutputParam.m_DecOutInfo.m_iPicType;
		tDispInfo.m_iPicStructure 		= ptInstance->tOutputParam.m_DecOutInfo.m_iPictureStructure;
		tDispInfo.m_lextTimeStamp 		= 0;
		tDispInfo.m_iM2vFieldSequence 	= 0;
		tDispInfo.m_iFrameSize 			= ptInstance->tOutputParam.m_DecOutInfo.m_iConsumedBytes;
		tDispInfo.m_iFrameDuration 		= 2;

		switch(ptInstance->tInitParam.m_iBitstreamFormat)
		{
		case STD_AVC:
			{
				tDispInfo.m_iM2vFieldSequence = 0;

				ptInstance->tDispInfoIn.m_iFrameIdx = ptInstance->tOutputParam.m_DecOutInfo.m_iDecodedIdx;

				vdec_disp_pic_info(CVDEC_DISP_INFO_UPDATE, (void *)&ptInstance->tDispInfoCtrl, (void *)&tDispInfo, (void *)&ptInstance->tDispInfoIn, ptInstance->iFrameRate, 1, ptInstance->pvDecoder);
			}
			break;
		case STD_MPEG2:
			{
				if(tDispInfo.m_iPicStructure != 3)
				{
					tDispInfo.m_iFrameDuration = 1;
				}
				else if(ptInstance->tOutputParam.m_pInitialInfo->m_iInterlace == 0)
				{
					if(ptInstance->tOutputParam.m_DecOutInfo.m_iRepeatFirstField == 0)
					{
						tDispInfo.m_iFrameDuration = 2;
					}
					else
					{
						tDispInfo.m_iFrameDuration = (ptInstance->tOutputParam.m_DecOutInfo.m_iTopFieldFirst == 0) ? 4 : 6;
					}
				}
				else
				{
					/* interlaced sequence */
					if(ptInstance->tOutputParam.m_DecOutInfo.m_iInterlacedFrame == 0)
					{
						tDispInfo.m_iFrameDuration = 2;
					}
					else
					{
						tDispInfo.m_iFrameDuration = (ptInstance->tOutputParam.m_DecOutInfo.m_iRepeatFirstField == 0) ? 2 : 3;
					}
				}

				tDispInfo.m_iM2vFieldSequence = ptInstance->tOutputParam.m_DecOutInfo.m_iM2vFieldSequence;

				ptInstance->tDispInfoIn.m_iFrameIdx 	= ptInstance->tOutputParam.m_DecOutInfo.m_iDecodedIdx;
				ptInstance->tDispInfoIn.m_iFrameRate 	= ptInstance->tOutputParam.m_DecOutInfo.m_iM2vFrameRate;

				vdec_disp_pic_info(CVDEC_DISP_INFO_UPDATE, (void *)&ptInstance->tDispInfoCtrl, (void *)&tDispInfo, (void *)&ptInstance->tDispInfoIn, ptInstance->iFrameRate, 1, ptInstance->pvDecoder);
			}
			break;
		default:
			{
				tDispInfo.m_iM2vFieldSequence = 0;

				ptInstance->tDispInfoIn.m_iFrameIdx = ptInstance->tOutputParam.m_DecOutInfo.m_iDecodedIdx;

				vdec_disp_pic_info(CVDEC_DISP_INFO_UPDATE, (void *)&ptInstance->tDispInfoCtrl, (void *)&tDispInfo, (void *)&ptInstance->tDispInfoIn, ptInstance->iFrameRate, 1, ptInstance->pvDecoder);
			}
			break;
		}
	}

	if((ptInstance->iSearchOrSkip != 0)
		&& ptInstance->tOutputParam.m_DecOutInfo.m_iDecodedIdx >= 0
		&& (ptInstance->tOutputParam.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS
			|| ptInstance->tOutputParam.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS_FIELD_PICTURE))
	{
		if(ptInstance->tInputParam.m_iFrameSearchEnable)
		{
			ptInstance->iSearchOrSkip = 2;
		}
		else if(ptInstance->tInputParam.m_iSkipFrameMode == VDEC_SKIP_FRAME_ONLY_B)
		{
            if(ptInstance->tOutputParam.m_DecOutInfo.m_iPicType != 3)
            {
			    ptInstance->iSearchOrSkip = 0;
            }
		}
        else if(ptInstance->tInputParam.m_iSkipFrameMode == VDEC_SKIP_FRAME_EXCEPT_I)
		{
			if(ptInstance->tOutputParam.m_DecOutInfo.m_iPicType == 1)
			{
				ptInstance->iSearchOrSkip = 0;
			}
		}
	}

	if(ptInstance->tInitParam.m_iBitstreamFormat == STD_AVC)
	{
		ptOutputParam->iWidth 	= ptInstance->tOutputParam.m_DecOutInfo.m_iWidth - ptInstance->tOutputParam.m_DecOutInfo.m_CropInfo.m_iCropLeft - ptInstance->tOutputParam.m_DecOutInfo.m_CropInfo.m_iCropRight;
		ptOutputParam->iHeight 	= ptInstance->tOutputParam.m_DecOutInfo.m_iHeight - ptInstance->tOutputParam.m_DecOutInfo.m_CropInfo.m_iCropBottom - ptInstance->tOutputParam.m_DecOutInfo.m_CropInfo.m_iCropTop;
	}
	else
	{
	    ptOutputParam->iWidth 	= ptInstance->tOutputParam.m_DecOutInfo.m_iWidth;
	    ptOutputParam->iHeight 	= ptInstance->tOutputParam.m_DecOutInfo.m_iHeight;
	}

	ptOutputParam->iStride = ((ptInstance->tOutputParam.m_DecOutInfo.m_iWidth + 15) >> 4) << 4;

	ptOutputParam->bFrameOut = VPU_CODEC_FALSE;

	if(ptInstance->tOutputParam.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)
	{
		dec_disp_info_t *ptDispInfo;

		ptOutputParam->pucBufferPhy[0] = (unsigned char *)ptInstance->tOutputParam.m_pDispOut[PA][0];
	    ptOutputParam->pucBufferPhy[1] = (unsigned char *)ptInstance->tOutputParam.m_pDispOut[PA][1];
	    ptOutputParam->pucBufferPhy[2] = (unsigned char *)ptInstance->tOutputParam.m_pDispOut[PA][2];

	    ptOutputParam->pucBufferVir[0] = (unsigned char *)ptInstance->tOutputParam.m_pDispOut[VA][0];
	    ptOutputParam->pucBufferVir[1] = (unsigned char *)ptInstance->tOutputParam.m_pDispOut[VA][1];
	    ptOutputParam->pucBufferVir[2] = (unsigned char *)ptInstance->tOutputParam.m_pDispOut[VA][2];
		
		ptOutputParam->crop_top = ptInstance->tOutputParam.m_DecOutInfo.m_CropInfo.m_iCropTop;
		ptOutputParam->crop_left = ptInstance->tOutputParam.m_DecOutInfo.m_CropInfo.m_iCropLeft;
		ptOutputParam->crop_right = ptInstance->tOutputParam.m_DecOutInfo.m_CropInfo.m_iCropRight;
		ptOutputParam->crop_bottom = ptInstance->tOutputParam.m_DecOutInfo.m_CropInfo.m_iCropBottom;

		ptOutputParam->iDispFrameIdx = ptInstance->tOutputParam.m_DecOutInfo.m_iDispOutIdx;

		ptInstance->tDispInfoIn.m_iFrameIdx = ptInstance->tOutputParam.m_DecOutInfo.m_iDispOutIdx;

		vdec_disp_pic_info(CVDEC_DISP_INFO_GET, (void *)&ptInstance->tDispInfoCtrl, (void *)&ptDispInfo, (void *)&ptInstance->tDispInfoIn, ptInstance->iFrameRate, 1, ptInstance->pvDecoder);

		if(ptDispInfo != NULL)
		{
			ptOutputParam->llTimeStamp = ptDispInfo->m_lTimeStamp;
		}
		else
		{
			ptOutputParam->llTimeStamp = ptInputParam->llTimeStamp;
		}

		if(ptInstance->tInitParam.m_bCbCrInterleaveMode)
		{
			ptOutputParam->eFormat = VIOC_IMG_FMT_YUV420IL0;//VPU_CODEC_Format_YUV420I;
		}
		else
		{
			ptOutputParam->eFormat = VIOC_IMG_FMT_YUV420SEP;//VPU_CODEC_Format_YUV420P;
		}

		switch(ptInstance->tOutputParam.m_DecOutInfo.m_iPicType)
		{
		case 0:
			ptOutputParam->ePictureType = VPU_CODEC_PictureType_I;
			break;
		case 1:
			ptOutputParam->ePictureType = VPU_CODEC_PictureType_P;
			break;
		case 2:
			ptOutputParam->ePictureType = VPU_CODEC_PictureType_B;
			break;
		default:
			ptOutputParam->ePictureType = ptInstance->tOutputParam.m_DecOutInfo.m_iPicType;//VPU_CODEC_PictureType_O;
			break;
		}

		ptOutputParam->bFrameOut = VPU_CODEC_TRUE;
	}

	return iRetValue;
}

int VPU_DEC_ReleaseDecodedFrame(int iDispFrameIdx)
{
	int iRetValue;
	VPU_DEC_Instance_T *ptInstance = G_ptInstance;

	if(ptInstance == NULL)
		return -RETCODE_FAILURE;

	if(ptInstance->pfnCdk(VDEC_BUF_FLAG_CLEAR, NULL, &iDispFrameIdx, NULL, ptInstance->pvDecoder) < 0)
	{
		printf("[VPU_DEC] VDEC_BUF_FLAG_CLEAR error! - %d\n", iRetValue);
		return -1;
	}

	return 0;
}
