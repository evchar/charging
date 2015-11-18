#include "rn8209.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "global_value.h"

#define RN8209_CS     PAout(4)
#define RN8209_MISO   PAin(6)
#define RN8209_MOSI   PAout(7)
#define RN8209_CLK    PAout(5)

void PF_Init(void);

void DelayNus(u32 nus)
{
  u16 i=0;
  while(nus--)
  {
    i = 9;
    while(i--) ;
  }
}

//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ������RN8209
void RN8209_SPI_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );//PORTAʱ��ʹ��

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;//MISO
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5 | GPIO_Pin_7;  // CKL MOSI CS
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA

  RN8209_CS = 1;        //SPI ��ѡ��
  RN8209_CLK = 0;
  RN8209_MOSI = 1;

}

/***********************************************************************
* ��������: void RN8209_SPI_Write(u8 bdata)
* ��������: д��1���ֽ����ݵ�8209��
* �������: bdata-����
* �������: ��
***********************************************************************/
void RN8209_SPI_Write(u8 bdata)
{
  u8 i;
  u8 byte;
  //portENTER_CRITICAL();
  for(i=0;i<8;i++)
  {
    byte = bdata & 0x80;
    if (byte==0)
    {
      RN8209_MOSI = 0;
    }
    else
    {
      RN8209_MOSI = 1;
    }
    RN8209_CLK = 1;
    DelayNus(Time_CLK);//ʱ��Ƶ��
    RN8209_CLK = 0;
    DelayNus(Time_CLK);
    bdata <<= 1;
  }
  RN8209_MOSI = 1;
  DelayNus(Time_T1);//T1
  //portEXIT_CRITICAL();
  return;
}
/***********************************************************************
*��������: u8 RN8209_SPI_Read(void)
*��������: ��ȡ8209һ�ֽ�����
*�������: ��
*�������: ��ȡ��1�ֽ�����
***********************************************************************/
u8 RN8209_SPI_Read(void)
{
  u8 i,rbyte=0;
  //portENTER_CRITICAL();
  for(i=0;i<8;i++)
  {
    RN8209_CLK = 1;
    DelayNus(Time_CLK);
    rbyte <<= 1;
    RN8209_CLK = 0;         //�½��ض�ȡ����
    if (RN8209_MISO == 1)
    {
      rbyte |= 1;
    }
    DelayNus(Time_CLK);
  }
  RN8209_CLK = 0;
  DelayNus(Time_T2);//T2
  //portEXIT_CRITICAL();
  return(rbyte);
}

/***********************************************************************
* ��������: u8 RN8209_ReadData(u8 add,u8 *data,u8 *len)
* ��������: ��ȡ8209�Ĵ�������
* �������: add-8209�Ĵ�����ַ *data-��ȡ����õĻ����� *len-���ݳ���
* �������: 0-��ȡʧ�� 1-��ȡ�ɹ�
***********************************************************************/
u8 RN8209_ReadData(u8 add,u8 *data,u8 *len)
{
  u8 ret=1;

  RN8209_CS = 0;
  DelayNus(Time_CS);//CS���ͺ�һ��ʱ�䣬��CLk
  RN8209_SPI_Write(add);

  switch(add)
  {
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
  case 0x06:
  case 0x09:
  case 0x0A:
  case 0x0B:
  case 0x0C:
  case 0x0D:
  case 0x0E:
  case 0x0F:
  case 0x10:
  case 0x20:
  case 0x21:
  case 0x25:
  case 0x45:
    *data = RN8209_SPI_Read();
    *(data+1) = RN8209_SPI_Read();
    *len = 2;
    break;
  case 0x07:
  case 0x08:
  case 0x40:
  case 0x41:
  case 0x42:
  case 0x43:
    *data =RN8209_SPI_Read();
    *len = 1;
    break;
  case 0x22:
  case 0x23:
  case 0x24:
  case 0x29:
  case 0x2A:
  case 0x2B:
  case 0x2C:
  case 0x2D:
  case 0x7f :
    *data = RN8209_SPI_Read();
    *(data+1) = RN8209_SPI_Read();
    *(data+2) = RN8209_SPI_Read();
    *len = 3;
    break;
  case 0x26 :
  case 0x27 :
  case 0x28 :
  case 0x44 :
    *data = RN8209_SPI_Read();
    *(data+1) = RN8209_SPI_Read();
    *(data+2) = RN8209_SPI_Read();
    *(data+3) = RN8209_SPI_Read();
    *len = 4;
    break;
  default :
    ret = 0;
    break;
  }

  RN8209_CS = 1;
  return(ret);
}

