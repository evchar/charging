/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdio.h>
#include<string.h>
#include "drv_usart.h"
#include "global_value.h"
#include "self_buffer.h"
#include "user_lib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "esp8266.h"
#include "protocol.h"
#include "drv_led_relay.h"

volatile u16 Esp8266ScheFlags;

//OS_SEM TimeSem;
//OS_SEM Sim5218Sche;
//OS_SEM Sim5218CallBack;
//OS_SEM SignArrow;//weile tcp jieshou ">"

/* Synchronisation */
xQueueHandle esp8266ScheMsg;
xSemaphoreHandle esp8266CallbackSem;
xSemaphoreHandle UsartTimeoutSem;


s8 ESP8266WaitResponse(u16 wait_time, InsResponseHandler p_handler, void *pdata);
/**
* @brief  查找字符串
* @param  char *s, char *t ;在s中查找t
* @retval s_temp(t在s中的位置)成功     0 （失败 ）
*/
char *mystrstr(char *s, char *t)
{
  char    *s_temp;        /*the s_temp point to the s*/
  char    *m_temp;        /*the mv_tmp used to move in the loop*/
  char    *t_temp;        /*point to the pattern string*/

  if (NULL == s || NULL == t) return NULL;

  /*s_temp point to the s string*/
  for (s_temp = s; *s_temp != '\0'; s_temp++)
  {
    /*the move_tmp used for pattern loop*/
    m_temp = s_temp;
    /*the pattern string loop from head every time*/
    for (t_temp = t; *t_temp == *m_temp; t_temp++, m_temp++);
    /*if at the tail of the pattern string return s_tmp*/
    if (*t_temp == '\0') return s_temp;

  }
  return NULL;
}

/**
* @brief  开关工作
* @param  operate:1--使能，2--失能；task_bit:需要执行的工作
* @retval None
*/
void Esp8266WriteScheFlags(u8 operate,u16 task_bit)
{
  taskENTER_CRITICAL();
  if(operate)
    Esp8266ScheFlags |= task_bit;
  else
    Esp8266ScheFlags &= ~task_bit;
  taskEXIT_CRITICAL();
}

/**
* @brief  读取工作状态
* @param  None
* @retval 需要执行的工作
* @note   读取目前打开的工作，赋值给flags
*/
u16 Esp8266ReadScheFlags(void)
{
  u16 flags;
  taskENTER_CRITICAL();
  flags = Esp8266ScheFlags;
  taskEXIT_CRITICAL();
  return flags;
}

/**
* @brief  添加工作
* @param  需要执行的工作
* @retval None
* @note   使能工作，并且释放sche调度任务信号量
*/
void Esp8266AddWork(u8 task_bit)
{
  u8 tem;
  if(task_bit == ESP8266_SCHE_NONE)
    return;
  tem = task_bit;
  //Esp8266WriteScheFlags(1,task_bit);
  xQueueSend(esp8266ScheMsg, &tem, 0);/*队列最前面，优先处理*/
}

/**
* @brief  接收数据是否为想要的回复做比较
* @param  None
* @retval 1--回复是对的；0--回复是错误的
* @note   如果这里只要返回正确或失败，那么可以直接定义为void类型，在CallInsHandler中根据pdata去判断
*/
s8 ReturnOKHandler(char *p, void *pdata)
{
  if(mystrstr(p,"OK") != NULL || mystrstr(p,"no change") != NULL)
  {
    *(s8 *)pdata = 1;
    return 1;
  }
  return 0;
}


/**
* @brief  接收数据是否为想要的回复做比较
* @param  None
* @retval 1--回复是对的；0--回复是错误的
* @note   如果这里只要返回正确或失败，那么可以直接定义为void类型，在CallInsHandler中根据pdata去判断
*/
s8 ReturnWifiStatusHandler(char *p, void *pdata)
{
  unsigned int result = 0;
  if(mystrstr(p,"STATUS") != NULL)
  {
    sscanf(p,"STATUS:%d\r\n",&result);/*提取链接ID,数据长度*/

    if(5 == result)
    {
       *(s8 *)pdata = 1;
    }
    else
    {
       *(s8 *)pdata = 2;
    }
    return 1;
  }
  return 0;
}

