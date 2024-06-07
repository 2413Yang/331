/****************************************************************************
One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#ifndef _TCC_CAMERA_IOCTL_H_
#define _TCC_CAMERA_IOCTL_H_

/*	IDs reserved for driver specific controls */
#define BASE_VIDIOC_PRIVATE 192 	/* 192-255 are private */

#define RCAM_GET_STATUS						_IOR  ('V', BASE_VIDIOC_PRIVATE+30, unsigned long)
#define RCAM_STREAMON						_IO   ('V', BASE_VIDIOC_PRIVATE+31)
#define RCAM_STREAMOFF						_IO   ('V', BASE_VIDIOC_PRIVATE+32)
#define RCAM_LINEBUF_ADDR_WITH_INDEX		_IOWR ('V', BASE_VIDIOC_PRIVATE+33, RCAM_LINE_BUFFER_INFO)
#define RCAM_LINE_UPDATE					_IOW  ('V', BASE_VIDIOC_PRIVATE+34, RCAM_LINE_BUFFER_UPDATE_INFO)

#ifndef KERNEL_VERSION_3_4
#define RCAM_PROCESS_HANDOVER               _IO ('V', BASE_VIDIOC_PRIVATE+35)

#define DIRECT_DISPLAY_IF_INITIALIZE		_IOWR ('V', BASE_VIDIOC_PRIVATE+50, int)
#define DIRECT_DISPLAY_IF_START				_IOWR ('V', BASE_VIDIOC_PRIVATE+51, DIRECT_DISPLAY_IF_PARAMETERS)
#define DIRECT_DISPLAY_IF_STOP				_IOWR ('V', BASE_VIDIOC_PRIVATE+52, int)
#define DIRECT_DISPLAY_IF_TERMINATE			_IOWR ('V', BASE_VIDIOC_PRIVATE+53, int)
#endif

#define VIDEO_USER_CAMERA_SWITCH			_IOWR ('V', BASE_VIDIOC_PRIVATE+70, int)
#define VIDEO_GET_CAMERA_SIGNAL			_IOWR ('V', BASE_VIDIOC_PRIVATE+71, int)

#define VIDEO_TEST_CAMERA_CVBS			_IOWR ('V', BASE_VIDIOC_PRIVATE+72, int)

#endif//_TCC_CAMERA_IOCTL_H_

