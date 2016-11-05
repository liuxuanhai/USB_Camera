
#include "AlCommon.h"

#ifndef _VIDEO_CAPTURE_H
#define _VIDEO_CAPTURE_H


struct buffer 
{
	void *start;
	size_t length;
};

struct usb_camera 
{
	char *device_name;
	unsigned int pixelformat;	//V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_MJPEG
	
	int dev_fd;
	int width;
	int height;
	
	//V4L2读取格式 for MJPEG or YUYV
	uint8 *frame_buffer;
	int frame_bytesused;
	
	struct v4l2_capability v4l2_cap;
	struct v4l2_cropcap v4l2_cropcap;
	struct v4l2_format v4l2_fmt;
	struct v4l2_crop crop;
	
	int n_buffers;
	struct buffer *buffers;
};


static int xioctl(int fd, int request, void *arg);

static int open_camera(struct usb_camera *cam);

static void close_camera(struct usb_camera *cam);

static int start_capturing(struct usb_camera *cam);

static int stop_capturing(struct usb_camera *cam);

static int init_camera(struct usb_camera *cam);

static int init_mmap(struct usb_camera *cam);

static void uninit_camera(struct usb_camera *cam);

static int read_frame(struct usb_camera *cam);

int video_capture_init(struct usb_camera *cam);

void video_capture_uninit(struct usb_camera *cam);

int query_frame(struct usb_camera *camera);


#endif