/**
* @brief  接收数据是否为想要的回复做比较
* @param  None
* @retval 1--回复是对的；0--回复是错误的
* @note   如果这里只要返回正确或失败，那么可以直接定义为void类型，在CallInsHandler中根据pdata去判断
*/
s8 ReturnConnectHandler(char *p, void *pdata)
{
  if(mystrstr(p,"OK") != NULL || mystrstr(p,"ALREAY CONNECT") != NULL)
  {
    *(s8 *)pdata = 1;
    return 1;
  }
  return 0;
}

/**
* @brief  接收数据是否为想要的回复做比较
* @param  None
* @retval 1--回复是对的；0--回复是错误的
* @note   如果这里只要返回正确或失败，那么可以直接定义为void类型，在CallInsHandler中根据pdata去判断
*/
s8 ReturnSendOKHandler(char *p, void *pdata)
{
  if(mystrstr(p,"SEND OK") != NULL)
  {
    *(s8 *)pdata = 1;
    return 1;
  }
  return 0;
}

/**
* @brief  接收数据是否为想要的回复做比较
* @param  None
* @retval 1--回复是对的；0--回复是错误的
* @note   如果这里只要返回正确或失败，那么可以直接定义为void类型，在CallInsHandler中根据pdata去判断
*/
s8 ReturnArrowHandler(char *p, void *pdata)
{
  if(mystrstr(p,">") != NULL)
  {
    *(s8 *)pdata = 1;
    return 1;
  }
  return 0;
}

/**
* @brief  接收数据是否为想要的回复做比较
* @param  None
* @retval 1--回复是对的；0--回复是错误的
* @note   如果这里只要返回正确或失败，那么可以直接定义为void类型，在CallInsHandler中根据pdata去判断
*/
s8 Esp8266TCPSend(void)
{
   volatile uint32 state = 0;
   uint8 auch_buffer[128];
   uint8 uch_length;
   uint8 uch_id;
   static uint8 err_cnt = 0;
   char buffer[20] ={0};

   if(!QueueDelete(auch_buffer,&uch_length, &uch_id))
     return 1;

  sprintf(buffer,"AT+CIPSEND=%d,%d\r\n",uch_id,uch_length);
  //strcpy(buffer,"AT+CIPSEND=1,5\r\n");
  USART4SendBuffer((u8 *)buffer,strlen(buffer));
  ESP8266WaitResponse(1000,ReturnArrowHandler,(uint32*)(&state));

  /*发送失败将重新初始化模块*/
  if(state == 0)
  {
    err_cnt++;
    if(err_cnt >= 4)
    {
      err_cnt = 0;
      InitQueue();
      Esp8266AddWork(ESP8266_SCHE_INIT); //重新初始化模块
      g_esp_avilable_flag = false;
      return 2;
    }
  }


  USART4SendBuffer((u8 *)auch_buffer,uch_length);
  ESP8266WaitResponse(500,ReturnSendOKHandler,(uint32*)(&state));

  if(state == 0)
  {
    return 3;
  }

  //printf("send-ok\r\n");


  return 0;
}



/**
* @brief  等待指令的回复
* @param  None
* @retval 1--回复是对的；0--回复是错误的
* @note   根据第二个形参决定回复内容。如果回复内容正确，那么CallInsHandler函数中会释放信号量
Sim5218CallBack回来，并返回1；否则返回0
*/
s8 ESP8266WaitResponse(u16 wait_time, InsResponseHandler p_handler, void *pdata)
{
  if (p_handler == NULL)
    return -1;

  taskENTER_CRITICAL();
  Esp8266InsHandler = p_handler;         //注册回调函数
  InsPdata = pdata;               //注册参数
  taskEXIT_CRITICAL();

  xSemaphoreTake(esp8266CallbackSem, wait_time);

  taskENTER_CRITICAL();
  Esp8266InsHandler = NULL;
  taskEXIT_CRITICAL();

  return -1;
}



s8 Esp8266PowerOn( void )
{
  return 0;
}


/**
* @brief  ESP8266重启
* @param  None
* @retval 1--回复是对的；0--回复是错误的
 * @note
*/
s8 Esp8266Reset( void )
{
  volatile uint32 state = 0;
  char buffer[128] ={0};

  /*重启*/
  strcpy(buffer,"AT+RST\r\n");

  USART4SendBuffer((u8 *)buffer,strlen(buffer));

  ESP8266WaitResponse(500,ReturnOKHandler,(uint32*)(&state));
  if(state == 0)
  {
    return 1;
  }

  return 0;
}


