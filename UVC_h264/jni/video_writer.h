
#include "AlCommon.h"

#ifndef _VIDEO_WRITER_H
#define _VIDEO_WRITER_H

int video_writer_init(char *file_name, int width, int height, char *out_fmt);

int video_writer_uninit(void);

void video_write(uint8 *frame, int length);

#endif

