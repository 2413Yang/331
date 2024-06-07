//********************************************************************************************
/**
 * @file        vpu_player.c
 * @brief		play H.264 ES by calling VPU directly
 *
 * @author      Telechips Shenzhen Inc.
 * @date        2021/10/08
 */
//********************************************************************************************

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "tcc_vdec_api.h"

// define screen(LCD) size
#define SCREEN_W		1920
#define SCREEN_H		720

// view position and size
#define VIEW_X			0
#define VIEW_Y			0
#define VIEW_W			SCREEN_W
#define VIEW_H			SCREEN_H
#define IMG_FMT			VIOC_IMG_FMT_YUV420SEP

#define IMG_DISP_MODE_C	0
#define IMG_DISP_MODE_S	1

#define MAX_BUFFER_SIZE	(256*1024)
#define MAX_PATH_LEN	(260)

typedef struct tag_config {
	int  iScreenW, iScreenH;		// FB device size
	int  iViewX, iViewY;			// view position
	int  iViewW, iViewH;			// view size
	char strFileName[MAX_PATH_LEN];	// directory/file name
	int  iPlayType;					// 0: embedded video; 1: directory
	int  iDispDelay;				// unit: ms
	int  iDispMode;					// 0: center, 1: stretch	
	int  iImageFmt;					// video format (ex: VIOC_IMG_FMT_YUV420IL0, see vioc_global.h)
} DecoderConfig_T;

#include "video_data.h"

static int play_embedded_video(int delay_ms)
{
	int iRet = 0;
	int iFrameSize;
	unsigned char *pucBuffer;
	
	while(1) {
		iFrameSize = get_next_video_frame(&pucBuffer);
		if (iFrameSize==0)
			return 0;
		if((iRet=tcc_vdec_process( (unsigned char*)pucBuffer, (int)iFrameSize)) < 0) {
			printf("VPU_DEC_Decode() error!!\n");
			return -1;
		}
		if (delay_ms)
			usleep( delay_ms*1000 );	// play speed control
	}
}

static unsigned long get_file_size(const char *filename)  
{  
	struct stat buf;  
	if(stat(filename, &buf)<0)  
		return 0;  
	return (unsigned long)buf.st_size;  
}  
static int play_h264_splited_files(char *directory, int delay_ms)
{
	int FileNo = 0;
	FILE* hInputFile;
	long lSize;
	int iRet;
	char filepath[MAX_PATH_LEN] = {0};
	unsigned char* pucBuffer = NULL;
	
	pucBuffer = (unsigned char*)malloc( MAX_BUFFER_SIZE );
	
	// decode header
	sprintf( filepath, "%s%s.h264", directory, "header" );
	lSize = get_file_size(filepath);
	hInputFile = fopen(filepath, "r");
	if (!hInputFile) {
		printf( "%s can't open\n", filepath);
		return -1;
	}
	fread(pucBuffer, 1, lSize, hInputFile);
	fclose(hInputFile);
	iRet = tcc_vdec_process((unsigned char*)pucBuffer, (int)lSize);

	while( 1 ) {
		memset( filepath, 0, sizeof(filepath) );
		sprintf( filepath, "%s%04d.h264", directory, FileNo );
		lSize = get_file_size(filepath);
		FileNo++;
		
		// read file to buffer
		hInputFile = fopen(filepath,"r" );
		if( !hInputFile ) {
			printf( "%s can't open\n", filepath );
			break;
		}
		
		fread( pucBuffer, 1, lSize, hInputFile );
		fclose( hInputFile );
		
		// decode
		iRet = tcc_vdec_process( (unsigned char*)pucBuffer, (int)lSize);
		
		if( iRet < 0 )
			printf( "Decode Error [%d]\n", (FileNo-1) );

		if (delay_ms)
			usleep(delay_ms * 1000 );
	}

	free( pucBuffer );
	pucBuffer = NULL;
	
	return 0;
}

static int play_h264_single_file(char *fn, int delay_ms)
{
	int iFrameSize, iPadSize;
	int iRet;
	unsigned char *pucBuffer;
	FILE *hInputFile;

	pucBuffer = (unsigned char*)malloc( MAX_BUFFER_SIZE );
	if (pucBuffer==NULL) return -1;

	hInputFile = fopen(fn, "rb");
	if(hInputFile == NULL) {
		printf("file open failed!! - %s\n", fn);
		free (pucBuffer);
		return -1;
	}
	
	while(1) {
		// get one frame from input file
		if (((iRet=fread(&iFrameSize, sizeof(int), 1, hInputFile))<=0) || ((iRet=fread(&iPadSize, sizeof(int), 1, hInputFile))<=0))
			break;
		if ((iRet=fread(pucBuffer, sizeof(unsigned char), iFrameSize, hInputFile))<=0) 
			break;
		if (iPadSize)
			fseek(hInputFile, iPadSize, SEEK_CUR); // skip padding bytes
			
		// decode the frame		
		if((iRet=tcc_vdec_process((unsigned char*)pucBuffer, (int)iFrameSize)) < 0) {
			printf("VPU_DEC_Decode() error!!\n");
			break;
		}

		// play speed control
		if (delay_ms)
			usleep( delay_ms*1000 );
	}

	fclose(hInputFile);
	free(pucBuffer);

	return iRet;
}