/**
* @brief  ESP8266 PING
* @param  None
* @retval 1--回复是对的；0--回复是错误的
 * @note
*/
s8 Esp8266Ping( void )
{
  volatile uint32 state = 0;
  char buffer[128] ={0};

  /*PING*/
  strcpy(buffer,"AT+PING=\"www.baidu.com\"\r\n");

  USART4SendBuffer((u8 *)buffer,strlen(buffer));

  ESP8266WaitResponse(500,ReturnOKHandler,(uint32*)(&state));
  if(state == 0)
  {
    return 1;
  }

  return 0;
}


s8 Esp8266Init( void )
{
  volatile uint32 state = 0;
  char buffer[128] ={0};
  //bool b_flag = false;
  g_esp_avilable_flag = false;

  /*关闭回显*/
  strcpy(buffer,"ATE0\r\n");
  USART4SendBuffer((u8 *)buffer,strlen(buffer));
  ESP8266WaitResponse(500,ReturnOKHandler,(uint32*)(&state));
  if(state == 0)
  {
    printf("1.ate0-err\r\n");
    return 1;
  }
  printf("1.ate0-ok\r\n");

  /*设置ap,station共存模式*/
  state = 0;
  strcpy(buffer,"AT+CWMODE_DEF=3\r\n");
  USART4SendBuffer((u8 *)buffer,strlen(buffer));
  ESP8266WaitResponse(500,ReturnOKHandler,(uint32*)(&state));
  if(state == 0)
  {
    printf("2.at+cwmode-err\r\n");
    return 2;
  }
  printf("2.at+cwmode-ok\r\n");

#if 0
  /*查看是否已加入WIFI网络*/
  state = 0;
  strcpy(buffer,"AT+CIPSTATUS\r\n");
  USART1SendBuffer((u8 *)buffer,strlen(buffer));
  ESP8266WaitResponse(10000,ReturnWifiStatusHandler,(uint32*)(&state));
  if(state == 0)
  {
    printf("3.at+cipstatus-err\r\n");
    return 3;
  }
  else if(state == 1)
  {
    b_flag = true;
  }
  printf("3.at+cipstatus-ok\r\n");
#endif

  /*加入wifi网络*/
  state = 0;
  //strcpy(buffer,"AT+CWJAP_DEF=\"cuckoo\",\"csc764062915\"\r\n");
  //strcpy(buffer,"AT+CWJAP_DEF=\"E-Hub-1\",\"15858274917\"\r\n");
  
  strcpy(buffer,"AT+CWJAP_DEF=\"E-Hub\",\"12345678\"\r\n");
  USART4SendBuffer((u8 *)buffer,strlen(buffer));
  ESP8266WaitResponse(10000,ReturnOKHandler,(uint32*)(&state));
  if(state == 0)
  {
    printf("4.at+cwjap-err\r\n");
    return 4;
  }
  printf("4.at+cwjap-ok\r\n");

  /*选择多链接*/
  state = 0;
  strcpy(buffer,"AT+CIPMUX=1\r\n");
  USART4SendBuffer((u8 *)buffer,strlen(buffer));
  ESP8266WaitResponse(500,ReturnOKHandler,(uint32*)(&state));
  if(state == 0)
  {
    printf("5.at+cipmux-err\r\n");
    return 5;
  }
  printf("5.at+cipmux-ok\r\n");


  /*连接远程服务器*/
  state = 0;
//  g_server_ip[0] = 192;
//  g_server_ip[1] = 168;
//  g_server_ip[2] = 0;
//  g_server_ip[3] = 112;
//
//  g_server_port = 6666;

  g_server_ip[0] = 120;
  g_server_ip[1] = 26;
  g_server_ip[2] = 80;
  g_server_ip[3] = 165;

  g_server_port = 55555;

//strcpy(buffer,"AT+CIPSTART=1,\"TCP\",\"192.168.0.105\",6800\r\n"); //strcpy(buffer,"AT+CIPSTART=\"TCP\",\"115.29.249.204\",6789\r");
  sprintf(buffer,"AT+CIPSTART=1,\"TCP\",\"%d.%d.%d.%d\",%d\r\n",g_server_ip[0],g_server_ip[1],g_server_ip[2],g_server_ip[3],g_server_port);
  USART4SendBuffer((u8 *)buffer,strlen(buffer));
  ESP8266WaitResponse(20000,ReturnOKHandler,(uint32*)(&state));
  if(state == 0)
  {
    printf("6.at+cipstart1-err\r\n");
    return 6;
  }
  printf("6.at+cipstart1-ok\r\n");

  g_esp_avilable_flag = true;

#if 0
  state = 0;
  strcpy(buffer,"AT+CIPSTART=2,\"TCP\",\"192.168.0.105\",6801\r\n");//strcpy(buffer,"AT+CIPSTART=\"TCP\",\"115.29.249.204\",6789\r"); //strcpy(buffer,"AT+CIPSTART=\"TCP\",\"192.168.0.105\",6800\r"); //strcpy(buffer,"AT+CIPSTART=\"TCP\",\"115.29.249.204\",6789\r");
  USART1SendBuffer((u8 *)buffer,strlen(buffer));
  ESP8266WaitResponse(20000,ReturnOKHandler,(uint32*)(&state));
  if(state == 0)
  {
    printf("6.at+cipstart2-err\r\n");
    return 6;
  }
  printf("6.at+cipstart2-ok\r\n");

#endif

  return 0;
}


