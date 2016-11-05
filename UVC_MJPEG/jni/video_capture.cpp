
#include "video_capture.h"


static int xioctl(int fd, int request, void *arg) 
{
	int r = 0;
	do 
	{
		r = ioctl(fd, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

static int open_camera(struct usb_camera *cam) 
{
	
	//luoyouren
	cam->frame_buffer = NULL;

	//V4L2_PIX_FMT_YUYV; V4L2_PIX_FMT_MJPEG
	//灏哹uffer鍒嗛厤鐨勮冻澶熷ぇ锛屼笉绠℃槸鍝鏍煎紡鐨勬暟鎹兘鑳藉淇濆瓨
	cam->frame_buffer = (unsigned char *) calloc(1, cam->width * cam->height * 2);
	ERROR((NULL == cam->frame_buffer), err0, "calloc cam->frame_buffer\n");		

	cam->dev_fd = open(cam->device_name, O_RDWR, 0); //  | O_NONBLOCK
	ERROR(cam->dev_fd <= 0, err1, "can't open device\n");
	
	return 0;
	
err0:
	return -1;
err1:
	FREE(cam->frame_buffer);	
	return -1;	
}

static void close_camera(struct usb_camera *cam) 
{	
	LOGI("close_camera");
	CLOSE(cam->dev_fd);
	
	FREE(cam->frame_buffer);
}


static int start_capturing(struct usb_camera *cam) 
{
	int i, ret;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;

	//缂撳啿鍖哄叆闃熷垪
	for (i = 0; i < cam->n_buffers; ++i) 
	{
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		ret = xioctl(cam->dev_fd, VIDIOC_QBUF, &buf);
		ERROR(ret < 0, err0, "VIDIOC_QBUF\n");		
	}

	//鍚姩frame浼犺緭
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = xioctl(cam->dev_fd, VIDIOC_STREAMON, &type);
	ERROR(ret < 0, err1, "VIDIOC_STREAMON\n");

	return 0;
err0:
err1:
	return -1;
}

static int stop_capturing(struct usb_camera *cam) 
{
	int ret;
	enum v4l2_buf_type type;
	LOGI("stop_capturing");

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = xioctl(cam->dev_fd, VIDIOC_STREAMOFF, &type);
	ERROR(ret < 0, err0, "VIDIOC_STREAMOFF\n");
	
	return 0;
err0:
	return -1;
}

static int init_camera(struct usb_camera *cam) 
{
	int ret;
	
	struct v4l2_capability *cap = &(cam->v4l2_cap);
	struct v4l2_cropcap *cropcap = &(cam->v4l2_cropcap);
	struct v4l2_crop *crop = &(cam->crop);
	struct v4l2_format *fmt = &(cam->v4l2_fmt);
	unsigned int min;

	ret = xioctl(cam->dev_fd, VIDIOC_QUERYCAP, cap);
	ERROR(ret < 0, err0, "VIDIOC_QUERYCAP failed!\n");

	ERROR(!(cap->capabilities & V4L2_CAP_VIDEO_CAPTURE), err1, "This camera is no video capture device!");	

	ERROR(!(cap->capabilities & V4L2_CAP_STREAMING), err2, "This camera does not support streaming!");
	
	//#ifdef DEBUG_CAM
	LOGI("\nVIDOOC_QUERYCAP\n");
	LOGI("the camera driver is %s\n", cap->driver);
	LOGI("the camera card is %s\n", cap->card);
	LOGI("the camera bus info is %s\n", cap->bus_info);
	LOGI("the version is %d\n", cap->version);
	//#endif
	/* Select video input, video standard and tune here. */

	CLEAR(*cropcap);
	//cropcap->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//crop->c.width = cam->width;
	//crop->c.height = cam->height;
	//crop->c.left = 0;
	//crop->c.top = 0;
	//crop->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	CLEAR(*fmt);
	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt->fmt.pix.width = cam->width;
	fmt->fmt.pix.height = cam->height;
	fmt->fmt.pix.pixelformat = cam->pixelformat; 	//V4L2_PIX_FMT_YUYV; V4L2_PIX_FMT_MJPEG
	fmt->fmt.pix.field = V4L2_FIELD_ANY;			//V4L2_FIELD_INTERLACED; V4L2_FIELD_ANY
	
	LOGI("before set: fmt->fmt.pix.pixelformat=%p, fmt->fmt.pix.field=%d, fmt->fmt.pix.bytesperline=%d, fmt->fmt.pix.sizeimage=%d\n", \
		fmt->fmt.pix.pixelformat, fmt->fmt.pix.field, fmt->fmt.pix.bytesperline, fmt->fmt.pix.sizeimage);
	LOGI("before set: fmt->fmt.pix.width =%d, fmt->fmt.pix.height=%d\n", fmt->fmt.pix.width , fmt->fmt.pix.height);
	
	ret = xioctl(cam->dev_fd, VIDIOC_S_FMT, fmt);
	ERROR(ret < 0, err3, "VIDIOC_S_FMT failed!\n");	
	
	/* Note VIDIOC_S_FMT may change pixelformat / width / height. */
	LOGI("after set: fmt->fmt.pix.pixelformat=%p, fmt->fmt.pix.field=%d, fmt->fmt.pix.bytesperline=%d, fmt->fmt.pix.sizeimage=%d\n", \
		fmt->fmt.pix.pixelformat, fmt->fmt.pix.field, fmt->fmt.pix.bytesperline, fmt->fmt.pix.sizeimage);
	LOGI("after set: fmt->fmt.pix.width =%d, fmt->fmt.pix.height=%d\n", fmt->fmt.pix.width , fmt->fmt.pix.height);
	
	//褰撹缃簡閿欒鐨勫垎杈ㄧ巼鍜屾牸寮忥紝鍦╒IDIOC_S_FMT鍚庝細鑷姩閫夋嫨涓�绉嶅彲鐢ㄧ殑鍒嗚鲸鐜囧拰鏍煎紡锛屽鑷村墠鍚庣殑涓嶄竴鑷�
	//涓轰簡闃叉鐢ㄦ埛鍙戠敓杩欑鎯呭喌锛屾垜浠妸鍒嗚鲸鐜囧拰鏍煎紡鍙嶉鍥炵敤鎴�
	cam->width = fmt->fmt.pix.width;
	cam->height = fmt->fmt.pix.height;
	cam->pixelformat = fmt->fmt.pix.pixelformat; 

	/* Buggy driver paranoia. */
	min = fmt->fmt.pix.width * 2;
	fmt->fmt.pix.bytesperline = (fmt->fmt.pix.bytesperline < min) ? min : fmt->fmt.pix.bytesperline;
		
	min = fmt->fmt.pix.bytesperline * fmt->fmt.pix.height;
	fmt->fmt.pix.sizeimage = (fmt->fmt.pix.sizeimage < min) ? min : fmt->fmt.pix.sizeimage;
	
	return init_mmap(cam);

err0:
err1:
err2:
err3:
	return -1;
}

static int init_mmap(struct usb_camera *cam) 
{
	int ret;
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	//璇锋眰鍐呭瓨
	ret = xioctl(cam->dev_fd, VIDIOC_REQBUFS, &req);
	ERROR(ret < 0, err0, "This device does not support memory mapping\n");
	
	ERROR(req.count < 2, err1, "Insufficient buffer memory on this device\n");
	
	cam->n_buffers = req.count;
	
	cam->buffers = NULL;
	cam->buffers = (struct buffer *)calloc(req.count, sizeof(struct buffer));
	ERROR(NULL == cam->buffers, err2, "calloc(req.count, sizeof(struct buffer))\n");

	int i;
	struct v4l2_buffer buf;
	for (i = 0; i < req.count; ++i) 
	{
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		//灏哣IDIOC_REQBUFS涓垎閰嶇殑鏁版嵁缂撳瓨杞崲鎴愮墿鐞嗗湴鍧�
		ret = xioctl(cam->dev_fd, VIDIOC_QUERYBUF, &buf);
		ERROR(ret < 0, err3, "VIDIOC_QUERYBUF\n");		

		cam->buffers[i].length = buf.length;
		cam->buffers[i].start = mmap(NULL /* start anywhere */,
				buf.length, PROT_READ | PROT_WRITE /* required */,
				MAP_SHARED /* recommended */, cam->dev_fd, buf.m.offset);

		ERROR(MAP_FAILED == cam->buffers[i].start, err4, "mmap\n");		
	}
	
	return 0;
err0:
err1:
err2:
err3:
err4:
	return -1;
}

static void uninit_camera(struct usb_camera *cam) 
{
	int i;
	LOGI("uninit_camera");
	for (i = 0; i < cam->n_buffers; ++i)
	{
		munmap(cam->buffers[i].start, cam->buffers[i].length);
	}

	FREE(cam->buffers);
}

int video_capture_init(struct usb_camera *cam) 
{
	int ret;
	
	ret = open_camera(cam);
	ERROR(ret < 0, err0, "open camera failed!\n");
	
	ret = init_camera(cam);
	ERROR(ret < 0, err1, "init camera failed!\n");
	
	ret = start_capturing(cam);
	ERROR(ret < 0, err2, "start capturing failed!\n");
	
	return 0;
err0:
	return -1;
err1:
	close_camera(cam);
	return -2;
err2:
	uninit_camera(cam);
	close_camera(cam);
	return -3;
}

void video_capture_uninit(struct usb_camera *cam) 
{
	LOGI("-----video_capture_uninit start");
	stop_capturing(cam);
	uninit_camera(cam);
	close_camera(cam);
	LOGI("-----video_capture_uninit end");
}

int query_frame(struct usb_camera *camera) 
{
	for (;;) 
	{
		fd_set fds;
		struct timeval tv;
		int ret;

		FD_ZERO(&fds);
		FD_SET(camera->dev_fd, &fds);

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		ret = select(camera->dev_fd + 1, &fds, NULL, NULL, &tv);
		if (-1 == ret) 
		{
			if (EINTR == errno)
			{
				continue;
			}
				
			return -1;
		}

		if (0 == ret) 
		{
			LOGI("select timeout\n");
			return -2;
		}

		if(0 == read_frame(camera)) 
		{
			LOGI("query_frame success\n");
			return 0;
		}
		else
		{
			return -3;
		}
	}	
}

static int read_frame(struct usb_camera *cam) 
{
	int ret;
	struct v4l2_buffer buf;	

	CLEAR(buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	//this operator below will change buf.index and (0 <= buf.index <= 3)
	ret = xioctl(cam->dev_fd, VIDIOC_DQBUF, &buf);
	if (-1 == ret) 
	{
		switch (errno) {
		case EAGAIN:
			LOGI("VIDIOC_DQBUF: case EAGAIN\n");
			
		case EIO:
			/* Could ignore EIO, see spec. */
			/* fall through */
			LOGI("VIDIOC_DQBUF: case EAGAIN\n");
		default:
			LOGI("VIDIOC_DQBUF: other defaults\n");;
		}
		return -1;
	}	
	
	LOGI("VIDIOC_DQBUF: buf.bytesused=%d\n", buf.bytesused);
	//V4L2_PIX_FMT_YUYV(YUYV422) ; V4L2_PIX_FMT_MJPEG
	memcpy(cam->frame_buffer, cam->buffers[buf.index].start, buf.bytesused);
	cam->frame_bytesused = buf.bytesused;
		

	ret = xioctl(cam->dev_fd, VIDIOC_QBUF, &buf);
	ERROR(-1 == ret, err0, "VIDIOC_QBUF\n");

	return 0;
	
err0:
	return -1;
}

