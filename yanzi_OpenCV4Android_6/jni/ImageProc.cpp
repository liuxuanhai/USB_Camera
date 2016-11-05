#include "ImageProc.h"

#include "cv.h"
#include "highgui.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <string>
#include <vector>

using namespace cv;
using namespace std;

#ifdef __cplusplus
extern "C"
{
#pragma message("------------------------ ImageProc.cpp")
#endif

#define NAMELEN	(64)

Mat frame;
VideoCapture capture;

char *prefix = NULL;
char fileName[NAMELEN] = {0};
VideoWriter writer;

int id;

JNIEXPORT jintArray JNICALL Java_luo_uvc_jni_ImageProc_grayProc(JNIEnv* env,
		jclass obj, jintArray buf, jint w, jint h)
{
	jint *cbuf;
	cbuf = env->GetIntArrayElements(buf, false);
	if (cbuf == NULL)
	{
		return 0;
	}

	Mat imgData(h, w, CV_8UC4, (unsigned char*) cbuf);

	uchar* ptr = imgData.ptr(0);
	for (int i = 0; i < w * h; i++)
	{
		//计算公式：Y(亮度) = 0.299*R + 0.587*G + 0.114*B
		//对于一个int四字节，其彩色值存储方式为：BGRA
		int grayScale = (int) (ptr[4 * i + 2] * 0.299 + ptr[4 * i + 1] * 0.587
				+ ptr[4 * i + 0] * 0.114);
		ptr[4 * i + 1] = grayScale;
		ptr[4 * i + 2] = grayScale;
		ptr[4 * i + 0] = grayScale;
	}

	int size = w * h;
	jintArray result = env->NewIntArray(size);
	env->SetIntArrayRegion(result, 0, size, cbuf);

	env->ReleaseIntArrayElements(buf, cbuf, 0);

	return result;
}

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_connectCamera(JNIEnv* env,
		jclass obj, jint device)
{

	id = device;


	if (capture.isOpened())
	{
		LOGE("camera is already opened!");
		return -2;
	}
	else
	{
		//打开一个默认的摄像头
		capture.open(-1);
		if (capture.isOpened())
		{
			LOGE("camera open success!");
			return 0;
		}
	}

//	LOGI("usb camera Java_luo_uvc_jni_ImageProc_connectCamera end ....\n");

	return -1;
}

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_releaseCamera(JNIEnv* env,
		jclass obj)
{
//	LOGI("usb camera Java_luo_uvc_jni_ImageProc_releaseCamera start ....\n");

	if (capture.isOpened())
	{
		capture.release();
		LOGE("camera release success!");
		return 0;
	}

//	LOGI("usb camera Java_luo_uvc_jni_ImageProc_releaseCamera end ....\n");

	return -1;
}

struct FrameInfoClass
{
	jfieldID width;
	jfieldID heigth;
	jfieldID imageSize;
	jfieldID pixels;
};

JNIEXPORT jobject JNICALL Java_luo_uvc_jni_ImageProc_getFrame(JNIEnv* _env,
		jclass obj)
{
//	LOGI("------------Java_luo_uvc_jni_ImageProc_getFrame\n");
	if (capture.isOpened())
	{
		LOGI("------------start capture frame\n");

		//取图片帧
		capture >> frame;
		if (frame.empty())
		{
			LOGE("capture frame empty!");
			return NULL;
		}

		//将图片写入视频文件
		if(writer.isOpened())
		{
			//录制视频用的frame不需要转格式
			writer.write(frame);
		}

		struct FrameInfoClass frameInfoClass;

		//内部类用$
		//luo/uvc/jni/ImageProc$FrameInfo
		jclass class2 = _env->FindClass("luo/uvc/jni/ImageProc$FrameInfo");

		frameInfoClass.width = _env->GetFieldID(class2, "mWidth", "I");

		frameInfoClass.heigth = _env->GetFieldID(class2, "mHeight", "I");

		frameInfoClass.imageSize = _env->GetFieldID(class2, "mImageSize", "I");

		frameInfoClass.pixels = _env->GetFieldID(class2, "mPixels", "[I");

		//
		jobject joFrame = _env->AllocObject(class2);

		LOGI("frame->cols = %d\n", frame.cols);
		LOGI("frame->rows = %d\n", frame.rows);
//		LOGI("frame->imageSize = %d\n", frame->imageSize);

		_env->SetIntField(joFrame, frameInfoClass.width, frame.cols);
		_env->SetIntField(joFrame, frameInfoClass.heigth, frame.rows);
//		_env->SetIntField(joFrame, frameInfoClass.imageSize, frame->imageSize);

		int size = frame.cols * frame.rows;
		//创建一个新的java数组(jarray)，但是jarray不是C数组类型，不能直接访问jarray
		jintArray jiArr = _env->NewIntArray(size);
		jint *ji;

#if 1   //可用		//RGB --> ARGB8888
		Mat frameARGB;
		cvtColor(frame, frameARGB, CV_RGB2RGBA);

		//JNI支持一系列的Get/Release<Type>ArrayElement 函数，允许本地代码获取一个指向基本C类型数组的元素的指针。
		ji = _env->GetIntArrayElements(jiArr, 0);

		memcpy((jbyte *) ji, frameARGB.data, 4 * size);

		_env->ReleaseIntArrayElements(jiArr, ji, 0); //可加，可不加

		_env->SetObjectField(joFrame, frameInfoClass.pixels, jiArr);


#else   //可用		//可以使用GetIntArrayRegion函数来把一个 int数组中的所有元素复制到一个C缓冲区中		//SetIntArrayRegion则是逆过程		_env->SetIntArrayRegion(jiArr, 0, 2, abc);
		_env->SetObjectField(joFrame, company_class.money, jiArr);

#endif

//		LOGI("Java_luo_uvc_jni_ImageProc_getFrame end\n");

		return joFrame;

	}

//	LOGI("=================Java_luo_uvc_jni_ImageProc_getFrame failed\n");

	return 0;
}

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_startRecord
  (JNIEnv *env, jclass, jstring jstr)
{
	if(writer.isOpened() == false)
	{
		 prefix = jstringToChar(env, jstr);
		 memset(fileName, 0, NAMELEN);
		 sprintf(fileName, "/storage/sdcard0/Movies/%s.avi", prefix);
		 LOGI("fileName: %s", fileName);
		 FREE(prefix);

		 writer.open(fileName, CV_FOURCC('F', 'L', 'V', '1')/*有效*/, 30, cv::Size(640, 480),true);
//		 writer.open(fileName, CV_FOURCC('M', 'J', 'P', 'G')/*有效*/, 30, cv::Size(640, 480),true);
//		 writer.open(fileName, CV_FOURCC('D', 'I', 'V', 'X')/*有效*/, 30, cv::Size(640, 480),true);
//		 writer.open(fileName, CV_FOURCC('X', 'V', 'I', 'D')/*有效*/, 30, cv::Size(640, 480),true);
		 if(writer.isOpened())
		 {
			 LOGE("writer open successful!");
			 return 0;
		 }
		 else
		 {
			 LOGE("writer open failed!");
			 return -2;
		 }

	}
	else
	{
		//实际上已经在录像
		return -1;
	}
}

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_stopRecord
  (JNIEnv *, jclass)
{
	if (writer.isOpened())
	{
		//如果正在录像，则停止录像
		writer.release();
		LOGI("%s end record!", fileName);

		return 0;
	}

	return -1;
}

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_getWidth(JNIEnv* env,
		jclass obj)
{
	return 0;
}

JNIEXPORT jint JNICALL Java_luo_uvc_jni_ImageProc_getHeight(JNIEnv* env,
		jclass obj)
{
	return 0;
}

//jstring to char*
char* jstringToChar(JNIEnv* env, jstring jstr)
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
/* end of extern */
