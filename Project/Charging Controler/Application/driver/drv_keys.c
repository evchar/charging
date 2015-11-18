#include "stm32f10x.h"
#include "user_lib.h"
#include "drv_led_relay.h"
#include "drv_keys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


extern xQueueHandle   msgQueue;


void KEY_Configuration(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB, ENABLE);	 
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;          
  GPIO_Init(GPIOC, &GPIO_InitStructure);					
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
  
}


static u8 KeyScan(void)
{
   u8 u8_key_value = KEY_NULL;
   
   if(KEY0_Bit == 0)u8_key_value |= KEY_0;
   if(KEY1_Bit == 0)u8_key_value |= KEY_1;
   
   return u8_key_value;
}


void GetKey(u8 *pKeyValue) 
{ 
  static u8 s_u8KeyState = KEY_STATE_INIT ; 
  static u8 s_u8KeyTimeCount = 0 ; 
  static u8 s_u8LastKey = KEY_NULL ;  
  static u8 s_u8FirstKey = KEY_NULL ;   
  u8 KeyTemp = KEY_NULL ; 
  
  KeyTemp = KeyScan() ;    
  
  switch(s_u8KeyState) 
  { 
  case KEY_STATE_INIT : 
    { 
      if(KEY_NULL != (KeyTemp)) 
      { 
        s_u8FirstKey = KeyTemp;
        s_u8KeyState = KEY_STATE_WOBBLE ; 
      } 
    } 
    break ; 
    
  case KEY_STATE_WOBBLE :     
    { 
      s_u8KeyState = KEY_STATE_PRESS ;     
    } 
    break ; 
    
  case KEY_STATE_PRESS : 
    { 
      if(KEY_NULL != (KeyTemp) && KeyTemp == s_u8FirstKey)  /*有键按下且初值未变*/
      { 
        s_u8LastKey = KeyTemp ; 
        KeyTemp |= KEY_DOWN ;   
        xQueueSend(msgQueue, &KeyTemp, 0);
        s_u8KeyState = KEY_STATE_LONG ; 
      } 
      else 
      { 
        s_u8KeyState = KEY_STATE_INIT ; 
      } 
    } 
    break ; 
    
  case KEY_STATE_LONG : 
    { 
      if(KEY_NULL != (KeyTemp) && KeyTemp == s_u8FirstKey) /*有键按下且初值未变*/
      { 
        if(++s_u8KeyTimeCount > KEY_LONG_PERIOD) 
        { 
          s_u8KeyTimeCount = 0 ; 
          KeyTemp |= KEY_LONG ;   //长按键事件发生 
          xQueueSend(msgQueue, &KeyTemp, 0);
          s_u8KeyState = KEY_STATE_CONTINUE ; 
        } 
      } 
      else 
      { 
        s_u8KeyState = KEY_STATE_RELEASE ; 
      } 
    } 
    break ; 
    
  case KEY_STATE_CONTINUE : 
    { 
      if(KEY_NULL != (KeyTemp) && KeyTemp == s_u8FirstKey) /*有键按下且初值未变*/ 
      { 
        if(++s_u8KeyTimeCount > KEY_CONTINUE_PERIOD) 
        { 
          s_u8KeyTimeCount = 0 ; 
          KeyTemp |= KEY_CONTINUE ; 
          xQueueSend(msgQueue, &KeyTemp, 0);
        } 
      } 
      else 
      { 
        s_u8KeyState = KEY_STATE_RELEASE ; 
      } 
    } 
    break ; 
    
  case KEY_STATE_RELEASE : 
    { 
      s_u8LastKey |= KEY_UP ; /*按键抬起*/
      KeyTemp = s_u8LastKey ; 
      s_u8KeyState = KEY_STATE_INIT ; 
      xQueueSend(msgQueue, &KeyTemp, 0);
    } 
    break ; 
    
  default : break ; 
  } 
  *pKeyValue = KeyTemp ; 
} 

/**
  * @brief  Key task
  * @param  pvParameters not used
  * @retval None
  */
void KeyTask(void * pvParameters)
{
  u8 key_value = 0;
  KEY_Configuration();
  for( ;; )
  {
    GetKey(&key_value);
    vTaskDelay(20);
  }
}



















