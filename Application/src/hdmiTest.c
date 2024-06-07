#include "hdmiTest.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include "tcc_cam_ioctrl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/types.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/kd.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/poll.h>
#include <getopt.h>
#include <termios.h>
#include <ctype.h>

#include <pthread.h>

#include <condition_variable>
#include <mutex>




static int mHdmiDriverFD = -1;
static int mSwitchManagerDriverFD = -1;
static int mRearCamDriverFD = -1;

int mScreenWidth = 1024,mScreenHeight = 600;
int mPreview_x = 448, mPreview_y = 60;

static int mRearGearPrevStatus;

static std::condition_variable s_hdmiCv;
static std::mutex s_hdmiLock;
static bool s_hdmiIsReady = false;
bool m_bExit = false;


int Handover(void)
{
	int cmd, arg, ret;


	cmd = RCAM_PROCESS_HANDOVER;
    //printf("\n ioctl(mRearCamDriverFD, RCAM_PROCESS_HANDOVER(0x%08x))\n", RCAM_PROCESS_HANDOVER);
	if((ret = ioctl(mRearCamDriverFD, cmd)) < 0) {
		fprintf(stderr, "ERROR: camera command(%d) error with return %d. \n", cmd, ret);
		return -1;
	}

	// todo
	// get rearcam status to sync driver & app

	return 0;
}

bool GetHdmiDev()
{
	unsigned long dev_flag;

	dev_flag = 0;
	ioctl(mHdmiDriverFD,0x8000,&dev_flag);
    //printf("\n ioctl(mHdmiDriverFD,0x8000,&dev_flag)\n");
	//fprintf(stderr, "GetHdmiDev [%d]\n",dev_flag);	

	return (bool) dev_flag;
}
int getSwitchStatus(void) {
	int cmd = 0, arg = 0, ret = 0;

	cmd = SWITCHMANAGER_CMD_GET_STATE;
    //printf("\n ioctl(mSwitchManagerDriverFD, cmd(0x%08x), &arg)\n", cmd);
	if((ret = ioctl(mSwitchManagerDriverFD, cmd, &arg)) < 0) {
		fprintf(stderr, "ERROR: switchmanager command(%d) error with return %d. \n", cmd, ret);
		return -1;
	}

//	fprintf(stdout, "state: %d\n", arg);

	return arg;
}
bool GetRearGearStatus()
{
	unsigned long rear_event_flag;
    //printf("\n ioctl(mRearCamDriverFD,RCAM_GET_STATUS(0x%08x),&rear_event_flag)\n", RCAM_GET_STATUS);
	ioctl(mRearCamDriverFD,RCAM_GET_STATUS,&rear_event_flag);
	rear_event_flag = (rear_event_flag & 0x1);

	return (bool) rear_event_flag;
}

bool SetVideoSwitch(int ch)
{
	unsigned long rear_event_flag = ch;

//printf("\n ioctl(mRearCamDriverFD,VIDEO_USER_CAMERA_SWITCH=(0x%08x),&rear_event_flag)\n", VIDEO_USER_CAMERA_SWITCH);
#ifdef FACTORY_TEST
	ioctl(mRearCamDriverFD,VIDEO_TEST_CAMERA_CVBS,&rear_event_flag);
#else
	ioctl(mRearCamDriverFD,VIDEO_USER_CAMERA_SWITCH,&rear_event_flag);
#endif
	return(true);
}

