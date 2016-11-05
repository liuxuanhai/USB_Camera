
#include "AlCommon.h"
#include "utils.h"
#include "h264encoder.h"
#include "video_process.h"

#define USE_ENCODER

//YUYV422 to ARGB
int *rgb = NULL;
int *ybuf = NULL;
int yuv_tbl_ready = 0;
int y1192_tbl[256];
int v1634_tbl[256];
int v833_tbl[256];
int u400_tbl[256];
int u2066_tbl[256];

Encoder en;
uint8 *h264_buf = NULL;
int h264_length = 0;

int is_recording = FALSE;
int is_preview = FALSE;
pthread_t record_id;

pthread_mutex_t record_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	record_buffer_cond = PTHREAD_COND_INITIALIZER;

//usb camera
struct usb_camera *camera = NULL;

/*
 * for preview
 * MJPEG-->YUYV422-->ARGB
 * frame_buffer-->yuyv_buffer-->rgb[]
 *
 * for record
 * MJPEG-->h264-->avi
 * frame_buffer-->yuyv_buffer-->yuyv_buffer_dup-->h264buf
 */
uint8 *yuyv_buffer = NULL;
uint8 *yuyv_buffer_dup = NULL;

struct video_format *fmt = NULL;

struct video_format fmts[] =
		{
		/*
		 V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_MJPEG
		 "YUYV" "MJPG" "h264"
		 */
//		{ VIDEO_640x480_YUYV_TO_YUYV, 640, 480, V4L2_PIX_FMT_YUYV, "YUYV" },
//		{ VIDEO_1280x720_YUYV_TO_YUYV, 1280, 720, V4L2_PIX_FMT_YUYV, "YUYV" },
//		{ VIDEO_1920x1080_YUYV_TO_YUYV, 1920, 1080, V4L2_PIX_FMT_YUYV, "YUYV" },
//
//		{ VIDEO_640x480_YUYV_TO_H264, 640, 480, V4L2_PIX_FMT_YUYV, "h264" },
//		{ VIDEO_1280x720_YUYV_TO_H264, 1280, 720, V4L2_PIX_FMT_YUYV, "h264" },
//		{ VIDEO_1920x1080_YUYV_TO_H264, 1920, 1080, V4L2_PIX_FMT_YUYV, "h264" },
//
//		{ VIDEO_640x480_MJPEG_TO_YUYV, 640, 480, V4L2_PIX_FMT_MJPEG, "YUYV" },
//		{ VIDEO_1280x720_MJPEG_TO_YUYV, 1280, 720, V4L2_PIX_FMT_MJPEG, "YUYV" },
//				{ VIDEO_1920x1080_MJPEG_TO_YUYV, 1920, 1080, V4L2_PIX_FMT_MJPEG,
//						"YUYV" },
//
//				{ VIDEO_640x480_MJPEG_TO_MJPEG, 640, 480, V4L2_PIX_FMT_MJPEG,
//						"MJPG" },
//				{ VIDEO_1280x720_MJPEG_TO_MJPEG, 1280, 720, V4L2_PIX_FMT_MJPEG,
//						"MJPG" },
//				{ VIDEO_1920x1080_MJPEG_TO_MJPEG, 1920, 1080,
//						V4L2_PIX_FMT_MJPEG, "MJPG" },

				{ VIDEO_640x480_MJPEG_TO_H264, 640, 480, V4L2_PIX_FMT_MJPEG,
						"h264" },
				{ VIDEO_1280x720_MJPEG_TO_H264, 1280, 720, V4L2_PIX_FMT_MJPEG,
						"h264" },
				{ VIDEO_1920x1080_MJPEG_TO_H264, 1920, 1080, V4L2_PIX_FMT_MJPEG,
						"h264" },

		};

static struct video_format *find_video_format(enum format_desc desc)
{
	int i;
	int count = sizeof(fmts) / sizeof(fmts[0]);
	for (i = 0; i < count; i++)
	{
		if (desc == fmts[i].desc)
		{
			return &fmts[i];
		}
	}

	return NULL;
}

static int init_encoder(int width, int height)
{
	compress_begin(&en, width, height);

	h264_buf = (uint8 *)calloc(1, width * height * 3);
	ERROR(NULL == h264_buf, err0, "calloc\n");
	return 0;
	err0: return -1;
}

static void close_encoder()
{
	LOGI("close_encoder start");
	compress_end(&en);
	FREE(h264_buf);
	LOGI("close_encoder end");
}

