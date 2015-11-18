#include "stm32f10x.h"
#include "user_lib.h"
#include "drv_led_relay.h"
#include "drv_timer.h"
u16 pluse;

void TIM1_init_MY(void)
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;  

	//tim1_time=100;//3?��??��?������1S.
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;    //?��D?��??t 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //?��??��??��??0 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;          //?����|��??��??2 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             //?��D��?D?? 
	NVIC_Init(&NVIC_InitStructure);                             //D�䨨?����??
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE );
	/* Time Base configuration */
	TIM_DeInit(TIM1);
	TIM_TimeBaseStructure.TIM_Prescaler = 7199;                   //����???�衤??��?�¡�??��?�̨�y71��??��APB2=72M, TIM1_CLK=72/72=1MHz ��?															//?����?��??�̡�?D??��0x0000o��0xFFFF????
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ?����???��y?�꨺?
//	TIM_TimeBaseStructure.TIM_Period = 99;	               // 1ms?��������???��y?��?����???��y��?1000o��2������?��D?��??t��???��y?��1����?
	TIM_TimeBaseStructure.TIM_Period = 1000;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;			   //����??��??������?�¨���?����???��?
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;		  // ����??��??��?��??��y?��?�̡�??����?��??�̡�?D??��0x00o��0xFF?????��
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);	         // ?��?YTIM_TimeBaseInitStruct?D???����?2?��y3?��??��TIMx��?����???����y�̣�?? 
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);      //???D??��?��??a��???��??D??o������?��2������?D??   
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE); //��1?��TIM1?D???��  	 
	TIM_Cmd(TIM1, ENABLE); 		             //TIM1����?a1?��o?a??	             //TIM1����?a1?��o?a??
}

u16 Waiting_time;
u8 tmpi;
u16 tmpfluse[2]={360,400};
extern u16 pluse;
void TIM1_UP_IRQHandler(void)
{

	Waiting_time++;
        if(Waiting_time==120)
        {
          Waiting_time=0;
          pluse=tmpfluse[tmpi%2];
          tmpi++;
          if(tmpi==100)
            tmpi=1;          
//          TIM_OCInitStructure.TIM_Pulse = pluse;
//          TIM_OC4Init(TIM2, &TIM_OCInitStructure);
          TIM2->CCR4=pluse;
        }
	TIM_ClearITPendingBit(TIM1, TIM_FLAG_Update); //???D??	
}

void TIM2_Configuration(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  
  //GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//ʹ�ܶ�ʱ��3ʱ��
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //ʹ��GPIO����
  
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//TIM1-CH1
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOA, &GPIO_InitStructure);
  


  TIM_DeInit(TIM2);
  
  pluse=400;
  TIM_TimeBaseStructure.TIM_Period = 2000;//1ms
  TIM_TimeBaseStructure.TIM_Prescaler = 35;//36��Ƶ
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//72M
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  //TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  
  /* PWM1 ģʽ����: Channel3 Channel4*/
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//�Ƚ����ʹ��
  TIM_OCInitStructure.TIM_Pulse = 200;//340;//400*g_InsProperty.uch_LCDLightGrade;//�ߵ�ƽ���,��ʱռ�ձ�
  //TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//�������:TIM����Ƚϼ��Ը�
  TIM_OC4Init(TIM2, &TIM_OCInitStructure);//����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx
  TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);//ʹ��TIMx��CCR3�ϵ�Ԥװ�ؼĴ���
  TIM_ARRPreloadConfig(TIM2, ENABLE);//ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
  
  STOP_PWM();
  TIM_Cmd(TIM2, ENABLE);//ʹ��TIMx����
  
  
}