void ShowPreView(unsigned long OnOff)
{
    //printf("%s\n", __func__);
	DIRECT_DISPLAY_IF_PARAMETERS parameter = {0, };
	int cmd, arg, ret;

	// set paramters
	parameter.preview_width		= mScreenWidth;//960;
	parameter.preview_height	= mScreenHeight;//720;
	parameter.preview_x			= mPreview_x;
	parameter.preview_y			= mPreview_y;
	parameter.handover			= 0;

	if (OnOff) {
		cmd = DIRECT_DISPLAY_IF_INITIALIZE;
        //printf("\n ioctl(mRearCamDriverFD, cmd(0x%08x), &arg)\n", cmd);
		if((ret = ioctl(mRearCamDriverFD, cmd, &arg)) < 0) {
			fprintf(stderr, "ERROR: camera command(%d) error with return %d. \n", cmd, ret);
			return;
		}

		cmd = DIRECT_DISPLAY_IF_START;
        //printf("\n ioctl(mRearCamDriverFD, cmd(0x%08x), &parameter)\n", cmd);
		if((ret = ioctl(mRearCamDriverFD, cmd, &parameter)) < 0) {
			fprintf(stderr, "ERROR: camera command(%d) error with return %d. \n", cmd, ret);
			return;
		}
	}
	else {
		cmd = DIRECT_DISPLAY_IF_STOP;
        //printf("\n ioctl(mRearCamDriverFD, cmd(0x%08x), &arg)\n", cmd);
		if((ret = ioctl(mRearCamDriverFD, cmd, &arg)) < 0) {
			fprintf(stderr, "ERROR: camera command(%d) error with return %d. \n", cmd, ret);
			return;
		}

		cmd = DIRECT_DISPLAY_IF_TERMINATE;
        //printf("\n ioctl(mRearCamDriverFD, cmd(0x%08x), &arg)\n", cmd);
		if((ret = ioctl(mRearCamDriverFD, cmd, &arg)) < 0) {
			fprintf(stderr, "ERROR: camera command(%d) error with return %d. \n", cmd, ret);
			return;
		}
	}
}

int hdmi_init()
{
    unsigned long rear_event_flag;
    mRearCamDriverFD = open(REAR_CAM_CONTROL_NAME, O_RDWR);
    //printf("%s open(REAR_CAM_CONTROL_NAME, O_RDWR) = %d \n", __func__, mRearCamDriverFD);
    if(mRearCamDriverFD < 0)
    {
        //printf("hdmi_init mRearCamDriverFD=%d\n", mRearCamDriverFD);
        //return -1;
    }
    mHdmiDriverFD = open(HDMI_DEV_NAME, O_RDWR);
    //printf("%s open(HDMI_DEV_NAME, O_RDWR) = %d \n", __func__, mHdmiDriverFD);
    if(mHdmiDriverFD < 0)
    {
        //printf("hdmi_init mHdmiDriverFD=%d\n", mHdmiDriverFD);
        //return -1;
    }
    mSwitchManagerDriverFD = open(SWITCHMANAGER_DEVICE, O_RDWR);
    //printf("%s open(SWITCHMANAGER_DEVICE, O_RDWR) = %d \n", __func__, mSwitchManagerDriverFD);
    if(mSwitchManagerDriverFD < 0)
    {
        //printf("hdmi_init mSwitchManagerDriverFD=%d\n", mSwitchManagerDriverFD);
        //return -1;
    }
    Handover();
	mRearGearPrevStatus = GetRearGearStatus();
    rear_event_flag = mRearGearPrevStatus;
    //printf("mRearGearPrevStatus = GetRearGearStatus() = %d\n", mRearGearPrevStatus & 0x01);
    return 0;
}
/////////////////////////////////////////////////////////

void setShowPreView(unsigned long OnOff)
{
	SetVideoSwitch(0);
	if (GetRearGearStatus() == 0)
	{
		ShowPreView(OnOff);
	}
}

void* ProjectScreen(void* args)
{
	hdmi_init();
	usleep(100 * 1000);
	SetVideoSwitch(1);
	if (GetRearGearStatus() == 0)
	{
		ShowPreView(1);
	}
	return 0;
}

void* AccProjectScreen(void* args)
{
	while (!m_bExit)
	{
		std::unique_lock<std::mutex> guard(s_hdmiLock);
		while (!s_hdmiIsReady)
		{
			s_hdmiCv.wait(guard);
		}
		printf("AccProjectScreen ===========================");
		s_hdmiIsReady = false;
		usleep(200 * 1000);
		SetVideoSwitch(1);
		if (GetRearGearStatus() == 0)
		{
			ShowPreView(1);
		}
	}
}

int AccProjectionScreen()
{
	std::unique_lock<std::mutex> guard(s_hdmiLock);
	s_hdmiIsReady = true;
	s_hdmiCv.notify_all();
}

int StartProjectionScreen(/*int argc, char* argv[]*/)
{
	//pthread_t tid;
	//int ret = pthread_create(&tid, NULL, ProjectScreen, NULL);
	hdmi_init();
	usleep(100 * 1000);
	SetVideoSwitch(1);
	if (GetRearGearStatus() == 0)
	{
		ShowPreView(1);
	}
	pthread_t tid;
	int ret = pthread_create(&tid, NULL, AccProjectScreen, NULL);
	return 0;


}

