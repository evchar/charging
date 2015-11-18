#include "stm32f10x.h"
#include "user_lib.h"
#include "drv_adc.h"
#include "drv_led_relay.h"
#include "drv_keys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
//qepn
#include "qpn_port.h"
#include "qepn.h"
#include "qassert.h"
#include "charging.h"

#include <stdio.h>
u16 __IO Adc1DMAValue[10];//ADC1 DMA�ɼ�������
extern xQueueHandle     msgQueue;
/**
  * @brief  ADC1 Channel Vbat configuration (DMA, ADC, CLK)
  * @param  None
  * @retval None
  */
void ADC1_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  RCC_ADCCLKConfig(RCC_PCLK2_Div6);//72M/6=12,ADC���ʱ�Ӳ��ܳ���14M
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1,ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
  /* Configure ADCCLK such as ADCCLK = PCLK2/6 */

  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//AIN3
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;//DMA����ADC����ַ
  DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)Adc1DMAValue;//DMA�ڴ����ַ
  DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;//�ڴ���Ϊ���ݴ����Ŀ�ĵ�
  DMA_InitStructure.DMA_BufferSize         = 10;//DMAͨ����DMA����Ĵ�С
  DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;//�����ַ�Ĵ�������
  DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;//�ڴ��ַ�Ĵ�������
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//���ݿ��Ϊ16λ
  DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;//���ݿ��Ϊ16λ
  DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;//������ѭ������ģʽ
  DMA_InitStructure.DMA_Priority           = DMA_Priority_High;//DMAͨ��xӵ�и����ȼ�
  DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;//DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
  DMA_Cmd(DMA1_Channel1, ENABLE);

  /* ADC1 configuration ------------------------------------------------------*/
  ADC_DeInit(ADC1);
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//ADC����ģʽ:ADC1�����ڶ���ģʽ
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;//ģ��ת�������ڶ�ͨ��ģʽ
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//DISABLE;//ģ��ת�������ڵ���ת��ģʽ
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//ת��������������ⲿ��������
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//ADC�����Ҷ���
  ADC_InitStructure.ADC_NbrOfChannel = 10;//˳����й���ת����ADCͨ������Ŀ
  ADC_Init(ADC1, &ADC_InitStructure);

  for(uint8 i=1; i<11; i++)
    ADC_RegularChannelConfig(ADC1,ADC_Channel_13, i, ADC_SampleTime_55Cycles5);//ǰ��5·�����¶�


#if 0
  /* ADC1 regular channel16 configuration */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);//ADC1,ADCͨ��16,�������˳��ֵΪ1,����ʱ��Ϊ239.5����
  /* Enable the temperature sensor and vref internal channel */
  ADC_TempSensorVrefintCmd(ENABLE);
#endif

  ADC_DMACmd(ADC1, ENABLE);
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibaration register */
  ADC_ResetCalibration(ADC1);//����ָ����ADC1��У׼�Ĵ���
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));//��ȡADC1����У׼�Ĵ�����״̬,����״̬��ȴ�
  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);//��ʼָ��ADC1��У׼״̬
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));//��ȡָ��ADC1��У׼����,����״̬��ȴ�

  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);//ʹ��ָ����ADC1�����ת����������
}



void insertionSort(uint16_t *numbers, uint32_t array_size)
{
    uint32_t i, j;
    uint32_t index;

  for (i=1; i < array_size; i++) {
    index = numbers[i];
    j = i;
    while ((j > 0) && (numbers[j-1] > index)) {
      numbers[j] = numbers[j-1];
      j = j - 1;
    }
    numbers[j] = index;
  }
}

uint32_t interquartileMean(uint16_t *array, uint32_t numOfSamples)
{
    uint32_t sum=0;
    uint32_t  index;
    /* discard  the lowest and the highest data samples */

    for (index = 1; index < 9; index++){
            sum += array[index];
    }
    /* return the mean value of the remaining samples value*/
    return (uint32_t)( sum / 8.0 );
}




void processTempData(void)
{
  uint32_t tempAVG;
  uint8 temp = 0;
  static uint8 uch_last_msg = 0;
  /* sort received data in */
  insertionSort((uint16*)Adc1DMAValue, 10);

  /* Calculate the Interquartile mean */
  tempAVG = interquartileMean((uint16*)Adc1DMAValue, 10);

  printf("%f\r\n",tempAVG*3.3/4095);
  
   if(tempAVG > 459 && tempAVG < 657)
  {
    temp = Q_STANDBY_SIG;
  }
  else if(tempAVG > 831 && tempAVG < 1029)
  {
    temp = Q_CAR_DETECTED_SIG;
  }
  else if(tempAVG > 1203 && tempAVG < 1402)
  {
    temp = Q_CAR_REDAY_SIG;
  }
  else
  {
    temp = 0xff;
  }  
  
  if(temp != uch_last_msg && 0xff != temp)
  {
    uch_last_msg = temp;
    xQueueSend(msgQueue, &temp, 0);
  } 
  
}
