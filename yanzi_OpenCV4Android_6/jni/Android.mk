LOCAL_PATH := $(call my-dir)  
include $(CLEAR_VARS)  

#博客案例：OpenCV4Android释疑:，透析Android以JNI调OpenCV的三种方式(让OpenCVManager永不困扰)
LIB_TYPE = STATIC
ifeq ($(LIB_TYPE), STATIC) 
#STATIC链接时，可以不额外携带其依赖库
#当JNI里面调用了cvCaptureFromCAM去打开AndroidNativeCamera，就需要OPENCV_CAMERA_MODULES:=on
#来增加libnative_camera_r4.4.0.so
	OPENCV_LIB_TYPE:=STATIC
	OPENCV_INSTALL_MODULES:=on
	OPENCV_CAMERA_MODULES:=off
else 
#SHARED链接时，必须携带其依赖库，否则运行报错
	OPENCV_LIB_TYPE:=SHARED
	OPENCV_INSTALL_MODULES:=on
	OPENCV_CAMERA_MODULES:=on
endif 


#原始openCV4Android SDK
#include ../../openCV_2410_sdk/native/jni/OpenCV.mk

#添加V4L2，重新编译后的openCV4Android SDK
include ../../openCV_2410_sdk_v4l2/native/jni/OpenCV.mk

$(warning "****************************************")
$(warning $(LOCAL_C_INCLUDES))

LOCAL_LDLIBS += -llog 
LOCAL_SHARED_LIBRARIES += \
						libandroid_runtime\
						liblog \
						libcutils \
						libnativehelper \
						libcore/include	

LOCAL_C_INCLUDES += $(LOCAL_PATH)
						
LOCAL_SRC_FILES  := ImageProc.cpp  
LOCAL_MODULE     := image_proc  
include $(BUILD_SHARED_LIBRARY) 