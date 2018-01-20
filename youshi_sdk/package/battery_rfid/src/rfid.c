/*
   ============================================================================
Name        : ISO18KAPI_TEST_TempeTag.c
Author      : 
Version     :
Copyright   : Your copyright notice
Description : Hello World in C, Ansi-style
============================================================================
*/

#include "rfid.h"

/*

   gcc -c -O ISO18KAPI_TEST_TempeTag.c
   gcc ISO18KAPI_TEST_TempeTag.o -ldl -lm -lc -o ISO18KAPI_TEST_TempeTag
   ./ISO18KAPI_TEST_TempeTag


   /usr/WRTnoteTOOLS/bin/mipsel-openwrt-linux-gcc -c -O ISO18KAPI_TEST_TempeTag.c
   /usr/WRTnoteTOOLS/bin/mipsel-openwrt-linux-gcc ISO18KAPI_TEST_TempeTag.o -ldl -o ISO18KAPI_TEST_TempeTag
   ./ISO18KAPI_TEST_TempeTag

   /usr/WRTnoteTOOLS/bin/mipsel-openwrt-linux-gcc -ldl -o ISO18KAPI_TEST_TempeTag -c -O ISO18KAPI_TEST_TempeTag.c

*/


int iComportHandle;  //device Number
static unsigned char ISO18K_RAddr=0;    //device Address
static unsigned char Broadcast_RAddr=0xFF;   //device Address
void *handle;

extern pthread_mutex_t read_mutex;

gBUFstruct gTagReadQue;


//-------------------------------------------------------------------------------------------

//void Open_Comm()

