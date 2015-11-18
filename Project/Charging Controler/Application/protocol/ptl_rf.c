#include "main.h"
#include "global_value.h"
#include "app.h"
#include "drv_i2c24cxx.h"
#include "protocol.h"
#include "ptl_rf.h"
#include <string.h>
#include "phy.h"
#include "user_lib.h"

extern MsgT downMsg;
extern AplMemT *apl;
//extern ST_ClientInfo g_clientInfo[CLIENT_INFO_MAX];
extern struct udp_pcb *UdpServerPcb;
extern struct ErrReport ErrRep;
/*
�������ܣ��������ݸ�tag
���������
�����������
�޸ģ�hehe
*/
void SendMessageToTag(uint16 uin_dest,uint8 uch_len,void* pdata)
{
        uin_dest = ntohs(uin_dest);
        memcpy (downMsg.data, (uint8_t*)pdata,uch_len);
        downMsg.len = uch_len;
        downMsg.addr[4] = uin_dest;//tag��ַ
        downMsg.addr[5] = uin_dest>>8;
        if(0xFFFF == uin_dest)
        {
            downMsg.addr[2] = 0xFF;
            downMsg.addr[3] = 0xFF;
            downMsg.addr[0] = 0xFF;
            downMsg.addr[1] = 0xFF;
        }
        else
        {
            downMsg.addr[2] = 0;
            downMsg.addr[3] = 0;
            downMsg.addr[0] = 0;
            downMsg.addr[1] = 0;
        }
        downMsg.prim    = PD_DATA_REQUEST;//���ݰ�
        downMsg.status  = 0x00;
        PDSap(&downMsg);
//        do{
//            //PHYPoll();
//            Send_Proc();
//            if(cnt++>250)
//            {
//                cnt=0;
//                RF_Init();
//                printf("rf_send_timeout\r\n");
//                break;
//            }   
//          }while(ntrxState != TxIDLE);
}


void QueueOutputAndSend(void)
{
#if 0
   uint8 auch_buffer[128];
   uint8 uch_length;
   uint16 uch_dest;

   if(!QueueDelete(auch_buffer,&uch_length, &uch_dest))
     return;
   SendMessageToTag(uch_dest,uch_length,auch_buffer);
#endif
}

bool IsAlreadyKnowTagID(uint32 tag_id)
{
    for(uint8 i=0; i<TAG_NUM_MAX; i++)
    {
        if(g_tagInfo[i].tag_id == tag_id)
            return 1;
    }
    return 0;
}

int8 getUnuseTagInfo(void)
{
    for(uint8 i=0; i<TAG_NUM_MAX; i++)
    {
        if(g_tagInfo[i].tag_id == 0)
            return i;
    }
    return -1;
}

uint8 getAccesstagNum(uint16* tagaccessid)
{
    uint8 tag_cnt = 0;
    for(uint8 i=0; i<TAG_NUM_MAX; i++)
    {
        if(g_tagInfo[i].tag_id == 0)continue;
        tagaccessid[tag_cnt++] = g_tagInfo[i].tag_id;
    }
    return tag_cnt;
}

void OccupyOneTagInfo(int8_t tag_nr,uint32 tag_id,ST_TagStaticInfo* ptagStaticInfo)
{
    if(tag_nr >= 0 && tag_nr < TAG_NUM_MAX)
    {
        g_tagInfo[tag_nr].tag_id = tag_id;
        g_tagInfo[tag_nr].anchor_list_updata = true;//������tag����Ҫ����anchorlist

        g_tagInfo[tag_nr].anchor_list[0] = g_anchor.DefaultList[0];//������anchorid,anchorlist�����������anchor�ĵ�ַ
        g_tagInfo[tag_nr].anchor_list[1] = g_anchor.DefaultList[1];//���������nearby anchor����
        g_tagInfo[tag_nr].anchor_list[2] = g_anchor.DefaultList[2];
        g_tagInfo[tag_nr].anchor_list[3] = g_anchor.DefaultList[3];
        g_tagInfo[tag_nr].anchor_list[4] = g_anchor.DefaultList[4];
        g_tagInfo[tag_nr].anchor_list[5] = g_anchor.DefaultList[5];
        g_tagInfo[tag_nr].anchor_list[6] = g_anchor.DefaultList[6];
        g_tagInfo[tag_nr].anchor_list[7] = g_anchor.DefaultList[7];
        g_tagInfo[tag_nr].anchor_list[8] = g_anchor.DefaultList[8];
        g_tagInfo[tag_nr].anchor_list[9] = g_anchor.DefaultList[9];

        g_anchorListByteCnt = 20;         //10��anchor
        g_tagInfo[tag_nr].uch_txPwr= 0xaa;//������
        g_tagInfo[tag_nr].uch_swVer= ptagStaticInfo->uch_swVer;
        g_tagInfo[tag_nr].uch_hwVer= ptagStaticInfo->uch_hwVer;
        g_tagInfo[tag_nr].uch_ranging_freq = 0;
        g_tagInfo[tag_nr].temperature = ptagStaticInfo->uch_temperature;
        g_tagInfo[tag_nr].bat= ptagStaticInfo->uch_bat;
    }
}