/***********************************************************************
* ��������: void RN8209_WriteData(u8 *ptr)
* ��������: д�����ݵ�8209�Ĵ�����
* �������: *ptr-ָ��Ҫд������ݻ�����
* �������: ��
***********************************************************************/
void RN8209_WriteData(u8 *ptr)
{
  u8 temp[2];

  if ((*ptr) > 0x10)      /* ���ƼĴ�����ַ������0x10 */
  {
    return;
  }

  RN8209_CS = 0;
  DelayNus(Time_CS);//CS���ͺ�һ��ʱ�䣬��CLk

  temp[0] = 0xea;        /* дʹ������*/
  temp[1] = 0xe5;
  RN8209_SPI_Write(temp[0]);
  RN8209_SPI_Write(temp[1]);

  RN8209_SPI_Write((*ptr)+0x80);   /* д������ */
  if ( (*ptr != 0x07) && (*ptr != 0x08) )
  {
    RN8209_SPI_Write(*(ptr+1));
  }
  RN8209_SPI_Write(*(ptr+2));

  temp[0] = 0xea;       /* д�������� */
  temp[1] = 0xdc;
  RN8209_SPI_Write(temp[0]);
  RN8209_SPI_Write(temp[1]);

  DelayNus(Time_CS);
  RN8209_CS = 1;

  return;
}


/***********************************************************************
* ��������: RN8209_WriteCmdReg(u8 cmd)
* ��������: д�����ݵ�8209�Ĵ�����
* �������: *ptr-ָ��Ҫд������ݻ�����
* �������: ��
***********************************************************************/
void RN8209_WriteCmdReg(u8 cmd)
{
  RN8209_SPI_Write(RN8209_EA);
  RN8209_SPI_Write(cmd);
}

/***********************************************************************
* ��������: void RN8209_DisablePulse(void)
* ��������: �ر�8029�������
* �������: *ptr-ָ��Ҫд������ݻ�����
* �������: ��
***********************************************************************/
void  RN8209_DisablePulse(void)
{
   u8 temp[3];

   temp[0] = 0x01;
   temp[1] = 0x00;
   temp[2] = 0x00;
   RN8209_WriteData(&temp[0]);

   return;
}

/***********************************************************************
* ��������: void RN8209_EnablePulse(void)
* ��������: �ر�8029�������
* �������: *ptr-ָ��Ҫд������ݻ�����
* �������: ��
***********************************************************************/

void  RN8209_EnablePulse(void)
{
   u8 temp[3];

   temp[0] = 0x01;
   temp[1] = 0x00;
   temp[2] = 0x01;
   RN8209_WriteData(&temp[0]);

   return;
}

