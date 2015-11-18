#include "stm32f10x.h"
#include "user_lib.h"
#include "drv_led_relay.h"
#include "drv_timer.h"
u16 pluse;

void TIM1_init_MY(void)
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;  

	//tim1_time=100;//3?ê??ˉ?óê±1S.
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;    //?üD?ê??t 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //?à??ó??è??0 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;          //?ìó|ó??è??2 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             //?êDí?D?? 
	NVIC_Init(&NVIC_InitStructure);                             //D′è?éè??
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE );
	/* Time Base configuration */
	TIM_DeInit(TIM1);
	TIM_TimeBaseStructure.TIM_Prescaler = 7199;                   //éè???¤·??μ?÷·??μ?μêy71￡??′APB2=72M, TIM1_CLK=72/72=1MHz ￡?															//?üμ?è??μ±?D??ú0x0000oí0xFFFF????
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ?òé???êy?￡ê?
//	TIM_TimeBaseStructure.TIM_Period = 99;	               // 1ms?¨ê±￡???êy?÷?òé???êyμ?1000oó2úéú?üD?ê??t￡???êy?μ1éá?
	TIM_TimeBaseStructure.TIM_Period = 1000;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;			   //éè??á??¨ê±?÷ê±?ó·???￡?
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;		  // éè??á??ü?ú??êy?÷?μ￡??üμ?è??μ±?D??ú0x00oí0xFF?????￡
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);	         // ?ù?YTIM_TimeBaseInitStruct?D???¨μ?2?êy3?ê??ˉTIMxμ?ê±???ùêyμ￥?? 
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);      //???D??￡?ò??aò???ó??D??oóá￠?′2úéú?D??   
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE); //ê1?üTIM1?D???′  	 
	TIM_Cmd(TIM1, ENABLE); 		             //TIM1×ü?a1?￡o?a??	             //TIM1×ü?a1?￡o?a??
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
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//使能定时器3时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIO外设
  
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//TIM1-CH1
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOA, &GPIO_InitStructure);
  


  TIM_DeInit(TIM2);
  
  pluse=400;
  TIM_TimeBaseStructure.TIM_Period = 2000;//1ms
  TIM_TimeBaseStructure.TIM_Prescaler = 35;//36分频
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//72M
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  //TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  
  /* PWM1 模式配置: Channel3 Channel4*/
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//比较输出使能
  TIM_OCInitStructure.TIM_Pulse = 200;//340;//400*g_InsProperty.uch_LCDLightGrade;//高电平宽度,此时占空比
  //TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//输出极性:TIM输出比较极性高
  TIM_OC4Init(TIM2, &TIM_OCInitStructure);//根据TIM_OCInitStruct中指定的参数初始化外设TIMx
  TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);//使能TIMx在CCR3上的预装载寄存器
  TIM_ARRPreloadConfig(TIM2, ENABLE);//使能TIMx在ARR上的预装载寄存器
  
  STOP_PWM();
  TIM_Cmd(TIM2, ENABLE);//使能TIMx外设
  
  
}
