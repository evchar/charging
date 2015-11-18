/*! @file
********************************************************************************
<PRE>
--------------------------------------------------------------------------------
�ļ���       : plt_rf.h
�ļ�ʵ�ֹ��� : ͨѶЭ��
����         : hujiang
�汾         : V1.0
��ע         :
--------------------------------------------------------------------------------
�޸ļ�¼ :
�� ��        �汾     �޸���              �޸�����
2013/4/27  1.0      hujiang              ԭʼ�汾
--------------------------------------------------------------------------------
</PRE>
*******************************************************************************/

#ifndef __PTL_RF_H__
#define __PTL_RF_H__

/***********************MACRO************************************/

//type
#define RF_DATA  0X55

#define RF_RANGING  0XAA


//opcode
#define RF_GET 0X10

#define RF_SET 0X20

#define RF_SET_ONLY 0X30

#define RF_RESP 0XC0

//ctrlcode
#if 0
#define RF_CMD_TAGSWVER 0x01

#define RF_CMD_TAGHWVER 0x02

#define RF_CMD_TAG_ANCHORLIST 0x03

#define RF_CMD_TAGTXPWR 0x04

#define RF_CMD_TAGBAT 0x05

#define RF_CMD_TAGTMP 0x06

#define RF_CMD_TAGREG 0x07

#define RF_CMD_TAG_INTERVAL 0x08

#define RF_CMD_TAGEXTRA  0x09

#define RF_BC_TAGSLOT  0x0A

#define RF_CMD_RANDACESS  0x0B

#define RF_BC_TAGINFO  0x0C

#define RF_GET_STATIC_TAGINFO 0X0D

#define RF_DYNAMIC_TAGINFO 0X0E

#define RF_SYNC_WORD 0x0F

#define RF_BC_SENSORSLOT 0x10
#endif

typedef enum
{
    RF_CMD_TAGSWVER = 0x01,

    RF_CMD_TAGHWVER,

    RF_CMD_TAG_ANCHORLIST,

    RF_CMD_TAGTXPWR,

    RF_CMD_TAGBAT,

    RF_CMD_TAGTMP,

    RF_CMD_TAGREG,

    RF_CMD_TAG_INTERVAL,

    RF_CMD_TAGEXTRA,

    RF_BC_TAGSLOT,//0x0a

    RF_CMD_RANDACESS,//0x0b

    RF_BC_TAGINFO,//0x0c

    RF_GET_STATIC_TAGINFO,//0x0d

    RF_DYNAMIC_TAGINFO,//0x0e

    RF_SYNC_WORD,

    RF_BC_SENSORSLOT,

    RF_TAG_PWR_OFF,

    RF_ANCHOR_PWR = 0x21,

    RF_ANCHOR_SWVER,

    RF_ANCHOR_HWVER,

    RF_ANCHOR_TYPE,

    RF_ANCHOR_SYNC,

    RF_ANCHOR_REBOOT,
    RF_SEND_RASLOT=0x50,//���ʱ϶
    RF_SEND_SESLOT,//������ʱ϶
    RF_WORKINFO,
    RF_ONLYRANGESLOT,
    RF_REP=0x55,//���ͱ���
    RF_RST,//�ر�anchor��RF����
    RF_USR,//�û�������ڹر�tag���߸���tagͬ����
    RF_TAG_RESET=0XA0,
    RF_TAG_WORKINFO,
}FunctionCode;



typedef int8(*AirSigleInsHander)(uint8 uch_op, void *pdata, uint8 uch_length,
                                            void *data_area,uint16 uin_src, uint16 sn);

typedef struct
{
    uint8 funcCode;
    AirSigleInsHander h;
}AirInsHander;

#pragma pack(1)
typedef struct
{
    uint8 timeSlotNum;
    uint16 tag_id;
}ST_BcTagSlot;
#pragma pack()

#pragma pack(1)
typedef struct
{
    uint8 period;
    uint8 slot;
    uint16 tag_id;
}ST_BcTagSensorSlot;
#pragma pack()

#pragma pack(1)
typedef struct
{
    uint16 uin_PacketNum;
    uint8  uch_Type;
    uint8  uch_OpCode;
    uint8  uch_CtrlCode;
    uint8  uch_Lengh;
}ST_AirMessageHead;
#pragma pack()

void SendMessageToTag(uint16 uin_dest,uint8 uch_len,void* pdata);

void QueueOutputAndSend(void);

uint8 AirMessageProcess(void *in,uint16 len,void *out,uint16 src);

void AnchorCenterBroadcast(void);

void AnchorCenterBroadcastWithTagInfo(void);

void AnchorCenterSensorSlotBroadcast(void);

void RemoveOneTagInfo(int8_t tag_nr);

void OccupyOneTagInfo(int8_t tag_nr,uint32 tag_id,ST_TagStaticInfo* ptagStaticInfo);
int8 getUnuseTagInfo(void);

uint8 getAccesstagNum(uint16* tagid);

bool IsAlreadyKnowTagID(uint32 tag_id);
//���Ͳ��ʱ϶
void Send_RangeSlot();
//���ʹ�����ʱ϶
void Send_SensorSlot();
/*
�������ܣ������û������Ҫ�ǹػ��Լ��Ǹ��л�����
���������buf:�������ݻ���
�����������
���ߣ�hehe
*/
void SendUsrCmd(uint8* buf);
/*
�������ܣ����͹�����Ϣ
�����������
�����������
���ߣ�hehe
*/
void Send_WorkInfo();
/*
�������ܣ�ֻ���ʹ���������
�����������
�����������
���ߣ�hehe
*/
void Send_RangeOnlySlot();
/*
�������ܣ�����tag�������(��Ҫ��ʱ����)
�����������
�����������
���ߣ�hehe
*/
void Send_WorkGroup();
void Send_AllTagList();//�������е�tag�б�
typedef struct 
{
    uint16 TagID;
    uint8 RSSI[2];
    uint8 Index;//������ǰ��Ҫ����ĸ�buffer
}TagInfoTypeDef;
typedef struct
{
    uint8 Index;//��ǰ���õ�index
    TagInfoTypeDef TagInfo[20];
}TagListTypeDef;
/*
�������ܣ�����tag�����
�����������
�����������
���ߣ�hehe
*/
void Send_TagRegion();
#endif






