
#include "AlCommon.h"

#ifndef _H264ENCODER_H
#define _H264ENCODER_H


#include <x264.h>
#include <x264_config.h>


typedef struct 
{
	x264_param_t *param;
	x264_t *handle;
	x264_picture_t *picture; //璇存槑涓�涓棰戝簭鍒椾腑姣忓抚鐗圭偣
	x264_nal_t *nal;
} Encoder;

//鍒濆鍖栫紪鐮佸櫒锛屽苟杩斿洖涓�涓紪鐮佸櫒瀵硅薄
void compress_begin(Encoder *en, int width, int height);

//缂栫爜涓�甯�
int compress_frame(Encoder *en, int type, uint8 *in, uint8 *out);

//閲婃斁鍐呭瓨
void compress_end(Encoder *en);

#endif

