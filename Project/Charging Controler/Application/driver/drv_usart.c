/**
 * @file drv_usart.c
 *
 * @note This file contains the source code for the console
 *       and serial port support functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "drv_usart.h"
#include <stdio.h>
#include "user_lib.h"
#include "global_value.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "drv_led_relay.h"
extern xSemaphoreHandle UsartTimeoutSem;

void Usart4Init( void )
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//ʹ��GPIOCʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//ʹ��USART2ʱ��
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	//PC10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;    //PC11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
  GPIO_Init(GPIOC, &GPIO_InitStructure);  
  
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART4,ENABLE);//��λ����3
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART4,DISABLE);//ֹͣ��λ
    
  USART_DeInit(UART4);
  USART_InitStructure.USART_BaudRate = 115200;//����������
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8λ���ݳ���
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
  USART_InitStructure.USART_Parity = USART_Parity_No;///��żУ��λ
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
  
  USART_Init(UART4, &USART_InitStructure);//��ʼ������
  
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 14;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
  USART_ClearFlag(UART4 , USART_FLAG_TC); 
  USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ��� 
}


/**
 * @brief Initialize console i/o support
 *
 * This function initializes the usart interface of the STM32 uController
 */
void ConsoleInit(void)
{

  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//ʹ��GPIOAʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//ʹ��USART3ʱ��
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;		//PD7�˿�����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	//PA2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PA3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3,ENABLE);//��λ����3
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3,DISABLE);//ֹͣ��λ
  
  USART_DeInit(USART3);
  
  USART_InitStructure.USART_BaudRate = 115200;//����������
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8λ���ݳ���
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
  USART_InitStructure.USART_Parity = USART_Parity_No;///��żУ��λ
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
  
  USART_Init(USART3, &USART_InitStructure);  //��ʼ������
  
  USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ��� 
  
  RS485_ENABLE();
  
}  
/******************************************************************
** ��������: USART4SendByte
** ��������: ����һ���ֽ�
** �ա�  ��: 2013.8.6
*******************************************************************/
void USART4SendByte(uint8 Data)
{ 
  while (!(UART4->SR & USART_FLAG_TXE));
  UART4->DR = (Data & (uint16_t)0x01FF);	
}

/******************************************************************
** ��������: USART1SendBuffer
** ��������: ����ָ�����ȵ��ַ���,����
** �ա�  ��: 2013.8.6
*******************************************************************/
void USART4SendBuffer(u8 *ptr,uint8_t count)
{
  while(count--)
  {
    USART4SendByte(*ptr++);
  }
}


void Usart4RevBufInit( void )
{
  g_rev_usart1.uch_rev_cnt = 0;
  g_rev_usart1.uch_timeout = 0;
  g_rev_usart1.uch_parse_len = 0;
  g_rev_usart1.b_parse_busy = false;
  g_rev_usart1.p_rev = g_rev_usart1.auch_rev_buffer[0];
  g_rev_usart1.p_parse = NULL;   
}

/**
  * @brief  swapRevBuf
  * @param  None
  * @retval None
  */
int SwapRevBuf(usart_data_rev_t* rev_usart)
{
  if(true == rev_usart->b_parse_busy)
    return -1;
  
  rev_usart->b_parse_busy = true;
  
  rev_usart->p_parse = rev_usart->p_rev;
  
  rev_usart->uch_parse_len = rev_usart->uch_rev_cnt;
  
  if(rev_usart->p_rev == rev_usart->auch_rev_buffer[0])
  {
    rev_usart->p_rev = rev_usart->auch_rev_buffer[1];
  }
  else
  {
    rev_usart->p_rev = rev_usart->auch_rev_buffer[0];
  }  
  
  rev_usart->uch_rev_cnt = 0;
  
  return 0;
}



void UART4_IRQHandler(void)
{ 
  volatile u8 dummy;
  portBASE_TYPE  xHigherPriorityTaskWoken = pdFALSE;
  /*���ռĴ����ǿգ����մ��ڵ�����*/
  if(USART_GetITStatus(UART4,USART_IT_RXNE) != RESET)
  {
    dummy = (u8)UART4->DR ;
    
    g_rev_usart1.uch_timeout = USART_REV_TIMEOUT;
    
    g_rev_usart1.p_rev[g_rev_usart1.uch_rev_cnt++] = dummy;
    
    if( g_rev_usart1.uch_rev_cnt >= MAX_REV_LEN )
    {
      if(SwapRevBuf(&g_rev_usart1) < 0)
      {
        g_rev_usart1.uch_rev_cnt = 0;
      }
      
      xSemaphoreGiveFromISR(UsartTimeoutSem,&xHigherPriorityTaskWoken);
      
      if( xHigherPriorityTaskWoken != pdFALSE )
      {
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
      }      
    }
  }
}