/***********************************************************************
* ��������: void RN8209_CalJBCS(void)
* ��������: ����У�������д�뵽8209�Ĵ�����EEPROM��
* �������: regadr-8209�Ĵ�����ַ *ptr-ָ��Ҫд������ݻ�����
* �������: ��
***********************************************************************/
void  RN8209_CalJBCS(u8 regadr,u8 *ptr)
{

  u8 tmpbyte,tmpadr;
  u8 tmpbuf[5];
  u16 tmpcal,tmpu16;
  u32 tmp8209P,tmprealP;
  float tmpfloat;

  switch (regadr)
  {
  case 0x02:     /*HFConst����*/
    tmpbuf[1] = *(ptr+1);
    tmpbuf[2] = *(ptr);
    break;
  case 0x03:     /*��������ֵ����*/
    /*̨�Ӵ�0.2%Ib,�����й�����,ȡ�м�����λ�����й������Ĵ���03��*/
    RN8209_ReadData(RN8209_REG_PA,&tmpbuf[0],&tmpbyte);
    break;
  case 0x05:     /*�й�����1.0У��*/
  case 0x07:    /*�й�����0.5LУ��*/
    //tmp8209P = ((u32)(BN8209_CS.P[2]) << 16) + ((u32)(BN8209_CS.P[1]) << 8) + BN8209_CS.P[0];
    //tmp8209P &= 0x7fffff;    /*//////���ι��ʷ���λ/////*/
    //tmprealP = ((u32)(*(ptr+2)) << 16) + ((u32)(*(ptr+1)) << 8) + (*(ptr));
    //tmp8209P = BCDLongTOHexlong(tmp8209P); /*RN8209����ֵ*/
    //tmprealP = BCDLongTOHexlong(tmprealP); /*У��̨����ֵ*/

    if(regadr == 0x05)
    {

      if(tmprealP > tmp8209P)    /*���ƫ��*/
      {

        tmpfloat = ((float)(tmprealP - tmp8209P)) / ((float)(tmprealP));
        tmpfloat = tmpfloat/(1.0000-tmpfloat);
        tmpu16 = (u16)(32768*tmpfloat);

      }
      else                       /*���ƫ��*/
      {

        tmpfloat = ((float)(tmp8209P - tmprealP)) / ((float)(tmprealP));
        tmpfloat = tmpfloat/(1.0000+tmpfloat);
        tmpu16 = (u16)(32768*(2.0000-tmpfloat));

      }

    }
    else if(regadr == 0x07)
    {

      if(tmprealP > tmp8209P)    /*���ƫ��*/
      {

        tmpfloat = ((float)(tmprealP - tmp8209P)) / ((float)(tmprealP));
        tmpu16 = (u16)(1654*tmpfloat);

      }
      else
      {

        tmpfloat = ((float)(tmp8209P - tmprealP)) / ((float)(tmprealP));
        tmpu16 = (u16)(256-(u8)(1654*tmpfloat));
      }

    }

    tmpbuf[1] = (u8)(tmpu16 >> 8);
    tmpbuf[2] = (u8)(tmpu16);
    break;
  case 0x12:   /*������ЧֵУ��*/
  case 0x13:   /*���ߵ�����ЧֵУ��*/
  case 0x14:   /*��ѹ��ЧֵУ��*/
    if((regadr == 0x12) || (regadr == 0x13))
    {

      if(regadr == 0x12)
      {

        tmpadr = RN8209_REG_IA;

      }
      else
      {

        tmpadr = RN8209_REG_IB;

      }
      tmprealP = ((u32)(*(ptr+2)) << 16) + ((u32)(*(ptr+1)) << 8) + (*(ptr));
      //tmprealP = BCDLongTOHexlong(tmprealP); /*RN8209����ֵ*/
      tmpcal = 1000;

    }
    else
    {

      tmpadr = RN8209_REG_U;
      //tmprealP = (u32)(BcdTOHexInt(((u16)(*(ptr+1)) << 8) + (*(ptr))));
      tmpcal = 10;

    }

    RN8209_ReadData(tmpadr,&tmpbuf[0],&tmpbyte);  /*��ȡ�Ĵ�����ֵ*/
    tmp8209P = ((u32)(tmpbuf[0]) << 16) + ((u32)(tmpbuf[1]) << 8) + tmpbuf[2];
    tmprealP = (u32)((tmpcal*tmp8209P)/tmprealP); /*������ڼĴ���ֵ����ʵ��ֵ*/

    tmpbuf[1] = (u8)(tmprealP >> 16);
    tmpbuf[2] = (u8)(tmprealP >> 8);
    tmpbuf[3] = (u8)(tmprealP);
    break;
  case 0x0A:     /*�й�����offsetУ��*/
    /*̨�������5%Ib,1.0,����ʱ������,�����ֵ,�õĻ��Ͳ�������*/
    RN8209_ReadData(RN8209_REG_PA,&tmpbuf[0],&tmpbyte); /*��ȡ�Ĵ�����ֵ*/
    tmpu16 = ((u16)(tmpbuf[2]) << 8) + tmpbuf[3];
    tmpu16 = (~tmpu16)+1;
    tmpbuf[1] = (u8)(tmpu16 >> 8);
    tmpbuf[2] = (u8)(tmpu16);
    break;
  case 0x0E:     /*������ЧֵoffsetУ��*/
  case 0x0f:     /*���ߵ�����ЧֵoffsetУ��*/
    if(regadr == 0x0e)
    {

      tmpadr = RN8209_REG_IA;

    }
    else if(regadr == 0x0f)
    {

      tmpadr = RN8209_REG_IB;

    }
    RN8209_ReadData(RN8209_REG_IA,&tmpbuf[0],&tmpbyte); /*��ȡ�Ĵ�����ֵ*/

    tmpu16 = ((u16)(tmpbuf[1]) << 8) + tmpbuf[2];
    tmp8209P = ((u32)(tmpu16))*tmpu16;
    tmp8209P = (~tmp8209P)+1;

    tmpbuf[1] = (u8)(tmp8209P >> 16);
    tmpbuf[2] = (u8)(tmp8209P >> 8);
    break;
  case 0xff:     /*У���ʼ��*/
    //RN8209_PInitDefault();
    //RN8209_LoadJBCS();

    return;
    break;
  default:break;

  }

  tmpbuf[0] = regadr;     /*д��У�������EEP�к�RN8209��*/
  //RN8209_WriteDataToEE(&tmpbuf[0]);
  RN8209_WriteData(&tmpbuf[0]);

  if(regadr == 0x02)      /*���㹦�ʻ���ϵ��*/
  {

    tmpu16 = ((u16)(tmpbuf[1]) << 8)+tmpbuf[2];
    //   tmprealP = ((u32)(tmpu16))*1600;
    //tmpfloat = ((float)(tmpu16))*133.3013*(IMPULSE_CONSTANT);
    tmprealP = (u32)(tmpfloat);
    //UlongTOUcharbuf(tmprealP,&tmpbuf[1]);
    tmpbuf[0] = 0x11;
    //RN8209_WriteDataToEE(&tmpbuf[0]);

  }

  //RN8209_CalChkSum();   /* ����8209У�������ۼ�У��� */

  return;

}

