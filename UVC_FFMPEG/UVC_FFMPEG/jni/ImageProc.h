#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>

#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>
#include <linux/usbdevice_fs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define IMG_WIDTH 640
#define IMG_HEIGHT 480


int camerabase = -1;

jint Java_cc_along_uvcsimple_UVCJni_prepareCamera( JNIEnv* env,jobject thiz, jint videoid);
jint Java_cc_along_uvcsimple_UVCJni_prepareCameraWithBase( JNIEnv* env,jobject thiz, jint videoid, jint videobase);
void Java_cc_along_uvcsimple_UVCJni_processCamera( JNIEnv* env,jobject thiz);
void Java_cc_along_uvcsimple_UVCJni_stopCamera(JNIEnv* env,jobject thiz);

#ifdef __cplusplus
}
#endif