void RemoveOneTagInfo(int8_t tag_nr)
{
    if(tag_nr >= 0)
    {
        //RemoveOneTagTimeSlot(g_tagInfo[tag_nr].tag_id);//ɾ��ĳ������TAGռ�õ�ʱ϶��
        g_tagInfo[tag_nr].tag_id = 0;
        g_tagInfo[tag_nr].uch_hwVer = 0;
        g_tagInfo[tag_nr].uch_swVer = 0;
        g_tagInfo[tag_nr].bat= 0;
        g_tagInfo[tag_nr].anchor_list_updata = false;

        //�ڴ˴����ɾ��tag ��������Ϣ����
    }
}
/*
�������ܣ�tagϣ������ʱ϶
�������;
���������
�޸ģ�hehe
*/
static int8 RfTagRandAccess(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    uint16 uin_tagID;
    //ST_TagStaticInfo tagStaticInfo;
    if(RF_SET_ONLY == uch_op)
    {
        if(sizeof(ST_TagStaticInfo)+sizeof(uin_tagID) == uch_length)
        {
           uin_tagID = *(uint16_t*)pdata;
           
#if 0
           tagStaticInfo = *(ST_TagStaticInfo*)((uint16*)pdata+1);
           if(!IsAlreadyKnowTagID(*(uint16_t*)pdata))//�µ�tag?
           {
              OccupyOneTagInfo(getUnuseTagInfo(),uin_tagID,&tagStaticInfo);//ע��tag��Ϣ
              uin_tagID = htons(uin_tagID);
           }
           else
           {
                uin_tagID = htons(uin_tagID);
           }
#endif
           SendNotifyFrame(TIME_SLOT_NOTIFY,sizeof(uin_tagID),(uint8*)&uin_tagID);
//           
//           uin_tagID = htons(uin_tagID);
//           buf[0] = uin_tagID;
//           buf[1] = uin_tagID>>8;
//           Send_NotifyMsg(buf);
        }
        return -1;//do not need respone
    }
    else
    {
        return -1;//do not need respone
    }

}
#if 0
uint8 LoadBuffer[50];

static int8 RfRespTagTxPWR(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    struct pbuf *qq = NULL;

    if(RF_RESP == uch_op)
    {
       if(uch_length == sizeof(g_tagInfo[0].uch_txPwr))
       {

           for(uint8_t i=0; i<TAG_NUM_MAX;i++)
           {
                if(memcmp((uint8*)&g_tagInfo[i].tag_id,(uint8*)&uin_src,2) == 0)
                {
                     g_tagInfo[i].uch_txPwr= *(uint8*)pdata;

                     memcpy(LoadBuffer + sizeof(ST_RfManageHead),
                                                    &g_tagInfo[i].uch_txPwr,
                                                            sizeof(g_tagInfo[0].uch_txPwr));
                     for(uint8 j=0; j<CLIENT_INFO_MAX; j++)
                     {
                        if(g_clientInfo[j].SN == sn)
                        {
                            qq = pbuf_alloc(PBUF_TRANSPORT, sizeof(ST_RfManageHead)
                                                     + sizeof(g_tagInfo[0].uch_txPwr)+4,
                                                                                    PBUF_RAM);
                            qq->payload = LoadBuffer;

                            MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,
                                                            g_clientInfo[j].uch_FucCode,
                                                                sizeof(g_tagInfo[0].uch_txPwr),
                                                                                 NULL,LoadBuffer);

                            udp_sendto(UdpServerPcb, qq, &g_clientInfo[j].destAddr,
                                                                            g_clientInfo[j].port);
                            pbuf_free(qq);
                        }
                     }
                     break;
                }
            }
       }
    }
    return 0;
}
#endif
static int8 RfRespTagSwVer(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    if(RF_RESP == uch_op)
    {
       if(uch_length == sizeof(g_tagInfo[0].uch_swVer))
       {

           for(uint8_t i=0; i<TAG_NUM_MAX;i++)
           {
                if(memcmp((uint8*)&g_tagInfo[i].tag_id,(uint8*)&uin_src,2) == 0)
                {
                     g_tagInfo[i].uch_swVer= *(uint8*)pdata;
                     break;
                }
            }
       }
    }
    return 0;
}

static int8 RfRespTagHwVer(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    if(RF_RESP == uch_op)
    {
       if(uch_length == sizeof(g_tagInfo[0].uch_hwVer))
       {

           for(uint8_t i=0; i<TAG_NUM_MAX;i++)
           {
                if(memcmp((uint8*)&g_tagInfo[i].tag_id,(uint8*)&uin_src,2) == 0)
                {
                     g_tagInfo[i].uch_hwVer= *(uint8*)pdata;
                     break;
                }
            }
       }
    }
    return 0;
}

