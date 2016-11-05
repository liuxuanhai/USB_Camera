
#include "al_common.h"


//Linux...
#ifdef __cplusplus
extern "C"
{
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>


//=================================================================
//录锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟侥结构
typedef struct RecordContext
{
	int IsShow;
	
	int				videoindex;			//锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟�
	AVFormatContext	*pInFmtContext;		//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	AVCodecContext	*pInCodecContext;	//锟斤拷锟斤拷锟斤拷频锟斤拷锟侥憋拷锟斤拷锟斤拷息
	AVCodec			*pInCodec;			//锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷要锟侥斤拷锟斤拷锟斤拷
	AVInputFormat 	*input_fmt;			//锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷式
	
	int pix_size;						//锟斤拷锟截革拷锟斤拷
	
	/*锟芥储压锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟较拷慕峁癸拷濉VFrame锟角凤拷压锟斤拷锟斤拷*/
	AVPacket *in_packet;
	
	AVFormatContext	*pOutFmtContext;	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
	AVCodecContext	*pOutCodecContext;	//锟斤拷锟斤拷锟狡碉拷锟斤拷谋锟斤拷锟斤拷锟较�
	AVCodec			*pOutCodec;			//锟斤拷锟斤拷锟狡碉拷锟斤拷锟揭拷谋锟斤拷锟斤拷锟�
	AVOutputFormat	*output_fmt;		//锟斤拷锟斤拷锟狡碉拷锟斤拷锟绞�
	
	AVStream *out_vd_stream;			//AVStream锟角存储每一锟斤拷锟斤拷频/锟斤拷频锟斤拷锟斤拷息锟侥结构锟斤拷
	AVPacket *out_packet;				//压锟斤拷锟斤拷锟捷帮拷
	
	/*
	锟芥储锟斤拷
	1. 原始锟斤拷锟捷ｏ拷锟斤拷锟斤拷压锟斤拷锟斤拷锟捷ｏ拷锟斤拷锟斤拷锟斤拷锟狡碉拷锟剿碉拷锟結UV锟斤拷RGB锟斤拷锟斤拷锟斤拷频锟斤拷说锟斤拷PCM锟斤拷
	2. 帧锟斤拷息
	*/
	AVFrame	*pInFrame;					//锟斤拷锟斤拷锟斤拷锟狡碉拷锟斤拷锟饺★拷锟斤拷锟斤拷锟斤拷锟狡抵�
	int InFrameBufSize;
	
	AVFrame	*pOutFrame;					//转锟斤拷锟斤拷YUV420P锟斤拷式锟斤拷锟斤拷锟狡抵�
	uint8 *OutFrameBuffer;
	int OutFrameBufSize;
	
	char *device_name;					//video device name
	char *out_file_name;				//录锟狡憋拷锟斤拷锟斤拷频锟斤拷锟侥硷拷锟斤拷
	
	/*
	libswscale锟斤拷一锟斤拷锟斤拷要锟斤拷锟节达拷锟斤拷图片锟斤拷锟斤拷锟斤拷锟捷碉拷锟斤拷狻ｏ拷锟斤拷锟斤拷锟斤拷图片锟斤拷锟截革拷式锟斤拷转锟斤拷锟斤拷图片锟斤拷锟斤拷锟斤拷裙锟斤拷锟�
	sws_getContext()锟斤拷锟斤拷始锟斤拷一锟斤拷SwsContext锟斤拷
	sws_scale()锟斤拷锟斤拷锟斤拷图锟斤拷锟斤拷锟捷★拷
	sws_freeContext()锟斤拷锟酵凤拷一锟斤拷SwsContext锟斤拷
	锟斤拷锟斤拷sws_getContext()也锟斤拷锟斤拷锟斤拷sws_getCachedContext()取锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟杰★拷锟斤拷图片锟斤拷锟斤拷锟斤拷锟捷达拷锟斤拷锟斤拷锟�
	
	srcW锟斤拷源图锟斤拷目锟�
	srcH锟斤拷源图锟斤拷母锟�
	srcFormat锟斤拷源图锟斤拷锟斤拷锟斤拷馗锟绞�
	dstW锟斤拷目锟斤拷图锟斤拷目锟�
	dstH锟斤拷目锟斤拷图锟斤拷母锟�
	dstFormat锟斤拷目锟斤拷图锟斤拷锟斤拷锟斤拷馗锟绞�
	flags锟斤拷锟借定图锟斤拷锟斤拷锟斤拷使锟矫碉拷锟姐法
	锟缴癸拷执锟叫的伙拷锟斤拷锟斤拷锟斤拷锟缴碉拷SwsContext锟斤拷锟斤拷锟津返伙拷NULL锟斤拷
	*/
	struct SwsContext *img_convert_context;
	
	int FrameCount;						//Frames to encode 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫е★拷锟�
	
	uint8 *ShowBuffer;					//锟斤拷锟斤拷刷锟斤拷锟斤拷示锟侥伙拷锟斤拷锟斤拷
	int ShowBufferSize;
	
} RecordContext;
//=================================================================


//=================================================================
static int flush_encoder(AVFormatContext *fmt_context, unsigned int stream_index);

int _try_open(void);

int SetShowBufferAddr(uint8 *ShowBuffer);

int RecordInit(char *RecordFile, int IsShow);

int Recording(void);

int RecordUninit(void);

//=================================================================




#ifdef __cplusplus
};
#endif