void  RN8209_Chip_Init(void)
{
  u32 chipID;
  u8 length;
  u8 temp[3];

  RN8209_SPI_Init();
  
  PF_Init();
  
  /*rn8209c reset*/
  RN8209_WriteCmdReg(RN8209_CMD_RESET);

  /*�ȴ���λ���*/
  DelayNus(500000);

  /*read chip ID*/
  chipID = 0;
  RN8209_ReadData(REG_DEVID,(u8*)&chipID,&length);
  printf("chip id:0x%x\r\n",chipID);

  /*ѡ��Aͨ��*/
  RN8209_WriteCmdReg(RN8209_PATH_A);

  /*��ѹ����ͨ�������Ϊ1*/
  temp[0] = REG_SYSCON;
  temp[1] = 0x00;
  temp[2] = 0x00;
  RN8209_WriteData(&temp[0]);

  /*���ܼĴ���Ϊ�ۼ��ͣ�ʹ��PF����������й����ܼĴ����ۼ�*/
  temp[0] = REG_EMUCON;
  temp[1] = 0x00;
  temp[2] = 0x01;
  RN8209_WriteData(&temp[0]);

  /*
  HFCost = INT[(14.8528*Vu*Vi*10^11 ) / (Un*Ib*Ec)]

  HFCost = 14.8528*Vu*Vi*10^11/Un*Ib*Ec  18904

  HFCost = 14.8528*0.224*0.4*10^11 / (1600*220*20) = 18904 = 0x49D8  20304//16575
  HFCost = 14.8528*0.224*0.2*10^11 / (1600*220*20) = 9452 = 0x24EC*//*��ѹ����������11370,12572*/

  /*write HFConst*/
  temp[0] = REG_HFCONST;
  temp[1] = 0x28;
  temp[2] = 0xDC;
  RN8209_WriteData(&temp[0]);
}

