#include "stm32f10x.h"

#include <stdio.h>
#include <string.h>
#include "global_value.h"

#include "protocol.h"
#include "user_lib.h"
#include "self_buffer.h"

#include "FreeRTOS.h"
#include "task.h"

typedef struct
{
  uint16 uch_id;
  uint8 uch_DateLength;
  uint8 auch_DateBuf[DATE_BUF_LEN];
}self_date_t;

typedef struct
{
  uint8 uch_In;
  uint8 uch_Out;
  self_date_t QueueBuf[QUEUE_BUF_LEN];
}self_buffer_t;

static self_buffer_t g_send_buffer;

void InitQueue( void )
{
  //uint8 i,j;
  g_send_buffer.uch_In = 0;
  g_send_buffer.uch_Out = 0;
  
# if 0
  for(i=0; i<QUEUE_BUF_LEN; i++)
  {
    
    g_Queue.QueueBuf[i].uch_DateLength = 0;
    
    for(j=0; j<DATE_BUF_LEN; j++)
    {
      g_Queue.QueueBuf[i].auch_DateBuf[j] = 0;
    }
  }
#endif
}

uint8 QueueInsert(void *pInputDate,uint8 dateLength,uint8 uch_id)
{
  // uint8 i;
  
  if(!dateLength)
  {
    return 0;
  }
  
  if((g_send_buffer.uch_In+1)%QUEUE_BUF_LEN == g_send_buffer.uch_Out)
  {
    return 0;
  }
  taskENTER_CRITICAL();
  g_send_buffer.QueueBuf[g_send_buffer.uch_In].uch_id = uch_id;
  g_send_buffer.QueueBuf[g_send_buffer.uch_In].uch_DateLength = dateLength;
  
  //for(i=0; i<dateLength; i++)
  //{
  //g_Queue.QueueBuf[g_Queue.uch_In].auch_DateBuf[i] = pInputDate[i];
  //}
  memcpy(g_send_buffer.QueueBuf[g_send_buffer.uch_In].auch_DateBuf, (uint8*)pInputDate,dateLength);
  
  g_send_buffer.uch_In++;
  
  if(QUEUE_BUF_LEN == g_send_buffer.uch_In)
  {
    g_send_buffer.uch_In = 0;
  }
  taskEXIT_CRITICAL();
  return 1;
}

uint8 QueueDelete(void *pOutputDate,uint8* pLength, uint8 * p_id)
{
  //uint8 i;
  if( g_send_buffer.uch_Out ==  g_send_buffer.uch_In)
  {
    return 0;
  }
  else
  {
    taskENTER_CRITICAL();
    *p_id = g_send_buffer.QueueBuf[g_send_buffer.uch_Out].uch_id;
    *pLength = g_send_buffer.QueueBuf[g_send_buffer.uch_Out].uch_DateLength;
    // for(i=0; i<g_Queue.QueueBuf[g_Queue.uch_Out].uch_DateLength; i++)
    //{
    //  pOutputDate[i] = g_Queue.QueueBuf[g_Queue.uch_Out].auch_DateBuf[i];
    //}
    memcpy((uint8*)pOutputDate,g_send_buffer.QueueBuf[g_send_buffer.uch_Out].auch_DateBuf,*pLength);
    
    g_send_buffer.uch_Out++;
    
    if(QUEUE_BUF_LEN == g_send_buffer.uch_Out)
    {
      g_send_buffer.uch_Out = 0;
    }
    taskEXIT_CRITICAL();
  }
  return 1;
}