u8 CallInsHandler(char *p)
{
  s8 stat;
  if (Esp8266InsHandler == NULL)
    return 0;
  stat = Esp8266InsHandler(p,InsPdata);

  if(stat == 1)
  {
    xSemaphoreGive(esp8266CallbackSem);
    return 1;
  }
  return 0;
}



void Esp8266PromptProtocol(char *Buffer,uint8 len)
{
   uint8 pos,context_start;
   uint32 id,length;
   char str[20];

  if(mystrstr(Buffer,"+IPD") != NULL)
  {
    for(pos=0; pos<len; pos++)
    {
      if(Buffer[pos] == '+' && Buffer[pos+2] == 'P'&& Buffer[pos+1] == 'I'&& Buffer[pos+3] == 'D')
      {
        sscanf(&Buffer[pos],"%[^:]",str);
        sscanf(str,"+IPD,%u,%u",&id,&length);
        if(id > 10 || length > 128)
        {
          return;
        }

        for(context_start=0; Buffer[pos+context_start] != ':' ||
                                            context_start > 20;context_start++);


        FrameExtraction(&Buffer[pos+context_start+1],length,(uint8)id);

        pos += length+context_start;

      }
    }
  }

  if(mystrstr(Buffer,"CLOSED") != NULL)
  {
    printf("close\r\n");
    if(g_esp_avilable_flag)
    {
      g_esp_avilable_flag = false;
      Esp8266AddWork(ESP8266_SCHE_INIT); //重新初始化模块
    }

  }
}


/**
  * @brief   WIFIMoudleTask task
  * @param  pvParameters not used
  * @retval None
  */
void WifiMoudleTask(void * pvParameters)
{
  uint8 uch_Msg = 0;
  //static bool b_success_flag = false;
  //ESP8266_GPIO0_HIGH();
  ESP8266_ON();

  Usart4RevBufInit();

  InitQueue();

  Usart4Init();

  esp8266CallbackSem = xSemaphoreCreateCounting(2,0);
  esp8266ScheMsg = xQueueCreate(5,1);/*队列深度为5，数据大小为1*/
  vSemaphoreCreateBinary(UsartTimeoutSem);
  Esp8266AddWork(ESP8266_SCHE_INIT); //创建初始化任务

  for( ;; )
  {
    if (xQueueReceive(esp8266ScheMsg, &uch_Msg, portMAX_DELAY) == pdTRUE)
    {
       if (ESP8266_SCHE_INIT == uch_Msg)
       {
         if(!Esp8266Init())
         {
           //b_success_flag = true;
           
           SendStart();
           
         }
         else
         {
           Esp8266AddWork(ESP8266_SCHE_INIT);/*初始化失败重试*/
         }
       }
       if (ESP8266_SCHE_TCP_SEND == uch_Msg)
       {
         if(g_esp_avilable_flag)
         {
           Esp8266TCPSend();
         }
       }
    }
  }
}


void WifiParseTask(void * pvParameters)
{
  char auch_buf[MAX_REV_LEN];
  for( ;; )
  {
    if(xSemaphoreTake(UsartTimeoutSem, portMAX_DELAY) == pdTRUE)
    {
      memset(auch_buf, 0, MAX_REV_LEN);
      memcpy(auch_buf, g_rev_usart1.p_parse, g_rev_usart1.uch_parse_len);

      g_rev_usart1.b_parse_busy = false;
      CallInsHandler(auch_buf);
      Esp8266PromptProtocol(auch_buf,g_rev_usart1.uch_parse_len);
    }
  }
}