/***********************************************************************
* ��������: void RN8209_GET_FREQ(void)
* ��������: ��ȡ������ѹƵ��
* �������: ��
* �������: Ƶ��
***********************************************************************/
u8 RN8209_GET_UFREQ(void)
{
  u8 freq,length;
  u16 freq_reg;
  u32 chipID;
    /*read chip ID*/
  chipID = 0;
  RN8209_ReadData(REG_DEVID,(u8*)&chipID,&length);
  printf("chip id:0x%x\r\n",ntohs(chipID));
  
  RN8209_ReadData(REG_UFREQ,(u8*)&freq_reg,&length);

  freq = (u8)(3579545/8/(ntohs(freq_reg)));
  printf("freq:%d\r\n",freq);

  return freq;
}

/***********************************************************************
* ��������: void RN8209_GET_URMS(void)
* ��������: ��ȡ������ѹ��Чֵ
* �������: ��
* �������: ��ѹ��Чֵ
***********************************************************************/
u32 RN8209_GET_URMS(void)
{
  u8 length;
  int32 urms;
  u8 tem[3];
  RN8209_ReadData(REG_URMS,(u8*)tem,&length);
  
  if(tem[0]&0x80)
  {
    urms = 0;
  }
  else
  {
    urms = (tem[0]<<24) + (tem[1]<<16) + tem[2];
  }
  
  printf("urms:%d\r\n",urms);

  return urms;
}

/***********************************************************************
* ��������: void RN8209_GET_IARMS(void)
* ��������: ��ȡ����������Чֵ
* �������: ��
* �������: ��ѹ��Чֵ
***********************************************************************/
u32 RN8209_GET_IARMS(void)
{
  u8 length;
  int32 iarms;
  u8 tem[3];
  RN8209_ReadData(REG_IARMS,(u8*)tem,&length);
  
  if(tem[0]&0x80)
  {
    iarms = 0;
  }
  else
  {
    iarms = (tem[0]<<24) + (tem[1]<<16) + tem[2];
  }
  printf("iarms:%d\r\n",iarms);

  return iarms;
}



/***********************************************************************
* ��������: void RN8209_GET_ENERGYA(void)
* ��������: ��ȡ����������Чֵ
* �������: ��
* �������: ����,1/Ec kWh
***********************************************************************/
u32 RN8209_GET_ENERGYA(void)
{
  u8 length;
  u32 energy;
  
  u8 tem[3];
  
  RN8209_ReadData(REG_ENERGYPA,(u8*)tem,&length);

  energy = (tem[0]<<24) + (tem[1]<<16) + tem[2];
  printf("energy:%d\r\n",energy);

  return energy;
}


void PF_Init(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);	 

 
  
  /* Configure PB5 pin as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Clear the EXTI line 5 pending bit */
  EXTI_ClearITPendingBit(EXTI_Line5);
  

  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource5);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line5;   
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);     
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;            
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;    
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;                   
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                           
  NVIC_Init(&NVIC_InitStructure);
}

void EXTI9_5_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line5) != RESET)
  {
    //g_interrupt_cnt++;
    g_pf_cnt++;
      /* Clear the EXTI line 5 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line5);
  }
}











