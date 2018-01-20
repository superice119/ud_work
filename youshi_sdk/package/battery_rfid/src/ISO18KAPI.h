//#include "stdafx.h"
//#include <windows.h>
#ifndef __ISO18KAPI_H__
#define __ISO18KAPI_H__

#ifdef RFID18K_API_EXPORTS
#define RFID18K_API __declspec(dllexport)
#else
#define RFID18K_API //__declspec(dllimport)
#endif

#ifndef _MID_SOCKET2_H_ 
#define _MID_SOCKET2_H_
#include "winsock2.h"
#endif

typedef short apiStatus; // API Universal Return Value

/////////////////////////////////////////////////////////////////////////
//          �ͻ��˽ӿ�
/////////////////////////////////////////////////////////////////////////
#ifndef _MID_TCP_CLIENT_H_ 
#define _MID_TCP_CLIENT_H_
struct MID_TCP_Client
{	
	SOCKET		m_sockClient;			//  �������ӵĿͻ���Socket
	sockaddr_in m_sockServerAddr;		//  ��������ip��ַ
	BOOL		m_sockIsConnected;		//  �Ƿ��Ѿ������Ϸ�����
	BOOL		m_sockIsNoblock;		//  �Ƿ����ڷ�����ģʽ
	DWORD		m_sockErrno;			//  ���һ�β��������Ĵ���0��ʾû�д�������ֵ�ο�ϵͳ��Ϣ
	void*		m_sockUser;				//  �ṩ��ʹ���ߵ������������ڲ�������������
};
#endif

enum PROTOCOL_TYPE
{
	PROTO_NONE, 
	PROTO_COM,
	PROTO_TCPV4
};
struct RFID_CLIENT
{
	PROTOCOL_TYPE ptype;

	HANDLE			com_handle;
	MID_TCP_Client* mtcp;

};

#define INF_OK                      0x00 // 
#define INF_UNDEFIND                0xFF //û�ж���Ĵ���
#define INF_CMD_UNDEF               0xF0 //������δ����
#define INF_CMD_SUB_UNDEF           0xF1 //������δ����

#define INF_CMD_PARAM_INVAL         0x10 //������Ч�Ĳ��������Խ��
#define INF_CMD_CRC_ERR             0x11 //��д�����յ�������֡CRC����
#define INF_CMD_RES_CRC_ERR         0x12 //�������յ���д����Ӧ��֡CRC����
#define INF_R_BUF_EMPTY             0x13 //��д��������û������
#define INF_R_ANT_FAIL              0x14 //���߹��ϻ�����δ����
#define INF_R_PLL_FAIL              0x15 //PLLʧ��
#define INF_R_UART_FAIL	            0x16 //�������Ӵ�������ӳ�ʱ
#define INF_R_SET_PARAM_FAIL        0x17 //���ö�д����������
#define INF_R_GET_PARAM_FAIL        0x18 //��ȡ��д����������
#define INF_R_WR_FLASH_FAIL         0x19 //д���̶�д��FLASH����

#define INF_TAG_NO_TAG              0x20 //û���ҵ���ǩ
#define INF_TAG_LOCKED              0x21 //��ǩ�Ѿ�д����
#define INF_TAG_CRC_ERR             0x22 //��ǩ����CRC��
#define INF_TAG_WRITE_FAIL          0x23 //д��ǩ����
#define INF_TAG_READ_FAIL           0x24 //����ǩ����
#define INF_TAG_ERASE_FAIL          0x25 //������ǩ����
#define INF_TAG_LOCK_FAIL           0x26 //������ǩ����
#define INF_TAG_KILL_FAIL           0x27 //���ٱ�ǩ����
#define INF_TAG_VERIFY_FAIL         0x28 //��֤��ǩ����

#define INF_RS232_ERROR             0x7F //RS-232 ��������
#define INF_RS232_READ               0x7E //RS-232 ���մ��� 
#define INF_RS232_WRITE             0x7D //RS-232 ���ʹ��� 
#define INF_CRC_ERROR                 0x7C //RS-232 ����У�����
#define INF_HOST_RXBUF_EMPTY  0x7B  //��λ������������ǿ�
#define INF_HOST_IN_PROGRESS   0x7A  //��λ�����δ�������

