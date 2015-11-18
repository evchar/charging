#include "stm32f10x.h"
#include "user_lib.h"
#include "drv_led_relay.h"


void LED_RELAY_Configuration(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | 
                                                RCC_APB2Periph_GPIOB, ENABLE);	
               
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_0 | GPIO_Pin_1;				 //LED0-->PB.5 �˿�����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		      //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		      //IO���ٶ�Ϊ50MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5	
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_9;	    		 //LED1-->PE.5 �˿�����, �������
  GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
  
  /*ESP8266 POWER SWITCH*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;	    		 //LED1-->PE.5 �˿�����, �������
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
  
  BAT1_ON();
  BAT2_ON();
  
  RELAY_CHARGE_OFF();
  LED_FAULT_OFF();//fault
}


