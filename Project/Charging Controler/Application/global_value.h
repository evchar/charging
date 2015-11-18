#ifndef __GLOBAL_VALUE_H__
#define __GLOBAL_VALUE_H__

#include "user_lib.h"
#define MAX_REV_LEN          200
#define USART_REV_TIMEOUT    5

#pragma pack(1)
typedef struct
{
  u8 auch_rev_buffer[2][MAX_REV_LEN];
  u8 uch_rev_cnt;
  u8 uch_parse_len;
  u8 uch_timeout;
  bool b_parse_busy;
  u8 *p_rev;
  u8 *p_parse;
}usart_data_rev_t;
#pragma pack()

typedef s8 (*InsResponseHandler)(char *src,void *pdata);

extern InsResponseHandler Esp8266InsHandler;

extern void *InsPdata;

extern usart_data_rev_t g_rev_usart1;

extern uint32 g_device_id;


extern uint8 g_server_ip[4];

extern uint32 g_server_port;

extern bool g_esp_avilable_flag;

extern bool g_charging_en;

extern uint32 g_pf_cnt;
//extern char g_rec_buffer[128];
//extern char g_rec_cnt;
//extern char g_rec_time;
//typedef enum {CENTER_ANCHOR = 1,NORMAL_ANCHOR} ANCHOR_TYPE;
#endif