#define INF_NET_ERROR                  0x8F //NET ����
#define INF_NET_CLIENT_READ      0x8E //NET ���մ��� 
#define INF_NET_CLIENT_SEND      0x8D //NET ���ʹ��� 
#define INF_NET_CRC_ERROR          0x8C //NET ����У�����
#define INF_NET_TIMEOUT				0x8B  //NET ��ʱ

/////////////////////////////////////////////
//��ǩ�����У����ص���ش�����Ϣ
#define _INF_TAG_NO_TAG      0x20   //û�ж�ȡ����ǩ���ޱ�ǩ��Ӧ
#define _INF_TAG_LOCKED      0x21   //д��ǩ�������Ѿ�������������д��
#define _INF_TAG_CRC_ERR     0x22   //������ǩ����ǩ����CRC����  
#define _INF_TAG_WRITE_FAIL  0x23   //д��ǩʧ��
#define _INF_TAG_READ_FAIL   0x24   //����ǩ����ʧ��
#define _INF_TAG_LOCK_FAIL   0x26   //������ǩʧ��
#define _INF_TAG_KILL_FAIL   0x27   //���ٱ�ǩʧ��
#define _INF_TAG_VERIFY_FAIL 0x28   //У���ǩ����ʧ��
#define _INF_TAG_OUTTIME     0x29   //������ǩ��ʱ

#define _INF_TAG_EPC_INSUFFICIENT_RFPWR     0x30  //д��ǩ��RF���������㣬����̫С�����̫Զ
#define _INF_TAG_EPC_MEMORY_OVERRUN         0x31  //�����ı�ǩ�洢��Ԫ�����ڻ��ǩ�Ŀ����ֲ�֧��
#define _INF_TAG_EPC_OTHER_ERROR            0x32  //����δ����Ĵ���
#define _INF_TAG_EPC_HANDLE_ERROR           0x33  //������ǩʱ��ľ������
#define _INF_TAG_EPC_TAG_BUSY               0x34  //��ǩæ��������Ӧ��ǰ����


void WINAPI RFID_GetDLL_Ver(unsigned char * MainVersion,unsigned char * MinorVersion);

//=====================================================
unsigned short WINAPI RFID_DataCRC(IN UCHAR* bytearray,IN int arraylength);

BOOL WINAPI IsConnectedToReader(RFID_CLIENT* pClient);

BOOL WINAPI InitalRFIDTcpWSA(RFID_CLIENT* pClient);
void WINAPI CloseRFIDTcpWSA();
int GetRFIDTcpError();
	
apiStatus WINAPI RFID_Open(OUT RFID_CLIENT* pClient, IN char *param1, unsigned int param2);
apiStatus WINAPI RFID_Close(IN RFID_CLIENT* pClient,UCHAR RAddr);

#define UART_RS232_BAUD_SET  0
#define UART_RS232_BAUD_GET  1
#define UART_RS485_BAUD_SET  2
#define UART_RS485_BAUD_GET  3
apiStatus WINAPI RFID_SysBaudRate(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR UartPortOper, UCHAR *BaudRateIndex);

apiStatus WINAPI RFID_ReadAddrSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR ReaderAddr);
apiStatus WINAPI RFID_ReadAddrQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *ReaderAddr);

#define READER_NORMAL_TIMEOUT  1000
#define READER_FAST_TIMEOUT       100
 void WINAPI SetUartTimeOut(HANDLE &hCom,int TimeOut);
apiStatus WINAPI SetUartTimeOutEx(HANDLE hCom,int TimeOut);


#define _NET_TIMEOUT_SEND_NORMAL   500
#define _NET_TIMEOUT_RECV_NORMAL   3000
#define _NET_TIMEOUT_SEND_FAST       20
#define _NET_TIMEOUT_RECV_FAST        20
BOOL SetTcpClientTimeOut(MID_TCP_Client *pClient, int SendTimeOut, int RecvTimeOut);//2011-12-20
//*********************************************************************

typedef struct
{
	BYTE yTagID_Len;
	BYTE yTagID_Data[48];
	BYTE yTagID_AntNo;	
}*pTagRECstruct, gTagRECstruct;

