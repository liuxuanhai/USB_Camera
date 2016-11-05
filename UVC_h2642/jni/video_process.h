
#include "video_capture.h"
#include "video_writer.h"

#ifndef _VIDEO_PROCESS_H
#define _VIDEO_PROCESS_H


/*
璇存槑锛�
VIDEO_640x480_YUYV_TO_YUYV
640x480鎸嘨4L2鎻愬彇鏃剁殑鍒嗚鲸鐜囧拰淇濆瓨瑙嗛鏂囦欢鏃剁殑鍒嗚鲸鐜�
鍓嶉潰鐨刌UYV鎸嘨4L2鎻愬彇鏃剁殑缂栫爜鏍煎紡
鍚庨潰鐨刌UYV鎸囦繚瀛樿棰戞枃浠舵椂鐨勭紪鐮佹牸寮�

娉ㄦ剰鏈�缁堜繚瀛樼殑瑙嗛鏂囦欢灏佽鏍煎紡閮芥槸avi锛�
*/
enum format_desc
{
	VIDEO_640x480_YUYV_TO_YUYV = 0,
	VIDEO_1280x720_YUYV_TO_YUYV,
	VIDEO_1920x1080_YUYV_TO_YUYV,
	
	VIDEO_640x480_MJPEG_TO_YUYV = 21,
	VIDEO_1280x720_MJPEG_TO_YUYV,
	VIDEO_1920x1080_MJPEG_TO_YUYV,
	
	VIDEO_640x480_MJPEG_TO_MJPEG = 31,
	VIDEO_1280x720_MJPEG_TO_MJPEG,
	VIDEO_1920x1080_MJPEG_TO_MJPEG,
	
	VIDEO_640x480_YUYV_TO_H264 = 41,
	VIDEO_1280x720_YUYV_TO_H264,
	VIDEO_1920x1080_YUYV_TO_H264,
	
	VIDEO_640x480_MJPEG_TO_H264,
	VIDEO_1280x720_MJPEG_TO_H264,	//鏈�浼橀�夋嫨
	VIDEO_1920x1080_MJPEG_TO_H264,
};

struct video_format
{
	enum format_desc desc;
	int width;
	int height;
	unsigned int in_fmt;	//V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_MJPEG
	char *out_fmt;			//"YUYV" "MJPG" "h264"
};


int video_preview_init(void);

int video_preview_uninit(void);

//棰勮绾跨▼
void *video_preview(void *);

void video_get_preview_frame(void *pixels, int pixels_size);


//======================================================================================
int video_record_init(const char *file_name);

int video_record_uninit(void);

void *video_record(void *);


#endif