static int8 RfRespTagBat(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    if(RF_RESP == uch_op)
    {
       if(uch_length == sizeof(g_tagInfo[0].bat))
       {

           for(uint8_t i=0; i<TAG_NUM_MAX;i++)
           {
                if(memcmp((uint8*)&g_tagInfo[i].tag_id,(uint8*)&uin_src,2) == 0)
                {
                     g_tagInfo[i].bat= *(uint8*)pdata;
                     break;
                }
            }
       }
    }
    return 0;
}

static int8 RfRespTagTmp(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    if(RF_RESP == uch_op)
    {
       if(uch_length == sizeof(g_tagInfo[0].temperature))
       {

           for(uint8_t i=0; i<TAG_NUM_MAX;i++)
           {
                if(memcmp((uint8*)&g_tagInfo[i].tag_id,(uint8*)&uin_src,2) == 0)
                {
                     memcpy((uint8*)&g_tagInfo[i].temperature,(uint8*)pdata,2);
                     break;
                }
            }
       }
    }
    return 0;
}

static int8 RfRespTagStaticInfo(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    if(RF_RESP == uch_op)
    {
       if(uch_length == sizeof(ST_TagStaticInfo))
       {

           for(uint8_t i=0; i<TAG_NUM_MAX;i++)
           {
                if(memcmp((uint8*)&g_tagInfo[i].tag_id,(uint8*)&uin_src,2) == 0)
                {
                     //g_tagInfo[i].temperature= *(uint8*)pdata;
                     //memcpy((uint8*)&g_tagInfo[i].temperature,(uint8*)pdata,2);
                     g_tagInfo[i].uch_swVer = *(uint8*)pdata;
                     g_tagInfo[i].uch_hwVer = *((uint8*)pdata+1);
                     g_tagInfo[i].uch_txPwr = *((uint8*)pdata+2);
                     break;
                }
            }
       }
    }
    return 0;
}

extern uint16 altitude_tagid;
extern int8 altitude_low;
extern int8 altitude_high;
extern int8 altitude_delta;
static int8 RfRespTagDynamicInfo(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    ST_SensorData *st_sensor_data;
    ST_SensorData motion_data_air;
    ST_MotionSensorHead motion_data_udp;
    
    if(RF_SET_ONLY == uch_op)
    {
      st_sensor_data = (ST_SensorData*)pdata;
      
      motion_data_udp.uch_Type = UDP_MOTION_TYPE;
      motion_data_udp.AnchorID = htons(g_anchor.uin_anchor_id);
      motion_data_udp.TagID = htons(uin_src);
      motion_data_udp.uin_SN = sn;

      motion_data_udp.TimeStamp = htonl(hwclock());
      motion_data_udp.uin_Lengh = 0;
      
      motion_data_udp.altitude = st_sensor_data->altitude;          
      motion_data_udp.delta_altitude = st_sensor_data->delta_altitude;
      motion_data_udp.uch_temperature[0] = st_sensor_data->uch_temperature[0];
      motion_data_udp.uch_temperature[1] = st_sensor_data->uch_temperature[1];
      
      motion_data_udp.uch_voltage = motion_data_air.uch_voltage = st_sensor_data->uch_voltage;
      motion_data_udp.uin_compass = st_sensor_data->uin_compass;
      
      for(uint8 i=0; i<4; i++)
      {
        motion_data_udp.motion_data[i].uch_grade = st_sensor_data->motion_data[i].grade;
        motion_data_air.motion_data[i].orientation = st_sensor_data->motion_data[i].orientation;
      }
      
      for(uint8 i=0; i<4; i++)
      {
        motion_data_udp.motion_data[i].uin_orientation = htons(motion_data_air.motion_data[i].orientation);;
      }
      memcpy(motion_data_udp.motion_data_xor,st_sensor_data->motion_data_xor,8);
      
      MotionSensorDataSend(0,NULL,&motion_data_udp);

    }
    return -1;
}