//*********** Record��������**************
apiStatus WINAPI RFID_REC_Initialize(RFID_CLIENT* pClient,UCHAR RAddr);
apiStatus WINAPI RFID_REC_ClrAll(RFID_CLIENT* pClient,UCHAR RAddr);
apiStatus WINAPI RFID_REC_GetTagQuantity(RFID_CLIENT* pClient,UCHAR RAddr,UINT *TagCount);
apiStatus WINAPI RFID_REC_GetTagData(RFID_CLIENT* pClient, UCHAR RAddr, 
	 UCHAR InTagCount, UCHAR *FinalGetTagCount, UCHAR *TagData,UCHAR *TagByteLen,UCHAR *AntNo, bool bNeedClr);
apiStatus WINAPI RFID_REC_GetTagDataEx(RFID_CLIENT* pClient, UCHAR RAddr, 
	 UCHAR InTagCount, UCHAR *FinalGetTagCount, pTagRECstruct pTagRec, bool bNeedClr);//2011-12-15

apiStatus WINAPI RFID_REC_StorageGetTags(RFID_CLIENT* pClient, UCHAR RAddr, 
	 UCHAR InTagCount, UCHAR *FinalGetTagCount, pTagRECstruct pTagRec, bool bNeedClr);//2016-04-13

apiStatus WINAPI RFID_REC_ACK(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR TQuantity);

 //=============== ISO Tag Operation==================
apiStatus WINAPI RFID_ISO_Init(RFID_CLIENT* pClient,UCHAR RAddr);
apiStatus WINAPI RFID_ISO_SelectMode(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR SelectMode,UCHAR Address,UCHAR Mask,UCHAR IDValue[]);
apiStatus WINAPI RFID_ISO_SearchUID(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR* TagCount);
apiStatus WINAPI RFID_ISO_ReadWithUID(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR* TagID,UCHAR dataAddr,UCHAR OutData[]);
apiStatus WINAPI RFID_ISO_MultiTagRead(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR dataAddr,UCHAR *tagCount);
apiStatus WINAPI RFID_ISO_MTRVar(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR dataAddr,UCHAR dataLen,UCHAR *tagCount);
apiStatus WINAPI RFID_ISO_StartMTR(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR dataAddr);
apiStatus WINAPI RFID_ISO_SearchOneTag(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR *TagID,UCHAR *AntNo);
#define _OPER_6B_IDLE                 0
#define _OPER_6B_STR                   1
#define _OPER_6B_MTR                   2
apiStatus WINAPI RFID_ISO_FastSTR(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR StartAddr,UCHAR *RecordSum,UCHAR *TagData_Record);
apiStatus WINAPI RFID_ISO_FastAutoStart(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR StartAddr,UCHAR TagReadType);
apiStatus WINAPI RFID_ISO_FastStop(RFID_CLIENT* pClient,UCHAR RAddr);

apiStatus WINAPI RFID_ISO_Write(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR WrLen,UCHAR dataAddr,UCHAR tagData[]);
apiStatus WINAPI RFID_ISO_WriteWithUID(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR uWriteLenFlag,UCHAR *UID,UCHAR dataAddr,UCHAR tagData[]);
apiStatus WINAPI RFID_ISO_LockSet(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR Address);
apiStatus WINAPI RFID_ISO_LockSetWithUID(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR *UID,UCHAR Address);
apiStatus WINAPI RFID_ISO_LockQuery(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR Address,UCHAR* LockStatus);
apiStatus WINAPI RFID_ISO_LockQueryWithUID(RFID_CLIENT* pClient,UCHAR RAddr,UCHAR *UID,UCHAR Address,UCHAR* LockStatus);
apiStatus WINAPI RFID_ISO_EndOperation(RFID_CLIENT* pClient,UCHAR RAddr);

 //============Reader System Operation==================0
apiStatus WINAPI RFID_SysRFPwrOFF(RFID_CLIENT* pClient,UCHAR RAddr);
apiStatus WINAPI RFID_SysRFPwrON(RFID_CLIENT* pClient,UCHAR RAddr);
apiStatus WINAPI RFID_SysRFPwrSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR RF);
apiStatus WINAPI RFID_SysRFPwrQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR* RF);
#define _ANT_MAX_QUANTITY  4
apiStatus WINAPI RFID_SysRFPwrEx(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR* RF);  //2011-11-29
 
