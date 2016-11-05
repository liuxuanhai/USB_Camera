LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := avcodec-56-prebuilt
LOCAL_SRC_FILES := prebuilt/libavcodec-56.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avdevice-56-prebuilt
LOCAL_SRC_FILES := prebuilt/libavdevice-56.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avfilter-5-prebuilt
LOCAL_SRC_FILES := prebuilt/libavfilter-5.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat-56-prebuilt
LOCAL_SRC_FILES := prebuilt/libavformat-56.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE :=  avutil-54-prebuilt
LOCAL_SRC_FILES := prebuilt/libavutil-54.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE :=  avswresample-1-prebuilt
LOCAL_SRC_FILES := prebuilt/libswresample-1.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE :=  swscale-4-prebuilt
LOCAL_SRC_FILES := prebuilt/libswscale-3.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := ImageProc
LOCAL_SRC_FILES :=  record_camera.cpp ImageProc.cpp 

LOCAL_LDLIBS := -llog -ljnigraphics -lz -I.
LOCAL_SHARED_LIBRARIES :=   avcodec-56-prebuilt     \
                            avdevice-56-prebuilt    \
                            avfilter-5-prebuilt     \
                            avformat-56-prebuilt    \
                            avutil-54-prebuilt      \
                            swscale-4-prebuilt

include $(BUILD_SHARED_LIBRARY)