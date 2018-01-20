#ifndef __MAIN_H__
#define __MAIN_H__


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>

#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

#include <time.h>

extern char *optarg;
extern int optind, opterr, optopt;

#include "debug.h"

#include "ringbuffer.h"
#include "mqtt.h"
#include "parson.h"
#include "spi_bridge.h"

#include "rfid_test_mode.h"
#include "rfid.h"

//#include "rfid_list.h"

#define TIMEOUT_SECOND_MAX         1
#define TIMEOUT_MSEC_MAX           100

char help_info[] =    " [option] [para]\n"
						" read rfid data\n"
						" -r thread read rfid, then send in mqtt.\n"
						" -t rfid test mode.\n"
						"  NOTE -- value are in string\n";

struct thread_info {    /* Used as argument to thread func*/
	//pthread_t thread_id;        /* ID returned by pthread_create() */
	//int       thread_num;       /* Application-defined thread # */
	int       port_num;       /* Application-defined thread # */
	int 	  frame_size; //default max = 8k
	struct saddr 	  addr; //default max = 8k
	list_t * rfids;
};

#endif //__MAIN_H_

