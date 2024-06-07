#ifndef _HDMITEST_H_
#define _HDMITEST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define	HDMI_DEV_NAME	"/dev/zh_dev"
#define	REAR_CAM_CONTROL_NAME	"/dev/video0"
#define SWITCHMANAGER_DEVICE    "/dev/tcc_switchmanager"
#define SWITCHMANAGER_CMD_GET_STATE 0x50


typedef struct {
	int	camera_type;
	int	camera_encode;
	int	preview_x;
	int	preview_y;
	int	preview_width;
	int	preview_height;
	int	handover;
} DIRECT_DISPLAY_IF_PARAMETERS;

int StartProjectionScreen();
void setShowPreView(unsigned long OnOff);
int AccProjectionScreen();

#ifdef __cplusplus
}
#endif
#endif //!_HDMITEST_H_