apiStatus WINAPI RFID_SysRFFreqSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR inFrequency);
apiStatus WINAPI RFID_SysRFFreqQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR* CurrentFreqNo, UCHAR* MaxFreqChannel);
apiStatus WINAPI RFID_SysRFFreqArea(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR OperationType, UCHAR *SysRF_Freq_Area);
apiStatus WINAPI RFID_SysRFFreqExtSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR uFreqOper, UCHAR uOperValue); //2010-06-08
apiStatus WINAPI RFID_SysRFFreqExtQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR uFreqOper, UCHAR *uOperValue); //2010-06-08
apiStatus WINAPI RFID_SysRFFreqPLLSetDirect(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR FreqArea, UCHAR *uPLLVer);//2016.02.27

apiStatus WINAPI RFID_SysRFANTQuantitySet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR inAntQuantity);
apiStatus WINAPI RFID_SysRFANTQuantityQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR* outAntQuantity);
apiStatus WINAPI RFID_SysRFCurWorkAntSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR CurrAntNo);
apiStatus WINAPI RFID_SysRFCurWorkAntQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR* CurrAntNo);
apiStatus WINAPI RFID_SysRFAntPolling(RFID_CLIENT* pClient, UCHAR RAddr);
apiStatus WINAPI RFID_SysRFAntPollingEnd(RFID_CLIENT* pClient, UCHAR RAddr);
apiStatus WINAPI RFID_SysRFAntWorkStatusSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR AntWorkStatus);
apiStatus WINAPI RFID_SysRFAntWorkStatusQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR* AntWorkStatus);

apiStatus WINAPI RFID_SysRFAntDetectEnSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR AntDetectEn);//2009-07-28
apiStatus WINAPI RFID_SysRFAntDetectEnGet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR* AntDetectEn);//2009-07-28

apiStatus WINAPI RFID_SysReset(RFID_CLIENT* pClient, UCHAR RAddr);
apiStatus WINAPI RFID_SysGetStatus(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *rStatus);
apiStatus WINAPI RFID_SysGetFirmwareVersion(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR* HWMajorVer,UCHAR* HWMinorVer,UCHAR* SWMajorVer,UCHAR* SWMinorVer);

apiStatus WINAPI RFID_SysWorkModeSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR WorkMode);
apiStatus WINAPI RFID_SysWorkModeQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR* WorkMode);

apiStatus WINAPI RFID_SysBufTypeSelSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR BufType);
apiStatus WINAPI RFID_SysBufTypeSelQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *BufType);

apiStatus WINAPI RFID_SysEncrpyt(RFID_CLIENT* pClient,UCHAR RAddr);
apiStatus WINAPI RFID_SysGetSN(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR SnType,UCHAR* info);

#define TAG_PROTOCOL_ISO6B   0x01
#define TAG_PROTOCOL_ISO6C   0x02
#define TAG_PROTOCOL_ISO6     0x03   //˫Э��
apiStatus WINAPI RFID_SysTagProtocolSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR TagProtocol);
apiStatus WINAPI RFID_SysTagProtocolQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR* TagProtocol);
apiStatus WINAPI RFID_SysTagInventoryMode(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *TagInvMode);

//===========================================================
#define GEN2_BANK_RESERVE			0x00	//GEN2 ��ǩϵͳ����������
#define GEN2_BANK_EPC				0x01	//GEN2 ��ǩEPC������
#define GEN2_BANK_TID				0x02	//GEN2 ��ǩTID������
#define GEN2_BANK_USER				0x03	//GEN2 ��ǩ�û�������

apiStatus WINAPI RFID_Gen2_STR(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR Point,UCHAR ReadWordLength, UCHAR MaskLength, UCHAR Mask[], UCHAR* RecvTagRecNum, UCHAR* RecvTagRecLen, UCHAR Data[]);
apiStatus WINAPI RFID_Gen2_STR_ID(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue,UCHAR MaskLength, UCHAR Mask[], UCHAR *TagCount, UCHAR *IDBitCount, UCHAR TagID[]);
apiStatus WINAPI RFID_Gen2_MTR(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR Point,UCHAR ReadLength,UCHAR MaskLength, UCHAR Mask[]);
apiStatus WINAPI RFID_Gen2_OrdinaryTagsRSSI(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR Point,UCHAR ReadLength,UCHAR MaskLength, UCHAR Mask[]);
apiStatus WINAPI RFID_Gen2_TempeTagRead(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR Point,UCHAR ReadLength,UCHAR MaskLength, UCHAR Mask[]);
apiStatus WINAPI RFID_Gen2_TempeTagRSSIRead(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR Point,UCHAR ReadLength,UCHAR MaskLength, UCHAR Mask[]);
apiStatus WINAPI RFID_Gen2_MoistureTagRead(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR Point,UCHAR ReadLength,UCHAR MaskLength, UCHAR Mask[]);
apiStatus WINAPI RFID_Gen2_MoistureTagRSSIRead(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR Point,UCHAR ReadLength,UCHAR MaskLength, UCHAR Mask[]);



