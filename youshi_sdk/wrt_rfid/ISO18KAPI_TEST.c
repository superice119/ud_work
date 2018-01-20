/*
 ============================================================================
 Name        : ISO18KAPI_TEST.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>//LINUX
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/time.h>


/*

gcc -c -O ISO18KAPI_TEST.c
gcc ISO18KAPI_TEST.o -ldl -lm -lc -o ISO18KAPI_TEST
./ISO18KAPI_TEST


/usr/WRTnoteTOOLS/bin/mipsel-openwrt-linux-gcc -c -O ISO18KAPI_TEST.c
/usr/WRTnoteTOOLS/bin/mipsel-openwrt-linux-gcc ISO18KAPI_TEST.o -ldl -o ISO18KAPI_TEST
./ISO18KAPI_TEST

/usr/WRTnoteTOOLS/bin/mipsel-openwrt-linux-gcc -ldl -o ISO18KAPI_TEST -c -O ISO18KAPI_TEST.c

*/

typedef short apiStatus; // API Universal Return Value

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




int iComportHandle;  //device Number
static unsigned char ISO18K_RAddr=0;    //device Address
static unsigned char Broadcast_RAddr=0xFF;   //device Address
void *handle;

gBUFstruct gTagReadQue;

int menu();
void Open_Comm();
void Close_Comm();
void ReaderReset();
void ReadSingleTag();
void ReadMultiTag();
void ReadTagData();
void WriteTagData();
//-------------------------------------------------------------------------------------------

