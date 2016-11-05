
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>           //for map_shared
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <net/if.h>      //for struct ifreq
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <termios.h>
#include <assert.h>

#include <stdint.h>

#include <linux/videodev2.h>
#include <dirent.h>



#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>

#ifndef		__COMMON_H_
#define		__COMMON_H_

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define _OK		(0)					//function return: successful
#define _ERR	(-1)				//function return: failed
typedef unsigned int uint32;
typedef int	int32;
typedef	unsigned char uint8;
typedef char		int8;

#define TRUE	1
#define FALSE	0

static const char *UTAG = "UVC_LUO";

#define DEBUG
#ifdef	DEBUG
#define LOGI(fmt, args...)  __android_log_print(ANDROID_LOG_INFO, UTAG, fmt, ##args)
#define LOGW(fmt, args...)  __android_log_print(ANDROID_LOG_WARN, UTAG, fmt, ##args)
#define LOGE(fmt, args...)  __android_log_print(ANDROID_LOG_ERROR, UTAG, fmt, ##args)
#else
#define LOGI(fmt, args...)	printf(fmt, ##args)
#define LOGW(fmt, args...)	printf(fmt, ##args)
#define LOGE(fmt, args...)	printf(fmt, ##args)
#endif

#define		ERROR(cond, err, str)	\
			if(cond)				\
			{						\
				LOGE(str);			\
				goto err;			\
			}

#define FREE(ptr)		        \
		{				        \
            if(NULL != ptr)     \
            {                   \
                free(ptr);	    \
    			ptr = NULL;	    \
			}                   \
		}

#define CLOSE(fd)			\
		{					\
            if(-1 != fd)    \
            {               \
                close(fd);  \
			    fd = -1;	\
            }               \
		}





#endif /* end of file */