static void yuyv422toABGRY(unsigned char *src, int width, int height)
{

	int frameSize = width * height * 2;

	int i;

	if ((!rgb || !ybuf))
	{
		return;
	}
	int *lrgb = NULL;
	int *lybuf = NULL;

	lrgb = &rgb[0];
	lybuf = &ybuf[0];

	if (yuv_tbl_ready == 0)
	{
		for (i = 0; i < 256; i++)
		{
			y1192_tbl[i] = 1192 * (i - 16);
			if (y1192_tbl[i] < 0)
			{
				y1192_tbl[i] = 0;
			}

			v1634_tbl[i] = 1634 * (i - 128);
			v833_tbl[i] = 833 * (i - 128);
			u400_tbl[i] = 400 * (i - 128);
			u2066_tbl[i] = 2066 * (i - 128);
		}
		yuv_tbl_ready = 1;
	}

	for (i = 0; i < frameSize; i += 4)
	{
		unsigned char y1, y2, u, v;
		y1 = src[i];
		u = src[i + 1];
		y2 = src[i + 2];
		v = src[i + 3];

		int y1192_1 = y1192_tbl[y1];
		int r1 = (y1192_1 + v1634_tbl[v]) >> 10;
		int g1 = (y1192_1 - v833_tbl[v] - u400_tbl[u]) >> 10;
		int b1 = (y1192_1 + u2066_tbl[u]) >> 10;

		int y1192_2 = y1192_tbl[y2];
		int r2 = (y1192_2 + v1634_tbl[v]) >> 10;
		int g2 = (y1192_2 - v833_tbl[v] - u400_tbl[u]) >> 10;
		int b2 = (y1192_2 + u2066_tbl[u]) >> 10;

		r1 = r1 > 255 ? 255 : r1 < 0 ? 0 : r1;
		g1 = g1 > 255 ? 255 : g1 < 0 ? 0 : g1;
		b1 = b1 > 255 ? 255 : b1 < 0 ? 0 : b1;
		r2 = r2 > 255 ? 255 : r2 < 0 ? 0 : r2;
		g2 = g2 > 255 ? 255 : g2 < 0 ? 0 : g2;
		b2 = b2 > 255 ? 255 : b2 < 0 ? 0 : b2;

		*lrgb++ = 0xff000000 | b1 << 16 | g1 << 8 | r1;
		*lrgb++ = 0xff000000 | b2 << 16 | g2 << 8 | r2;

		if (lybuf != NULL)
		{
			*lybuf++ = y1;
			*lybuf++ = y2;
		}
	}
}

int video_preview_init(void)
{
	int ret;
	if (TRUE == is_preview)
	{
		return -100;
	}

	camera = NULL;
	camera = (struct usb_camera *)calloc(1, sizeof(struct usb_camera));
	ERROR(NULL == camera, err0, "calloc struct usb_camera\n");

	fmt = find_video_format(VIDEO_640x480_MJPEG_TO_H264);
	ERROR(NULL == fmt, err1, "Invalid video format\n");

	camera->device_name = "/dev/video0";
	camera->width = fmt->width;
	camera->height = fmt->height;
	camera->pixelformat = fmt->in_fmt;

	ret = video_capture_init(camera);
	ERROR(ret < 0, err2, "video_capture_init failed\n");

#ifdef USE_ENCODER
	LOGE("after video_capture_init: camera->width=%d, camera->height=%d", camera->width, camera->height);
	ret = init_encoder(camera->width, camera->height);
	ERROR(ret < 0, err4, "init_encoder");
#endif

	yuyv_buffer = NULL;
	yuyv_buffer = (uint8 *)calloc(1, camera->width * camera->height * 2);
	ERROR(NULL == yuyv_buffer, err5, "calloc yuyv_buffer\n");

	rgb = NULL;
	rgb = (int *) calloc(1, sizeof(int) * camera->width * camera->height);
	ERROR(NULL == rgb, err6, "calloc rgb\n");

	ybuf = NULL;
	ybuf = (int *) malloc(sizeof(int) * camera->width * camera->height);
	ERROR(NULL == ybuf, err7, "calloc ybuf\n");

	yuyv_buffer_dup = NULL;
	yuyv_buffer_dup = (uint8 *)calloc(1, camera->width * camera->height * 2);
	ERROR(NULL == yuyv_buffer_dup, err8, "calloc yuyv_buffer_dup\n");

	pthread_mutex_init(&record_buffer_mutex, NULL);
	pthread_cond_init(&record_buffer_cond, NULL);

	is_preview = TRUE;

	return 0;

err0:
	return -1;
err1:
err2:
	FREE(camera);
	return -2;
err4:
	video_capture_uninit(camera);
	FREE(camera);
	return -4;
err5:
#ifdef USE_ENCODER
	close_encoder();
#endif
	video_capture_uninit(camera);
	FREE(camera);
	return -5;
err6:
	FREE(yuyv_buffer);
#ifdef USE_ENCODER
	close_encoder();
#endif
	video_capture_uninit(camera);
	FREE(camera);
	return -6;
err7:
	FREE(rgb);
	FREE(yuyv_buffer);
#ifdef USE_ENCODER
	close_encoder();
#endif
	video_capture_uninit(camera);
	FREE(camera);
	return -7;
err8:
	FREE(ybuf);
	FREE(rgb);
	FREE(yuyv_buffer);
#ifdef USE_ENCODER
	close_encoder();
#endif
	video_capture_uninit(camera);
	FREE(camera);
	return -8;

}

