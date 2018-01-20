#ifndef __RFID_H__
#define __RFID_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>//LINUX
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/time.h>

#include "mqtt.h"
#include "rfid_list.h"
//#include "list.h"

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

#define TIMEOUT_SECOND_MAX         1
#define TIMEOUT_MSEC_MAX           100



/////////////////////////////////////////////////////////////////////////
//          客户端接口
/////////////////////////////////////////////////////////////////////////


#define INF_OK                      		0x00 //
#define INF_UNDEFIND                		0xFF //没有定义的错误
#define INF_CMD_UNDEF               		0xF0 //主命令未定义
#define INF_CMD_SUB_UNDEF           		0xF1 //子命令未定义

#define INF_CMD_PARAM_INVAL         		0x10 //输入无效的参数或参数越界
#define INF_CMD_CRC_ERR             		0x11 //读写器接收到的命令帧CRC错误
#define INF_CMD_RES_CRC_ERR         		0x12 //主机接收到读写器的应答帧CRC错误
#define INF_R_BUF_EMPTY             		0x13 //读写器缓存区没有数据
#define INF_R_ANT_FAIL              		0x14 //天线故障或天线未连接
#define INF_R_PLL_FAIL              		0x15 //PLL失锁
#define INF_R_UART_FAIL	            		0x16 //串口连接错误或连接超时
#define INF_R_SET_PARAM_FAIL        		0x17 //设置读写器参数错误
#define INF_R_GET_PARAM_FAIL        		0x18 //读取读写器参数错误
#define INF_R_WR_FLASH_FAIL         		0x19 //写或编程读写器FLASH错误

#define INF_TAG_NO_TAG              		0x20 //没有找到标签
#define INF_TAG_LOCKED              		0x21 //标签已经写保护
#define INF_TAG_CRC_ERR             		0x22 //标签数据CRC错
#define INF_TAG_WRITE_FAIL          		0x23 //写标签错误
#define INF_TAG_READ_FAIL           		0x24 //读标签错误
#define INF_TAG_ERASE_FAIL          		0x25 //擦除标签错误
#define INF_TAG_LOCK_FAIL           		0x26 //锁定标签错误
#define INF_TAG_KILL_FAIL           		0x27 //销毁标签错误
#define INF_TAG_VERIFY_FAIL         		0x28 //验证标签错误

#define INF_RS232_ERROR             		0x7F //RS-232 操作错误
#define INF_RS232_READ              		0x7E //RS-232 接收错误
#define INF_RS232_WRITE             		0x7D //RS-232 发送错误
#define INF_CRC_ERROR               		0x7C //RS-232 接收校验错误
#define INF_HOST_RXBUF_EMPTY			0x7B  //上位机软件缓冲区是空
#define INF_HOST_IN_PROGRESS			0x7A  //上位机软件未处理完成

#define INF_NET_ERROR               		0x8F //NET 错误
#define INF_NET_CLIENT_READ			0x8E //NET 接收错误
#define INF_NET_CLIENT_SEND			0x8D //NET 发送错误
#define INF_NET_CRC_ERROR			0x8C //NET 接收校验错误
#define INF_NET_TIMEOUT				0x8B  //NET 超时

/////////////////////////////////////////////
//标签操作中，返回的相关代码信息
#define _INF_TAG_NO_TAG				0x20   //没有读取到标签或无标签响应
#define _INF_TAG_LOCKED				0x21   //写标签的区域，已经锁定，不能再写入
#define _INF_TAG_CRC_ERR			0x22   //操作标签，标签数据CRC出错
#define _INF_TAG_WRITE_FAIL			0x23   //写标签失败
#define _INF_TAG_READ_FAIL			0x24   //读标签数据失败
#define _INF_TAG_LOCK_FAIL			0x26   //锁定标签失败
#define _INF_TAG_KILL_FAIL			0x27   //销毁标签失败
#define _INF_TAG_VERIFY_FAIL			0x28   //校验标签数据失败
#define _INF_TAG_OUTTIME			0x29   //操作标签超时

