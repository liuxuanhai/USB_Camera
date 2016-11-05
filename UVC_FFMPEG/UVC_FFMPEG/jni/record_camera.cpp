#include "record_camera.h"

//Linux...
#ifdef __cplusplus
extern "C"
{
#endif

static struct RecordContext *gRecordContext;

static int flush_encoder(AVFormatContext *fmt_context,
		unsigned int stream_index)
{
	int ret;
	int got_picture;
	AVPacket encode_packet;

	if (!(fmt_context->streams[stream_index]->codec->codec->capabilities
			& CODEC_CAP_DELAY))
		return 0;

	while (1)
	{
		encode_packet.data = NULL;
		encode_packet.size = 0;
		av_init_packet(&encode_packet);

		ret = avcodec_encode_video2(fmt_context->streams[stream_index]->codec,
				&encode_packet, NULL, &got_picture);

		av_frame_free (NULL);
		if (ret < 0)
			break;
		if (!got_picture)
		{
			ret = 0;
			break;
		}
		LOGI("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",
				encode_packet.size);
		/* mux encoded frame */
		ret = av_write_frame(fmt_context, &encode_packet);
		if (ret < 0)
			break;
	}
	return ret;
}

int _try_open(void)
{
	int ret = -1;
	char *device = "/dev/video0";
	LOGI("_try_open start ");

	ret = access(device, F_OK);
	ERROR(ret < 0, err1, "device is not exsist!\n");

	ret = open(device, O_RDWR);
	ERROR(ret <= 0, err1, "open failed");

	close(ret);

	LOGI("_try_open end ");

	err1: return ret;
}

int SetShowBufferAddr(uint8 *ShowBuffer)
{
	//锟斤拷始锟斤拷锟皆达拷
	if (NULL == ShowBuffer)
	{
		return -1;
	}
	else
	{
		gRecordContext->ShowBuffer = ShowBuffer;
	}

	return 0;
}

int RecordInit(char *RecordFile, int IsShow)
{
	int i;
	int ret = -1;
	char file[128] =
	{ 0 };
	AVDictionary *option = NULL;
	AVDictionary *param = NULL;

	gRecordContext = (struct RecordContext *) calloc(1,
			sizeof(struct RecordContext));
	ERROR(NULL == gRecordContext, err1, "calloc gRecordContext");

	//memset(gRecordContext, 0, sizeof(struct RecordContext));
	LOGI("RecordInit start\n");

	//锟斤拷始锟斤拷锟斤拷锟斤拷募锟斤拷锟�
	if (RecordFile == NULL)
	{
		LOGI("create a random file to record video\n");
		srand((unsigned) time(NULL));
		sprintf(file, "/storage/sdcard0/Download/1109-%d-%d.mp4", rand() % 10,
				rand() % 10);
		gRecordContext->out_file_name = file;
	}
	else
	{
		gRecordContext->out_file_name = RecordFile;
	}

	gRecordContext->FrameCount = 0;					//锟斤拷锟斤拷锟斤拷锟斤拷效帧锟斤拷锟斤拷锟斤拷
	gRecordContext->IsShow = IsShow;
	gRecordContext->device_name = "/dev/video0";

	av_register_all();				//锟斤拷始锟斤拷锟斤拷锟叫的憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷媒飧达拷锟斤拷锟�
	avformat_network_init();			//锟斤拷始锟斤拷锟斤拷媒锟斤拷锟斤拷锟斤拷锟斤拷锟叫拷锟�

	//锟斤拷锟斤拷占锟�
	gRecordContext->pInFmtContext = avformat_alloc_context();

	//锟斤拷锟斤拷占锟�
	gRecordContext->pOutFmtContext = avformat_alloc_context();
	ERROR(
			((gRecordContext->pInFmtContext == NULL)
					|| (gRecordContext->pOutFmtContext == NULL)), err2,
			"avformat_alloc_context");

	//锟斤拷始锟斤拷libavdevice锟斤拷
	avdevice_register_all();

	//寻锟斤拷video4linux2锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟绞�
	gRecordContext->input_fmt = av_find_input_format("video4linux2");
	ERROR((gRecordContext->input_fmt == NULL), err3,
			"Couldn't av_find_input_format\n");

	//锟斤拷锟斤拷锟斤拷锟斤拷募锟斤拷锟饺★拷锟斤拷募锟斤拷锟斤拷锟斤拷式锟斤拷也锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟绞�
	gRecordContext->output_fmt = av_guess_format(NULL,
			gRecordContext->out_file_name, NULL);
	gRecordContext->pOutFmtContext->oformat = gRecordContext->output_fmt;

	//------------------------------------------------------------------------------
	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷一些锟斤拷锟斤拷

	av_dict_set(&option, "video_size", "640x480", 0);		//锟斤拷锟矫分憋拷锟斤拷
	av_dict_set(&option, "pixel_format", "mjpeg", 0);
	//------------------------------------------------------------------------------	
	/*
	 锟斤拷锟斤拷input_fmt锟斤拷锟借备锟侥硷拷"/dev/video0"锟斤拷始锟斤拷pInFmtContext锟斤拷锟斤拷
	 锟斤拷锟斤拷锟斤拷锟轿拷锟絧InFmtContext锟斤拷锟斤拷锟角达拷/dev/video0锟借备锟斤拷ifmt锟侥革拷式锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷
	 avformat_open_input锟斤拷锟侥革拷锟斤拷锟斤拷option锟角讹拷input_fmt锟斤拷式锟侥诧拷锟斤拷锟斤拷锟斤拷直锟斤拷剩锟絅ULL锟斤拷锟斤拷>锟斤拷锟斤拷锟斤拷
	 */

	ret = avformat_open_input(&gRecordContext->pInFmtContext, "/dev/video0",
			gRecordContext->input_fmt, &option);
	LOGI("avformat_open_input ret = %d ", ret);
	ERROR((ret != 0), err3, "Couldn't open input stream.\n");

	/*
	 锟斤拷锟斤拷锟斤拷锟給ut_file锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷疲锟�
	 锟斤拷锟斤拷锟斤拷锟斤拷取锟矫碉拷output_fmt锟皆硷拷out_file锟斤拷锟斤拷式锟斤拷锟斤拷锟斤拷锟斤拷RMTP/UDP/TCP/file锟斤拷锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷锟�
	 */
	ret = avio_open(&gRecordContext->pOutFmtContext->pb,
			gRecordContext->out_file_name, AVIO_FLAG_READ_WRITE);
	ERROR(ret < 0, err7, "Failed to open output file! \n");

	//锟斤拷询锟斤拷锟斤拷锟斤拷锟斤拷锟叫碉拷锟斤拷锟斤拷锟斤拷锟斤拷息
	ret = avformat_find_stream_info(gRecordContext->pInFmtContext, NULL);
	ERROR(ret < 0, err8, "Couldn't find stream information.\n");

	//寻锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫碉拷锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟絭ideoindex
	gRecordContext->videoindex = -1;
	for (i = 0; i < gRecordContext->pInFmtContext->nb_streams; i++)
	{
		LOGI("nb_streams = %d; codec_type = %d\n", gRecordContext->pInFmtContext->nb_streams, gRecordContext->pInFmtContext->streams[i]->codec->codec_type);

	}

	for (i = 0; i < gRecordContext->pInFmtContext->nb_streams; i++)
	{
		if (gRecordContext->pInFmtContext->streams[i]->codec->codec_type
				== AVMEDIA_TYPE_VIDEO)
		{
			gRecordContext->videoindex = i;
			break;
		}
	}

	ERROR((gRecordContext->videoindex == -1), err9,
			"Couldn't find a video stream.\n");

	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷陆锟揭伙拷锟斤拷锟狡碉拷锟�
	gRecordContext->out_vd_stream = avformat_new_stream(
			gRecordContext->pOutFmtContext, 0);
	ERROR((gRecordContext->out_vd_stream == NULL), err10, "avformat_new_stream");

	gRecordContext->out_vd_stream->time_base.num = 1;
	gRecordContext->out_vd_stream->time_base.den = 25;

	//取锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟侥憋拷锟斤拷锟斤拷息
	gRecordContext->pInCodecContext =
			gRecordContext->pInFmtContext->streams[gRecordContext->videoindex]->codec;
	LOGI("--line %d-- in_w = %d\t in_h = %d\t in_fmt = %d\t in_encode = %d\n",
			__LINE__, gRecordContext->pInCodecContext->width,
			gRecordContext->pInCodecContext->height,
			gRecordContext->pInCodecContext->pix_fmt,
			gRecordContext->pInCodecContext->codec_id);

	//锟斤拷锟捷憋拷锟斤拷锟斤拷息锟斤拷锟斤拷谋锟斤拷锟斤拷锟絀D锟斤拷锟揭碉拷锟斤拷应锟侥斤拷锟斤拷锟斤拷
	gRecordContext->pInCodec = avcodec_find_decoder(
			gRecordContext->pInCodecContext->codec_id);
	ERROR((gRecordContext->pInCodec == NULL), err11, "Codec not found.\n");

	//锟津开诧拷锟斤拷始锟斤拷pInCodec锟斤拷锟斤拷锟斤拷
	ret = avcodec_open2(gRecordContext->pInCodecContext,
			gRecordContext->pInCodec, NULL);
	ERROR(ret < 0, err12, "Could not open input codec.\n")

	LOGI("--line %d-- in_w = %d\t in_h = %d\t in_fmt = %d\t in_encode = %d\n",
			__LINE__, gRecordContext->pInCodecContext->width,
			gRecordContext->pInCodecContext->height,
			gRecordContext->pInCodecContext->pix_fmt,
			gRecordContext->pInCodecContext->codec_id);

	//锟斤拷取锟斤拷锟斤拷锟狡碉拷锟斤拷谋锟斤拷锟斤拷锟较拷娲拷锟街凤拷锟饺伙拷锟斤拷锟叫革拷值锟斤拷始锟斤拷
	gRecordContext->pOutCodecContext = gRecordContext->out_vd_stream->codec;

	gRecordContext->pOutCodecContext->codec_id =
			gRecordContext->output_fmt->video_codec;		//锟斤拷锟斤拷锟斤拷ID
	gRecordContext->pOutCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;//IO锟斤拷锟斤拷锟酵ｏ拷锟斤拷频锟斤拷锟斤拷锟斤拷频锟斤拷
	gRecordContext->pOutCodecContext->pix_fmt = PIX_FMT_YUV420P;//锟斤拷频锟斤拷锟斤拷帧锟斤拷式
	gRecordContext->pOutCodecContext->width =
			gRecordContext->pInCodecContext->width; //帧锟斤拷使锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷帧锟斤拷
	gRecordContext->pOutCodecContext->height =
			gRecordContext->pInCodecContext->height;//帧锟竭ｏ拷使锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷帧锟竭ｏ拷
	gRecordContext->pOutCodecContext->time_base.num = 1;
	gRecordContext->pOutCodecContext->time_base.den = 25;  		//锟斤拷锟斤拷帧锟斤拷25
	gRecordContext->pOutCodecContext->bit_rate = 400000;			//锟斤拷锟斤拷锟斤拷
	gRecordContext->pOutCodecContext->gop_size = 250;//锟斤拷锟斤拷GOP锟斤拷小锟斤拷每250帧锟斤拷锟斤拷一锟斤拷I帧

	LOGI(
			"--line %d-- out_w = %d\t out_h = %d\t out_fmt = %d\t out_encode = %d\n",
			__LINE__, gRecordContext->pOutCodecContext->width,
			gRecordContext->pOutCodecContext->height,
			gRecordContext->pOutCodecContext->pix_fmt,
			gRecordContext->pOutCodecContext->codec_id);

	//H264
	//pOutCodecContext->me_range = 16;
	//pOutCodecContext->max_qdiff = 4;
	//pOutCodecContext->qcompress = 0.6;
	gRecordContext->pOutCodecContext->qmin = 10;
	gRecordContext->pOutCodecContext->qmax = 51;

	//Optional Param
	//锟斤拷值锟斤拷示锟斤拷锟斤拷锟斤拷锟斤拷B帧之锟戒，锟斤拷锟斤拷锟斤拷锟斤拷B帧锟斤拷锟斤拷锟街★拷锟�
	gRecordContext->pOutCodecContext->max_b_frames = 3;

	//Show some Information
	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟较拷锟绞撅拷锟斤拷斩恕锟�1,锟斤拷示锟斤拷锟斤拷锟斤拷锟�; 0, 锟斤拷示锟斤拷锟斤拷锟斤拷锟斤拷
	//av_dump_format(gRecordContext->pOutFmtContext, 0, gRecordContext->out_file_name, 1);

	//锟斤拷锟捷斤拷锟斤拷锟斤拷ID锟揭碉拷锟斤拷应锟侥斤拷锟斤拷锟斤拷
	gRecordContext->pOutCodec = avcodec_find_encoder(
			gRecordContext->pOutCodecContext->codec_id);
	ERROR(!gRecordContext->pOutCodec, err13, "Can not find encoder! \n");

	//------------------------------------------------------------------------------
	//锟斤拷锟斤拷一些锟斤拷锟斤拷

	//H.264
	if (gRecordContext->pOutCodecContext->codec_id == AV_CODEC_ID_H264)
	{
		av_dict_set(&param, "preset", "slow", 0);
		av_dict_set(&param, "tune", "zerolatency", 0);
		//av_dict_set(&param, "profile", "main", 0);
	}
	//H.265
	if (gRecordContext->pOutCodecContext->codec_id == AV_CODEC_ID_H265)
	{
		av_dict_set(&param, "preset", "ultrafast", 0);
		av_dict_set(&param, "tune", "zero-latency", 0);
	}
	//锟津开诧拷锟斤拷始锟斤拷pOutCodec锟斤拷锟斤拷锟斤拷
	if (avcodec_open2(gRecordContext->pOutCodecContext,
			gRecordContext->pOutCodec, &param) < 0)
	{
		LOGE("Failed to open encoder! \n");
		return -1;
	}
	//------------------------------------------------------------------------------

	//锟斤拷始锟斤拷帧
	gRecordContext->pInFrame = av_frame_alloc();
	//avpicture_get_size(目锟斤拷锟绞斤拷锟侥匡拷锟街★拷锟侥匡拷锟街★拷锟�)
	gRecordContext->InFrameBufSize = avpicture_get_size(
			gRecordContext->pInCodecContext->pix_fmt,
			gRecordContext->pInCodecContext->width,
			gRecordContext->pInCodecContext->height);

	gRecordContext->ShowBufferSize = gRecordContext->InFrameBufSize;
	LOGI("ShowBufferSize = %d\tInFrameBufSize = %d\n",
			gRecordContext->ShowBufferSize, gRecordContext->InFrameBufSize);

	gRecordContext->pOutFrame = av_frame_alloc();

	/*
	 锟斤拷锟斤拷pInFrame锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟诫函锟斤拷锟斤拷锟皆讹拷为锟斤拷锟斤拷锟街★拷锟斤拷菘占锟�
	 锟斤拷pOutFrame锟斤拷转锟斤拷锟斤拷式锟斤拷锟街★拷锟斤拷锟揭わ拷确锟斤拷锟秸硷拷锟斤拷锟�
	 **注锟斤拷:
	 锟斤拷锟斤拷锟斤拷锟叫的憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫碉拷锟斤拷锟斤拷源锟斤拷式锟斤拷锟斤拷锟斤拷锟斤拷YUV420P锟斤拷锟斤拷锟皆碉拷锟斤拷频锟借备取锟斤拷锟斤拷锟斤拷帧锟斤拷式锟斤拷锟斤拷锟斤拷锟斤拷锟绞绞憋拷锟�
	 锟斤拷要锟斤拷libswscale锟斤拷锟斤拷锟斤拷锟叫革拷式锟酵分憋拷锟绞碉拷转锟斤拷锟斤拷锟斤拷YUV420P之锟襟，诧拷锟杰斤拷锟叫憋拷锟斤拷压锟斤拷锟斤拷
	 锟斤拷锟斤拷锟斤拷蹋锟紸VStream --> AVPacket --> AVFrame锟斤拷AVFrame锟角凤拷压锟斤拷锟斤拷锟捷帮拷锟斤拷锟斤拷直锟斤拷锟斤拷锟斤拷锟斤拷示锟斤拷
	 锟斤拷锟斤拷锟斤拷蹋锟紸VFrame --> AVPacket --> AVStream
	 */
	//avpicture_get_size(目锟斤拷锟绞斤拷锟侥匡拷锟街★拷锟侥匡拷锟街★拷锟�)
	gRecordContext->OutFrameBufSize = avpicture_get_size(
			gRecordContext->pOutCodecContext->pix_fmt,
			gRecordContext->pOutCodecContext->width,
			gRecordContext->pOutCodecContext->height);
	LOGI("OutFrameBufSize = %d\n", gRecordContext->OutFrameBufSize);

	gRecordContext->OutFrameBuffer = (uint8_t *) av_malloc(
			gRecordContext->OutFrameBufSize);
	avpicture_fill((AVPicture *) gRecordContext->pOutFrame,
			gRecordContext->OutFrameBuffer,
			gRecordContext->pOutCodecContext->pix_fmt,
			gRecordContext->pOutCodecContext->width,
			gRecordContext->pOutCodecContext->height);

	int got_picture;

	/*锟芥储压锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟较拷慕峁癸拷濉VFrame锟角凤拷压锟斤拷锟斤拷*/
	gRecordContext->in_packet = (AVPacket *) av_malloc(sizeof(AVPacket));

	//Be care full of these: av_new_packet should be call
	gRecordContext->out_packet = (AVPacket *) av_malloc(sizeof(AVPacket));//20150918
	av_new_packet(gRecordContext->out_packet, gRecordContext->OutFrameBufSize);	//20150918

	//img_convert_context锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷式转锟斤拷锟斤拷协锟介：锟斤拷式锟斤拷锟街憋拷锟绞★拷转锟斤拷锟姐法...
	gRecordContext->img_convert_context = sws_getContext(
			gRecordContext->pInCodecContext->width,
			gRecordContext->pInCodecContext->height,
			gRecordContext->pInCodecContext->pix_fmt,
			gRecordContext->pOutCodecContext->width,
			gRecordContext->pOutCodecContext->height,
			gRecordContext->pOutCodecContext->pix_fmt, SWS_BICUBIC, NULL, NULL,
			NULL);

	gRecordContext->pix_size = gRecordContext->pOutCodecContext->width
			* gRecordContext->pOutCodecContext->height;

	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫达拷锟絟eader
	avformat_write_header(gRecordContext->pOutFmtContext, NULL);

	LOGI("ShowBufferSize = %d\tInFrameBufSize = %d\n",
			gRecordContext->ShowBufferSize, gRecordContext->InFrameBufSize);
	LOGI("RecordInit end\n");

	return gRecordContext->ShowBufferSize;

	err1: return -1;

	err2:
	LOGI("err2 11111\n");
	FREE(gRecordContext);
	LOGI("err2 22222\n");
	return -1;
	err3:
	LOGI("err3 11111\n");
	avformat_free_context(gRecordContext->pOutFmtContext);
	avformat_free_context(gRecordContext->pInFmtContext);
	FREE(gRecordContext);

	LOGI("err3 22222\n");
	return -1;

	err7: err8: err9: err10: err11: err12: err13:
	LOGI("err3 11111\n");
	avformat_close_input(&gRecordContext->pInFmtContext);
	avformat_free_context(gRecordContext->pOutFmtContext);
	avformat_free_context(gRecordContext->pInFmtContext);
	FREE(gRecordContext);

	LOGI("err3 22222\n");
	return -1;

}

int Recording(void)
{
	int ret = -1;
	int got_picture = -1;

	LOGI("Recording start\n");
	/*锟斤拷取锟斤拷锟斤拷锟叫碉拷锟斤拷频锟斤拷锟斤拷帧锟斤拷锟斤拷锟斤拷频一帧锟斤拷锟斤拷锟界，锟斤拷锟斤拷锟斤拷频锟斤拷时锟斤拷
	 每锟斤拷锟斤拷一锟斤拷锟斤拷频帧锟斤拷锟斤拷要锟饺碉拷锟斤拷 av_read_frame()锟斤拷锟揭伙拷锟斤拷锟狡碉拷锟窖癸拷锟斤拷锟斤拷莅锟斤拷锟�
	 然锟斤拷锟斤拷芏愿锟斤拷锟斤拷萁锟斤拷薪锟斤拷耄拷锟斤拷锟紿.264锟斤拷一帧压锟斤拷锟斤拷锟斤拷通锟斤拷锟斤拷应一锟斤拷NAL锟斤拷锟斤拷
	 packet锟斤拷压锟斤拷锟斤拷锟斤拷
	 */
	ret = av_read_frame(gRecordContext->pInFmtContext,
			gRecordContext->in_packet);
	if (ret >= 0)
	{
		LOGI("lines= %d\tfunc = %s, frame count: %5d\n", __LINE__, __func__,
				gRecordContext->FrameCount);

		//锟斤拷锟斤拷锟斤拷锟斤拷莅锟斤拷锟斤拷锟狡碉拷锟斤拷莅锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷
		if (gRecordContext->in_packet->stream_index
				== gRecordContext->videoindex)
		{
			if (1 == gRecordContext->IsShow)
			{
				//actually buffer data size
				gRecordContext->ShowBufferSize =
						gRecordContext->in_packet->size;
				LOGI(
						"ShowBuffer=0x%x, pInFrame->data=0x%x, ShowBufferSize=%d\n",
						gRecordContext->ShowBuffer,
						gRecordContext->in_packet->data,
						gRecordContext->in_packet->size);

				//锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷图片锟斤拷锟斤拷锟斤拷apk锟斤拷锟皆达拷
				memcpy(gRecordContext->ShowBuffer,
						(const uint8_t* const *) gRecordContext->in_packet->data,
						gRecordContext->ShowBufferSize);
			}
			LOGI("lines= %d\tfunc = %s, frame count: %5d\n", __LINE__, __func__,
					gRecordContext->FrameCount);

			/*
			 锟斤拷packet压锟斤拷锟斤拷锟斤拷锟斤拷取锟斤拷一帧AVFrame锟斤拷压锟斤拷锟斤拷锟斤拷
			 锟斤拷锟斤拷一锟斤拷压锟斤拷锟斤拷锟斤拷慕峁癸拷锟紸VPacket锟斤拷锟斤拷锟揭伙拷锟斤拷锟斤拷锟斤拷慕峁癸拷锟紸VFrame
			 */
			ret = avcodec_decode_video2(gRecordContext->pInCodecContext,
					gRecordContext->pInFrame, &got_picture,
					gRecordContext->in_packet);
			if (ret < 0)
			{
				LOGE("Decode Error.\n");
				av_free_packet(gRecordContext->in_packet);
				return -1;
			}
			LOGI("lines= %d\tfunc = %s, frame count: %5d\n", __LINE__, __func__,
					gRecordContext->FrameCount);

			//锟缴癸拷锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟叫斤拷锟斤拷锟揭恢★拷锟斤拷锟�
			if (got_picture == 1)
			{
				LOGI("lines= %d\tfunc = %s, frame count: %5d\n", __LINE__,
						__func__, gRecordContext->FrameCount);

				LOGI("lines= %d\tfunc = %s, frame count: %5d\n", __LINE__,
						__func__, gRecordContext->FrameCount);

				//转锟斤拷帧锟斤拷式
				sws_scale(gRecordContext->img_convert_context,
						(const uint8_t* const *) gRecordContext->pInFrame->data,
						gRecordContext->pInFrame->linesize, 0,
						gRecordContext->pInCodecContext->height,
						gRecordContext->pOutFrame->data,
						gRecordContext->pOutFrame->linesize);
				LOGI("lines= %d\tfunc = %s, frame count: %5d\n", __LINE__,
						__func__, gRecordContext->FrameCount);

				//PTS: 帧时锟斤拷锟�
				gRecordContext->pOutFrame->pts = gRecordContext->FrameCount;
				gRecordContext->FrameCount++;

				//锟斤拷始压锟斤拷锟斤拷锟斤拷
				got_picture = 0;
				/*
				 锟斤拷帧锟斤拷锟斤拷砂锟斤拷锟斤拷锟斤拷锟揭伙拷锟街★拷锟斤拷锟斤拷一锟斤拷锟斤拷
				 **注锟斤拷:
				 锟斤拷锟斤拷锟斤拷锟叫的憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫碉拷锟斤拷锟斤拷源锟斤拷式锟斤拷锟斤拷锟斤拷锟斤拷YUV420P锟斤拷锟斤拷锟皆碉拷锟斤拷频锟借备取锟斤拷锟斤拷锟斤拷帧锟斤拷式锟斤拷锟斤拷锟斤拷锟斤拷锟绞绞憋拷锟�
				 锟斤拷要锟斤拷libswscale锟斤拷锟斤拷锟斤拷锟叫革拷式锟酵分憋拷锟绞碉拷转锟斤拷锟斤拷锟斤拷YUV420P之锟襟，诧拷锟杰斤拷锟叫憋拷锟斤拷压锟斤拷锟斤拷
				 锟斤拷锟斤拷锟斤拷蹋锟紸VStream --> AVPacket --> AVFrame锟斤拷AVFrame锟角凤拷压锟斤拷锟斤拷锟捷帮拷锟斤拷锟斤拷直锟斤拷锟斤拷锟斤拷锟斤拷示锟斤拷
				 锟斤拷锟斤拷锟斤拷蹋锟紸VFrame --> AVPacket --> AVStream
				 */
				ret = avcodec_encode_video2(gRecordContext->pOutCodecContext,
						gRecordContext->out_packet, gRecordContext->pOutFrame,
						&got_picture);
				if (ret < 0)
				{
					LOGE("Failed to encode! \n");
					av_free_packet(gRecordContext->in_packet);
					return -1;
				}
				LOGI("lines= %d\tfunc = %s, frame count: %5d\n", __LINE__,
						__func__, gRecordContext->FrameCount);

				if (got_picture == 1)
				{
					LOGI(
							"Succeed to encode frame: %5d\tsize:%5d \tindex = %d\n",
							gRecordContext->FrameCount,
							gRecordContext->out_packet->size,
							gRecordContext->out_vd_stream->index);

					gRecordContext->out_packet->stream_index =
							gRecordContext->out_vd_stream->index;//锟斤拷识锟斤拷锟斤拷频/锟斤拷频锟斤拷: 锟斤拷锟�

					//锟斤拷锟斤拷频锟斤拷写锟诫到锟斤拷锟斤拷锟斤拷锟�
					ret = av_write_frame(gRecordContext->pOutFmtContext,
							gRecordContext->out_packet);

					//锟酵放该帮拷
					av_free_packet(gRecordContext->out_packet);
				}
			}
		}
		av_free_packet(gRecordContext->in_packet);
	}

	LOGI("Recording end\n");

	return gRecordContext->ShowBufferSize;
}

int RecordUninit(void)
{
	int ret = -1;

	LOGI("RecordUninit start\n");
	//=========================================
	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷剩锟斤拷锟街★拷锟斤拷莩锟剿拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫ｏ拷锟斤拷写锟斤拷锟侥硷拷锟斤拷锟斤拷止锟斤拷失帧
	ret = flush_encoder(gRecordContext->pOutFmtContext,
			gRecordContext->out_vd_stream->index);
	if (ret < 0)
	{
		LOGE("Flushing encoder failed\n");
	}

	LOGI("av_write_trailer\n");
	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫达拷锟絫ail
	av_write_trailer(gRecordContext->pOutFmtContext);

	sws_freeContext(gRecordContext->img_convert_context);

	//Clean
	if (gRecordContext->out_vd_stream)
	{
		//锟截憋拷锟斤拷锟斤拷锟狡碉拷锟斤拷谋锟斤拷锟斤拷锟�
		avcodec_close(gRecordContext->pOutCodecContext);

		//锟酵凤拷帧
		av_frame_free(&gRecordContext->pOutFrame);

		//锟酵放帮拷
		av_free(gRecordContext->out_packet);

		//锟酵放伙拷锟斤拷
		av_free(gRecordContext->OutFrameBuffer);
	}

	//锟截憋拷锟斤拷锟斤拷锟狡碉拷锟�
	avio_close(gRecordContext->pOutFmtContext->pb);
	//锟截憋拷锟斤拷锟斤拷锟斤拷锟�
	avformat_free_context(gRecordContext->pOutFmtContext);

	//锟酵凤拷帧
	av_frame_free(&gRecordContext->pInFrame);

	//锟酵放帮拷
	//av_free_packet(gRecordContext->in_packet);
	av_free(gRecordContext->in_packet);

	//锟截憋拷锟斤拷锟斤拷锟斤拷频锟斤拷锟侥斤拷锟斤拷锟斤拷
	avcodec_close(gRecordContext->pInCodecContext);

	//锟截憋拷锟斤拷锟斤拷锟斤拷锟斤拷
	avformat_close_input(&gRecordContext->pInFmtContext);
	avformat_free_context(gRecordContext->pInFmtContext);

	FREE(gRecordContext);

	LOGI("RecordUninit end\n");

	return 0;
}

#ifdef __cplusplus
}
;
#endif