int video_preview_uninit(void)
{
	LOGE("=============video_preview_uninit start");

	if (FALSE == is_preview)
	{
		return -100;
	}

	if (TRUE == is_recording)
	{
		video_record_uninit();
	}

	pthread_mutex_destroy(&record_buffer_mutex);
	pthread_cond_destroy(&record_buffer_cond);

	FREE(yuyv_buffer_dup);
	FREE(rgb);
	FREE(ybuf);
	FREE(yuyv_buffer);
#ifdef USE_ENCODER
	close_encoder();
#endif
	video_capture_uninit(camera);
	FREE(camera);

	is_preview = FALSE;

	LOGE("=============video_preview_uninit end");

	return 0;
}


void video_get_preview_frame(void *pixels, int pixels_size)
{
	int ret;
	int width = camera->width;
	int height = camera->height;

	if (FALSE == is_preview)
	{
		return ;
	}

	ret = query_frame(camera);
	if (0 == ret)
	{
		//V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_MJPEG
		//Actually MTK only support MJPEG format
//		if (camera->pixelformat == V4L2_PIX_FMT_MJPEG)
		{
			pthread_mutex_lock(&record_buffer_mutex);
			//decode MJPEG to YUYV422
			ret = jpeg_decode(&yuyv_buffer, camera->frame_buffer, &width,
					&height);
			LOGI("after jpeg_decode: ret=%d, width=%d, height=%d\n", ret, width, height);

			pthread_cond_signal(&record_buffer_cond);		//signal thread waitting for yuyv_buffer data
			pthread_mutex_unlock(&record_buffer_mutex);

			if(0 == ret)
			{
				LOGI("==============================yuyv422toABGRY\n");
				yuyv422toABGRY(yuyv_buffer, width, height);

				memcpy(pixels, (void *)rgb, pixels_size);

			}
		}

	}


	return;
}


//======================================================================================
int video_record_init(const char *file_name)
{
	int ret;
	char *out_file = (char *)file_name;	//"luo.avi";

	if (FALSE == is_preview)
	{
		return -100;
	}
	if (TRUE == is_recording)
	{
		return -101;
	}

	LOGE("video_record_init: camera->width=%d, camera->height=%d\n", camera->width, camera->height);
	ret = video_writer_init(out_file, camera->width, camera->height, fmt->out_fmt);
	ERROR(ret < 0, err0, "video_writer_init failed\n");

	is_recording = TRUE;
	ret = pthread_create(&record_id, NULL, video_record, NULL);
	ERROR(ret != 0, err1, "pthread_create video_record\n");

	return 0;
err0:
	return -1;
err1:
	is_recording = FALSE;
	video_writer_uninit();
	return -2;
}

int video_record_uninit(void)
{
	if ((FALSE == is_preview) || (FALSE == is_recording))
	{
		return -100;
	}

	LOGE("=============video_record_uninit start");
	is_recording = FALSE;
	pthread_cond_signal(&record_buffer_cond);		//signal thread waitting for yuyv_buffer data to end waitting
	pthread_join(record_id, NULL);

	video_writer_uninit();

	LOGE("=============video_record_uninit end");

	return 0;
}

void *video_record(void *)
{
	int ret;

	int yuyv_size = camera->width * camera->height * 2;

	ERROR(((FALSE == is_preview) && (FALSE == is_recording)), err0, "It does not preview and recording\n");

	while(TRUE == is_recording)
	{
		//V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_MJPEG
		//Actually MTK only support MJPEG format
//		if (camera->pixelformat == V4L2_PIX_FMT_MJPEG)
		{
			pthread_mutex_lock(&record_buffer_mutex);
			LOGE("0 == strcmp(h264, fmt->out_fmt)");

			pthread_cond_wait(&record_buffer_cond, &record_buffer_mutex);		//wait for yuyv_buffer data ready

			memcpy(yuyv_buffer_dup, yuyv_buffer, yuyv_size);

			pthread_mutex_unlock(&record_buffer_mutex);

#ifdef USE_ENCODER
			h264_length = compress_frame(&en, -1, yuyv_buffer_dup, h264_buf);
			LOGE("video_write: yuyv_buffer_dup=%p, h264_buf=%p, h264_length=%d", yuyv_buffer_dup, h264_buf, h264_length);

			if (h264_length > 0)
			{
				video_write(h264_buf, h264_length);
			}
#endif
		}
	}

	LOGE("video_record thread end!!!\n");
	return ((void *)0);

err0:
	return ((void *)-1);
}

