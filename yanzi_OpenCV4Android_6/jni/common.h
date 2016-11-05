#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
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

#include <jni.h>
#include <android/log.h>

#ifndef		__COMMON_H_
#define		__COMMON_H_

#define _OK		(0)					//function return: successful
#define _ERR	(-1)				//function return: failed
typedef unsigned int uint32;
typedef int	int32;
typedef	unsigned char uint8;
typedef char		int8;

#define TRUE	1
#define FALSE	0

static const char *UTAG = "OPENCV_LUO";

#define DEBUG
#ifdef	DEBUG
#define LOGI(fmt, args...)  __android_log_print(ANDROID_LOG_INFO, UTAG, fmt, ##args)
#define LOGW(fmt, args...)  __android_log_print(ANDROID_LOG_WARN, UTAG, fmt, ##args)
#define LOGE(fmt, args...)  __android_log_print(ANDROID_LOG_ERROR, UTAG, fmt, ##args)
#else
#define LOGI(fmt, args...)
#define LOGW(fmt, args...)
#define LOGE(fmt, args...)
#endif

#define		ERROR(cond, err, str)	\
			if(cond)				\
			{						\
				perror(str);		\
				goto err;			\
			}

#define FREE(ptr)		\
		{				\
			free(ptr);	\
			ptr = NULL;	\
		}

#define CLOSE(fd)			\
		{					\
			close(fd);		\
			fd = -1;		\
		}

		
		


#endif /* end of file */