#if 0
static int8 RfRespAnchorTxPWR(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    struct pbuf *qq = NULL;

    if(RF_RESP == uch_op)
    {
        if(uch_length == sizeof(g_anchor.uch_txPwr))
        {

            memcpy(LoadBuffer + sizeof(ST_RfManageHead),
                                                    (uint8*)pdata,
                                                            sizeof(g_anchor.uch_txPwr));
            for(uint8 j=0; j<CLIENT_INFO_MAX; j++)
            {
                if(g_clientInfo[j].SN == sn)
                {
                    qq = pbuf_alloc(PBUF_TRANSPORT, sizeof(ST_RfManageHead)+
                                                sizeof(g_anchor.uch_txPwr)+4,
                                                PBUF_RAM);
                    qq->payload = LoadBuffer;

                    MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,
                                                     g_clientInfo[j].uch_FucCode,
                                                                sizeof(g_anchor.uch_txPwr),
                                                                                 NULL,LoadBuffer);

                    udp_sendto(UdpServerPcb, qq, &g_clientInfo[j].destAddr,
                                                                g_clientInfo[j].port);
                    pbuf_free(qq);
                }
            }
       }
       else if(uch_length == 0)
       {
           for(uint8 j=0; j<CLIENT_INFO_MAX; j++)
           {
               if(g_clientInfo[j].SN == sn)
               {
                   qq = pbuf_alloc(PBUF_TRANSPORT, sizeof(ST_RfManageHead)+4,PBUF_RAM);

                   qq->payload = LoadBuffer;

                   MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,
                                                 g_clientInfo[j].uch_FucCode,
                                                                0, NULL,LoadBuffer);

                   udp_sendto(UdpServerPcb, qq, &g_clientInfo[j].destAddr,
                                                               g_clientInfo[j].port);
                   pbuf_free(qq);
               }
           }
       }
    }
    return 0;
}


static int8 RfRespAnchorSwVer(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    struct pbuf *qq = NULL;

    if(RF_RESP == uch_op)
    {
        if(uch_length == sizeof(g_anchor.uch_swVer))
        {

            memcpy(LoadBuffer + sizeof(ST_RfManageHead),
                                                    (uint8*)pdata,
                                                            sizeof(g_anchor.uch_swVer));
            for(uint8 j=0; j<CLIENT_INFO_MAX; j++)
            {
                if(g_clientInfo[j].SN == sn)
                {
                    qq = pbuf_alloc(PBUF_TRANSPORT, sizeof(ST_RfManageHead)+
                                                sizeof(g_anchor.uch_swVer)+4,
                                                PBUF_RAM);
                    qq->payload = LoadBuffer;

                    MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,
                                                     g_clientInfo[j].uch_FucCode,
                                                                sizeof(g_anchor.uch_swVer),
                                                                                 NULL,LoadBuffer);

                    udp_sendto(UdpServerPcb, qq, &g_clientInfo[j].destAddr,
                                                                g_clientInfo[j].port);
                    pbuf_free(qq);
                }
            }
       }
    }
    return 0;
}

static int8 RfRespAnchorHwVer(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    struct pbuf *qq = NULL;

    if(RF_RESP == uch_op)
    {
        if(uch_length == sizeof(g_anchor.uch_hwVer))
        {

            memcpy(LoadBuffer + sizeof(ST_RfManageHead),
                                                    (uint8*)pdata,
                                                            sizeof(g_anchor.uch_hwVer));
            for(uint8 j=0; j<CLIENT_INFO_MAX; j++)
            {
                if(g_clientInfo[j].SN == sn)
                {
                    qq = pbuf_alloc(PBUF_TRANSPORT, sizeof(ST_RfManageHead)+
                                                sizeof(g_anchor.uch_hwVer)+4,
                                                PBUF_RAM);
                    qq->payload = LoadBuffer;

                    MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,
                                                     g_clientInfo[j].uch_FucCode,
                                                                sizeof(g_anchor.uch_hwVer),
                                                                                 NULL,LoadBuffer);

                    udp_sendto(UdpServerPcb, qq, &g_clientInfo[j].destAddr,
                                                                g_clientInfo[j].port);
                    pbuf_free(qq);
                }
            }
       }
    }
    return 0;
}

static int8 RfRespAnchorType(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    struct pbuf *qq = NULL;

    if(RF_RESP == uch_op)
    {
        if(uch_length == sizeof(g_anchor.uch_anchorType))
        {

            memcpy(LoadBuffer + sizeof(ST_RfManageHead),
                                                    (uint8*)pdata,
                                                            sizeof(g_anchor.uch_anchorType));
            for(uint8 j=0; j<CLIENT_INFO_MAX; j++)
            {
                if(g_clientInfo[j].SN == sn)
                {
                    qq = pbuf_alloc(PBUF_TRANSPORT, sizeof(ST_RfManageHead)+
                                                sizeof(g_anchor.uch_anchorType)+4,
                                                PBUF_RAM);
                    qq->payload = LoadBuffer;

                    MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,
                                                     g_clientInfo[j].uch_FucCode,
                                                                sizeof(g_anchor.uch_anchorType),
                                                                                 NULL,LoadBuffer);

                    udp_sendto(UdpServerPcb, qq, &g_clientInfo[j].destAddr,
                                                                g_clientInfo[j].port);
                    pbuf_free(qq);
                }
            }
       }
    }
    return 0;
}


