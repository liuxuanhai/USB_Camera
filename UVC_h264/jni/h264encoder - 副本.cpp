
#include "h264encoder.h"

void compress_begin(Encoder *en, int width, int height)
{
	en->param = (x264_param_t *) malloc(sizeof(x264_param_t));
	en->picture = (x264_picture_t *) malloc(sizeof(x264_picture_t));
	
	x264_param_default(en->param); //set default param
	//en->param->rc.i_rc_method = X264_RC_CQP;//璁剧疆涓烘亽瀹氱爜鐜�
	// en->param->i_log_level = X264_LOG_NONE;

	// en->param->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;//鍙栫┖缂撳瓨鍖轰娇鐢ㄤ笉姝婚攣淇濊瘉

	en->param->i_width = width; //set frame width
	en->param->i_height = height; //set frame height

	//en->param->i_frame_total = 0;

	//  en->param->i_keyint_max = 10;
	en->param->rc.i_lookahead = 0; //琛ㄧずi甯у悜鍓嶇紦鍐插尯
	//   en->param->i_bframe = 5; //涓や釜鍙傝�冨抚涔嬮棿b甯х殑鏁扮洰

	//  en->param->b_open_gop = 0;
	//  en->param->i_bframe_pyramid = 0;
	//   en->param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

	//en->param->rc.i_bitrate = 1024 * 10;//rate 涓�10 kbps
	en->param->i_fps_num = 5; //甯х巼鍒嗗瓙
	en->param->i_fps_den = 1; //甯х巼鍒嗘瘝
	x264_param_apply_profile(en->param, x264_profile_names[0]); //浣跨敤baseline

	if ((en->handle = x264_encoder_open(en->param)) == 0) {
		return;
	}
	/* Create a new pic */
	x264_picture_alloc(en->picture, X264_CSP_I422, en->param->i_width,
			en->param->i_height);
	en->picture->img.i_csp = X264_CSP_I422;
	en->picture->img.i_plane = 3;
}

int compress_frame(Encoder *en, int type, uint8 *in, uint8 *out) {
	x264_picture_t pic_out;
	int nNal = -1;
	int result = 0;
	int i = 0, j = 0;
	uint8_t *p_out = out;
	uint8_t *p422 = NULL;
	int width = en->param->i_width;
	int height = en->param->i_height;

	uint8 *y = en->picture->img.plane[0];
	uint8 *u = en->picture->img.plane[1];
	uint8 *v = en->picture->img.plane[2];

	int is_y = 1, is_u = 1;
	int y_index = 0, u_index = 0, v_index = 0;

	int yuv422_length = 2 * width * height;

#if 0
	//搴忓垪涓篩U YV YU YV锛屼竴涓獃uv422甯х殑闀垮害 width * height * 2 涓瓧鑺�
	for (i = 0; i < yuv422_length; ++i) 
	{
		if (is_y) {
			*(y + y_index) = *(in + i);
			++y_index;
			is_y = 0;
		} else {
			if (is_u) {
				*(u + u_index) = *(in + i);
				++u_index;
				is_u = 0;
			} else {
				*(v + v_index) = *(in + i);
				++v_index;
				is_u = 1;
			}
			is_y = 1;
		}
	}
#else
	/* luoyouren
	YUYV(422): YU YV YU YV
	YUV420: Y Y Y Y U V	
	杞崲鏂规硶锛�
	灏哬UYV鏁版嵁鍒嗘垚鍋舵暟琛�(i += 2), 姣忔浠巌琛屽彇U鍒嗛噺锛屼粠i+1琛屽彇V鍒嗛噺锛岃�孻鍒欐瘡琛岄兘鍙�
	*/
	int widthStep422 = width * 2;
	for(i = 0; i < height; i += 2)
	{
		p422 = in + i * widthStep422;
		for(j = 0; j < widthStep422; j+=4)
		{
			*(y++) = p422[j];
			*(u++) = p422[j+1];
			*(y++) = p422[j+2];
		}
		p422 += widthStep422;
		for(j = 0; j < widthStep422; j+=4)
		{
			*(y++) = p422[j];
			*(v++) = p422[j+3];
			*(y++) = p422[j+2];
		}
	}

#endif

	switch (type) {
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

	if (x264_encoder_encode(en->handle, &(en->nal), &nNal, en->picture,
			&pic_out) < 0) {
		return -1;
	}

	for (i = 0; i < nNal; i++) 
	{
		memcpy(p_out, en->nal[i].p_payload, en->nal[i].i_payload);
		p_out += en->nal[i].i_payload;
		result += en->nal[i].i_payload;
	}

	return result;
}

void compress_end(Encoder *en) {
	if (en->picture) {
		x264_picture_clean(en->picture);
		free(en->picture);
		en->picture = 0;
	}
	if (en->param) {
		free(en->param);
		en->param = 0;
	}
	if (en->handle) {
		x264_encoder_close(en->handle);
	}
	
}
