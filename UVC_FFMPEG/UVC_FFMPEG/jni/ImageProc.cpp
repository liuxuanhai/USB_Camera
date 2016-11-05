#include "ImageProc.h"
#include <sys/socket.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <netdb.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <pthread.h>

#include "record_camera.h"

//01-24 07:43:06.439: E/TEST(3756): VIDIOC_DQBUF error 22, Invalid argument

#ifdef __cplusplus
extern "C" {
#endif

int init_pre_send();
int	init_pre_receiver();
int convert2MJPEG(const void *p, int size);

static char *pImageBuf = NULL;
static int imageBufSize = 0;
static int realImageSize = 0;

int setDirectBuffer(char *pDirectBuffer) {

    if (NULL != pDirectBuffer) {
        pImageBuf = pDirectBuffer;
        SetShowBufferAddr((uint8 *)pDirectBuffer);
    } else {
        return -1;
    }

    return 0;
}


int getFrameBufferSize() {

    return imageBufSize;
}


/*******************prepareCamera********************/
jint Java_cc_along_uvcsimple_UVCJni_prepareCamera( JNIEnv* env,jobject thiz, jint videoid){

	int ret = -1;

	LOGI("ret = %d\n", ret);
	ret = RecordInit(NULL, 1);
	LOGI("ret = %d\n", ret);
	if(ret <= 0)
	{
		LOGE("RecordInit failed");
		return -1;
	}

	imageBufSize = ret;
	LOGI("imageBufSize %d\n", imageBufSize);

	return ret;
}

jint Java_cc_along_uvcsimple_UVCJni_prepareCameraWithBase( JNIEnv* env,jobject thiz, jint videoid, jint videobase){

		int ret;

		camerabase = videobase;

		return Java_cc_along_uvcsimple_UVCJni_prepareCamera(env,thiz,videoid);

}


void Java_cc_along_uvcsimple_UVCJni_stopCamera(JNIEnv* env,jobject thiz){


}


/***************camera process****************/
//cc.along.uvcsimple.UVCJni.prepareCamera
jint Java_cc_along_uvcsimple_UVCJni_startRecording(JNIEnv *env, jobject thiz) {

	LOGI("Java_cc_along_uvcsimple_UVCJni_startRecording");

	int ret = -1;

	ret = Recording();
	realImageSize = ret;

	return ret;
}

jint Java_cc_along_uvcsimple_UVCJni_stopRecording(JNIEnv *env, jobject thiz) {

	int ret ;
	ret = RecordUninit();

    return 	ret;
}

void Java_cc_along_uvcsimple_UVCJni_processCamera( JNIEnv* env, jobject thiz){

}

/*****************Buffer operation*******************/

jint Java_cc_along_uvcsimple_UVCJni_getRealImageSize() {
    return realImageSize;
}

jint Java_cc_along_uvcsimple_UVCJni_getBufferSize(JNIEnv *env, jobject thiz) {
    return getFrameBufferSize();
}


jint Java_cc_along_uvcsimple_UVCJni_setDirectBuffer(JNIEnv *env, jobject thiz,
    jobject jbuf, jint jlen) {

    char *pImageBuf = NULL;

    pImageBuf = (char *)env->GetDirectBufferAddress(jbuf);
    if (NULL == pImageBuf) {
        LOGE("Failed to get direct buffer");
        return -1;
    }

    setDirectBuffer(pImageBuf);

    return 0;
}



jint Java_cc_along_uvcsimple_UVCJni_tryOpen(JNIEnv *env, jobject thiz)
{

    int ret  = _try_open();

    return ret;
}


#ifdef __cplusplus
}
#endif
