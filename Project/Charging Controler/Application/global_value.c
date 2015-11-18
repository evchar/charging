#include "stm32f10x.h"
#include <stdio.h>
#include "global_value.h"


InsResponseHandler Esp8266InsHandler;

void *InsPdata;

//char g_usart1_rec_buffer[2][MAX_REV_LEN];
//char g_usart1_rec_cnt = 0;
//char g_usart1_rec_time = 5;

usart_data_rev_t g_rev_usart1;

uint32 g_device_id = 0x12345678;//0xffff45ff;

uint8 g_server_ip[4];

uint32 g_server_port;

bool g_esp_avilable_flag = false;

bool g_charging_en = false;

uint32 g_pf_cnt = 0;
