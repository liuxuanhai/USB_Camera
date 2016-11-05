
#include "AlCommon.h"
#include "video_process.h"
#include "ImageProc.h"


#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_connectCamera
  (JNIEnv *env, jclass jcl)
{

	return video_preview_init();
}

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_releaseCamera
  (JNIEnv *env, jclass jcl)
{
	return video_preview_uninit();
}

JNIEXPORT void JNICALL Java_luo_uvc_jni_ImageProc_getPreviewFrame
  (JNIEnv *env, jclass jcl, jobject bitmap)
{
	int ret;
	int pixels_size = 0;
	AndroidBitmapInfo info;
	void* pixels;


	ret = AndroidBitmap_getInfo(env, bitmap, &info);
	ERROR(ret < 0, err1, "AndroidBitmap_getInfo() failed !");

	ERROR((info.format != ANDROID_BITMAP_FORMAT_RGBA_8888), err2, "Bitmap format is not RGBA_8888 !");

	LOGI("info.width=%d, info.height=%d", info.width, info.height);
	pixels_size = info.width * info.height * 4;

	ret = AndroidBitmap_lockPixels(env, bitmap, &pixels);
	ERROR(ret < 0, err3, "AndroidBitmap_lockPixels() failed !");

	video_get_preview_frame(pixels, pixels_size);

	AndroidBitmap_unlockPixels(env, bitmap);

err1:
err2:
err3:
	return;
}

#define NAMELEN (128)
char fileName[NAMELEN] = {0};

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_startRecord
  (JNIEnv *env, jclass jcl, jstring jstr)
{
	char *prefix = jstringToChar(env, jstr);
	memset(fileName, 0, NAMELEN);
	sprintf(fileName, "/storage/sdcard0/DCIM/%s.avi", prefix);
//	sprintf(fileName, "/data/data/luo.uvc.jni/%s.avi", prefix);
	LOGI("Java_luo_uvc_jni_ImageProc_startRecord----fileName: %s", fileName);
	FREE(prefix);

	return video_record_init(fileName);

}

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_stopRecord
  (JNIEnv *env, jclass jcl)
{

	return video_record_uninit();
}

//jstring to char*
char* jstringToChar(JNIEnv *env, jstring jstr)
{
	char* rtn = NULL;
	jclass clsstring = env->FindClass("java/lang/String");
	jstring strencode = env->NewStringUTF("utf-8");
	jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
	jsize alen = env->GetArrayLength(barr);
	jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);

	if (alen > 0)
	{
		rtn = (char*)calloc(1, alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}

	env->ReleaseByteArrayElements(barr, ba, 0);
	return rtn;
}

#ifdef __cplusplus
}
#endif