static int8 RfRespAnchorSYNC(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    struct pbuf *qq = NULL;

    if(RF_RESP == uch_op)
    {
        if(uch_length == sizeof(g_anchor.uch_syncWord))
        {

            memcpy(LoadBuffer + sizeof(ST_RfManageHead),
                                                    (uint8*)pdata,
                                                            sizeof(g_anchor.uch_syncWord));
            for(uint8 j=0; j<CLIENT_INFO_MAX; j++)
            {
                if(g_clientInfo[j].SN == sn)
                {
                    qq = pbuf_alloc(PBUF_TRANSPORT, sizeof(ST_RfManageHead)+
                                                sizeof(g_anchor.uch_syncWord)+4,
                                                PBUF_RAM);
                    qq->payload = LoadBuffer;

                    MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,
                                                     g_clientInfo[j].uch_FucCode,
                                                                sizeof(g_anchor.uch_syncWord),
                                                                                 NULL,LoadBuffer);

                    udp_sendto(UdpServerPcb, qq, &g_clientInfo[j].destAddr,
                                                                g_clientInfo[j].port);
                    pbuf_free(qq);
                }
            }
        }
        if(uch_length == 0)
        {
            for(uint8 j=0; j<CLIENT_INFO_MAX; j++)
            {
                if(g_clientInfo[j].SN == sn)
                {
                    qq = pbuf_alloc(PBUF_TRANSPORT, sizeof(ST_RfManageHead)+4, PBUF_RAM);
                    qq->payload = LoadBuffer;
                    MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,
                                                     g_clientInfo[j].uch_FucCode, 0,
                                                                           NULL,LoadBuffer);
                    udp_sendto(UdpServerPcb, qq, &g_clientInfo[j].destAddr,
                                                                g_clientInfo[j].port);
                    pbuf_free(qq);
                }
            }
       }
    }
    return 0;
}

static int8 RfRespAnchorReboot(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    struct pbuf *qq = NULL;

    if(RF_RESP == uch_op)
    {
        if(uch_length == 0)
        {
            for(uint8 j=0; j<CLIENT_INFO_MAX; j++)
            {
                if(g_clientInfo[j].SN == sn)
                {
                    qq = pbuf_alloc(PBUF_TRANSPORT, sizeof(ST_RfManageHead)+4, PBUF_RAM);

                    qq->payload = LoadBuffer;

                    MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,
                                                     g_clientInfo[j].uch_FucCode,
                                                                       0,
                                                                         NULL,LoadBuffer);

                    udp_sendto(UdpServerPcb, qq, &g_clientInfo[j].destAddr,
                                                                     g_clientInfo[j].port);
                    pbuf_free(qq);
                }
            }
       }
    }
    return 0;
}
#endif

