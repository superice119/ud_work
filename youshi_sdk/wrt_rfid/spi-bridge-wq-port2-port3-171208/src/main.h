#ifndef __MAIN_H__
#define __MAIN_H__

#include "ringbuffer.h"
#include "mqtt.h"
#include "parson.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <linux/spi/spidev.h>

#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

#include <time.h>

extern char *optarg;
extern int optind, opterr, optopt;



#define SPI_SUCCESS     0
#define SPI_ERROR       -1
#define BURST_CHUNK     1024


//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT   printf
#else
#define DEBUG_PRINT(...)
#endif

#ifdef DEBUG
    #define DEBUG_MSG(str)                fprintf(stderr, str)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stderr,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return SPI_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                if(a==NULL){return SPI_ERROR;}
#endif

//for mqtt
//#define HOST "14.152.73.246"
//#define PORT  8883

//#define HOST "0.0.0.0"
//#define PORT  1883
//#define KEEP_ALIVE 60
#define MSG_MAX_SIZE  512

#define PORT_MAX 2

#define TIMEOUT_SECOND_MAX         1
#define TIMEOUT_MSEC_MAX           100

#define SPI_7688 

#ifdef SPI_7688

#define SPI_DEVICE_PRE "/dev/spidev0."

//#define SPI_DEVICE     "/dev/spidev0.1"
#define SPI_DEVICE     "/dev/spidev0.2"
//#define SPI_DEVICE     "/dev/spidev0.3"

#else

#define SPI_DEVICE     "/dev/spidev1.0"

#endif

#define SPI_MODE       SPI_MODE_1
#define SPI_WORD_LEN   8
//#define SPI_HZ         1000000
#define SPI_HZ         6000000

/************  SPI Bridge  **************/
//#define SPI_BRIDGE_MAX_DATA_LEN         (32)

#define SPI_BRIDGE_MAX_DATA_LEN         (1024)

#define SPI_BRIDGE_ONE_FRAME_MAX_LEN    (255)
#define SPI_BRIDGE_READ_RETRY_TIMES     (5)
#define SPI_BRIDGE_WRITE_RETRY_TIMES    (5)
//#define SPI_BRIDGE_RESP_RETRY_TIMES     (200)
#define SPI_BRIDGE_RESP_RETRY_TIMES     (50)

struct spi_bridge {
	int fd;
	//int fd[16];

	/* for console */
	pthread_t read_mcu_tidp;
	pthread_t read_stdin_tidp;
	pthread_mutex_t spi_mutex;

	pthread_t mqtt_tidp;

	pthread_mutex_t buf_mutex;

	uint8_t read_buf_pool[SPI_BRIDGE_MAX_DATA_LEN];
	struct rt_ringbuffer read_buf;

	uint8_t write_buf_pool[SPI_BRIDGE_MAX_DATA_LEN];
	struct rt_ringbuffer write_buf;
	

#define SPI_BRIDGE_STATUS_7688_READ_FROM_STM32_E    (1<<0)
#define SPI_BRIDGE_STATUS_7688_READ_FROM_STM32_NE   (0<<0)
#define SPI_BRIDGE_STATUS_7688_WRITE_TO_STM32_F     (1<<1)
#define SPI_BRIDGE_STATUS_7688_WRITE_TO_STM32_NF    (0<<1)
#define SPI_BRIDGE_STATUS_SET_PARAMETER_ERR         (1<<2)
#define SPI_BRIDGE_STATUS_SET_PARAMETER_OK          (0<<2)
#define SPI_BRIDGE_STATUS_CHECK_ERR                 (1<<3)
#define SPI_BRIDGE_STATUS_CHECK_OK                  (0<<3)
#define SPI_BRIDGE_STATUS_OK                        (0x50)
#define SPI_BRIDGE_STATUS_HEAD_MASK                 (0xf0)
#define SPI_BRIDGE_STATUS_ERR_MASK                  (0x0f)
#define SPI_BRIDGE_STATUS_NULL                      (0x00)
	uint8_t status;

#define SPI_BRIDGE_CMD_GET_STATUS                   (1U)
#define SPI_BRIDGE_CMD_7688_READ_FROM_STM32         (10U)
#define SPI_BRIDGE_CMD_7688_WRITE_TO_STM32          (20U)
#define SPI_BRIDGE_CMD_SET_BLOCK_LEN                (30U)
#define SPI_BRIDGE_CMD_NULL                         (0U)
	uint8_t cmd;

	/* block length */
#define SPI_BRIDGE_LEN_8_BYTES                      (8U)
#define SPI_BRIDGE_LEN_16_BYTES                     (16U)
#define SPI_BRIDGE_LEN_224_BYTES                     (224U)
#define SPI_BRIDGE_LEN_32_BYTES                     (32U)
	uint8_t len;
	/* in block length count */
	//uint8_t count;
	uint16_t count;
	uint8_t* xfet_buf;
};

struct bridge_group {

	pthread_mutex_t group_mutex;
	pthread_mutex_t buf_mutex;

	pthread_t read_mcu_tidp;
	pthread_t read_stdin_tidp;

	pthread_t mqtt_tidp;

	struct spi_bridge bridge[PORT_MAX];
	
};

#define RT2880_SPI_READ_STR    "read"
#define RT2880_SPI_WRITE_STR   "write"
#define RT2880_SPI_STATUS_STR  "status"

#define RT2880_SPI_READ     (2)
#define RT2880_SPI_STATUS   (2)
#define RT2880_SPI_WRITE    (3)

//#define SPI_MCU_READ_DELAY_US          (100000U)
#define SPI_MCU_READ_DELAY_US          (5000U)
#define SPI_MCU_WRITE_DELAY_US         (200U)
//#define SPI_MCU_RESP_DELAY_US          (100U)
//#define SPI_MCU_RESP_DELAY_US          (50000U)
#define SPI_MCU_RESP_DELAY_US          (500U)
#define SPI_MCU_CHECK_STATUS_DELAY_US  (50000U)
#define SPI_MCU_CS_HIGH_DELAY_US       (500U)
//#define SPI_MCU_CHECK_STATUS_DELAY_US  (100000U)
//#define SPI_MCU_CHECK_STATUS_DELAY_US  (10000U)


#endif //__MAIN_H_

