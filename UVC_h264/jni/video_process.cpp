
#include "AlCommon.h"
#include "utils.h"
#include "h264encoder.h"
#include "video_process.h"

//YUYV422 to ARGB
int *rgb = NULL;
int *ybuf = NULL;

//rgb鐠囪鍟撻柨锟�
pthread_mutex_t rgb_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	rgb_cond = PTHREAD_COND_INITIALIZER;

int yuv_tbl_ready = 0;
int y1192_tbl[256];
int v1634_tbl[256];
int v833_tbl[256];
int u400_tbl[256];
int u2066_tbl[256];

//h264缂傛牜鐖滈崳锟�
Encoder en;
uint8 *h264_buf = NULL;
int h264_length = 0;

int is_preview = FALSE;
int is_recording = FALSE;
pthread_t preview_id;
pthread_t record_id;

//frame_buffer鐠囪鍟撻柨锟�
pthread_mutex_t record_buffer_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	record_buffer_cond = PTHREAD_COND_INITIALIZER;

//usb camera
struct usb_camera *camera = NULL;

/*
 * 瑜版彜4L2閻⑩垥JPEG閺嶇厧绱￠崣鏍ㄦ殶閹诡喗妞�, 闂囷拷鐟曚焦绁︾粙锟�:
 * MJPEG-->YUYV422-->(YUV420-->h264)
 * frame_buffer-->yuyv_buffer-->h264_buf
 */
uint8 *yuyv_buffer = NULL;

struct video_format *fmt = NULL;

struct video_format fmts[] =
		{
		/*
		 V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_MJPEG
		 "YUYV" "MJPG" "h264"
		 濞夈劍鍓伴敍姘晼闁插繋绗夌憰浣峰▏閻拷19208x1080閸掑棜椴搁悳鍥风礉娑旂喍绗夌憰浣峰▏閻拷"YUYV"閺嶇厧绱￠弶銉ょ箽鐎涙顫嬫０鎴炴瀮娴狅拷
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
						"h264" },	//閺堬拷娴兼﹢锟藉瀚�
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

	// 鐠佸墽鐤嗙紓鎾冲暱閸栵拷
	h264_buf = (uint8 *)calloc(1, width * height * 3);
	ERROR(NULL == h264_buf, err0, "calloc\n");
	return 0;
	err0: return -1;
}

static void close_encoder()
{
	LOGI("close_encoder");
	compress_end(&en);
	FREE(h264_buf);
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

	/*
	 鐠囧瓨妲戦敍锟�
	 VIDEO_1280x720_MJPEG_TO_H264
	 1280x720閹稿槰4L2閹绘劕褰囬弮鍓佹畱閸掑棜椴搁悳鍥ф嫲娣囨繂鐡ㄧ憴鍡涱暥閺傚洣娆㈤弮鍓佹畱閸掑棜椴搁悳锟�
	 閸撳秹娼伴惃鍑狫PEG閹稿槰4L2閹绘劕褰囬弮鍓佹畱缂傛牜鐖滈弽鐓庣础
	 閸氬酣娼伴惃鍑�264閹稿洣绻氱�涙顫嬫０鎴炴瀮娴犺埖妞傞惃鍕椽閻焦鐗稿锟�

	 濞夈劍鍓伴張锟界紒鍫滅箽鐎涙娈戠憴鍡涱暥閺傚洣娆㈢亸浣筋棅閺嶇厧绱￠柈鑺ユЦavi閿涳拷
	 */
	fmt = find_video_format(VIDEO_640x480_MJPEG_TO_H264);
	ERROR(NULL == fmt, err1, "Invalid video format\n");

	camera->device_name = "/dev/video0";
	camera->width = fmt->width;
	camera->height = fmt->height;
	camera->pixelformat = fmt->in_fmt;

	//閻楃懓鍩嗗▔銊﹀壈閿涙艾鍨垫慨瀣camera閺冭绱濋崣顖濆厴鐠佸墽鐤嗛惃鍓媔dth/height閺冪姵鏅ラ敍宀冿拷宀勫櫚閻€劑绮拋銈囨畱閺堝鏅ラ崚鍡氶哺閻滐拷
	ret = video_capture_init(camera);
	ERROR(ret < 0, err2, "video_capture_init failed\n");

#ifdef USE_ENCODER
	LOGE("after video_capture_init: camera->width=%d, camera->height=%d", camera->width, camera->height);
	//濞夈劍鍓版稉瀣桨閻ㄥ嫭澧嶉張澶夊敩閻緤绱濇稉宥堝厴閺�鐟板綁娴ｅ秶鐤嗛敍灞芥礈娑撻缚顩﹂弽瑙勫祦鐎圭偤妾惃鍓媔dth/height閺夈儱鍨庨柊宥団敄闂傦拷
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

	//閸掓稑缂撴０鍕潔缁捐法鈻�
	is_preview = TRUE;
	ret = pthread_create(&preview_id, NULL, video_preview, NULL);
	ERROR(ret != 0, err8, "pthread_create video_preview\n");

	//閸掓繂顫愰崠鏍敚
	pthread_mutex_init(&record_buffer_cond, NULL);
	pthread_mutex_init(&rgb_mutex, NULL);
	pthread_cond_init(&record_buffer_cond, NULL);
	pthread_cond_init(&rgb_cond, NULL);

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
	is_preview = FALSE;
	FREE(ybuf);
	FREE(rgb);
	FREE(yuyv_buffer);
	close_encoder();
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

	//缁涘绶熸０鍕潔缁捐法鈻肩紒鎾存将
	is_preview = FALSE;
	pthread_cond_signal(&rgb_cond);					//signal thread waitting for rgb_cond to end waitting
	pthread_join(preview_id, NULL);

	//闁匡拷濮ｄ線鏀�
	pthread_mutex_destroy(&record_buffer_cond);
	pthread_mutex_destroy(&rgb_mutex);
	pthread_cond_destroy(&record_buffer_cond);
	pthread_cond_destroy(&rgb_cond);

	FREE(rgb);
	FREE(ybuf);
	FREE(yuyv_buffer);
#ifdef USE_ENCODER
	close_encoder();
#endif
	video_capture_uninit(camera);
	FREE(camera);

	LOGE("=============video_preview_uninit end");

	return 0;
}