#define _INF_TAG_EPC_INSUFFICIENT_RFPWR     	0x30  //写标签的RF能量场不足，功率太小或距离太远
#define _INF_TAG_EPC_MEMORY_OVERRUN         	0x31  //操作的标签存储单元不存在或标签的控制字不支持
#define _INF_TAG_EPC_OTHER_ERROR            	0x32  //其它未定义的错误
#define _INF_TAG_EPC_HANDLE_ERROR           	0x33  //操作标签时候的句柄错误
#define _INF_TAG_EPC_TAG_BUSY               	0x34  //标签忙，不能响应当前操作

#define MAX_BUF_SIZE 655350

#define _OPERATION_SET_PARAM  0x00
#define _OPERATION_GET_PARAM  0x01


typedef short apiStatus; // API Universal Return Value

typedef struct
{
	unsigned char yTagID_Len;
	unsigned char yTagID_Data[48];
	unsigned char yTagID_AntNo;
}*pTagRECstruct, gTagRECstruct;

typedef struct
{
	unsigned int  Head_Pointer; //数据接收缓冲区头指针
	unsigned int  Tail_Pointer; //数据接收缓冲区尾指针
	unsigned int  BufCount;     //缓冲区标签个数计数器
	//unsigned char DataLen;     //一个数据存储的长度 >=1
	//unsigned int  MaxDataNum;  //缓冲区那个存储的最多数据的数目
	unsigned char Buffer[MAX_BUF_SIZE];
}*pBUFstruct, gBUFstruct;

typedef struct _RC4_KEY
{
   unsigned char bySTab[256];   //256字节的S表
   unsigned char byIt,byJt;     //t时刻的两个指针
}RC4_KEY,*PRC4_KEY;

struct data {
	unsigned char tag_data[128];
	unsigned char temperture[128];
	unsigned char moisture[8];
	unsigned char rssi[32];
	unsigned char ant_num;

};

int rfid_open(char * dev, void * handle, unsigned char *rfid_addr );

int rfid_close(int fd, void * handle, unsigned char rfid_addr);

int rfid_reset(int fd, void * handle, unsigned char rfid_addr);

int rfid_set_sys_RF_ant_auto_poll_on(int fd, void * handle);

int rfid_set_sys_RF_ant_auto_poll_off(int fd, void * handle);

int rfid_sys_RF_ant_poll_time_ctrl(int fd, void * handle, unsigned char *polling_time, unsigned char operation);

int rfid_RF_ant_poll_time_set(int fd, void * handle, unsigned char *polling_time);

int rfid_RF_ant_poll_time_get(int fd, void * handle, unsigned char *polling_time);

int rfid_set_sys_RF_ant_enable(int fd, void * handle, unsigned char AntSt[]);

int rfid_get_sys_status(int fd, void * handle, unsigned char rfid_addr);

int rfid_set_sys_RF_cur_work_ant_query(int fd, void * handle);

int rfid_set_sys_RF_ant_status_query(int fd, void * handle);

int rfid_set_sys_cur_ant(int fd, void * handle, unsigned char rfid_addr);

//int rfid_read_temperature_tag_data(int fd, void * handle, unsigned char rfid_addr, struct data *pd );

int rfid_read_temperature_tag_data(int fd, void * handle, unsigned char rfid_addr, list_t *plist );

//int ReadSingleTag(int fd, void * handle, unsigned char rfid_addr, int FlagTagInventory, struct data *pd);

int ReadSingleTag(int fd, void * handle, unsigned char rfid_addr, int FlagTagInventory, list_t *plist);

//int ReadMultiTag(int fd, void * handle, unsigned char rfid_addr, int FlagTagInventory, struct data *pd);
int ReadMultiTag(int fd, void * handle, unsigned char rfid_addr, int FlagTagInventory, list_t *plist);

#endif //__RFID_H_

