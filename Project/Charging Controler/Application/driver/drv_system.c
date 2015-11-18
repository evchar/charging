#include "stm32f2xx.h"
#include "drv_system.h"
#include "user_typedefine.h"
#include "drv_adc.h"
#include "drv_led.h"
#include "drv_i2c24cxx.h"
#include "global_value.h"

void IWDG_Configuration(void)
{
    //��Ƶ��Ϊ32,����ֵΪ2500,���ʱ��Ϊ2s
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//ʹ�ܶԼĴ���IWDG_PR��IWDG_RLR��д����
    while (IWDG->SR & 0x01);
    IWDG_SetPrescaler(IWDG_Prescaler_32);//����IWDGԤ��Ƶֵ:����IWDGԤ��ƵֵΪ32
    while (IWDG->SR & 0x02);
    IWDG_SetReload(2500);//����IWDG��װ��ֵ
    IWDG_ReloadCounter();//����IWDG��װ�ؼĴ�����ֵ��װ��IWDG������
    IWDG_Enable();//ʹ��IWDG
    //RCC_LSICmd(ENABLE);
    //while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable the GPIO_LED Clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG , ENABLE);//LDO
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);//LDO
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//EEPROM
    /* Configure the GPIO_LED pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    /* Configure the GPIO_LED pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    /****************EEPROM***************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);

}

void System_Configuration(void)
{
    #ifdef  CONFIG_CONSOLE
    console_init();
    #endif  /* CONFIG_CONSOLE */
    GPIO_Configuration();
    ETH_PWR_ON();
    RF0_PWR_ON();  

    EEPROMInit();
    /* configure LED Light */ 
    LED_Configuration();
}

//����ϵͳʱ��,ʹ�ܸ�����ʱ��
void RCC_Configuration(void)
{
    //SystemInit(); //�����ٴε��ã���startup�ѵ��ô˺�����Ĭ��120M��Ƶ
    #ifdef  CONFIG_CONSOLE
    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//console
    /* Enable USART1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//console
    #endif  /* CONFIG_CONSOLE */

    /* Enable the SPI clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);  //NA5TR1
     /* Enable GPIO clocks */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  //NA5TR1
    
    #ifdef CONFIG_NTRX_IRQ
    /* Enable GPIOB clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    #endif  /* CONFIG_NTRX_IRQ */
       
}

//ϵͳ�жϹ���
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    //�����������λ�ú�ƫ��
    #ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x20000000 */
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);//������λ��RAM
    #else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000);//������λ��FLASH
    #endif
    
    /* Configure one bit for preemption priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    #ifdef  CONFIG_CONSOLE
    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    #endif  /* CONFIG_CONSOLE */

    /* Enable the USART3 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
   
    #ifdef CONFIG_NTRX_IRQ
    /* Enable and set EXTI Line5 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    #endif  /* CONFIG_NTRX_IRQ */
    
    //��ʱ��2�ж�
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    #ifdef CONFIG_ETH_IRQ
      /* Enable the Ethernet global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);    
    #endif
}

//������������
void Init_All_Periph(void)
{
    RCC_Configuration();

    NVIC_Configuration();

    System_Configuration();
}