/*
�������ܣ���λϵͳ/������û�����
���������
���������
���ߣ�hehe 
*/
static int8 RfResetSystem(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    if(RF_SET_ONLY == uch_op)
    {
        //if(uch_length == 0)
        {
            NVIC_SystemReset();//ǿ�и�λ
            return 0;
        }
    }
    return 0;
}
/*
�������ܣ����յ�tag�ı���
���������
���������
���ߣ�hehe 
*/
static int8 RfTagReport(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area,uint16 uin_src, uint16 sn)
{
    if(RF_SET_ONLY == uch_op)
    {
        if(*(uint8*)pdata==1)
        {//tagУ׼ʧ��
//            ErrRep.ErrClass=REP_TAG;
//            ErrRep.ErrValue=TAG_SENSOR_ERR;
//            ErrRep.ErrValid=1;
//            ErrRep.TagID=0;
//            Send_ErrMsg();
            return -1;
        }
        else if(*(uint8*)pdata==2)
        {//tag��������
//            ErrRep.ErrClass=REP_TAG;
//            ErrRep.ErrValue=TAG_RESET_ERR;
//            ErrRep.ErrValid=1;
//            ErrRep.TagID=uin_src;
//            Send_ErrMsg();
            return -1;
        }
    }
    return -1;
}
/***********************************************************************************************
ָ����
***********************************************************************************************/
const AirInsHander AirInsTable[] =
{
    {RF_CMD_RANDACESS,          RfTagRandAccess},
    {RF_CMD_TAGSWVER,           RfRespTagSwVer},
    {RF_CMD_TAGHWVER,           RfRespTagHwVer},
   // {RF_CMD_TAGTXPWR,           RfRespTagTxPWR},
    {RF_CMD_TAGBAT,             RfRespTagBat},
    {RF_CMD_TAGTMP,             RfRespTagTmp},
    {RF_GET_STATIC_TAGINFO,     RfRespTagStaticInfo},
    {RF_DYNAMIC_TAGINFO,        RfRespTagDynamicInfo},
    {RF_RST,                    RfResetSystem},
    {RF_REP,                    RfTagReport},
   // {RF_ANCHOR_PWR,             RfRespAnchorTxPWR},
   // {RF_ANCHOR_SWVER,           RfRespAnchorSwVer},
   // {RF_ANCHOR_HWVER,           RfRespAnchorHwVer},
   // {RF_ANCHOR_TYPE,            RfRespAnchorType},
   // {RF_ANCHOR_SYNC,            RfRespAnchorSYNC},
    //{RF_ANCHOR_REBOOT,          RfRespAnchorReboot}
};
uint8 AirMessageProcess(void *in,uint16 len,void *out,uint16 src)
{
    ST_AirMessageHead *h = in;
    int8 back;

    if(RF_DATA != h->uch_Type)
        return 0;

    for (uint8 i = 0; i < sizeof(AirInsTable) / sizeof(AirInsHander); i++)
    {
        if ((h->uch_CtrlCode) == AirInsTable[i].funcCode)
        {
            if (AirInsTable[i].h != NULL)//����ָ����
            {
                back = (*AirInsTable[i].h)(h->uch_OpCode, h + 1, h->uch_Lengh,(uint8 *)out + sizeof(ST_AirMessageHead), src, h->uin_PacketNum);
                if (back < 0)
                    return 0;
                //return MakeFrame(UDP_MANAGEMENT,UDP_MGMNTIF_VERSION,UDP_RESP,h->uch_FucCode,back,NULL,out);
            }
            return 0;
        }
    }
    return 0;

}
/*
�������ܣ����ʱ϶����
�����������
�����������
�޸ģ�hehe
*/
void Send_AnchorList()
{
    uint8 buffer[100];
    ST_AirMessageHead *ah = (ST_AirMessageHead*)buffer;
    ah->uin_PacketNum = 0x7777;
    ah->uch_CtrlCode = RF_CMD_TAG_ANCHORLIST;
    ah->uch_OpCode = RF_SET_ONLY;
    ah->uch_Type = RF_DATA;
    ah->uch_Lengh = 20;//10���ֽ�
    memcpy((uint8*)(ah+1),(uint8*)(g_anchor.DefaultList),20);
    SendMessageToTag(0xFFFF,sizeof(ST_AirMessageHead)+ah->uch_Lengh,buffer);//�㲥����
}
/*
�������ܣ����ʱ϶����
�����������
�����������
�޸ģ�hehe
*/
uint16 AllTag[20];
void Send_AllTagList()
{
    uint8 buffer[90];
    ST_AirMessageHead *ah = (ST_AirMessageHead*)buffer;
    ah->uin_PacketNum = 0x7777;
    ah->uch_CtrlCode = 0x80;
    ah->uch_OpCode = RF_SET;
    ah->uch_Type = RF_DATA;
    ah->uch_Lengh = 40;//20��tag
    memcpy((uint16*)(ah+1),AllTag,40);
    SendMessageToTag(0xFFFF,sizeof(ST_AirMessageHead)+ah->uch_Lengh,buffer);//�㲥����
}
/*
�������ܣ����Ͳ��ʱ϶
�����������
�����������
���ߣ�hehe
*/
void Send_RangeSlot()
{
//    uint8 buf[73];//72
//    memset(buf,0,73);
//    buf[0]=SlotNum++;//ʱ϶���
//    buf[1]=0x00;
//    buf[2]=RF_DATA;
//    buf[3]=RF_SET_ONLY;
//    buf[4]=RF_SEND_RASLOT;
//    buf[5] = 67;//66
//    memcpy(&buf[6],(uint8*)(apl->RangeSlot),46);//Ŀǰ�ݶ�23��ʱ϶
//    if(SysFlag.Assist!=ANCHOR_ASSIST)
//      memcpy((uint16*)&buf[52],(uint16*)(g_anchor.DefaultList),20);//����anchor�ĵ�ַ��ʱ��֡�з���
//    else
//      memcpy((uint16*)&buf[52],0,20);//����anchor�ĵ�ַ��ʱ��֡�з���
//    buf[72]=SysFlag.AbsHigh;//�ž��Ը߶�
//    SendMessageToTag(0xFFFF,73,buf);//�㲥����-72
    //memset(buf,0,72);
}

