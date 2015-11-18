#include "stm32f10x.h"

#include <stdio.h>
#include <string.h>
#include "global_value.h"

#include "protocol.h"
#include "user_lib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "self_buffer.h"
#include "esp8266.h"


static int8 CmdGetDeviceSn(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area, uint16 sn,uint8 *uch_back_type)
{

  if(GET_SN == uch_op)
  {
    if(uch_length == 0)//get命令数据长度为0
    {
      //printf("ok\r\n");
      *uch_back_type = 0x81;
      //device_id = htonl(g_device_id);
      //memcpy((uint8 *)data_area,(uint8*)&device_id,sizeof(device_id));
      return 0;
    }
  }
  return -1;
}

static int8 CmdSetWorkStatus(uint8 uch_op, void *pdata, uint8 uch_length, void *data_area, uint16 sn,uint8 *uch_back_type)
{
  uint16 status;
  if(SET_WORK_STATUS == uch_op)
  {
    if(uch_length == 2)
    {

      *uch_back_type = RESP_WORK_STATUS;

      if(0x0100 == *(uint16*)pdata)
      {
        printf("server cmd start\r\n");
        g_charging_en = true;

        status = 4;
        memcpy((uint8 *)data_area,(uint8*)&status,sizeof(status));
      }
      else if(0x0200 == *(uint16*)pdata)
      {
        printf("server cmd stop\r\n");
        g_charging_en = false;

        status = 6;
        memcpy((uint8 *)data_area,(uint8*)&status,sizeof(status));
      }
      else
      {
        return -1;
      }
      return sizeof(status);
    }
  }
  return -1;
}
/***********************************************************************************************
指令句柄
***********************************************************************************************/
const InsHander InsTable[] =
{

  {GET_SN,                    CmdGetDeviceSn},
  {SET_WORK_STATUS,           CmdSetWorkStatus}
};




uint16 MakeBackFrame(uint8 uin_type,uint8 uch_len,void* out,uint8 uch_link_id)
{
  char buffer[128];
  char i,j;
  ptl_frame_head_t* h = (ptl_frame_head_t*)out;

  h->uch_type = uin_type;
  h->u32_device_sn = htonl(g_device_id);
  h->uch_Lengh = uch_len;

  buffer[0] = 0x45;
  buffer[1] = 0x56;

  for(i=0,j=0; i<sizeof(ptl_frame_head_t) + uch_len; i++)
  {
    if(*((uint8*)out+i) == 0x45)
    {
      buffer[2+i+j] = *((uint8*)out+i);
      buffer[2+i+j+1] = 0x45;
      j++;
    }
    else
    {
      buffer[2+i+j] = *((uint8*)out+i);
    }
  }
  buffer[2+sizeof(ptl_frame_head_t)+uch_len+j] = 0x55;
  buffer[2+sizeof(ptl_frame_head_t)+uch_len+j+1] = 0x45;
  buffer[2+sizeof(ptl_frame_head_t)+uch_len+j+2] = 0x43;


  if(QueueInsert(buffer,sizeof(ptl_frame_head_t) + uch_len + j + 5,uch_link_id))
  {
    Esp8266AddWork(ESP8266_SCHE_TCP_SEND); 
  }

  return (sizeof(ptl_frame_head_t) + uch_len + 5);
}



uint16 ProtocolAnalysis(void *in,uint16 len,void *out,uint8 uch_link_id)
{
  unsigned char i;
  int8 back;
  uint8 uch_back_type;
  ptl_frame_head_t *h = (ptl_frame_head_t *)in;

  if(h->u32_device_sn != ntohl(g_device_id) && h->u32_device_sn != 0xffffffff)
  {
    return 0;
  }
  for (i = 0; i < sizeof(InsTable) / sizeof(InsHander); i++)
  {
    if ((h->uch_type) == InsTable[i].funcCode)
    {
      if (InsTable[i].h != NULL)
      {
        back = (*InsTable[i].h)(h->uch_type, h + 1, h->uch_Lengh,(uint8 *)out + sizeof(ptl_frame_head_t), 0,&uch_back_type);
        if (back < 0)
          return 0;
        return MakeBackFrame(uch_back_type,back,out,uch_link_id);
      }
      return 0;
    }
  }
  return 0;
}