apiStatus WINAPI RFID_Gen2_MTR_ID(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue,UCHAR MaskLength, UCHAR Mask[], UCHAR *TagCount);
apiStatus WINAPI RFID_Gen2_ReadWithID(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR WordAddr,UCHAR ReadWordLength,UCHAR MaskLength, UCHAR Mask[], UCHAR *RecvTagRecLen, UCHAR TagRecord[]);
apiStatus WINAPI RFID_Gen2_FastMTR(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR Point,UCHAR ReadLength,UCHAR MaskLength, UCHAR Mask[]);
apiStatus WINAPI RFID_Common_Send(RFID_CLIENT* pClient,UCHAR RAddr,unsigned char CMDH,unsigned char CMDL, unsigned char DataLenth,unsigned char *DataBuff);

apiStatus WINAPI RFID_Gen2_InitWrite(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue);
apiStatus WINAPI RFID_Gen2_Write(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR WordAddr,UCHAR ByteDataLen,UCHAR ByteData[]);
apiStatus WINAPI RFID_Gen2_WriteID(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR ByteDataLen, UCHAR TagNewID[]);
apiStatus WINAPI RFID_Gen2_WriteWithID(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR WordAddr,UCHAR ByteDataLen, UCHAR ByteData[], UCHAR IDbitLen,UCHAR UID[]);
apiStatus WINAPI RFID_Gen2_Lock(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Password[], UCHAR Select, UCHAR Action, UCHAR IDBitCount,UCHAR TagID[]);
apiStatus WINAPI RFID_Gen2_Kill(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Password[],UCHAR IDBitCount,UCHAR TagID[]);
apiStatus WINAPI RFID_Gen2_Erase(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR Bank,UCHAR Point,UCHAR EraseLength,UCHAR IDBitCount,UCHAR TagID[]);
apiStatus WINAPI RFID_Gen2_EraseID(RFID_CLIENT* pClient, UCHAR RAddr,UCHAR QValue, UCHAR EraseLength,UCHAR IDBitCount,UCHAR TagID[]);
apiStatus WINAPI RFID_Gen2_EndOperation(RFID_CLIENT* pClient,UCHAR RAddr);
 //=====================Comunication Operation ======================

apiStatus WINAPI RFID_ComPortSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR PortType, UCHAR PortProtocol);
apiStatus WINAPI RFID_ComPortQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *PortType,UCHAR *PortProtocol);
apiStatus WINAPI RFID_TimerSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR TimerPeriod, UCHAR TagReadStartAddr);
apiStatus WINAPI RFID_TimerQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *TimerPeriod,UCHAR *TagReadStartAddr);
apiStatus WINAPI RFID_WiegandPortSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR WiegandPort, UINT WiegandIntvl, UCHAR uOutNum);
apiStatus WINAPI RFID_WiegandPortQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *WiegandPort, UINT *WiegandIntvl,UCHAR *uOutNum);
apiStatus WINAPI RFID_WiegandPortSetEx(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR WiegandPulsePeriod, UCHAR WiegandPulseWidth);//2010-05-24
apiStatus WINAPI RFID_WiegandPortQueryEx(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *WiegandPulsePeriod, UCHAR *WiegandPulseWidth);//2010-05-24

apiStatus WINAPI RFID_TriggerSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR TriggerPortEn, UCHAR DelayTimeToStop, UCHAR nLevel);
apiStatus WINAPI RFID_TriggerQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *TriggerPortStatus, UCHAR *DelayTimeToStop, UCHAR *nLevel);

apiStatus WINAPI RFID_RS485Set(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR RS485Baud);
apiStatus WINAPI RFID_RS485Query(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *RS485Baud);
apiStatus WINAPI RFID_OutputTagCompareSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR compareEnable, UCHAR KeepCmpDealy);
apiStatus WINAPI RFID_OutputTagCompareQuery(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR *compareEnable,UCHAR *KeepCmpDealy);

apiStatus WINAPI RFID_UserAuthorPwdSet(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR inUserCode[]);

