/*
 ============================================================================
 Name        : ISO18KAPI_TEST_TempeTag.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "rfid_test_mode.h"

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

gBUFstruct gTagReadQue;


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

void ReadTemperatureTagData()
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
	unsigned char MemBank=0x01;
	unsigned char ReadOffsetAddr=0x02;
	unsigned char ReadWordLen=0x06;
	unsigned char Mask[8]={0};
	unsigned char RecvTagNum=0x00, MaskLength=0x00, RecvRecLen=0x00;
	int FlagTagInventory=1;//当前操作标签类型
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

	rc=RFID_SysWorkModeQuery(iComportHandle,ISO18K_RAddr,&WorkMode);
	printf("Reader Work Mode: %02d\n",WorkMode);

	if(WorkMode!=0)
	{
		printf("The Reader is not work on Command Mode!\r\nPlease set it to Command Mode!");
		return;
	}

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

			if(MemBank==0)	MemBank =GEN2_BANK_EPC;

			if(FlagTagInventory==InventoryTag)
			{
				rc=RFID_Gen2_MTR(iComportHandle,ISO18K_RAddr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
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
			else if (FlagTagInventory==InventoryOrdinaryTagsRSSI)
			{
				rc=RFID_Gen2_OrdinaryTagsRSSI(iComportHandle,ISO18K_RAddr,gQV, MemBank,ReadOffsetAddr,ReadWordLen,MaskLength, Mask);
			}

			if (rc==INF_OK)
			{
				printf("Start Multi-Tag Read (MTR)\n");
				InitQueue(&gTagReadQue);
				//printf("InitQueue OK\n");

				for(ii=0;ii<60;ii++)//while(1)
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
							if(FlagTagInventory==InventoryTag)//
							{
								printf("TagData[%s] AntNo[%d]\n",strtemp,AntNo);
							}
							else if(FlagTagInventory==InventoryTemperatureTag)//湿度标签读取
							{
								printf("TagData[%s] Temperature[%s ] AntNo[%d]\n",strtemp,tempedata,AntNo);
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

#if 0
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