//struct WorkCell WorkBuf[20];//
/*
�������ܣ�����tag�����
�����������
�����������
���ߣ�hehe
*/
void Send_TagRegion()
{//3820=60
//    uint8 buf[73];//72
//    memset(buf,0,73);
//    buf[0]=SysFlag.UniqueCnt;//ʱ϶���
//    buf[1]=0xaa;
//    buf[2]=RF_DATA;
//    buf[3]=RF_SET_ONLY;
//    buf[4]=RF_TAG_WORKINFO;
//    buf[5] = 60;//60
//    memcpy(&buf[6],(uint8*)(WorkBuf),SysFlag.ValidTagCnt);
//    SendMessageToTag(0xFFFF,66,buf);//�㲥����
}
#ifdef TEST
extern struct WORKINFO *WorkInfo;
extern struct WORKINFO  WorkInfoBuf;
/*
�������ܣ����͹�����Ϣ
�����������
�����������
���ߣ�hehe
*/
void Send_WorkInfo()
{
    uint8 buf[73];//72
    memset(buf,0,73);
    buf[0]=SlotNum++;//ʱ϶���
    buf[1]=0xaa;
    buf[2]=RF_DATA;
    buf[3]=RF_SET_ONLY;
    buf[4]=RF_WORKINFO;
    buf[5] = 61;//66
    memcpy(&buf[6],(uint8*)(WorkInfoBuf.CurTagID),40);//��ǰtagID
    //������anchor�б�
    memcpy((uint16*)&buf[6+40],(uint16*)(WorkInfoBuf.AnchorList),sizeof(WorkInfoBuf.AnchorList));
    buf[66]=SysFlag.UniqueCnt;//CNT
    SendMessageToTag(0xFFFF,67,buf);//�㲥����-72
    //memset(buf,0,72);
}
/*
�������ܣ�ֻ���Ͳ��ʱ϶(��Ҫ��ʱ����)
�����������
�����������
���ߣ�hehe
*/
void Send_RangeOnlySlot()
{
    uint8 buf[73];//72
    memset(buf,0,73);
    buf[0]=0x88;//ʱ϶���
    buf[1]=0xaa;
    buf[2]=RF_DATA;
    buf[3]=RF_SET_ONLY;
    buf[4]=RF_ONLYRANGESLOT;
    buf[5] = 46;//66
    memcpy(&buf[6],(uint8*)(apl->RangeSlot),46);//Ŀǰ�ݶ�23��ʱ϶
    SendMessageToTag(0xFFFF,52,buf);//�㲥����-72
    //memset(buf,0,72);
}
typedef struct
{
    uint8 GroupNum;
    uint16 TagID[4];//�ܹ���5�飬
}WorkGroup;
WorkGroup Grp1,Grp2,Grp3,Grp4,Grp5;
/*
�������ܣ�����tag�������(��Ҫ��ʱ����)
�����������
�����������
���ߣ�hehe
*/
void Send_WorkGroup()
{
    uint8 buf[73];//72
    memset(buf,0,73);
    buf[0]=0x88;//ʱ϶���
    buf[1]=0xaa;
    buf[2]=RF_DATA;
    buf[3]=RF_SET_ONLY;
    buf[4]=RF_ONLYRANGESLOT;
    buf[5] = 46;//66
    memcpy(&buf[6],(uint8*)&Grp1,sizeof(WorkGroup));//��һ��
    memcpy(&buf[6]+9,(uint8*)&Grp2,sizeof(WorkGroup));//�ڶ���
    memcpy(&buf[6]+18,(uint8*)&Grp3,sizeof(WorkGroup));//������
    memcpy(&buf[6]+27,(uint8*)&Grp4,sizeof(WorkGroup));//������
    memcpy(&buf[6]+36,(uint8*)&Grp5,sizeof(WorkGroup));//������
    SendMessageToTag(0xFFFF,52,buf);//�㲥����-72
}
#endif

/*
�������ܣ�������ʱ϶����
�����������
�����������
�޸ģ�hehe
*/
void Send_SensorSlot(void)
{
//    uint8 buff[86];
//    memset(buff,0,86);
//    buff[0]=0x88;
//    buff[1]=0x88;
//    buff[2]=RF_DATA;
//    buff[3]=RF_SET_ONLY;
//    buff[4]=RF_SEND_SESLOT;
//    buff[5] = 80;
//    //buff[6]=0xb6;
//    //buff[7]=0x00;//ʵ������0x00b6
//    memcpy(&buff[6],apl->SensorDataSlotTable,80);
//    SendMessageToTag(0xFFFF,86,buff);//�㲥����
}



/*
�������ܣ������û������Ҫ�ǹػ��Լ��Ǹ��л�����
���������buf:�������ݻ���
�����������
���ߣ�hehe
*/
void SendUsrCmd(uint8* buf)
{
    uint8 buff[32];
    ST_AirMessageHead *ah = (ST_AirMessageHead*)buff;
    ah->uin_PacketNum = 0x8888;
    ah->uch_CtrlCode = buf[0];
    ah->uch_OpCode = RF_SET_ONLY;//;RF_SET_ONLY
    ah->uch_Type = RF_DATA;
    ah->uch_Lengh = 3;//Ĭ��2����ֵ
    memcpy((uint8*)(ah+1),buf+1,3);//ǰ������ID��1���Ǿ��������
    SendMessageToTag(0xFFFF,sizeof(ST_AirMessageHead)+ah->uch_Lengh,buff);//�㲥����
}