//return device fd
int rfid_open(char * dev, void * handle, unsigned char *rfid_addr )
{
	char devicename[64]="/dev/ttyS1"; //default
	char path[64] = {0};
	int fd = -1;

	unsigned char Broadcast_rfid_addr = 0xFF;   //device Address

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

	if (NULL == dev)
		memcpy(path, devicename, strlen(devicename));
	else
		memcpy(path, dev, strlen(dev));

	//打开读写器连接的端口
	short (*RFID_Open)(int *,char *,unsigned int);
	if((RFID_Open = dlsym(handle,"RFID_Open")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//查询读写器状态
	short (*RFID_SysGetStatus)(int, unsigned char, unsigned char *);
	if((RFID_SysGetStatus = dlsym(handle,"RFID_SysGetStatus")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//查询读写器系列号
	short (*RFID_SysGetSN)(int ,unsigned char, unsigned char, unsigned char* );
	if((RFID_SysGetSN = dlsym(handle,"RFID_SysGetSN")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//查询读写器地址
	short (*RFID_ReadAddrQuery)(int ,unsigned char, unsigned char *);
	if((RFID_ReadAddrQuery = dlsym(handle,"RFID_ReadAddrQuery")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//查询读写器版本号
	short (*RFID_SysGetFirmwareVersion)(int ,unsigned char,unsigned char* ,unsigned char* ,unsigned char* ,unsigned char*);
	if((RFID_SysGetFirmwareVersion = dlsym(handle,"RFID_SysGetFirmwareVersion")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//系统蜂鸣器
	short (*RFID_SysBeepCtrl)(int ,unsigned char,unsigned char ,unsigned char*);
	if((RFID_SysBeepCtrl = dlsym(handle,"RFID_SysBeepCtrl")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

#if 0
	printf("Sample of the device name  \n");
	printf("format of COM1 PORT is /dev/ttyS0  etc.\n");
	printf("format of USB PORT 1 is /dev/ttyUSB0  etc.\n");
	printf("Please input the device name(/dev/ttyS0):\n");

	Reback=scanf("%s",devicename);
#endif

	//打开读写器连接的端口
	//rc = RFID_Open(&iComportHandle,devicename,115200);
	rc = RFID_Open(&fd, path, 115200);

	//if (rc==INF_OK && iComportHandle>0)
	if (rc == INF_OK && fd > 0)
	{
		printf("RFID_Open OK\n");
		//printf("Device adrress is %d\n",iComportHandle);
		printf("Device adrress is %d\n", fd);

		//查询读写器状态
		//rc=RFID_SysGetStatus(iComportHandle,Broadcast_RAddr,rStatus);
		rc = RFID_SysGetStatus(fd, Broadcast_rfid_addr, rStatus);

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

			//rc=RFID_ReadAddrQuery(iComportHandle,Broadcast_RAddr,&ISO18K_RAddr);

			rc = RFID_ReadAddrQuery(fd, Broadcast_rfid_addr, rfid_addr);
			if (rc==INF_OK)
			{
				printf("RFID_ReadAddrQuery OK\n");
				//printf("Reader Device Address: %d\n",ISO18K_RAddr);
				printf("Reader Device Address: %d\n", *rfid_addr);

				//rc=RFID_SysGetFirmwareVersion(iComportHandle,ISO18K_RAddr,&HWMajorVer,&HWMinorVer,&SWMajorVer,&SWMinorVer);
				rc=RFID_SysGetFirmwareVersion(fd, *rfid_addr, &HWMajorVer, &HWMinorVer, &SWMajorVer, &SWMinorVer);

				if (rc==INF_OK)
				{
					printf("RFID_SysGetFirmwareVersion OK\n");
					printf("Reader Firmware Version: V%02d.%02d.%02d.%02d\n",HWMajorVer,HWMinorVer,SWMajorVer,SWMinorVer);

					//系统蜂鸣器
					//rc=RFID_SysBeepCtrl(iComportHandle,ISO18K_RAddr,BEEP_OPERATION_GET_PARAM1,&BEEP_OPERATION_GET_PARAM2);
					rc=RFID_SysBeepCtrl(fd, *rfid_addr, BEEP_OPERATION_GET_PARAM1, &BEEP_OPERATION_GET_PARAM2);
					if (rc==INF_OK)
					{
						printf("RFID_SysBeepCtrl OK\n");
						printf("BEEP_OPERATION_GET_PARAM2: %02d\n\n\n",BEEP_OPERATION_GET_PARAM2);
					}
					else
					{
						printf("RFID_SysBeepCtrl Error\n\n\n");
						goto error;
					}
				}
				else
				{
					printf("RFID_SysGetFirmwareVersion Error\n\n\n");
					goto error;
				}
			}
			else
			{
				printf("RFID_ReadAddrQuery Error\n\n\n");
				goto error;
			}
		}
		else
		{
			printf("RFID_SysGetStatus Error\n\n\n");
			goto error;
		}
	}
	else
	{
		printf("RFID_Open error\n\n\n");
		goto error;
	}

	return fd;
error:

	return -1;
}


//int Close_Comm(int fd)
int rfid_close(int fd, void * handle, unsigned char rfid_addr)
{
	short rc;

	short (*RFID_Close)(int pClientHandle,unsigned char Addr);

	if((RFID_Close = dlsym(handle,"RFID_Close")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//rc=RFID_Close(iComportHandle,ISO18K_RAddr);
	rc = RFID_Close(fd, rfid_addr);

	if (rc==INF_OK)
	{
		printf("RFID_Close OK\n\n\n");
		//iComportHandle=0;
	}
	else
	{
		printf("RFID_Close Error\n\n\n");
		goto error;
	}

	return 0;

error:
	return -1;
}
//================================================================================================

//void ReaderReset()   //Reader Reset
int rfid_reset(int fd, void * handle, unsigned char rfid_addr)
{
	short rc;

	//读写器复位
	short (*RFID_SysReset)(int ,unsigned char);
	if((RFID_SysReset = dlsym(handle,"RFID_SysReset")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//读写器复位
	//rc = RFID_SysReset(iComportHandle,ISO18K_RAddr);
	rc = RFID_SysReset(fd, rfid_addr);

	if (rc==INF_OK)
	{
		printf("RFID_SysReset OK\n");
		//printf("Reader Device Address: %d\n\n\n",ISO18K_RAddr);
		printf("Reader Device Address: %d\n\n\n", rfid_addr);
	}
	else
	{
		printf("RFID_SysReset Error\n\n\n");
		goto error;
	}
	return 0;
error:
	return -1;
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

#if 0
struct data {
	unsigned char Tag_data[128];
	unsigned char temperture[128];
	unsigned char moisture[8];
	unsigned char rssi[32];
	unsigned char ant_num;

};
#endif

//void ReadTemperatureTagData()
//int rfid_read_temperature_tag_data(int fd, void * handle, unsigned char rfid_addr, struct data *pd )
int rfid_read_temperature_tag_data(int fd, void * handle, unsigned char rfid_addr, list_t *plist )
{
	//ReadMultiTag(int fd, void * handle, unsigned char rfid_addr, int FlagTagInventory);
	return ReadMultiTag(fd, handle, rfid_addr, InventoryTemperatureTag, plist);
	//return ReadSingleTag(fd, handle, rfid_addr, InventoryTemperatureTag, pd);
}

//int ReadSingleTag(int fd, void * handle, unsigned char rfid_addr, int FlagTagInventory, struct data *pd)
int ReadSingleTag(int fd, void * handle, unsigned char rfid_addr, int FlagTagInventory, list_t *plist)
//void ReadSingleTag()
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
		goto error;
	}

	short (*RFID_REC_ClrAll)(int ,unsigned char);
	if((RFID_REC_ClrAll = dlsym(handle,"RFID_REC_ClrAll")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_Gen2_STR)(int , unsigned char ,unsigned char , unsigned char ,unsigned char ,unsigned char , unsigned char , unsigned char [], unsigned char* , unsigned char* , unsigned char []);
	if((RFID_Gen2_STR = dlsym(handle,"RFID_Gen2_STR")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_SysRFPwrOFF)(int ,unsigned char);
	if((RFID_SysRFPwrOFF = dlsym(handle,"RFID_SysRFPwrOFF")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_Gen2_EndOperation)(int ,unsigned char);
	if((RFID_Gen2_EndOperation = dlsym(handle,"RFID_Gen2_EndOperation")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//START WORKFLOW
	//rc=RFID_SysWorkModeQuery(iComportHandle,ISO18K_RAddr,&WorkMode);
	rc = RFID_SysWorkModeQuery(fd, rfid_addr, &WorkMode);
	if (rc==INF_OK)
	{
		printf("RFID_SysWorkModeQuery OK\n");
		printf("Reader Work Mode: %02d\n",WorkMode);
		if(WorkMode!=0)
		{
			printf("The Reader is not work on Command Mode!\r\nPlease set it to Command Mode!\r\n");
			goto error;
		}

		//rc=RFID_REC_ClrAll(iComportHandle,ISO18K_RAddr);
		rc = RFID_REC_ClrAll(fd, rfid_addr);
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

				//rc=RFID_Gen2_STR(iComportHandle,ISO18K_RAddr,gQV,gEPC_MemBank,ReadOffsetAddr,ReadWordLen,MaskLength,Mask,&RecvTagNum,RecvRecLen,TagRecord);
				rc = RFID_Gen2_STR(fd, rfid_addr, gQV, gEPC_MemBank, ReadOffsetAddr, ReadWordLen, MaskLength, Mask, &RecvTagNum, RecvRecLen, TagRecord);
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
					printf("RFID_Gen2_STR Error\n");
					continue;
					//goto error;
				}
				usleep(2000);

			}
			printf("Run RFID_Gen2_STR Finish\n");
			rc=RFID_SysRFPwrOFF(fd, rfid_addr);
			rc=RFID_Gen2_EndOperation(fd, rfid_addr);
			printf("RFID_SysRFPwrOFF \n\n\n");
		}
		else
		{
			printf("RFID_SysBeepCtrl Error\n\n\n");
			goto error;
		}
	}
	else
	{
		printf("RFID_SysWorkModeQuery Error\n\n\n");
		goto error;
	}

	return 0;

error:
	return -1;
}
//void ReadTemperatureTagData()
//int ReadMultiTag(int fd, void * handle, unsigned char rfid_addr, int FlagTagInventory, struct data *pd)
int ReadMultiTag(int fd, void * handle, unsigned char rfid_addr, int FlagTagInventory, list_t *plist)
{
#if 0
	struct data *pd = NULL; 
	struct data data; 
#endif

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
	//unsigned char TagBuffer[32], TagBufLen=0;
	unsigned char TagBuffer[512], TagBufLen=0;
	unsigned char TempeBuff[20],TempeLen=0;
	char MoistureData[10];

	unsigned char AntNo;
	unsigned char TagCount=0;
	short TempeValue;
	unsigned char Value;

	unsigned char gQV=0x06;
	unsigned char MemBank=0x01;
	unsigned char ReadOffsetAddr=0x02;
	unsigned char ReadWordLen=0x06;
	unsigned char Mask[8]={0};
	unsigned char RecvTagNum=0x00, MaskLength=0x00, RecvRecLen=0x00;

	//int FlagTagInventory=1;//当前操作标签类型

	//
	short (*RFID_SysWorkModeQuery)(int ,unsigned char, unsigned char *);
	if((RFID_SysWorkModeQuery = dlsym(handle,"RFID_SysWorkModeQuery")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//
	short (*RFID_REC_ClrAll)(int ,unsigned char);
	if((RFID_REC_ClrAll = dlsym(handle,"RFID_REC_ClrAll")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_Gen2_MTR)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_MTR = dlsym(handle,"RFID_Gen2_MTR")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_Gen2_OrdinaryTagsRSSI)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_OrdinaryTagsRSSI = dlsym(handle,"RFID_Gen2_OrdinaryTagsRSSI")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_Gen2_TempeTagRead)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_TempeTagRead = dlsym(handle,"RFID_Gen2_TempeTagRead")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_Gen2_TempeTagRSSIRead)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_TempeTagRSSIRead = dlsym(handle,"RFID_Gen2_TempeTagRSSIRead")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_Gen2_MoistureTagRead)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_MoistureTagRead = dlsym(handle,"RFID_Gen2_MoistureTagRead")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_Gen2_MoistureTagRSSIRead)(int hdl, unsigned char RAddr,unsigned char QValue, unsigned char Bank,unsigned char Point,unsigned char ReadLength,unsigned char MaskLength, unsigned char Mask[]);
	if((RFID_Gen2_MoistureTagRSSIRead = dlsym(handle,"RFID_Gen2_MoistureTagRSSIRead")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}


	void (*InitQueue)(pBUFstruct pBuf);
	if((InitQueue = dlsym(handle,"InitQueue")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*Write2Queue)(pBUFstruct pBuf,unsigned char *uData,unsigned int Len);
	if((Write2Queue = dlsym(handle,"Write2Queue")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*Read4Queue)(pBUFstruct pBuf,unsigned char *uData,unsigned int Len);
	if((Read4Queue = dlsym(handle,"Read4Queue")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}


	short (*RFID_REC_GetTagDataEx)(int hdl, unsigned char RAddr, unsigned char InTagCount, unsigned char *FinalGetTagCount, pTagRECstruct pTagRec, unsigned char bNeedClr);
	if((RFID_REC_GetTagDataEx = dlsym(handle,"RFID_REC_GetTagDataEx")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//
	short (*RFID_SysRFPwrOFF)(int ,unsigned char);
	if((RFID_SysRFPwrOFF = dlsym(handle,"RFID_SysRFPwrOFF")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	short (*RFID_Gen2_EndOperation)(int ,unsigned char);
	if((RFID_Gen2_EndOperation = dlsym(handle,"RFID_Gen2_EndOperation")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	//START WORKFLOW

	rc=RFID_SysWorkModeQuery(fd,rfid_addr,&WorkMode);
	printf("Reader Work Mode: %02d\n",WorkMode);

	if(WorkMode!=0)
	{
		printf("The Reader is not work on Command Mode!\r\nPlease set it to Command Mode!\r\n");
		goto error;
	}
#if 1
	InitQueue(&gTagReadQue);
	printf("InitQueue OK\n");
#endif
	if (rc==INF_OK)
	{
		//printf("InitQueue OK\n");
		//
		rc=RFID_REC_ClrAll(fd,rfid_addr);
		if (rc==INF_OK)
		{
			printf("RFID_REC_ClrAll OK\n");

			if(MemBank==0)	MemBank =GEN2_BANK_EPC;

			if(FlagTagInventory==InventoryTag)
			{
				rc=RFID_Gen2_MTR(fd,rfid_addr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if(FlagTagInventory==InventoryTemperatureTag)
			{
				rc=RFID_Gen2_TempeTagRead(fd,rfid_addr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if (FlagTagInventory==InventoryTemperatureTagRSSI)
			{
				rc=RFID_Gen2_TempeTagRSSIRead(fd,rfid_addr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if (FlagTagInventory==InventoryMoistureTags)
			{
				rc=RFID_Gen2_MoistureTagRead(fd,rfid_addr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if (FlagTagInventory==InventoryMoistureTagsRSSI)
			{
				rc=RFID_Gen2_MoistureTagRSSIRead(fd,rfid_addr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}
			else if (FlagTagInventory==InventoryOrdinaryTagsRSSI)
			{
				rc=RFID_Gen2_OrdinaryTagsRSSI(fd,rfid_addr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}

			if (rc==INF_OK)
			{
				printf("Start Multi-Tag Read (MTR)\n");
#if 0
				InitQueue(&gTagReadQue);
				printf("InitQueue OK\n");
#endif
				//for(ii=0;ii<60;ii++)//while(1)
				while(1)
				{
#if 0
					if(plist->len)
					{
						list_iterator_t *it = list_iterator_new(plist, LIST_HEAD);
						list_node_t *d;

						while(NULL != (d = list_iterator_next(it)))
						{
							rfid_info *p = d->val;

							printf("list TagData[%s] Temperature[%s] AntNo[%d]\n", p->tag_data, p->temperture, p->ant_num);

						}
					}
#endif

					sst=RFID_REC_GetTagDataEx(fd,rfid_addr,16, &TagQuantity_GetFromReaderBuf, &TagRec[0], 1);//true
					//printf("RFID_REC_GetTagDataEx OK(%02X)\n",sst);
					if(sst == INF_OK && ( TagQuantity_GetFromReaderBuf > 0) )
						//if(sst == INF_OK )
					{
#if 1
						//printf("TagQuantity_GetFromReaderBuf>0\n");
						for(TagIndex=0;TagIndex<TagQuantity_GetFromReaderBuf;TagIndex++)
						{
							//由于是多任务的系统，为了不丢失数据，首先存储到缓冲区中
							Write2Queue(&gTagReadQue,  &TagRec[TagIndex].yTagID_Len, 1);//数据长度入列
							Write2Queue(&gTagReadQue,  TagRec[TagIndex].yTagID_Data, TagRec[TagIndex].yTagID_Len);//数据入列
							Write2Queue(&gTagReadQue,  &TagRec[TagIndex].yTagID_AntNo, 1);//天线号入列
						}
						//printf("Write2Queue OK\n");
#endif

						{
							Read4Queue(&gTagReadQue, &TagBufLen, 1);//read the lenth
							printf("TagBufLen : %d\n", TagBufLen);

							if(TagBufLen <= 0 )
							{
								//continue;
								goto error;
							}

							strcpy(temp,"");//
							strcpy(tempedata,"");//
							strcpy(RssiData,"");//
							strcpy(MoistureData,"");

							if(FlagTagInventory==InventoryTag)//
							{
								Read4Queue(&gTagReadQue, TagBuffer, TagBufLen);//读出数据
								Read4Queue(&gTagReadQue, &AntNo, 1);//天线号
							}
							else if(FlagTagInventory==InventoryTemperatureTag)//温度标签读取
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
									//sprintf(temp,"%d.%d℃",TempeValue,Value);
									sprintf(temp,"%d.%d",TempeValue,Value);
									strcat(tempedata,temp);
								}
								else if(TempeValue<0)
								{
									//sprintf(temp,"%d.%d℃",TempeValue/10,-TempeValue%10);
									sprintf(temp,"%d.%d",TempeValue/10,-TempeValue%10);
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
									sprintf(temp,"%02X", TagBuffer[j]);
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

							if(FlagTagInventory==InventoryTag)//
							{
								printf("TagData[%s] AntNo[%d]\n",strtemp,AntNo);
							}
							else if(FlagTagInventory==InventoryTemperatureTag)//湿度标签读取
							{
								char tmp[512];
								memset(tmp, 0, 512);

								printf("TagData[%s] Temperature[%s] AntNo[%d]\n",strtemp,tempedata,AntNo);

								pthread_mutex_lock(&read_mutex);
								rfid_info *prfid = NULL;

								prfid = (rfid_info*)malloc(sizeof(rfid_info));
								if (NULL == prfid) {
									fprintf(stderr, "malloc read buffer error.\n");
								}

								memset(prfid, 0, sizeof(rfid_info));
#if 1
								memcpy(prfid->tag_data, strtemp, strlen(strtemp));
								memcpy(prfid->temperture, tempedata, strlen(tempedata));
								//memcpy(&(pd->ant_num), &AntNo, 1);
								prfid->ant_num = AntNo;
								prfid->tag_data_len = strlen(strtemp);

								//printf("++++++++++++++++++TagData len %d, %s\n", prfid->tag_data_len, prfid->tag_data);

								//new	
								list_node_t *node = list_node_new(prfid);

								printf("+new node++++++++++TagData len %d, %s\n", ((rfid_info*) (node->val))->tag_data_len, ((rfid_info*) (node->val))->tag_data);
								//printf("++++++++prfid ++++++++++TagData len %d, %s\n", prfid->tag_data_len, prfid->tag_data);

								// insert

								rfid_list_insert(plist, node);

								pthread_mutex_unlock(&read_mutex);

#endif



								// mqtt send data
								// format for pirnt
								//sprintf(tmp, "TagData[%s], Temperature[%s ], AntNo[%d]\n", strtemp, tempedata, AntNo);

								// data format 
								sprintf(tmp, "%s|%s|%d", strtemp, tempedata, AntNo);
								
								// json format 
								//sprintf(tmp, "{\"TagData\":%s,\"Temperature\":%s,\"AntNo\":%d}", strtemp, tempedata, AntNo);


#if 0
								printf("data: %s \n\n", tmp);
								int len = strlen(tmp);

								// use default ip and port
								if (len > 0)
									mqtt_pub(tmp, len, NULL);
#endif

							}
							else if(FlagTagInventory==InventoryTemperatureTagRSSI)//
							{
								printf("TemperatureTagData[%s] RssiData[%s] AntNo[%d]\n",strtemp,RssiData,AntNo);
							}
							else if(FlagTagInventory==InventoryMoistureTags)//
							{
								printf("MoistureTagData[%s] MoistureData[%s] AntNo[%d]\n",strtemp,MoistureData,AntNo);
							}
							else if(FlagTagInventory==InventoryMoistureTagsRSSI)//
							{
								printf("MoistureTagData[%s] RssiData[%s] AntNo[%d]\n",strtemp,RssiData,AntNo);
							}
							else if (FlagTagInventory==InventoryOrdinaryTagsRSSI)
							{
								printf("OrdinaryTagData[%s] RssiData[%s] AntNo[%d]\n",strtemp,RssiData,AntNo);
							}
						}
						//sleep(1);
					}
					else
					{
						rc=RFID_SysRFPwrOFF(fd,rfid_addr);
						rc=RFID_Gen2_EndOperation(fd,rfid_addr);
						printf("RFID_REC_GetTagDataEx Error\n\n\n");
						//sleep(1);
						goto error;
					}

				}
				rc=RFID_SysRFPwrOFF(fd,rfid_addr);
				rc=RFID_Gen2_EndOperation(fd,rfid_addr);
				printf("Read Muilti-Tag finish\n\n\n");
			}
			else
			{
				printf("RFID_Gen2 function Error\n\n\n");
				goto error;
			}
		}
		else
		{
			printf("RFID_REC_ClrAll Error\n");
			goto error;
		}
	}
	else
	{
		printf("RFID_SysWorkModeQuery Error\n\n\n");
		goto error;
	}

	return 0;

error:

	printf("Read Muilti-Tag error!! \n\n\n");
	return -1;

}

#if 1
int rfid_set_sys_RF_ant_auto_poll_on(int fd, void * handle)
{
	short rc = -1;
	unsigned char Broadcast_rfid_addr = 0xFF;   //device Address

	//查询读写器状态
	short (*RFID_SysRFAntPolling)(int, unsigned char);
	if((RFID_SysRFAntPolling = dlsym(handle,"RFID_SysRFAntPolling")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	rc = RFID_SysRFAntPolling(fd, Broadcast_rfid_addr);
	if(rc == INF_OK)
	{
		printf(" \nAuto polling antenna port is enable!\n");
		printf(" Reader will poll the antenna port while scan tags!\n");
	}
	else
	{
		printf("\nSet Auto polling antenna Fail! (0x%02X)\n", rc);
		goto error;
	}

	return 0;

error:
	return -1;
}

int rfid_set_sys_RF_ant_auto_poll_off(int fd, void * handle)
{
	short rc = -1;
	unsigned char Broadcast_rfid_addr = 0xFF;   //device Address

	//查询读写器状态
	short (*RFID_SysRFAntPollingEnd)(int, unsigned char );
	if((RFID_SysRFAntPollingEnd = dlsym(handle,"RFID_SysRFAntPollingEnd")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	rc = RFID_SysRFAntPollingEnd(fd, Broadcast_rfid_addr);
	if(rc == INF_OK)
	{
		printf("Auto polling antenna port is disable!");
	}
	else
	{
		printf("Set Auto polling antenna Fail! (0x%02X)\n", rc);
		goto error;
	}
	return 0;

error:
	return -1;
}


int rfid_set_sys_RF_cur_work_ant_query(int fd, void * handle)
{
	short rc = -1;
	unsigned char CurrentAntNo;  
	unsigned char Broadcast_rfid_addr = 0xFF;   //device Address

	//查询读写器状态
	short (*RFID_SysRFCurWorkAntQuery)(int, unsigned char, unsigned char * );
	if((RFID_SysRFCurWorkAntQuery = dlsym(handle,"RFID_SysRFCurWorkAntQuery")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}


	rc = RFID_SysRFCurWorkAntQuery(fd, Broadcast_rfid_addr, &CurrentAntNo);
	if(rc == INF_OK)
	{    
		printf("Get Current Woking Antenna: Ant%d!", CurrentAntNo);
	}  
	else 
	{  
		printf("Get Current Woking Antenna Fail! (0x%02X)\n", rc); 
		goto error;
	}   

	return 0;

error:

	return -1;
}

/*
 * Dwell Time=40+n*20 mS
 * PollingTime=1,  40+1*20 mS
 * PollingTime=23,  40+23*20 mS = 500mS
 * default time is 400ms, max time is 800ms
 * 
 * polling_time is index. if set 500ms polling time, then use polling_time=23
 */
int rfid_RF_ant_poll_time_set(int fd, void * handle, unsigned char *polling_time)
{
	int rc = -1;

	rc = rfid_sys_RF_ant_poll_time_ctrl(fd, handle, polling_time, _OPERATION_GET_PARAM);
	if (rc < 0)
		goto error;

	return 0;
error:

	return -1;

}

int rfid_RF_ant_poll_time_get(int fd, void * handle, unsigned char *polling_time)
{
	int rc = -1;

	rc = rfid_sys_RF_ant_poll_time_ctrl(fd, handle, polling_time, _OPERATION_SET_PARAM);
	if (rc < 0)
		goto error;

	return 0;
error:

	return -1;
}

/*
 * Dwell Time=40+n*20 mS
 * PollingTime=1,  40+1*20 mS
 * PollingTime=23,  40+23*20 mS = 500mS
 * default time is 400ms, max time is 800ms
 * 
 * polling_time is index. if set 500ms polling time, then use polling_time=23
 */

int rfid_sys_RF_ant_poll_time_ctrl(int fd, void * handle, unsigned char *polling_time, unsigned char operation)
{
	short rc = -1;

	unsigned char AntStatus,AntSt;

	unsigned char Broadcast_rfid_addr = 0xFF;   //device Address

	//apiStatus WINAPI RFID_SysPollingTimeCtrl(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *PollingTime);
	//   RFID_SysPollingTimeCtrl(g_RFIDClient, g_RAddr, _OPERATION_GET_PARAM,  &TempData);
	
	//查询读写器状态
	short (*RFID_SysPollingTimeCtrl)(int, unsigned char, unsigned char, unsigned char * );
	if((RFID_SysPollingTimeCtrl = dlsym(handle,"RFID_SysPollingTimeCtrl")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	rc = RFID_SysPollingTimeCtrl(fd, Broadcast_rfid_addr, operation, polling_time);
	if(rc == INF_OK)  
	{  
		printf("ctrl Anttena polling time ok: \n");   
	}
	else
	{ 
		printf("ctrl Anttena polling time Fail! (0x%02X) \n", rc);
		goto error;
	} 
	
	return 0;
error:

	return -1;

}


int rfid_set_sys_RF_ant_status_query(int fd, void * handle)
{
	short rc = -1;

	unsigned char AntStatus,AntSt;

	unsigned char Broadcast_rfid_addr = 0xFF;   //device Address

	//查询读写器状态
	short (*RFID_SysRFAntWorkStatusQuery)(int, unsigned char, unsigned char * );
	if((RFID_SysRFAntWorkStatusQuery = dlsym(handle,"RFID_SysRFAntWorkStatusQuery")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	rc = RFID_SysRFAntWorkStatusQuery(fd, Broadcast_rfid_addr, &AntStatus);

	if(rc == INF_OK)  
	{  
		printf("Get Anttena Working Enable Status: \n");   
		for(int i=0;i<4;i++) 
		{ 
			AntSt= (AntStatus>>i)&0x01;   
			if(AntSt==1)   
			{  
				printf("  Ant%d: Enable \n", i); 

			} 
			else 
			{ 
				printf("  Ant%d: Close \n", i);  
			}  
		} 
	}
	else
	{ 
		printf("Get Anttena Working Enable Status Fail! (0x%02X) \n", rc);
		goto error;
	} 

	return 0;
error:
	return -1;
}

//default enable all ants ( 4 )
int rfid_set_sys_RF_ant_enable(int fd, void * handle, unsigned char AntSt[])
{
	short rc = -1;
#if 0
	unsigned char AntSt[4]; 
	AntSt[0] = 0x01;
	AntSt[1] = 0x01; 
	AntSt[2] = 0x01; 
	AntSt[3] = 0x01;
#endif
	unsigned char  AntStatus = (AntSt[0] | (AntSt[1]<<1) | (AntSt[2]<<2) | (AntSt[3]<<3)); 
	unsigned char Broadcast_rfid_addr = 0xFF;   //device Address

	//查询读写器状态
	short (*RFID_SysRFAntWorkStatusSet)(int, unsigned char, unsigned char );
	if((RFID_SysRFAntWorkStatusSet = dlsym(handle,"RFID_SysRFAntWorkStatusSet")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	rc = RFID_SysRFAntWorkStatusSet(fd, Broadcast_rfid_addr, AntStatus);
	if(rc == INF_OK)  
	{
		printf("\nSet Antenna Enable Status: \n"); 

		for(int i=0;i<4;i++)  
		{
			if(AntSt[i]==1)
			{     
				printf("  Ant%d: Enable; \n", i);   
			}  
			else    
			{    
				printf("  Ant%d: Disable; \n", i); 
			}  
		} 
	} 
	else 
	{   
		printf("Set Antenna Enable Status Fail! (0x%02X)\n", rc);  
		goto error;
	} 

	return 0;

error:
	return -1;
}

int rfid_set_sys_cur_ant(int fd, void * handle, unsigned char rfid_addr)
{
	short rc = -1;
	unsigned char Broadcast_rfid_addr = 0xFF;   //device Address
	unsigned char AntNo;

	//查询读写器状态
	short (*RFID_SysRFCurWorkAntSet)(int, unsigned char, unsigned char );
	if((RFID_SysRFCurWorkAntSet = dlsym(handle,"RFID_SysRFCurWorkAntSet")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	rc = RFID_SysRFCurWorkAntSet(fd, Broadcast_rfid_addr, AntNo);  
	if(rc == INF_OK) 
	{  
		printf("Set Current Woking Antenna to: Ant%d!", AntNo);
		 
	} 
	else 
	{ 
		printf("Set Current Woking Antenna Fail! (0x%02X)", rc);  
		goto error;
	} 
	 
	return 0;
error:

	return -1;
}

int rfid_get_sys_status(int fd, void * handle, unsigned char rfid_addr)
{
	short rc = -1;
	unsigned char Broadcast_rfid_addr = 0xFF;   //device Address
	unsigned char ReaderStatus[12];  
	unsigned char rStatus[10]={0};

	unsigned char CurrentAntNo = -1; 
	unsigned char AntNum = -1; 
	unsigned char TT;   //-----------------------------------------0~1 字节   


	//查询读写器状态
	short (*RFID_SysGetStatus)(int, unsigned char, unsigned char *);
	if((RFID_SysGetStatus = dlsym(handle,"RFID_SysGetStatus")) == NULL)
	{
		fprintf(stderr,"dlsym:%s\n\n\n",dlerror());
		goto error;
	}

	rc = RFID_SysGetStatus(fd, Broadcast_rfid_addr, rStatus);
	if (rc == INF_OK)
	{
		CurrentAntNo = rStatus[2] & 0x0F; 
		AntNum = rStatus[2] & 0xF0; 

		TT = rStatus[3]&0x02;///////////D1  
		TT = TT>>1;  
		if(TT != 0)   
		{
			printf("Antenna is Auto Polling! \n\n\n");
		}

		TT = rStatus[3]&0x01;///////////D0   
		if(TT != 0) 
		{
			printf("Reader RF Power is Opened! \n\n\n");
		}

		//------------------------   
		TT = rStatus[4]&0x0F;///////////Antenna Connect status   
		{   
			printf("Atenna Connect Status: ");  

			unsigned char ANT_ST;  
			for(int k=0; k<4; k++)   
			{    
				ANT_ST=TT>>k;     
				ANT_ST=ANT_ST & 0x01;     
				if(ANT_ST)    
				{
					printf("  Ant%d is Connected! ", k);  
				}
				else    
				{
					printf("  Ant%d is not Connected! ", k);  
				}
			}  
		} 
	}
	else
		goto error;


	return 0;
error:
	return -1;
}
#endif


#if 0

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
			printf("    3:  Read Temperature Tag Data.\n");
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
			if (i<0 || i>3)
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

	int test_mode()
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
				ReadTemperatureTagData();
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
				ReadTemperatureTagData();
			}
		}
	}

#endif