static void cmdline_help(void)
{
	printf("\n");
	printf("################## tcc_vpu_player help ################\n");
	printf(" \n");
	printf("Usage:  tcc_vpu_decoder [Options]\n");
	printf("\n");
	printf("Options:\n");
	printf("    -f  <file>\n");
	printf("        play a H.264 file\n");
	printf("    -d  <dir>\n");
	printf("        play splitted H.264 files in a directory\n");
	printf("    -v  <X Y W H>\n");
	printf("        set view postion and size\n");
	printf("    -m  <mode>\n");
	printf("        set display mode. 0: center; 1:stretch\n");
	printf("    -t  <ms>\n");
	printf("        set frame display interval to <ms>\n");
	printf("    -s  <fmt>\n");
	printf("        specify stream format (see vioc_global.h)\n");
	printf("    -h  (none)\n");
	printf("        display this help\n");
	printf("\n");
}

static int cmdline_parse(int argc, char **argv, DecoderConfig_T *pCfg)
{
	int i;

	for (i=1; i<argc; i++) {
		if (!strcmp(argv[i], "-f")) {
			// play built-in video
			strcpy(pCfg->strFileName, argv[++i]);
			pCfg->iPlayType = 1;
		}
		else if (!strcmp(argv[i], "-d")) {
			// play single file: "-d relative path directory"
			strcpy(pCfg->strFileName, argv[++i]);
			pCfg->iPlayType = 2;
		}
		else if (!strcmp(argv[i], "-v")) {
			// set view position and size: "-v x y w h"
			pCfg->iViewX = atoi(argv[++i]);
			pCfg->iViewY = atoi(argv[++i]);
			pCfg->iViewW = atoi(argv[++i]);
			pCfg->iViewH = atoi(argv[++i]);
			// sanity check
			if (pCfg->iViewX<0) pCfg->iViewX = 0;
			if ((pCfg->iViewX+pCfg->iViewW) > pCfg->iScreenW)
				pCfg->iViewW = pCfg->iScreenW-pCfg->iViewX;
			if (pCfg->iViewY<0) pCfg->iViewY = 0;
			if ((pCfg->iViewY+pCfg->iViewH) > pCfg->iScreenH)
				pCfg->iViewY = pCfg->iScreenH-pCfg->iViewY;
		}
		else if (!strcmp(argv[i], "-m")) {
			// set view display mode: "-m 0/1"
			pCfg->iDispMode = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-t")) {
			pCfg->iDispDelay = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-s")) {
			// video format (ex: VIOC_IMG_FMT_YUV420IL0, see vioc_global.h)
			pCfg->iImageFmt = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-h")) {
			// display help
			cmdline_help();
			exit(0);
		}
	}
}

#if 0 
int main( int argc, char** argv )
{
	DecoderConfig_T tconfig;

	// initialize the default configuration
	memset(&tconfig, 0, sizeof(DecoderConfig_T));
	tconfig.iScreenW   = SCREEN_W;
	tconfig.iScreenH   = SCREEN_H;
	tconfig.iViewX     = (SCREEN_W - VIEW_W)/2;
	tconfig.iViewY     = (SCREEN_H - VIEW_H)/2;	
	tconfig.iViewW     = VIEW_W;
	tconfig.iViewH     = VIEW_H;
	tconfig.iDispMode  = 0;		// play embedded video
	tconfig.iDispDelay = 25;	// delay 25ms per frame
	tconfig.iImageFmt  = VIOC_IMG_FMT_YUV420SEP; // =24
	
	cmdline_parse(argc, argv, &tconfig);
	
	// image display mode: center
	tcc_vdec_init((tconfig.iViewX<<16)|tconfig.iViewY, (tconfig.iViewW<<16)|tconfig.iViewH, tconfig.iDispMode, tconfig.iImageFmt, 1); 

	tcc_vdec_open();

	if (tconfig.iPlayType == 1)
		play_h264_single_file(tconfig.strFileName, tconfig.iDispDelay);
	else if (tconfig.iPlayType == 2)
		play_h264_splited_files(tconfig.strFileName, tconfig.iDispDelay);
	else
		play_embedded_video(tconfig.iDispDelay);

	tcc_vdec_close();

	return 0;
}

#endif