void AnchorCenterBroadcast(void)
{
    uint8 buffer[128];
    ST_BcTagSlot tag_timeslot;
    uint8 uch_tag_cnt = 0;
    uint8 slot_number = 0;
    uint8 pre_slot_number = 0;
    int8 timeslot_buffer[24]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};//���12��ʱ϶
    ST_AirMessageHead *ah = (ST_AirMessageHead*)buffer;
    ah->uin_PacketNum = 0x8888;
    ah->uch_CtrlCode = RF_BC_TAGSLOT;
    ah->uch_OpCode = RF_SET_ONLY;
    ah->uch_Type = RF_DATA;
    ah->uch_Lengh = 0;
    memcpy((uint8*)(ah+1),(uint8*)(&tagcount),sizeof(tagcount));         
    for(uint8 i=0; i<tagcount; i++)
    {
        if(taglist[i] != 0)
        {
            uch_tag_cnt++;//���ν����TAG����
            tag_timeslot.tag_id = taglist[i];
            //slot_number = getTimeSlot(timeslot_buffer,step,tag_timeslot.tag_id);
            tag_timeslot.timeSlotNum = slot_number;
            tag_timeslot.tag_id = htons(tag_timeslot.tag_id);
            memcpy((uint8*)((uint8*)(ah+1)+sizeof(tagcount)+i*sizeof(ST_BcTagSlot)+pre_slot_number),(uint8*)(&tag_timeslot),sizeof(ST_BcTagSlot));
            memcpy((uint8*)((uint8*)(ah+1)+sizeof(tagcount)+i*sizeof(ST_BcTagSlot)+pre_slot_number+sizeof(ST_BcTagSlot)),timeslot_buffer,slot_number);
            pre_slot_number += slot_number;
            ah->uch_Lengh += tag_timeslot.timeSlotNum;
        }
    }
    ah->uch_Lengh += uch_tag_cnt*sizeof(ST_BcTagSlot)+sizeof(tagcount);//����Э��ͷ����
    SendMessageToTag(0xFFFF,sizeof(ST_AirMessageHead)+ah->uch_Lengh,buffer);//�㲥����
    memset(buffer,0,128);
}
/*
�������ܣ����ʹ�����ʱ϶
�����������
�����������
���ߣ�hehe
*/
void AnchorCenterSensorSlotBroadcast()
{
    uint8 buffer[128];
    //ST_BcTagSensorSlot tag_sensor_slot;
    uint8 uch_tag_cnt = 0;
    ST_AirMessageHead *ah = (ST_AirMessageHead*)buffer;
    ah->uin_PacketNum = 0x8888;
    ah->uch_CtrlCode = RF_BC_SENSORSLOT;
    ah->uch_OpCode = RF_SET_ONLY;
    ah->uch_Type = RF_DATA;
//    for(uint8 i=0; i<ArrayLen(apl->SensorDataSlotTable); i++)
//    {
//        if(apl->SensorDataSlotTable[i] != 0)
//        {
//            tag_sensor_slot.tag_id = ntohs(apl->SensorDataSlotTable[i]);
//            tag_sensor_slot.period = 1;
//            tag_sensor_slot.slot = i;//getSensorSlot(tag_sensor_slot.tag_id);
//            memcpy((uint8*)((uint8*)(ah+1)+uch_tag_cnt*(sizeof(ST_BcTagSensorSlot))),(uint8*)(&tag_sensor_slot),sizeof(ST_BcTagSensorSlot));
//            uch_tag_cnt++;//���ν����TAG����
//        }
//    }
    ah->uch_Lengh = uch_tag_cnt*(sizeof(ST_BcTagSensorSlot));
    SendMessageToTag(0xFFFF,sizeof(ST_AirMessageHead)+ah->uch_Lengh,buffer);//�㲥����
}


void AnchorCenterBroadcastWithTagInfo(void)
{
    uint8 buffer[128];
    uint8 i=0;
    uint8 uch_cnt = 0;
    uint16 tag_id_buff[TAG_NUM_MAX];
    ST_AirMessageHead *ah = (ST_AirMessageHead*)buffer;
    ah->uin_PacketNum = 0x8888;
    ah->uch_CtrlCode = RF_BC_TAGINFO;
    ah->uch_OpCode = RF_SET_ONLY;
    ah->uch_Type = RF_DATA;
    for(i=0; i<TAG_NUM_MAX; i++)//�㲥������tag��Ϣ(��������anchor����������ʧ��tag������Ϣ)
    {
        if(taglist[i]!= 0)//����������tag?
        {
            tag_id_buff[uch_cnt++] = taglist[i];
        }
    }
    ah->uch_Lengh = uch_cnt*sizeof(taglist[0]);
    memcpy((uint8*)(ah+1),(uint8*)tag_id_buff,ah->uch_Lengh);
    SendMessageToTag(0xFFFF,sizeof(ST_AirMessageHead)+ah->uch_Lengh,buffer);//�㲥����
}