typedef struct
{
	unsigned char DeviceIPAddress[4];
	unsigned char DeviceSubNetMask[4];
	unsigned char DeviceGateWay[4];
	unsigned short DeviceIPPort;
	unsigned char DeviceMacAddr[6];
	
	unsigned char TargetIPAddress[4];
	unsigned short TargetIPPort;
}*pIP4vstruct, gIP4vstruct;
apiStatus WINAPI RFID_Sys_DeviceIPConfig(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, pIP4vstruct pDeviceIP); //20151128

typedef struct
{
	unsigned char WifiIPAddress[4];
	unsigned char WifiSubNetMask[4];
	unsigned char WifiGateWay[4];
	unsigned short WifiIPPort;
  unsigned char WifiMode; //AP or STA

}*pIP4vstruct_wifi,gIP4vstruct_wifi;

apiStatus WINAPI RFID_Sys_WifiIPConfig(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, pIP4vstruct_wifi pWifiIP); //20160317


//apiStatus WINAPI RFID_SysRFFreq(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *SysRF_Freq_Area);
#define _OPERATION_SET_PARAM  0x00
#define _OPERATION_GET_PARAM  0x01

apiStatus WINAPI RFID_SysBeepCtrl(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *BeepOnOff);
apiStatus WINAPI RFID_SysPollingTimeCtrl(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *PollingTime);
apiStatus WINAPI RFID_TagIDEncryptUnderPollingMode(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *EncryptFlag);//2011-01-08

//#define _EPC_STR_TYPE_EPC_ONLY   1    //2011-05-26   EPC STR ֻ��EPC
//#define _EPC_STR_TYPE_TID_ONLY   2    //2011-05-26   EPC STR ֻ��TID
//#define _EPC_STR_TYPE_EPC_TID     3     //2011-05-26   EPC STR ��EPC��TID

#define EPC_MENBANK_RFU            0
#define EPC_MENBANK_EPC            1
#define EPC_MENBANK_TID            2
#define EPC_MENBANK_USER          3
#define EPC_MENBANK_EPC_TID          4     //����ʵ�ʴ��ڵ�BANK    2011-11-15
#define EPC_MENBANK_EPC_USER          5   //����ʵ�ʴ��ڵ�BANK    2011-11-15

apiStatus WINAPI RFID_Sys_6COutFormat(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *EPC_TID_Flag, UCHAR *QV); //2011-11-15
apiStatus WINAPI RFID_Sys_6COutFormatA(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *EPC_PAR) ;
////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_BUF_SIZE 655350  
typedef struct
{
	unsigned long  Head_Pointer; //���ݽ��ջ�����ͷָ��
	unsigned long  Tail_Pointer; //���ݽ��ջ�����βָ��
	unsigned long  BufCount;    //��������ǩ����������
	//unsigned char DataLen;     //һ�����ݴ洢�ĳ��� >=1
	//unsigned int  MaxDataNum;  //�������Ǹ��洢��������ݵ���Ŀ
	unsigned char Buffer[MAX_BUF_SIZE];
}*pBUFstruct, gBUFstruct;
//extern gBUFstruct Respons4ReaderQueque;

apiStatus WINAPI InitialFastRead(pBUFstruct pRespons4ReaderQueque);
apiStatus WINAPI GetRecvDataToQue(RFID_CLIENT* pClient, pBUFstruct pRespons4ReaderQueque);

apiStatus  WINAPI GetFrame4Host(UCHAR RAddr, UCHAR *RxFsmState, UCHAR *RecvBuf, UCHAR *Index, UCHAR UART_RX);

///////////////////////////////////////////////////////////////
void WINAPI InitQueue(pBUFstruct pBuf);

unsigned char WINAPI IsFull(pBUFstruct pBuf);
unsigned char WINAPI IsEmpty(pBUFstruct pBuf);
void WINAPI Write2Queue(pBUFstruct pBuf,unsigned char *uData,unsigned int Len);
void WINAPI Read4Queue(pBUFstruct pBuf,unsigned char *uData,unsigned int Len);

//===================================================================================
apiStatus WINAPI RFID_Sys_GPIO_GPO(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *GPIO_StatusValue); //20170306
apiStatus WINAPI RFID_Sys_GPIO_GPI(RFID_CLIENT* pClient, UCHAR RAddr, UCHAR SetOrGet, UCHAR *GPIO_StatusValue); //20170306



#endif