uint16 FrameExtraction(char *in,uint16 len,uint8 uch_link_id)
{
  uint8_t pos;
  uint8_t buf[128];
  uint8_t uch_index = 0;

  bool b_head_flag = false;
  bool b_char_flag = false;

  if(len < 10)return -1;

  for(pos=0; pos<len; pos++)
  {
    if(true == b_char_flag)
    {
      b_char_flag = false;

      if(0x56 == *(in+pos))
      {
        uch_index = 0;
        b_head_flag = true;
      }
      else if(0x43 == *(in+pos))
      {
        b_head_flag = false;

        ProtocolAnalysis(buf,uch_index,buf,uch_link_id);
      }
      else if(0x45 == *(in+pos))
      {
        if(true == b_head_flag)
        {
          buf[uch_index++] = *(in+pos);
        }
      }
      else /*帧编码出错*/
      {
        return -1;
      }
    }
    else if(*(in+pos) == 0x45)
    {
      b_char_flag = true;
    }
    else
    {
      if(true == b_head_flag)
      {
        buf[uch_index++] = *(in+pos);
      }
    }
  }

  return 0;
}

//void SendChargingstatus(void )
//{
//  char buffer[15];
//  if(g_esp_avilable_flag)
//  {
//    buffer[0] = 0x45;
//    buffer[1] = 0x56;
//    buffer[2] = 0x27;
//    
//    buffer[3] = 0x12;
//    buffer[4] = 0x34;
//    buffer[5] = 0x56;
//    buffer[6] = 0x78;
//    
//    buffer[7] = 0x02;
//    buffer[8] = 0x05;
//    buffer[9] = 0x00;
//    buffer[10] = 0x45;
//    buffer[11] = 0x43;
//
//    if(QueueInsert(buffer,12,1))
//    {
//      Esp8266AddWork(ESP8266_SCHE_TCP_SEND); //创建初始化任务
//    }
//  }
//}
void SendChargingstatus(void )
{
  char buffer[15];
  if(g_esp_avilable_flag)
  {
    buffer[0] = 0x45;
    buffer[1] = 0x56;
    buffer[2] = 0x87;
    buffer[3] = 0x12;
    buffer[4] = 0x34;
    buffer[5] = 0x56;
    buffer[6] = 0x78;
    
    buffer[7] = 0x02;
    buffer[8] = 0x05;
    buffer[9] = 0x00;
    
    buffer[10] = 0x00;//校验和
    
    buffer[11] = 0x45;
    buffer[12] = 0x43;
    if(QueueInsert(buffer,13,1))
    {
      Esp8266AddWork(ESP8266_SCHE_TCP_SEND); //创建初始化任务
    }
  }
}



//void SendChargingOver(void)
//{
//  char buffer[15];
//  if(g_esp_avilable_flag)
//  {
//    buffer[0] = 0x45;
//    buffer[1] = 0x56;
//    buffer[2] = 0x27;
//    
//    buffer[3] = 0x12;
//    buffer[4] = 0x34;
//    buffer[5] = 0x56;
//    buffer[6] = 0x78;
//    
//    buffer[7] = 0x02;
//    buffer[8] = 0x06;
//    buffer[9] = 0x00;
//    buffer[10] = 0x45;
//    buffer[11] = 0x43;
//
//    if(QueueInsert(buffer,12,1))
//    {
//      Esp8266AddWork(ESP8266_SCHE_TCP_SEND); //创建初始化任务
//    }
//  }
//}
void SendChargingOver(void)
{
  char buffer[15];
  if(g_esp_avilable_flag)
  {
    buffer[0] = 0x45;
    buffer[1] = 0x56;
    buffer[2] = 0x87;
    
    buffer[3] = 0x12;
    buffer[4] = 0x34;
    buffer[5] = 0x56;
    buffer[6] = 0x78;
    
    buffer[7] = 0x02;
    buffer[8] = 0x06;
    buffer[9] = 0x00;
    
    buffer[10] = 0x00;
    
    buffer[11] = 0x45;
    buffer[12] = 0x43;

    if(QueueInsert(buffer,13,1))
    {
      Esp8266AddWork(ESP8266_SCHE_TCP_SEND); //创建初始化任务
    }
  }
}


void SendStart(void)
{
  char buffer[15];
  if(g_esp_avilable_flag)
  {
    buffer[0] = 0x45;
    buffer[1] = 0x56;
    buffer[2] = 0x81;
    
    buffer[3] = 0x12;
    buffer[4] = 0x34;
    buffer[5] = 0x56;
    buffer[6] = 0x78;
    
    buffer[7] = 0x00;
    buffer[8] = 0x83;
    buffer[9] = 0x45;
    buffer[10] = 0x43;

    if(QueueInsert(buffer,12,1))
    {
      Esp8266AddWork(ESP8266_SCHE_TCP_SEND); //创建初始化任务
    }
  }
}