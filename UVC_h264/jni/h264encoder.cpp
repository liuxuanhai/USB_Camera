
#include "h264encoder.h"

void compress_begin(Encoder *en, int width, int height)
{
	int m_frameRate = 25;	//甯х巼
	int m_bitRate = 1000;	//鐮佺巼

	en->param = (x264_param_t *) malloc(sizeof(x264_param_t));
	en->picture = (x264_picture_t *) malloc(sizeof(x264_picture_t));

#if 0
	if( x264_param_default_preset( en->param, "medium", NULL ) < 0 )
	return;
	en->param->i_csp = X264_CSP_I420;
	en->param->i_width = width;
	en->param->i_height = height;
	en->param->b_vfr_input = 0;
	en->param->b_repeat_headers = 1;
	en->param->b_annexb = 1;

	/* Apply profile restrictions. */
	if( x264_param_apply_profile( en->param, "high" ) < 0 )
	{
		LOGE("compress_begin error1");
		return;
	}
	if( x264_picture_alloc(en->picture, en->param->i_csp, en->param->i_width, en->param->i_height ) < 0 )
	{
		LOGE("compress_begin error2");
		return;
	}
	if ((en->handle = x264_encoder_open(en->param)) == 0)
	{
		LOGE("compress_begin error3");
		return;
	}
#else
	x264_param_default_preset(en->param, "ultrafast", "zerolatency");
	en->param->i_width = width;	//璁剧疆鐢婚潰瀹藉害
	en->param->i_height = height;	//璁剧疆鐢婚潰楂樺害
	en->param->b_repeat_headers = 1;  // 閲嶅SPS/PPS 鏀惧埌鍏抽敭甯у墠闈�
	en->param->b_cabac = 1;
	en->param->i_threads = 1;
	en->param->i_fps_num = (int) m_frameRate;
	en->param->i_fps_den = 1;
	en->param->i_keyint_max = 1;
	en->param->i_log_level = X264_LOG_NONE; //涓嶆樉绀篹ncoder鐨勪俊鎭�
	if ((en->handle = x264_encoder_open(en->param)) == 0)
	{
		return;
	}
	x264_picture_alloc(en->picture, X264_CSP_I420, en->param->i_width,
			en->param->i_height);
	en->picture->img.i_csp = X264_CSP_I420;
	en->picture->img.i_plane = 3;
#if 0
	x264_param_default(en->param); //set default param
	//en->param->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
	//en->param->rc.i_rc_method = X264_RC_CQP;//璁剧疆涓烘亽瀹氱爜鐜�
	en->param->i_log_level = X264_LOG_DEBUG;

	//en->param->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;//鍙栫┖缂撳瓨鍖轰娇鐢ㄤ笉姝婚攣淇濊瘉
	//en->param->i_csp = X264_CSP_I420;
	en->param->i_width = width;//set frame width
	en->param->i_height = height;//set frame height

	//en->param->i_frame_total = 0;

	//en->param->i_keyint_max = 10;
	en->param->rc.i_lookahead = 0;//琛ㄧずi甯у悜鍓嶇紦鍐插尯
	//en->param->i_bframe = 5; //涓や釜鍙傦拷锟藉抚涔嬮棿b甯х殑鏁扮洰

	//en->param->b_open_gop = 0;
	//en->param->i_bframe_pyramid = 0;
	//en->param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

	//en->param->rc.i_bitrate = 1024 * 10;//rate 锟�?0 kbps
	en->param->i_fps_num = 5;//甯х巼鍒嗗瓙
	en->param->i_fps_den = 1;//甯х巼鍒嗘瘝
	//en->param->i_timebase_den = en->param->i_fps_num;
	//en->param->i_timebase_num = en->param->i_fps_den;
	en->param->pf_log = x264Log;
	if(x264_param_apply_profile(en->param, x264_profile_names[0]) < 0 )//浣跨敤baseline
	{
		LOGE("compress_begin error1");
		return;
	}

	/* Create a new pic */
	if (x264_picture_alloc(en->picture, X264_CSP_I420, en->param->i_width,en->param->i_height) < 0 )
	{
		LOGE("compress_begin error2, en->picture->img.i_csp:%d",en->picture->img.i_csp);
		return;
	}
	LOGE("x264_picture_alloc, en->picture->img.i_csp:%d,en->picture->img.i_plane:%d",en->picture->img.i_csp, en->picture->img.i_plane);
	en->picture->img.i_csp = X264_CSP_I422;
	en->picture->img.i_plane = 3;

	if ((en->handle = x264_encoder_open(en->param)) == 0)
	{
		LOGE("compress_begin error");
		return;
	}
#endif
	//LOGE("x264_encoder_open,en->handle:%d,en->picture->img.i_csp:%d",en->handle, en->picture->img.i_csp);
#endif
}

int compress_frame(Encoder *en, int type, uint8_t *in, uint8_t *out)
{
	x264_picture_t pic_out;
	int nNal = -1;
	int result = 0;
	int i = 0, j = 0;
	int i_frame_size = 0;
	uint8_t *p_out = out;
	en->nal = NULL;
	uint8_t *p422;

	uint8_t *y = en->picture->img.plane[0];
	uint8_t *u = en->picture->img.plane[1];
	uint8_t *v = en->picture->img.plane[2];
	int widthStep422 = en->param->i_width * 2;
	
	for (i = 0; i < en->param->i_height; i += 2)
	{
		p422 = in + i * widthStep422;
		for (j = 0; j < widthStep422; j += 4)
		{
			*(y++) = p422[j];
			*(u++) = p422[j + 1];
			*(y++) = p422[j + 2];
		}
		p422 += widthStep422;
		for (j = 0; j < widthStep422; j += 4)
		{
			*(y++) = p422[j];
			*(v++) = p422[j + 3];
			*(y++) = p422[j + 2];
		}
	}

	switch (type)
	{
	case 0:
		en->picture->i_type = X264_TYPE_P;
		break;
	case 1:
		en->picture->i_type = X264_TYPE_IDR;
		break;
	case 2:
		en->picture->i_type = X264_TYPE_I;
		break;
	default:
		en->picture->i_type = X264_TYPE_AUTO;
		break;
	}
	//LOGE("x264_encoder_encode,en->handle:%d",(int)en->handle);
	i_frame_size = x264_encoder_encode(en->handle, &(en->nal), &nNal,
			en->picture, &pic_out);
	if (i_frame_size < 0)
	{
		LOGE("compress_frame error1: en->picture->img.i_csp:%d",
				en->picture->img.i_csp);
		return -1;
	}
	en->picture->i_pts++;
	//LOGE("x264_encoder_encode OK: nNal:%d,length:%d",nNal,i_frame_size);
	for (i = 0; i < nNal; i++)
	{
		memcpy(p_out, en->nal[i].p_payload, en->nal[i].i_payload);
		p_out += en->nal[i].i_payload;
		result += en->nal[i].i_payload;
		//LOGE("x264_encoder_encode OK: nNal:%d,en->nal[i].i_payload:%d",nNal,en->nal[i].i_payload);
	}

	return result;
}

void compress_end(Encoder *en)
{
	if (en->picture)
	{
		x264_picture_clean(en->picture);
		free(en->picture);
		en->picture = 0;
	}
	if (en->param)
	{
		free(en->param);
		en->param = 0;
	}
	if (en->handle)
	{
		x264_encoder_close(en->handle);
	}
	free(en);
}
