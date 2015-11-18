#include "stm32f10x.h"
#include "user_lib.h"
#include "drv_spi.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/**
 * SPI1_Configuration:
 * SPI1_Configuration() initializes the spi interface on the microcontroller
 * Returns: none
 */
void SPI1_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
  //EXTI_InitTypeDef EXTI_InitStructure;
  //NVIC_InitTypeDef NVIC_InitStructure;
  
    /* Enable the SPI clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOB, ENABLE);  

  /* Configure SPI1 pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure I/O for Chip select */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//SPI CS
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  SPI_I2S_DeInit(SPI1);
  
  /* SPI1 configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;//����SPI����ģʽ:����Ϊ��SPI
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//;//ѡ���˴���ʱ�ӵ���̬:ʱ�����յ�
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;//;//���ݲ����ڵ�1��ʱ����
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;//SPI_BaudRatePrescaler_32;//���岨����Ԥ��Ƶ��ֵ
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
  SPI_InitStructure.SPI_CRCPolynomial = 7;//CRCֵ����Ķ���ʽ
  SPI_Init(SPI1, &SPI_InitStructure);//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
  //SPI_SSOutputCmd(SPI1,ENABLE);//SS���
  SPI_Cmd(SPI1, ENABLE);//ʹ��SPI����
  
}

/*************************************************
Function: SpiTxRxByte(uint8_t byte)
Description: Spi�շ�һ�ֽ�����
Input: ���͵�һ�ֽ�����
Output: ���յ�һ�ֽ�����
Return:
*************************************************/
uint8_t SpiTxRxByte(uint8_t byte)
{
  // Loop while DR register in not emplty
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  
  //Send byte through the SPI2 peripheral
  SPI_I2S_SendData(SPI1, byte);
  
  // Wait to receive a byte
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  
  // Return the byte read from the SPI bus
  return SPI_I2S_ReceiveData(SPI1);
}

/**
  * @brief  This function handles External line 0 interrupt request.
  *         This is an interrupt service routine for the nanochip.
  *         It calls the nanoInterrupt service routine.
  * @param  None
  * @retval None
  */

void EXTI9_5_IRQHandler(void)
{
  portBASE_TYPE  xHigherPriorityTaskWoken = pdFALSE;
  if(EXTI_GetITStatus(EXTI_Line5) != RESET)
  {
    //g_interrupt_cnt++;
    EXTI_ClearITPendingBit(EXTI_Line5);
    //xQueueSendFromISR(nanoisr, NULL,&xHigherPriorityTaskWoken);
    //xSemaphoreGiveFromISR( nanoisr, &xHigherPriorityTaskWoken );
    /* Switch tasks if necessary. */
    if( xHigherPriorityTaskWoken != pdFALSE )
    {
      portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    }
  }
}