//妫板嫯顫嶇痪璺ㄢ柤
//棰勮绾跨▼
void *video_preview(void *)
{
	int ret;
	int width = camera->width;
	int height = camera->height;

	ERROR(FALSE == is_preview, err0, "It does not preview\n");

	while (TRUE == is_preview)
	{

		//閼惧嘲褰囬崶鎯у剼
		ret = query_frame(camera);
		if (ret < 0)
		{
			continue;
		}

		//鏉烆剚宕查幋鎬窾YV422閺嶇厧绱￠敍灞惧閼崇晫绮皔uyv422toABGRY閸掗攱妯夌粈锟�
		//V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_MJPEG
		if (camera->pixelformat == V4L2_PIX_FMT_MJPEG)
		{
			pthread_mutex_lock(&record_buffer_cond);		//娑撳﹪鏀�
			//鐎圭偤妾稉濂橳K楠炶櫕婢橀崣顏呮暜閹镐府JPEG閺嶇厧绱�
			//decode MJPEG to YUYV422
			ret = jpeg_decode(&yuyv_buffer, camera->frame_buffer, &width,
					&height);
			LOGI("after jpeg_decode: ret=%d, width=%d, height=%d\n", ret, width, height);

			pthread_cond_signal(&record_buffer_cond);		//signal thread waitting for yuyv_buffer data
			pthread_mutex_unlock(&record_buffer_cond);		//鐟欙綁鏀�

			if(0 == ret)
			{
				pthread_mutex_lock(&rgb_mutex);					//娑撳﹪鏀�

				LOGI("yuyv422toABGRY\n");
				yuyv422toABGRY(yuyv_buffer, width, height);

				pthread_cond_signal(&rgb_cond);					//signal thread waitting for rgb_cond
				pthread_mutex_unlock(&rgb_mutex);				//鐟欙綁鏀�
			}
		}

	}

	LOGE("video_preview thread end!!!\n");
	return ((void *)0);

err0:
	return ((void *)-1);
}

void video_get_preview_frame(void *pixels, int pixels_size)
{
	ERROR(FALSE == is_preview, err0, "video_get_preview_frame: It does not preview\n");

	pthread_mutex_lock(&rgb_mutex);					//娑撳﹪鏀�

	pthread_cond_wait(&rgb_cond, &rgb_mutex);		//wait for new rgb data

	memcpy(pixels, (void *)rgb, pixels_size);

	pthread_mutex_unlock(&rgb_mutex);				//鐟欙綁鏀�

err0:
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
		return -100;
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
	//缁涘绶熻ぐ鏇炲煑缁捐法鈻肩紒鎾存将
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

	int width = camera->width;
	int height = camera->height;

	ERROR(((FALSE == is_preview) && (FALSE == is_recording)), err0, "It does not preview and recording\n");

	while(TRUE == is_recording)
	{
		//V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_MJPEG
		//鐎圭偤妾稉濂橳K楠炶櫕婢橀崣顏呮暜閹镐府JPEG閺嶇厧绱�
		if (camera->pixelformat == V4L2_PIX_FMT_MJPEG)
		{
			pthread_mutex_lock(&record_buffer_cond);		//娑撳﹪鏀�
			{
				LOGE("0 == strcmp(h264, fmt->out_fmt)");

				pthread_cond_wait(&record_buffer_cond, &record_buffer_cond);		//wait for yuyv_buffer data ready

				//鐏忓摤UYV422鏉烆剚宕查幋鎬窾V420閿涘苯鍟�鏉╂稖顢慼264閸樺缂�
				h264_length = compress_frame(&en, -1, yuyv_buffer, h264_buf);
				LOGE("video_write: yuyv_buffer=%p, h264_buf=%p, h264_length=%d", yuyv_buffer, h264_buf, h264_length);

				if (h264_length > 0)
				{
					//閸氭垼顫嬫０鎴炴瀮娴犳湹鑵戦崘娆忓弳娑擄拷鐢冩禈閸嶏拷
					video_write(h264_buf, h264_length);
				}
			}
			pthread_mutex_unlock(&record_buffer_cond);		//鐟欙綁鏀�
		}
	}

	LOGE("video_record thread end!!!\n");
	return ((void *)0);

err0:
	return ((void *)-1);
}

