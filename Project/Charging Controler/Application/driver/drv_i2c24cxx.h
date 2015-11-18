/***********************************************************************************************
*                      Copyright (C)���ݿ�Դ�����з���
*�ļ�����iic24cxx.h
*�ļ����������ļ�����IIC�洢��оƬ�Ĵ󲿷��ͺŵ���������ͷ�ļ�����ֲ��ֻҪ�޸ļ����ؼ�����ĺ�
*   �Ϳ���ʹ��������
*   IIC����ģ��I2Cʱ������Ҫʹ��Ӳ��IICʱ�������ñ���������
*   ������������������ļ���iic24cxx.h��iic24cxx.c
*�����ͺţ�FM24_64          2011��3��11��
*          24_04            2011��3��11��
*          24_08            2011��3��11��
*          24_256           2011��3��11��
*          24_01            2011��12��30��
*          24_02            2011��12��30��
*          24_08            2011��12��30��
*          24_16            2011��12��30��
*          24_64            2011��12��30��
*          24_128           2011��12��30��
*          24_512           2011��12��30��
*���Ͷ��壺typedef unsigned char u8
*          typedef unsigned short u16
*����������в���֮��������ϵczhf201@163.com
*���ߣ����Է�
*����ʱ�䣺2011��3��09��
*��ǰ�汾��V1.0
*�޸ļ�¼
*----------------------------------------------------------------------------------------------
*
***********************************************************************************************/
#ifndef __IIC24CXX_H
#define __IIC24CXX_H
/*�����ﶨ�����оƬ�ͺ�*/
//#define IIC_24_01
//#define IIC_24_62
//#define IIC_24_04
//#define IIC_24_08
//#define IIC_24_64
//#define IIC_24_128
//#define IIC_24_256
//#define IIC_FM24_64
#define IIC_24_32
//typedef unsigned char u8
//typedef signed char         s8
//typedef unsigned short u16
//typedef signed short  s16

/*�����ﶨ��Ӳ����·���Ƿ�������д��������*/
//#define IIC24_WP

/*�����ﶨ��IICоƬ��SCL��SDA��WP�ȹܽţ������ز����Ƚϸ��ӣ�����Ҫʹ�þֲ��������沿�ֲ�����
�����²��ֻ���ȫ������������iic24xx.c�ļ����Ժ�����ʽ����*/
#define Iic24SCLPin          *(uint32_t *)(PERIPH_BB_BASE + GPIOB_ODR_OFFSET * 32 + 8 * 4)//PB6
#define Iic24SDAPin          *(uint32_t *)(PERIPH_BB_BASE + GPIOB_ODR_OFFSET * 32 + 9 * 4)//PB7
#define Iic24ReadSDA()       *(uint32_t *)(PERIPH_BB_BASE + GPIOB_IDR_OFFSET * 32 + 9 * 4)//PB7��
#define Iic24SCLInput()      // GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)
#define Iic24SCLOutput()    do{GPIOB->MODER &= ~(0x3 << (8 * 2));GPIOB->MODER |=  0x01 << (8 * 2);}while(0)//SCl�������
#define Iic24SDAInput()     do{GPIOB->MODER  &= ~(0x3 << (9 * 2));}while(0)//SDA��������
#define Iic24SDAOutput()    do{GPIOB->MODER  &= ~(0x3 << (9 * 2));GPIOB->MODER |=  0x01 << (9 * 2);}while(0)//SDA�������

#define IicSCLInput()       Iic24SCLInput()
#define IicSCLOutput()      Iic24SCLOutput()
#define IicSDAInput()       Iic24SDAInput()
#define IicSDAOutput()      Iic24SDAOutput()
#define IicResetSCLPin()    Iic24SCLPin = 0
#define IicSetSCLPin()      Iic24SCLPin = 1
#define IicResetSDAPin()    Iic24SDAPin = 0
#define IicSetSDAPin()      Iic24SDAPin = 1
#define IicReadSDA()        Iic24ReadSDA()

/***********************************************************************************************
IIC����ĳ�ʼ�����������ڶ�IIC���߽������ȳ�ʼ�����ú��Ӧָ�������������IIC�������г�ʼ��֮ǰ
���á�
����͵�Ӧ��������ARMϵ��CPU���ȴ�SCL��SDAʱ�ӡ����ȶ���˿����Եȡ�
***********************************************************************************************/
#ifndef IIC_EXTRA_INIT
#define IIC_EXTRA_INIT
#endif

