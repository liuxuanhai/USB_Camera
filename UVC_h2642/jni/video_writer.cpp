
#include "avilib.h"
#include "video_writer.h"

avi_t *out_fp = NULL;

int video_writer_init(char *file_name, int width, int height, char *out_fmt)
{
	ERROR(((NULL == file_name) || (NULL == out_fmt)), err0, "Invalid args\n");

	ERROR((strstr(file_name, ".avi") == NULL), err1, "Invalid file_name\n");	
	
	out_fp = AVI_open_output_file(file_name);
	LOGE("video_writer_init: %s", AVI_strerror());
	ERROR(NULL == out_fp, err2, "AVI_open_output_file failed!\n");
		
	AVI_set_video(out_fp, width, height, 25, out_fmt);
	
	return 0;
err0:
err1:
err2:
	return -1;
}

int video_writer_uninit(void)
{
	if(NULL !=  out_fp)
	{
		AVI_close(out_fp);
	}	
	return 0;
}

void video_write(uint8 *frame, int length)
{
		
	if(NULL ==  out_fp)
	{
		return;
	}
		
	if(AVI_write_frame(out_fp, (char *)frame, length) < 0)
	{               
	   LOGE("AVI_write_frame fail in thread\n");
	}
	else
	{           
		LOGI("AVI_write_frame: length = %d\n", length);
		length = 0;
	}
}











