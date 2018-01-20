/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

 ##############################################*/
 
#ifndef	_SPI_BRIDGE_H_
#define	_SPI_BRIDGE_H_    1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/param.h> 
#include <string.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <linux/spi/spidev.h>

#include "ringbuffer.h"
#include "debug.h"

//#define PORT_MAX 1
#define PORT_MAX 8

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

//#define SPI_BRIDGE_MAX_DATA_LEN         (1024)
//#define SPI_BRIDGE_MAX_DATA_LEN         (4096)
#define SPI_BRIDGE_MAX_DATA_LEN         (8192)

//#define SPI_BRIDGE_ONE_FRAME_MAX_LEN    (255)
//#define SPI_BRIDGE_ONE_FRAME_MAX_LEN    (2048)
//#define SPI_BRIDGE_ONE_FRAME_MAX_LEN    (4096)
//#define SPI_BRIDGE_ONE_FRAME_MAX_LEN    (8192)
#define SPI_BRIDGE_ONE_FRAME_MAX_LEN    (8160)

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
#define SPI_BRIDGE_LEN_32_BYTES                     (32U)
#define SPI_BRIDGE_LEN_224_BYTES                    (224U)
#define SPI_BRIDGE_LEN_2048_BYTES                   (2048U)
#define SPI_BRIDGE_LEN_4064_BYTES                   (4064U)
#define SPI_BRIDGE_LEN_4096_BYTES                   (4096U)
#define SPI_BRIDGE_LEN_8192_BYTES                   (8192U)
#define SPI_BRIDGE_LEN_8160_BYTES                   (8160U)
	uint32_t len;
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

struct spi_bridge spi_bridge;

struct spi_bridge spi_bridge_group[16];

struct bridge_group  bridge_group;

inline bool _is_spi_bridge_status_head_ok(uint8_t status)
{
	return (status & SPI_BRIDGE_STATUS_HEAD_MASK) == SPI_BRIDGE_STATUS_OK;
}

inline bool _can_spi_bridge_status_read(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_7688_READ_FROM_STM32_E);
}

inline bool _can_spi_bridge_status_write(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_7688_WRITE_TO_STM32_F);
}

inline bool _is_spi_bridge_status_set_ok(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_SET_PARAMETER_ERR);
}

inline bool _is_spi_bridge_status_check_ok(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_CHECK_ERR);
}

inline bool _is_spi_bridge_status_ok(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_ERR_MASK);
}


inline uint8_t spi_bridge_calulate_check(unsigned char* data, uint32_t len)
{
	int i;
	uint8_t chk = 0;
	unsigned char* ptr = data;
	//for (i = (SPI_BRIDGE_LEN_224_BYTES); i > 0; i--) {
	for (i = (len); i > 0; i--) {
		chk ^= *ptr++;
	}
	return chk;
}

int open_spi_device(const char* dev);

int spi_bridge_init(int port_num, int frame_size);

bool is_valid_fd(int fd);

uint8_t spi_bridge_send_cmd(struct spi_bridge *p, const uint8_t cmd, bool retry);

uint8_t spi_bridge_read_one_frame(struct spi_bridge *p, void* data, bool retry);

ssize_t spi_bridge_read(struct spi_bridge *p, void* data, size_t len);

char* spi_bridge_fgets(char* str, int size, FILE* stream);

#if 0

uint8_t spi_bridge_write_one_frame(const void* data, bool retry);

ssize_t spi_bridge_write(const void* data, size_t len);
#endif

inline uint8_t spi_bridge_read_status(struct spi_bridge *p )
{
	return spi_bridge_send_cmd(p, SPI_BRIDGE_CMD_GET_STATUS, true);
}

#endif /* _SPI_BRIDGE_H_ */
