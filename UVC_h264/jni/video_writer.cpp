
#include "avilib.h"
#include "video_writer.h"

//avilib搴撲腑瀹氫箟鐨勬枃浠舵暟鎹粨鏋�
avi_t *out_fp = NULL;

int video_writer_init(char *file_name, int width, int height, char *out_fmt)
{
	ERROR(((NULL == file_name) || (NULL == out_fmt)), err0, "Invalid args\n");

	ERROR((strstr(file_name, ".avi") == NULL), err1, "Invalid file_name\n");	
	
	out_fp = AVI_open_output_file(file_name);
	ERROR(NULL == out_fp, err2, "AVI_open_output_file failed!\n");
		
	AVI_set_video(out_fp, width, height, 30, out_fmt);
	
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
		AVI_close(out_fp);  //鍏抽棴鏂囦欢鎻忚堪绗︼紝骞朵繚瀛樻枃浠�
	}	
	return 0;
}

void video_write(uint8 *frame, int length)
{
		
	if(NULL ==  out_fp)
	{
		return;
	}
		
	//鍚戣棰戞枃浠朵腑鍐欏叆涓�甯у浘鍍�
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