void Open_Comm()
{
    char devicename[50]="/dev/ttyS0";
    unsigned char rStatus[10]={0};
    unsigned char info[64]={0};
    int Reback;
    int n;
    short rc;
    unsigned char HWMajorVer;
    unsigned char HWMinorVer;
    unsigned char SWMajorVer;
    unsigned char SWMinorVer;
    unsigned char BEEP_OPERATION_GET_PARAM1=1,BEEP_OPERATION_GET_PARAM2=0;

    //打开读写器连接的端口
    short (*RFID_Open)(int *,char *,unsigned int);
    if((RFID_Open = dlsym(handle,"RFID_Open")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

    //查询读写器状态
    short (*RFID_SysGetStatus)(int, unsigned char, unsigned char *);
    if((RFID_SysGetStatus = dlsym(handle,"RFID_SysGetStatus")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

    //查询读写器系列号
    short (*RFID_SysGetSN)(int ,unsigned char, unsigned char, unsigned char* );
    if((RFID_SysGetSN = dlsym(handle,"RFID_SysGetSN")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

    //查询读写器地址
    short (*RFID_ReadAddrQuery)(int ,unsigned char, unsigned char *);
    if((RFID_ReadAddrQuery = dlsym(handle,"RFID_ReadAddrQuery")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

    //查询读写器版本号
    short (*RFID_SysGetFirmwareVersion)(int ,unsigned char,unsigned char* ,unsigned char* ,unsigned char* ,unsigned char*);
    if((RFID_SysGetFirmwareVersion = dlsym(handle,"RFID_SysGetFirmwareVersion")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

    //系统蜂鸣器
    short (*RFID_SysBeepCtrl)(int ,unsigned char,unsigned char ,unsigned char*);
    if((RFID_SysBeepCtrl = dlsym(handle,"RFID_SysBeepCtrl")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }


    printf("Sample of the device name  \n");
    printf("format of COM1 PORT is /dev/ttyS0  etc.\n");
    printf("format of USB PORT 1 is /dev/ttyUSB0  etc.\n");
    printf("Please input the device name(/dev/ttyS0):\n");
    Reback=scanf("%s",devicename);
    //打开读写器连接的端口
    rc=RFID_Open(&iComportHandle,devicename,115200);
    if (rc==INF_OK && iComportHandle>0)
    {
        printf("RFID_Open OK\n");
        printf("Device adrress is %d\n",iComportHandle);

        //查询读写器状态
    	rc=RFID_SysGetStatus(iComportHandle,Broadcast_RAddr,rStatus);
    	//rStatus[0];//当前频率
    	//rStatus[1];//当前功率值
    	//rStatus[2];//当前天线号
    	//rStatus[3];// 4~7跳频表,  PLL状态, 跳频状态, 天线自动循环的状态,功放开关状态
    	//rStatus[4];//天线连接状态
    	//rStatus[5];//工作模式
    	//rStatus[6];//天线检测使能   2009-07-28
    	//rStatus[7];//  跳频周期，
    	if (rc==INF_OK)
    	{
            printf("RFID_SysGetStatus OK\n");
            printf("rStatus[0]=%02X\n",rStatus[0]);
            printf("rStatus[1]=%02X\n",rStatus[1]);
            printf("rStatus[2]=%02X\n",rStatus[2]);
            printf("rStatus[3]=%02X\n",rStatus[3]);
            printf("rStatus[4]=%02X\n",rStatus[4]);
            printf("rStatus[5]=%02X\n",rStatus[5]);
            printf("rStatus[6]=%02X\n",rStatus[6]);
            printf("rStatus[7]=%02X\n",rStatus[7]);

        	rc=RFID_ReadAddrQuery(iComportHandle,Broadcast_RAddr,&ISO18K_RAddr);
        	if (rc==INF_OK)
        	{
                printf("RFID_ReadAddrQuery OK\n");
                printf("Reader Device Address: %d\n",ISO18K_RAddr);
                /*
				////查询读写器系列号
				rc=RFID_SysGetSN(iComportHandle,ISO18K_RAddr,2,(unsigned char* )info);
				if (rc==INF_OK)
				{
					printf("RFID_SysGetSN OK\n");
					//printf("Reader SN: %s",info);
					printf("Reader SN: ");
					for (n=0;n<64;n++)
					{
						 printf("%c",info[n]");
					}
					printf("\n");
				}
				else
				{
					printf("RFID_SysBeepCtrl Error\n");
				}
				*/
				//查询读写器版本号
				rc=RFID_SysGetFirmwareVersion(iComportHandle,ISO18K_RAddr,&HWMajorVer,&HWMinorVer,&SWMajorVer,&SWMinorVer);
				if (rc==INF_OK)
				{
					printf("RFID_SysGetFirmwareVersion OK\n");
					printf("Reader Firmware Version: V%02d.%02d.%02d.%02d\n",HWMajorVer,HWMinorVer,SWMajorVer,SWMinorVer);

					//系统蜂鸣器
					rc=RFID_SysBeepCtrl(iComportHandle,ISO18K_RAddr,BEEP_OPERATION_GET_PARAM1,&BEEP_OPERATION_GET_PARAM2);
					if (rc==INF_OK)
					{
						printf("RFID_SysBeepCtrl OK\n");
						printf("BEEP_OPERATION_GET_PARAM2: %02d\n\n\n",BEEP_OPERATION_GET_PARAM2);
					}
					else
					{
						printf("RFID_SysBeepCtrl Error\n\n\n");
					}
				}
				else
				{
					printf("RFID_SysGetFirmwareVersion Error\n\n\n");
				}
        	}
        	else
        	{
        		printf("RFID_ReadAddrQuery Error\n\n\n");
        	}
    	}
    	else
    	{
            printf("RFID_SysGetStatus Error\n\n\n");
    	}
    }
    else
    {
        printf("RFID_Open error\n\n\n");
    }
}



void Close_Comm()
{
    short rc;
    short (*RFID_Close)(int pClientHandle,unsigned char Addr);
    if((RFID_Close = dlsym(handle,"RFID_Close")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }
    rc=RFID_Close(iComportHandle,ISO18K_RAddr);
	if (rc==INF_OK)
	{
        printf("RFID_Close OK\n\n\n");
        iComportHandle=0;
	}
	else
	{
        printf("RFID_Close Error\n\n\n");
 	}
}
//================================================================================================

void ReaderReset()   //Reader Reset
{
    short rc;

    //读写器复位
    short (*RFID_SysReset)(int ,unsigned char);
    if((RFID_SysReset = dlsym(handle,"RFID_SysReset")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

	//读写器复位
	rc=RFID_SysReset(iComportHandle,ISO18K_RAddr);
	if (rc==INF_OK)
	{
        printf("RFID_SysReset OK\n");
        printf("Reader Device Address: %d\n\n\n",ISO18K_RAddr);
	}
	else
	{
		printf("RFID_SysReset Error\n\n\n");
	}
}

void ReadSingleTag()
{
    unsigned char WorkMode;
    unsigned char MemBank=0x00;

    unsigned char gQV=0x06;
    unsigned char gEPC_MemBank=MemBank+1;
    unsigned char ReadOffsetAddr=0x02;
    unsigned char ReadWordLen=0x06;
    unsigned char MaskLength=0x00;
    unsigned char Mask[8]={0};
    unsigned char RecvTagNum=0x00;
    unsigned char RecvRecLen[6]={0};
    unsigned char TagRecord[128]={0};

    int ii=0;
    short rc;
    char temp[64]={0},tp[4]={0};
    unsigned char k,n;
    unsigned char AntNo;
    unsigned char uOffset=0;

    short (*RFID_SysWorkModeQuery)(int ,unsigned char, unsigned char *);
    if((RFID_SysWorkModeQuery = dlsym(handle,"RFID_SysWorkModeQuery")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

    short (*RFID_REC_ClrAll)(int ,unsigned char);
    if((RFID_REC_ClrAll = dlsym(handle,"RFID_REC_ClrAll")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

    short (*RFID_Gen2_STR)(int , unsigned char ,unsigned char , unsigned char ,unsigned char ,unsigned char , unsigned char , unsigned char [], unsigned char* , unsigned char* , unsigned char []);
    if((RFID_Gen2_STR = dlsym(handle,"RFID_Gen2_STR")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

    short (*RFID_SysRFPwrOFF)(int ,unsigned char);
    if((RFID_SysRFPwrOFF = dlsym(handle,"RFID_SysRFPwrOFF")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

    short (*RFID_Gen2_EndOperation)(int ,unsigned char);
    if((RFID_Gen2_EndOperation = dlsym(handle,"RFID_Gen2_EndOperation")) == NULL)
    {
        fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
        exit(0);
    }

	//START WORKFLOW
	rc=RFID_SysWorkModeQuery(iComportHandle,ISO18K_RAddr,&WorkMode);
	if (rc==INF_OK)
	{
		printf("RFID_SysWorkModeQuery OK\n");
		printf("Reader Work Mode: %02d\n",WorkMode);
		if(WorkMode!=0)
		{
			printf("The Reader is not work on Command Mode!\r\nPlease set it to Command Mode!\r\n");
			return;
		}

		rc=RFID_REC_ClrAll(iComportHandle,ISO18K_RAddr);
		if (rc==INF_OK)
		{
			printf("RFID_REC_ClrAll OK\n");
			for(ii=0;ii<30;ii++)
			{
				gQV=0x06;
				MemBank=0x00;
				gEPC_MemBank=MemBank+1;
				ReadOffsetAddr=0x02;
				ReadWordLen=0x06;
				MaskLength=0x00;
				memset(Mask,0x00,8);
				RecvTagNum=0x00;
				memset(RecvRecLen,0x00,6);
				memset(TagRecord,0x00,128);
				uOffset=0;

				rc=RFID_Gen2_STR(iComportHandle,ISO18K_RAddr,gQV,gEPC_MemBank,ReadOffsetAddr,ReadWordLen,MaskLength,Mask,&RecvTagNum,RecvRecLen,TagRecord);
				if (rc==INF_OK)
				{
					ReadWordLen = ReadWordLen*2;
					if(gEPC_MemBank>=4)
					{
						ReadWordLen=ReadWordLen+12;
					}
					//printf("RecvTagNum=%d \n",RecvTagNum);

					for(n=0;n<RecvTagNum;n++)  // 标签数量RecvTagNum=1, 单标签读固定为1
					{
						//printf("RecvRecLen[%d]=%02X\n",n,RecvRecLen[n]);

						strcpy(temp,"");//memset(temp,0,64);
						memset(tp,0x00,4);

						for(k=0;k<RecvRecLen[n]-1;k++)
						{
							sprintf(tp,"%02X",TagRecord[k+uOffset]);
							strcat(temp,tp);
						}

						AntNo=TagRecord[uOffset+(RecvRecLen[n]-1)];

						//printf("RFID_Gen2_STR OK\n");
						printf("TagRecord[%s] AntNo[%d]:\n", temp,AntNo);

						uOffset=uOffset+RecvRecLen[n];//指向下一个记录
					}
				}
				else
				{
					//printf("RFID_Gen2_STR Error\n");
				}
				usleep(2000);

			}
			printf("Run RFID_Gen2_STR Finish\n");
			rc=RFID_SysRFPwrOFF(iComportHandle,ISO18K_RAddr);
			rc=RFID_Gen2_EndOperation(iComportHandle,ISO18K_RAddr);
			printf("RFID_SysRFPwrOFF \n\n\n");
		}
		else
		{
			printf("RFID_SysBeepCtrl Error\n\n\n");
		}
	}
	else
	{
		printf("RFID_SysWorkModeQuery Error\n\n\n");
	}
}

#define InventoryTag                0
#define InventoryTemperatureTag     1
#define InventoryTemperatureTagRSSI 2
#define InventoryMoistureTags       3
#define InventoryMoistureTagsRSSI   4
#define InventoryOrdinaryTagsRSSI   5

#define GEN2_BANK_RESERVE			0x00	//GEN2 标签系统保留数据区
#define GEN2_BANK_EPC				0x01	//GEN2 标签EPC数据区
#define GEN2_BANK_TID				0x02	//GEN2 标签TID数据区
#define GEN2_BANK_USER				0x03	//GEN2 标签用户数据区

void ReadMultiTag()
{

	int TagIndex;
	int ii=0;
	unsigned int j=0;
	unsigned char TagQuantity_GetFromReaderBuf;
	gTagRECstruct  TagRec[20];
	apiStatus sst;
	int Reback;
	short rc;
	unsigned char WorkMode;

	unsigned char temp[128]={0}, strtemp[128]={0},tempedata[128]={0};

	char RssiData[128];
	unsigned char TagCounter=0;
	unsigned char TagBuffer[32], TagBufLen=0;
	unsigned char TempeBuff[20],TempeLen=0;
	char MoistureData[10];

	unsigned char AntNo;
	unsigned char TagCount=0;
	short TempeValue;
	unsigned char Value;

	unsigned char gQV=0x06;
	unsigned char MemBank=0x00;
	unsigned char ReadOffsetAddr=0x02;
	unsigned char ReadWordLen=0x06;
	unsigned char Mask[8];
	unsigned char RecvTagNum=0x00, MaskLength=0x00, RecvRecLen=0x00;
	int FlagTagInventory=0;
	//
	short (*RFID_SysWorkModeQuery)(int ,unsigned char, unsigned char *);
	if((RFID_SysWorkModeQuery = dlsym(handle,"RFID_SysWorkModeQuery")) == NULL)
	{
	fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
	exit(0);
	}

	//
	short (*RFID_REC_ClrAll)(int ,unsigned char);
	if((RFID_REC_ClrAll = dlsym(handle,"RFID_REC_ClrAll")) == NULL)
	{
	fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
	exit(0);
	}

	short (*RFID_Gen2_MTR)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_MTR = dlsym(handle,"RFID_Gen2_MTR")) == NULL)
	{
	fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
	exit(0);
	}

	short (*RFID_Gen2_OrdinaryTagsRSSI)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_OrdinaryTagsRSSI = dlsym(handle,"RFID_Gen2_OrdinaryTagsRSSI")) == NULL)
	{
	fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
	exit(0);
	}

	short (*RFID_Gen2_TempeTagRead)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_TempeTagRead = dlsym(handle,"RFID_Gen2_TempeTagRead")) == NULL)
	{
	fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
	exit(0);
	}

	short (*RFID_Gen2_TempeTagRSSIRead)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_TempeTagRSSIRead = dlsym(handle,"RFID_Gen2_TempeTagRSSIRead")) == NULL)
	{
	fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
	exit(0);
	}

	short (*RFID_Gen2_MoistureTagRead)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_MoistureTagRead = dlsym(handle,"RFID_Gen2_MoistureTagRead")) == NULL)
	{
	fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
	exit(0);
	}

	short (*RFID_Gen2_MoistureTagRSSIRead)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_MoistureTagRSSIRead = dlsym(handle,"RFID_Gen2_MoistureTagRSSIRead")) == NULL)
	{
	fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
	exit(0);
	}


	/*
	void WINAPI InitQueue(pBUFstruct pBuf);
	void WINAPI SetUartTimeOut(int hdl,int TimeOut);
	short WINAPI RFID_REC_GetTagDataEx(int hdl, UCHAR RAddr, UCHAR InTagCount, UCHAR *FinalGetTagCount, pTagRECstruct pTagRec, bool bNeedClr);//2011-12-15
	void WINAPI Write2Queue(pBUFstruct pBuf,unsigned char *uData,unsigned int Len);
	void WINAPI Read4Queue(pBUFstruct pBuf,unsigned char *uData,unsigned int Len);
	*/
	void (*InitQueue)(pBUFstruct pBuf);
	if((InitQueue = dlsym(handle,"InitQueue")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	short (*Write2Queue)(pBUFstruct pBuf,unsigned char *uData,unsigned int Len);
	if((Write2Queue = dlsym(handle,"Write2Queue")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	short (*Read4Queue)(pBUFstruct pBuf,unsigned char *uData,unsigned int Len);
	if((Read4Queue = dlsym(handle,"Read4Queue")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}


	short (*RFID_REC_GetTagDataEx)(int hdl, unsigned char RAddr, unsigned char InTagCount, unsigned char *FinalGetTagCount, pTagRECstruct pTagRec, unsigned char bNeedClr);
	if((RFID_REC_GetTagDataEx = dlsym(handle,"RFID_REC_GetTagDataEx")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	//
	short (*RFID_SysRFPwrOFF)(int ,unsigned char);
	if((RFID_SysRFPwrOFF = dlsym(handle,"RFID_SysRFPwrOFF")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	short (*RFID_Gen2_EndOperation)(int ,unsigned char);
	if((RFID_Gen2_EndOperation = dlsym(handle,"RFID_Gen2_EndOperation")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	//START WORKFLOW
/*
	rc=RFID_SysWorkModeQuery(iComportHandle,ISO18K_RAddr,&WorkMode);
	printf("Reader Work Mode: %02d\n",WorkMode);

	if(WorkMode!=0)
	{
		printf("The Reader is not work on Command Mode!\r\nPlease set it to Command Mode!");
		return;
	}
*/
	InitQueue(&gTagReadQue);
	printf("InitQueue OK\n");
	if (rc==INF_OK)
	{
		printf("InitQueue OK\n");
		//
		rc=RFID_REC_ClrAll(iComportHandle,ISO18K_RAddr);
		if (rc==INF_OK)
		{
			printf("RFID_REC_ClrAll OK\n");
			RecvTagNum=0x00;
			MaskLength=0x00;
			gQV=0x06;
		    MemBank=0x00;
			ReadOffsetAddr=0x02;
			ReadWordLen=0x06;
			if(MemBank==0)	MemBank =GEN2_BANK_EPC;

		    if(FlagTagInventory==InventoryTag)
			{
		      rc=RFID_Gen2_MTR(iComportHandle,ISO18K_RAddr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if (FlagTagInventory==InventoryOrdinaryTagsRSSI)
			{
				rc=RFID_Gen2_OrdinaryTagsRSSI(iComportHandle,ISO18K_RAddr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if(FlagTagInventory==InventoryTemperatureTag)
			{
			  rc=RFID_Gen2_TempeTagRead(iComportHandle,ISO18K_RAddr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if (FlagTagInventory==InventoryTemperatureTagRSSI)
			{
				rc=RFID_Gen2_TempeTagRSSIRead(iComportHandle,ISO18K_RAddr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if (FlagTagInventory==InventoryMoistureTags)
			{
				rc=RFID_Gen2_MoistureTagRead(iComportHandle,ISO18K_RAddr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if (FlagTagInventory==InventoryMoistureTagsRSSI)
			{
				rc=RFID_Gen2_MoistureTagRSSIRead(iComportHandle,ISO18K_RAddr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			if (rc==INF_OK)
			{
				printf("Start Multi-Tag Read (MTR)\n");
				InitQueue(&gTagReadQue);
				//printf("InitQueue OK\n");

				for(ii=0;ii<30;ii++)//while(1)
				{
					sst=RFID_REC_GetTagDataEx(iComportHandle,ISO18K_RAddr,16, &TagQuantity_GetFromReaderBuf,TagRec,1);//true
					//printf("RFID_REC_GetTagDataEx OK(%02X)\n",sst);
					if(sst == INF_OK &&(TagQuantity_GetFromReaderBuf>0))
					{
						//printf("TagQuantity_GetFromReaderBuf>0\n");
						for(TagIndex=0;TagIndex<TagQuantity_GetFromReaderBuf;TagIndex++)
						{
							//由于是多任务的系统，为了不丢失数据，首先存储到缓冲区中
							Write2Queue(&gTagReadQue,  &TagRec[TagIndex].yTagID_Len, 1);//数据长度入列
							Write2Queue(&gTagReadQue,  TagRec[TagIndex].yTagID_Data, TagRec[TagIndex].yTagID_Len);//数据入列
							Write2Queue(&gTagReadQue,  &TagRec[TagIndex].yTagID_AntNo, 1);//天线号入列
						}
						//printf("Write2Queue OK\n");

						{
							Read4Queue(&gTagReadQue, &TagBufLen, 1);//read the lenth

							strcpy(temp,"");//
							strcpy(tempedata,"");//
							strcpy(RssiData,"");//
							strcpy(MoistureData,"");

							if(FlagTagInventory==InventoryTag)//
							{
								Read4Queue(&gTagReadQue, TagBuffer, TagBufLen);//读出数据
								Read4Queue(&gTagReadQue, &AntNo, 1);//天线号
							}
							else if(FlagTagInventory==InventoryTemperatureTag)//
							{
								Read4Queue(&gTagReadQue, TagBuffer, TagBufLen-2);//读出EPC数据
								Read4Queue(&gTagReadQue, TempeBuff,2 );
								Read4Queue(&gTagReadQue, &AntNo, 1);//天线号

								TagBufLen=TagBufLen-2;

								TempeValue=TempeBuff[0];
								TempeValue=TempeValue<<8;
								TempeValue=TempeValue|TempeBuff[1];

								if((TempeValue<2000)&&(TempeValue>=0))
								{
									Value=TempeValue%10;
									TempeValue=TempeValue/10;
									sprintf(temp,"%d.%d℃",TempeValue,Value);
									strcat(tempedata,temp);
								}
								else if(TempeValue<0)
								{
									sprintf(temp,"%d.%d℃",TempeValue/10,-TempeValue%10);
									strcat(tempedata,temp);
								}

							}
							else if(FlagTagInventory==InventoryTemperatureTagRSSI)//
							{
								Read4Queue(&gTagReadQue, TagBuffer, TagBufLen-1);//读出EPC数据
								Read4Queue(&gTagReadQue, TempeBuff,1 );
								Read4Queue(&gTagReadQue, &AntNo, 1);//天线号

								TagBufLen=TagBufLen-1;
								if ((TempeBuff[0]>0)&&(TempeBuff[0]<32))
								{
									sprintf(temp,"%d",TempeBuff[0]);
									strcat(RssiData,temp);
								}
								else
								{
									//continue;
								}

							}

							else if(FlagTagInventory==InventoryMoistureTags)//湿度标签读取
							{
								Read4Queue(&gTagReadQue, TagBuffer, TagBufLen-1);//读出EPC数据
								Read4Queue(&gTagReadQue, TempeBuff,1 );
								Read4Queue(&gTagReadQue, &AntNo, 1);//天线号

								TagBufLen=TagBufLen-1;

								sprintf(temp,"%d",TempeBuff[0]);
								strcat(MoistureData,temp);

							}
							else if(FlagTagInventory==InventoryMoistureTagsRSSI)//
							{
								Read4Queue(&gTagReadQue, TagBuffer, TagBufLen-1);//读出EPC数据
								Read4Queue(&gTagReadQue, TempeBuff,1 );
								Read4Queue(&gTagReadQue, &AntNo, 1);//天线号

								TagBufLen=TagBufLen-1;
								if ((TempeBuff[0]>0)&&(TempeBuff[0]<32))
								{
									sprintf(temp,"%d",TempeBuff[0]);
									strcat(RssiData,temp);
								}
								else
								{
									//continue;
								}

							}
							else if (FlagTagInventory==InventoryOrdinaryTagsRSSI)
							{
								//Read4Queue(&gTagReadQue, TagBuffer, TagBufLen);//读出数据
								//Read4Queue(&gTagReadQue, &AntNo, 1);//天线号

								Read4Queue(&gTagReadQue, TagBuffer, TagBufLen-2);//读出EPC数据
								Read4Queue(&gTagReadQue, TempeBuff,2 );
								Read4Queue(&gTagReadQue, &AntNo, 1);//天线号

								TagBufLen=TagBufLen-2;

								TempeValue=TempeBuff[0];
								TempeValue=TempeValue<<8;
								TempeValue=TempeValue|TempeBuff[1];
								TempeValue=65536-TempeValue;

								sprintf(temp,"%d.%d",TempeValue/10,TempeValue%10);
								strcat(RssiData,temp);
							}

							strcpy(strtemp,"");
							if(TagBufLen<=8)
							{
								for(j=0;j<TagBufLen;j++)
								{
									sprintf(temp,"%02X ", TagBuffer[j]);
									strcat(strtemp,temp);
								}
							}
							else
							{
								for(j=0;j<TagBufLen;j++)
								{
									sprintf(temp,"%02X",TagBuffer[j]);
									strcat(strtemp,temp);
								}
							}
							//ProcessTag(strtemp,tempedata,MoistureData,RssiData,AntNo);
							if(FlagTagInventory==InventoryTag)//
							{
								printf("TagData[%s] AntNo[%d]\n",strtemp,AntNo);
							}
							else if(FlagTagInventory==InventoryTemperatureTag)//
							{
								printf("TagTemperatureData[%s] AntNo[%d]\n",tempedata,AntNo);
							}
							else if(FlagTagInventory==InventoryTemperatureTagRSSI)//
							{
								printf("RssiData[%s] AntNo[%d]\n",RssiData,AntNo);
							}
							else if(FlagTagInventory==InventoryMoistureTags)//湿度标签读取
							{
								printf("MoistureData[%s] AntNo[%d]\n",MoistureData,AntNo);
							}
							else if(FlagTagInventory==InventoryMoistureTagsRSSI)//
							{
								printf("RssiData[%s] AntNo[%d]\n",RssiData,AntNo);
							}
							else if (FlagTagInventory==InventoryOrdinaryTagsRSSI)
							{
								printf("TagsRSSIData[%s] AntNo[%d]\n",RssiData,AntNo);
							}
							//printf("AntNo: %d\n",AntNo);
						}
						//sleep(1);
					}
					else
					{
						//sleep(1);
					}

				}
				rc=RFID_SysRFPwrOFF(iComportHandle,ISO18K_RAddr);
				rc=RFID_Gen2_EndOperation(iComportHandle,ISO18K_RAddr);
				printf("Read Muilti-Tag finish\n\n\n");
			}
			else
			{
				rc=RFID_SysRFPwrOFF(iComportHandle,ISO18K_RAddr);
				rc=RFID_Gen2_EndOperation(iComportHandle,ISO18K_RAddr);
				printf("RFID_REC_GetTagDataEx Error\n\n\n");
			}
		}
		else
		{
			printf("RFID_SysBeepCtrl Error\n\n\n");
		}
	}
	else
	{
		printf("RFID_SysWorkModeQuery Error\n\n\n");
	}
}



void ReadTagData()
{
	unsigned char IDBit=96;//EPC bit length
	unsigned char EPCID[64]={0};//EPC ID
	unsigned char QV=0;
	unsigned char MemBank =3;
	unsigned char AccessWordLen;
	unsigned char AccessStartAddr;
	unsigned char TagRecord[64]={0};
	unsigned char RecvRecLen;

	QV=0;
	MemBank =3;
	AccessStartAddr=0x00;
	AccessWordLen=0x06;

	const char* SourceEPCID="060C10200204020403060000";
	const char* MemName3="USER";
	char strAccess[128]={0};
	int SourceEPCIDLen=strlen(SourceEPCID);

	int ii=0;
	short rc;
	unsigned char WorkMode;

	char EPCID_Name[100]={0};
	int EPCID_NameLen=0;
	int Reback;


	if(MemBank==0)
	{
		MemBank =GEN2_BANK_RESERVE;
	}
	else if(MemBank==1)
	{
		MemBank =GEN2_BANK_EPC;
	}
	else if(MemBank==2)
	{
		MemBank =GEN2_BANK_TID;
	}
	else if(MemBank==3)
	{
		MemBank =GEN2_BANK_USER;
	}


	short (*RFID_SysWorkModeQuery)(int ,unsigned char, unsigned char *);
	if((RFID_SysWorkModeQuery = dlsym(handle,"RFID_SysWorkModeQuery")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	void (*RFID_HexStrToByte)(const char *, unsigned char * , int);
	if((RFID_HexStrToByte = dlsym(handle,"RFID_HexStrToByte")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	void (*RFID_ByteToHexStr)(const char *, unsigned char * , int);
	if((RFID_ByteToHexStr = dlsym(handle,"RFID_ByteToHexStr")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	short (*RFID_Gen2_ReadWithID)(int ,unsigned char , unsigned char ,unsigned char ,unsigned char , unsigned char , unsigned char , unsigned char [] , unsigned char * , unsigned char []);
	if((RFID_Gen2_ReadWithID = dlsym(handle,"RFID_Gen2_ReadWithID")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	//Input EPCID_Name
	Reback=scanf("%s",EPCID_Name);
	EPCID_NameLen=strlen(EPCID_Name);

	if (EPCID_NameLen==0 || ((EPCID_NameLen % 2) !=0))	  //字符串长度非法
	{	
		printf("Input EPCID Name Data(hex) is Invalid ");		
		return ;
	}


	//START WORKFLOW
	rc=RFID_SysWorkModeQuery(iComportHandle,ISO18K_RAddr,&WorkMode);
	if (rc==INF_OK)
	{
		printf("RFID_SysWorkModeQuery OK\n");
		printf("Reader Work Mode: %02d\n",WorkMode);
		if(WorkMode!=0)
		{
			printf("The Reader is not work on Command Mode!\r\nPlease set it to Command Mode!");
			return;
		}


		//RFID_HexStrToByte(SourceEPCID, EPCID, SourceEPCIDLen);
		RFID_HexStrToByte(EPCID_Name, EPCID, EPCID_NameLen);
		rc=RFID_Gen2_ReadWithID(iComportHandle,ISO18K_RAddr, QV, MemBank, AccessStartAddr, AccessWordLen, IDBit, EPCID, &RecvRecLen,TagRecord);
		if (rc==INF_OK)
		{
			RFID_ByteToHexStr(TagRecord,strAccess, (RecvRecLen-1));
			printf("Read Tag[EPC:0x%s] in memory bank[%s]:\n", SourceEPCID,MemName3);
			printf(" --Tag data{%s}.Antenna[%d]!\n", strAccess,TagRecord[RecvRecLen-1]);
			printf("RFID_Gen2_ReadWithID OK\n\n\n");
		}
		else
		{
			printf("RFID_Gen2_ReadWithID Error\n\n\n");
		}
	}
	else
	{
		printf("RFID_SysWorkModeQuery Error\n\n\n");
	}
}

#define iDebug	0
void WriteTagData()
{
	unsigned char IDBit=96;//EPC bit length
	unsigned char EPCID[64]={0};//EPC ID
	unsigned char WriteByteLen,WriteWordLen;
	unsigned char WriteStartAddr;
	unsigned char uWriteData[64]={0};
	unsigned char QV=0;
	unsigned char MemBank =3;
	unsigned char TagRecord[64]={0};
	unsigned char RecvRecLen;

	QV=0;
	MemBank =3;
	WriteStartAddr=0x00;
	WriteWordLen=0x06;
	//WriteByteLen=WriteWordLen*2;

	const char* SourceEPCID="060C10200204020403060000";
	const char* SourceWriteData="1122FF445566778899AABBCC";
	const char* MemName3="USER";
	int SourceEPCIDLen=strlen(SourceEPCID);
	int SourceWriteDataLen=strlen(SourceWriteData);

	short rc,n;
	unsigned char WorkMode;


	if (SourceEPCIDLen==0 || ((SourceEPCIDLen % 2) !=0))	  //字符串长度非法
	{	
		printf("Lenth of SourceEPCID Data(hex) is Invalid ");		
		return ;
	}
	if (SourceWriteDataLen==0 || ((SourceWriteDataLen % 2) !=0))	  //字符串长度非法
	{	
		printf("Lenth of SourceWriteData Data(hex) is Invalid ");		
		return ;
	}

	short (*RFID_SysWorkModeQuery)(int ,unsigned char, unsigned char *);
	if((RFID_SysWorkModeQuery = dlsym(handle,"RFID_SysWorkModeQuery")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	void (*RFID_HexStrToByte)(const char *, unsigned char * , int);
	if((RFID_HexStrToByte = dlsym(handle,"RFID_HexStrToByte")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	short (*RFID_Gen2_WriteWithID)(int,unsigned char,unsigned char,unsigned char,unsigned char,unsigned char,unsigned char [],unsigned char,unsigned char []);
	if((RFID_Gen2_WriteWithID = dlsym(handle,"RFID_Gen2_WriteWithID")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		exit(0);
	}

	RFID_HexStrToByte(SourceEPCID, EPCID, SourceEPCIDLen);
	RFID_HexStrToByte(SourceWriteData, uWriteData, SourceWriteDataLen);
	WriteByteLen=SourceWriteDataLen/2;

	//调试输出
	if (iDebug==1)
	{
		printf("EPCID Data(hex) is : ");
		for (n=0;n<SourceEPCIDLen/2;n++)
		{
			printf("0x%02X ",EPCID[n]);
		}
		printf("\n");
		printf("lenth of EPCID Data is : %d \n",SourceEPCIDLen);

		printf("uWriteData Data(hex) is : ");
		for (n=0;n<WriteByteLen;n++)
		{
			printf("0x%02X ",uWriteData[n]);
		}
		printf("\n");
		printf("lenth of uWriteData Data is : %d \n",WriteByteLen);
	 }

	if(MemBank==0)
	{
		MemBank =GEN2_BANK_RESERVE;
	}
	else if(MemBank==1)
	{
		MemBank =GEN2_BANK_EPC;
	}
	else if(MemBank==2)
	{
		MemBank =GEN2_BANK_TID;
	}
	else if(MemBank==3)
	{
		MemBank =GEN2_BANK_USER;
	}



	//START WORKFLOW
	rc=RFID_SysWorkModeQuery(iComportHandle,ISO18K_RAddr,&WorkMode);
	if (rc==INF_OK)
	{
		printf("RFID_SysWorkModeQuery OK\n");
		printf("Reader Work Mode: %02d\n",WorkMode);
		if(WorkMode!=0)
		{
			printf("The Reader is not work on Command Mode!\r\nPlease set it to Command Mode!");
			return;
		}


		rc=RFID_Gen2_WriteWithID(iComportHandle,ISO18K_RAddr,QV,MemBank,WriteStartAddr,WriteByteLen,uWriteData,IDBit,EPCID);
		if (rc==INF_OK)
		{
			printf("Write Tag[EPC:0x%s] in memory bank[%s]:\n", SourceEPCID,MemName3);
			printf(" --Write data[0x%s] to address[%d] \n", SourceWriteData,WriteStartAddr);
			printf("RFID_Gen2_WriteWithID OK\n\n\n");
		}
		else
		{
			printf("RFID_Gen2_WriteWithID Error\n\n\n");
		}
	}
	else
	{
		printf("RFID_SysWorkModeQuery Error\n\n\n");
	}
}

//================================================================================================================
int menu()
{
	int i;
    int Reback;
	i=0;
	while(1)
	{
		printf("Please select a menu(0~9):\n");
		printf("    ---------Communication Settings----------------------\n");
		printf("    1:  Open the Comm. port.\n");
		printf("    ---------Reader Operation----------------------------\n");
		printf("    2:  The Reader Init.\n");
		printf("    3:  Read Single-Tag.\n");
		printf("    4:  Read Multi-Tag.\n");
		printf("    5:  Read Tag Data.\n");
		printf("    6:  Write Tag Data.\n");
		printf("    0:  Exit (Close the Comm. port).\n");
		Reback=scanf("%d",&i);
		printf("\n");
        printf("You have chosen: %d\n",i);
		if (i==0)
		{
			if (iComportHandle>0)
			{
			Close_Comm();
			}
			printf("Byebye.\n\n\n");
			dlclose(handle);
			exit(1);
		}
		if (i<0 || i>6)
		{
			printf("Error.\n\n");
			continue;
		}
		else
		{
			return(i);
		}
	}
}

int main()
{

   int s;
   iComportHandle=-1;

   if((handle = dlopen("./libISO18KAPI.so",RTLD_LAZY))  == NULL)
   {
        fprintf(stderr,"dlopen:%s\n",dlerror());
        exit(0);
   }
   dlerror();
   while(1)
    {
       s=menu();
       if (s==1)
       {
          Open_Comm();
       }
       if (s==2)
       {
           ReaderReset();
       }
       if (s==3)
       {
    	   ReadSingleTag();
       }
       if (s==4)
       {
    	   ReadMultiTag();
       }
       if (s==5)
       {
    	   ReadTagData();
       }
       if (s==6)
       {
    	   WriteTagData();
       }
    }
}

