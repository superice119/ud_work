/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

 ##############################################*/
 
#ifndef	_BB_INTERNAL_H_
#define	_BB_INTERNAL_H_    1

#include <features.h>

#define full_version "0.0.0.1!"

#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/param.h> 

#define HOST "14.152.73.246"
#define PORT  8883

//#define HOST "0.0.0.0"
//#define PORT  1883
#define KEEP_ALIVE 60
#define MSG_MAX_SIZE  512

#ifndef setbit
#define NBBY            CHAR_BIT
#define setbit(a,i)     ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i)     ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define isset(a,i)      ((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define isclr(a,i)      (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)
#endif

struct saddr {    /* Used as argument to mqtt func*/
	int       port;       
	int       ssize;
	char 	  saddr[32]; //default
};

int mqtt_pub(void * buf, unsigned int len, struct saddr * addr);
//int mqtt_pub(void * buf, unsigned int len);

int mqtt_sub(void * buf, unsigned int len);

#endif /* _BB_INTERNAL_H_ */
