/*! @file
********************************************************************************
<PRE>
--------------------------------------------------------------------------------
�ļ���       : protocol.h
�ļ�ʵ�ֹ��� : ͨѶЭ��
����         : hujiang
�汾         : V1.0
��ע         :
--------------------------------------------------------------------------------
�޸ļ�¼ :
�� ��        �汾     �޸���              �޸�����
2013/4/15  1.0      hujiang              ԭʼ�汾
--------------------------------------------------------------------------------
</PRE>
*******************************************************************************/
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

/***********************RF Management MACRO************************************/
#define UDP_MANAGEMENT  0X10

#define TCP_MANAGEMENT  0X20

#define TELNET_MANAGEMENT  0X30

#define UDP_MGMNTIF_VERSION  0X01

#define UDP_GET 0X10

#define UDP_SET 0X20

#define UDP_RESP 0XC0


#define TIME_SLOT_NOTIFY         0x01
#define BATTERY_NOTIFY           0x02
#define TEMPERATURE_NOTIFY       0x03
#define ANCHOR_STARTUP_NOTIFY    0x04
#define MOTION_DATA_LEN           4


#define MAX_TAGID_NUM  6//֧��6��tag
#define MAX_ANCHOR_NUM 10//anchor���Ϊ10��
#define MAX_CURTAG_NUM 20//����tagID��Ŀ
struct WORKINFO
{
  //uint16 TagID[MAX_TAGID_NUM];//����tag--Ŀǰ����
  uint16 AnchorList[MAX_ANCHOR_NUM];//������anchor
  uint16 CurTagID[MAX_CURTAG_NUM];//20+20=40--������tag
};
typedef enum
{
    MGMNTIF_CMD_ANCHORID = 0x01,
    MGMNTIF_CMD_ANCHORIP,//
    MGMNTIF_CMD_ANCHOR_SERVERIP,
    MGMNTIF_CMD_ANCHORTXPWR,
    MGMNTIF_CMD_ANCHORSWVER,
    MGMNTIF_CMD_ANCHORHWVER,
    MGMNTIF_CMD_ANCHORREG,
    MGMNTIF_CMD_ANCHOR_INTERVAL,
    MGMNTIF_CMD_ANCHOR_MACADDR,
    MGMNTIF_CMD_ANCHOR_PORT,
    MGMNTIF_CMD_ANCHOR_REBOOT,
    MGMNTIF_CMD_ANCHOR_SUB_NET_MASK,
    MGMNTIF_CMD_ANCHOR_NET_GATE,
    MGMNTIF_CMD_ANCHOR_DHCP_EN,
    MGMNTIF_CMD_BOOT_ENTER,
    MGMNTIF_CMD_RANGING,
    MGMNTIF_CMD_TIME_SLOT,//0x11
    MGMNTIF_CMD_ENGINEERING_MODE,//0x12
    MGMNTIF_CMD_ANCHOR_TYPE,//
    MGMNTIF_CMD_ANCHOR_LIST_DEFAULT,
    MGMNTIF_CMD_ANCHOR_SYNC_WORD,
    MGMNTIF_CMD_STATION_NUMBER,//0x16
    MGMNTIF_CMD_STATION_SEQ,
    MGMNTIF_CMD_SENSOR_SLOT,//18
    MGMNTIF_ANCHOR_STATE,
    MGMNTIF_ANCHOR_HIGH = 0x20,
    NEARBY_ANCHOR_LIST = 0x21,//0x1d
    //SYSTEM_TAG_LIST,//0x1e
    CUR_TAGID_LIST = 0x22,//0x1f
    SER_BLINK_CNT = 0x23,
    RECV_REGION_NUM = 0x24,

    MGMNTIF_CMD_TAGSWVER =  0X81,
    MGMNTIF_CMD_TAGHWVER,
    MGMNTIF_CMD_TAG_ANCHORLIST,
    MGMNTIF_CMD_TAGTXPWR,
    MGMNTIF_CMD_TAGBAT,
    MGMNTIF_CMD_TAGTMP,
    MGMNTIF_CMD_TAGREG,
    MGMNTIF_CMD_TAG_INTERVAL,
    MGMNTIF_CMD_TAGEXTRA,
    MGMNTIF_CMD_TAG_SYNC_WORD,
    MGMNTIF_CMD_TAG_PWR_OFF
}FuctionCode;

/*
��ȡSN
��ȡ�ͺ�
��ȡ����Ӳ������
��ȡ������IP
��ȡ�������˿�
��ȡ����
��ȡ����״̬

��λ
���÷�����IP
���÷������˿�
���ù���״̬

�ͺ�
����Ӳ������
������IP
�������˿�
����
����״̬
�������
�����Ϣ
*/

typedef enum
{
    GET_SN = 0x01,
    GET_MODEL,
    GET_HARDWARE_INFO,
    GET_SERVER_IP,
    GET_SERVER_PORT,
    GET_QUANTITY,
    GET_WORK_STATUS,
    SET_REBOOT,
    SET_SERVER_IP,
    SET_SERVER_PORT,

    SET_WORK_STATUS = 0x17,
    RESP_WORK_STATUS = 0x87,

}fuction_code_t;

/******************************************************************************/

typedef int8(*SigleInsHander)(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area, uint16 sn,uint8 *uch_back_type);

typedef struct
{
  uint8 funcCode;
  SigleInsHander h;
}InsHander;

#pragma pack(1)
typedef struct ST_RfManageHead
{
  uint8 uch_Type;
  uint8 uch_Version;
  uint8 uch_OpCode;
  uint8 uch_FucCode;
  uint8 uin_Lengh;
}ptl_head_t;
#pragma pack()

#pragma pack(1)
typedef struct
{
  uint8    uch_type;
  uint32   u32_device_sn;
  uint8    uch_Lengh;
}ptl_frame_head_t;
#pragma pack()


uint16 ManageAndConfigureInterfaceAnalysis(void *in,uint16 len,void *out);


uint16 FrameExtraction(char *in,uint16 len,uint8 uch_link_id);
uint16 MakeFrame(uint16 uin_type,uint16 uin_version,uint8 uch_op,uint8 uch_funCode,uint8 uch_len,uint8* pData,void* out );

void SendChargingstatus(void );
void SendChargingOver(void );

void SendStart(void);

#endif //__PROTOCOL_H__