/*�ر�ȫ���жϣ������ж��������ϵͳʱ��Ҫ�����ٽ�δ����������ײ�Ĵ������ڵ�����ϵͳ�иú���Բ�����*/
#define IicCloseIsr()       

#ifdef IIC24_WP
#define IicWPPin
#define IicWPOpen() 
#define IicWPClose() 
#define IicWPInput()
#define IicWPOutput()
#endif

/*��ʱ���ڣ��û�������Ҫ��CPUƵ���޸�*/
#define IicNop() {u8 __coun = 2; do{__ASM("nop");__ASM("nop");}while(__coun--);}

//#define IicNop() do{volatile int i = 20; while (i)i--;}while(0)

/*api*/
#define EEPROMInit()        Iic24Init()
#define WriteEEPROM(a,b,c)  Iic24WriteBytes(3,a,b,c)
#define ReadEEPROM(a,b,c)   Iic24ReadBytes(3,a,b,c)

void WriteCharToEEPROM(uint16 uin_Addr, uint8 uch_Data);
void WriteIntToEEPROM(uint16 uin_Addr, uint16 uin_Data);
void WriteLongToEEPROM(uint16 uin_Addr, uint32 ul_Data);
void WriteFloatToEEPROM(uint16 uin_Addr, fp32 f_Data);
uint8 ReadCharFromEEPROM(uint16 uin_Addr);
uint16 ReadIntFromEEPROM(uint16 uin_Addr);
uint32 ReadLongFromEEPROM(uint16 uin_Addr);
fp32 ReadFloatFromEEPROM(uint16 uin_Addr);

#ifdef IIC_24_01
#define BLOCK_ADDR_BIT_LENGTH 0     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    1       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 8             //ҳ������С

#elif defined IIC_24_02
#define BLOCK_ADDR_BIT_LENGTH 0     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    1       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 8             //ҳ������С

#elif defined IIC_24_04
#define BLOCK_ADDR_BIT_LENGTH 1     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    1       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 16            //ҳ������С

#elif defined IIC_24_08
#define BLOCK_ADDR_BIT_LENGTH 2     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    1       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 16            //ҳ������С

#elif defined IIC_24_16
#define BLOCK_ADDR_BIT_LENGTH 3     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    1       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 16            //ҳ������С

#elif defined IIC_24_32
#define BLOCK_ADDR_BIT_LENGTH 0     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    2       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 32    

#elif defined IIC_24_64
#define BLOCK_ADDR_BIT_LENGTH 0     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    2       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 32            //ҳ������С

#elif defined IIC_24_128
#define BLOCK_ADDR_BIT_LENGTH 0     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    2       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 64            //ҳ������С

#elif defined IIC_24_256
#define BLOCK_ADDR_BIT_LENGTH 0     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    2       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 64            //ҳ������С

#elif defined IIC_24_256
#define BLOCK_ADDR_BIT_LENGTH 0     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    2       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 64            //ҳ������С

#elif defined IIC_FM24_64
#define BLOCK_ADDR_BIT_LENGTH 0     //���ַλ��������С�ڵ���3
#define ADDR_BYTE_LENGTH    2       //�洢����ַ�ֽڳ���
#define IIC_PAGE_SIZE 0             //ҳ������С

#endif

/*IIC���߳�ʼ��*/
extern void Iic24Init(void);
/***********************************************************************************************
*������:Iic24WriteBytes
*����:��Iic24оƬд�����ֽ�
*���:chip_addr,оƬ��ַ����λA2,A1,A0,src ����Դ,num,���ݸ���
*����:��
***********************************************************************************************/
extern void Iic24WriteBytes(u8 chip_addr,u16 addr,u8 *src,u16 num);
/***********************************************************************************************
*������:Iic24ReadBytes
*����:��ȡIic24�洢оƬ�ƶ���ַ�����ɸ��ֽ�
*���:chip_addr,оƬ��ַ����λA2,A1,A0,dst,�������ݵ�ַ,num,���ݸ���
*����:��
***********************************************************************************************/
extern void Iic24ReadBytes(u8 chip_addr,u16 addr,u8 *dst,u16 num);

#endif
