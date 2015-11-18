/* IOI2C.c file
��д�ߣ�lisn3188
��ַ��www.chiplab7.com
����E-mail��lisn3188@163.com
���뻷����MDK-Lite  Version: 4.23
����ʱ��: 2012-04-25
���ԣ� ���������ڵ���ʵ���ҵ�mini IMU����ɲ���
���ܣ�
�ṩI2C�ӿڲ���API ��
ʹ��IOģ�ⷽʽ
------------------------------------ */

#include "api.h"

#define deay_nop_5()    asm("nop");asm("nop");asm("nop");asm("nop");asm("nop")


#define SCL_PIN  6
#define SDA_PIN  7

#define SCL_PIN_Bit            *(uint32_t *)(PERIPH_BB_BASE + GPIOB_ODR_OFFSET * 32 + SCL_PIN * 4)
#define SDA_PIN_Bit            *(uint32_t *)(PERIPH_BB_BASE + GPIOB_ODR_OFFSET * 32 + SDA_PIN * 4)
#define SDA_PIN_Read()         *(uint32_t *)(PERIPH_BB_BASE + GPIOB_IDR_OFFSET * 32 + SDA_PIN * 4)//GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)//

#define SCL_H          SCL_PIN_Bit = 1 //GPIO_SetBits(GPIOB, GPIO_Pin_8)///* GPIO_SetBits(GPIOB , GPIO_Pin_10)   */
#define SCL_L          SCL_PIN_Bit = 0//GPIO_ResetBits(GPIOB, GPIO_Pin_8)// /* GPIO_ResetBits(GPIOB , GPIO_Pin_10) */

#define SDA_H          SDA_PIN_Bit = 1//GPIO_SetBits(GPIOB, GPIO_Pin_9)// /* GPIO_SetBits(GPIOB , GPIO_Pin_11)   */
#define SDA_L          SDA_PIN_Bit = 0//GPIO_ResetBits(GPIOB, GPIO_Pin_9)// /* GPIO_ResetBits(GPIOB , GPIO_Pin_11) */

#define SCLOutput()  do{GPIOB->MODER &= ~(0x3 << (SCL_PIN * 2));GPIOB->MODER |=  0x01 << (SCL_PIN * 2);}while(0)//SCl�������
#define SDAInput()   do{GPIOB->MODER  &= ~(0x3 << (SDA_PIN * 2));}while(0)//SDA��������
#define SDAOutput()  do{GPIOB->MODER  &= ~(0x3 << (SDA_PIN * 2));GPIOB->MODER |=  0x01 << (SDA_PIN * 2);}while(0)//SDA�������


static void I2C_delay(void)
{
    volatile int i = 7;
    while (i)
        i--;
    //deay_nop_5();
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      void IIC_Start(void)
*��������:      ����IIC��ʼ�ź�
*******************************************************************************/
void IIC_Start(void)
{
    //API_DISABLE_ALL_INT();
    SDAOutput(); //sda�����
    SDA_H;
    SCL_H;
    I2C_delay();
    SDA_L;//START:when CLK is high,DATA change form high to low
    I2C_delay();
    SCL_L;//ǯסI2C���ߣ�׼�����ͻ��������
}


/**************************ʵ�ֺ���********************************************
*����ԭ��:      void IIC_Stop(void)
*��������:      //����IICֹͣ�ź�
*******************************************************************************/
void IIC_Stop(void)
{
    SDAOutput();//sda�����
    SCL_L;
    SDA_L;//STOP:when CLK is high DATA change form low to high
    I2C_delay();
    SCL_H;
    SDA_H;//����I2C���߽����ź�
    I2C_delay();
    //API_ENABLE_ALL_INT();
}


/**************************ʵ�ֺ���********************************************
*����ԭ��:      void IIC_Ack(void)
*��������:      ����ACKӦ��
*******************************************************************************/
void IIC_Ack(void)
{
    SCL_L;
    SDAOutput();
    SDA_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;

}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      void IIC_NAck(void)
*��������:      ����NACKӦ��
*******************************************************************************/
void IIC_NAck(void)
{
    SCL_L;
    SDAOutput();
    SDA_H;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      u8 IIC_Wait_Ack(void)
*��������:      �ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
*******************************************************************************/
uint8 IIC_Wait_Ack(void)
{
    uint8 ucErrTime=0;
    SDAInput();  //SDA����Ϊ����
    SDA_H;I2C_delay();
    SCL_H;I2C_delay();
    while(SDA_PIN_Read())
    {
        ucErrTime++;
        if(ucErrTime>50)
        {
            IIC_Stop();
            return 1;
        }
       I2C_delay();
    }
    SCL_L;  //ʱ�����0
    return 0;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      void IIC_Send_Byte(u8 txd)
*��������:      IIC����һ���ֽ�
*******************************************************************************/
void IIC_Send_Byte(uint8 txd)
{
    uint8_t i = 8;
    SDAOutput();
    SCL_L;
    while (i--) {
        if (txd & 0x80)
            SDA_H;
        else
            SDA_L;
        txd <<= 1;
        I2C_delay();
        SCL_H;
        I2C_delay();
        SCL_L;
        I2C_delay();
    }
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      u8 IIC_Read_Byte(unsigned char ack)
*��������:      //��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
*******************************************************************************/
uint8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    SDAInput();
    for(i=0;i<8;i++ )
    {
        SCL_L;
        I2C_delay();
        SCL_H;
        receive<<=1;
        if(SDA_PIN_Read())receive++;
        I2C_delay();
    }
    if (ack)
        IIC_Ack(); //����ACK
    else
        IIC_NAck();//����nACK
    return receive;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      void IIC_Init(void)
*��������:      ��ʼ��I2C��Ӧ�Ľӿ����š�
*******************************************************************************/
void IIC_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    //GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr)
*��������:      ��ȡָ���豸 ָ���Ĵ�����һ��ֵ
����    I2C_Addr  Ŀ���豸��ַ
        addr       �Ĵ�����ַ
����   ��������ֵ
*******************************************************************************/
unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr)
{
    unsigned char res=0;

    IIC_Start();
    IIC_Send_Byte(I2C_Addr);//����д����
    res++;
    IIC_Wait_Ack();
    IIC_Send_Byte(addr); res++; //���͵�ַ
    IIC_Wait_Ack();
    //IIC_Stop();
    IIC_Start();
    IIC_Send_Byte(I2C_Addr+1); res++; //�������ģʽ
    IIC_Wait_Ack();
    res=IIC_Read_Byte(0);
    IIC_Stop();//����һ��ֹͣ����

    return res;
}


/**************************ʵ�ֺ���********************************************
*����ԭ��:  u8 IICreadBytes(u8 dev, u8 reg, u8 length, u8 *data)
*��������:   ȡָ���豸 ָ���Ĵ����� length��ֵ
����    dev  Ŀ���豸��ַ
        reg   �Ĵ�����ַ
        length Ҫ�����ֽ���
        *data  ���������ݽ�Ҫ��ŵ�ָ��
����   ���������ֽ�����
********************************************************************************/
uint8 IICreadBytes(uint8 dev, uint8 reg, uint8 length, uint8 *data)
{
    uint8 count = 0;
    IIC_Start();
    IIC_Send_Byte(dev);//����д����
    IIC_Wait_Ack();
    IIC_Send_Byte(reg);//���͵�ַ
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(dev+1);//�������ģʽ
    IIC_Wait_Ack();
    for(count=0;count<length;count++){
         if(count!=length-1)data[count]=IIC_Read_Byte(1); //��ACK�Ķ�����
            else  data[count]=IIC_Read_Byte(0); //���һ���ֽ�nACK
    }
    IIC_Stop();//����һ��ֹͣ����
    return 0;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:  u8 IICreadBytes(u8 dev, u8 reg, u8 length, u8 *data)
*��������:   ȡָ���豸 ָ���Ĵ����� length��ֵ
����    dev  Ŀ���豸��ַ
        reg   �Ĵ�����ַ
        length Ҫ�����ֽ���
        *data  ���������ݽ�Ҫ��ŵ�ָ��
����   ���������ֽ�����
********************************************************************************/
uint8 SMT480TreadBytes(uint8 dev, uint8 command, uint8 reg, uint8 length, uint8 *data)
{
    uint8 count = 0;
    IIC_Start();
    IIC_Send_Byte(dev);//����д����
    IIC_Wait_Ack();
    IIC_Send_Byte(command);//���͵�ַ
    IIC_Wait_Ack();
    IIC_Send_Byte(reg);//���͵�ַ
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(dev+1);//�������ģʽ
    IIC_Wait_Ack();
    for(count=0;count<length;count++){
         if(count!=length-1)data[count]=IIC_Read_Byte(1); //��ACK�Ķ�����
            else  data[count]=IIC_Read_Byte(0); //���һ���ֽ�nACK
    }
    IIC_Stop();//����һ��ֹͣ����
    return count;
}


/**************************ʵ�ֺ���********************************************
*����ԭ��:      u8 IICwriteBytes(u8 dev, u8 reg, u8 length, u8* data)
*��������:      ������ֽ�д��ָ���豸 ָ���Ĵ���
����    dev  Ŀ���豸��ַ
        reg   �Ĵ�����ַ
        length Ҫд���ֽ���
        *data  ��Ҫд�����ݵ��׵�ַ
����   �����Ƿ�ɹ�
*******************************************************************************/
uint8 IICwriteBytes(uint8 dev, uint8 reg, uint8 length, uint8* data)
{
    uint8 count = 0;
    IIC_Start();
    IIC_Send_Byte(dev);  //����д����
    IIC_Wait_Ack();
    IIC_Send_Byte(reg);  //���͵�ַ
    IIC_Wait_Ack();
    for(count=0;count<length;count++){
        IIC_Send_Byte(data[count]);
        IIC_Wait_Ack();
     }
    IIC_Stop();//����һ��ֹͣ����
    return 0;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      u8 IICreadByte(u8 dev, u8 reg, u8 *data)
*��������:      ��ȡָ���豸 ָ���Ĵ�����һ��ֵ
����    dev  Ŀ���豸��ַ
        reg    �Ĵ�����ַ
        *data  ���������ݽ�Ҫ��ŵĵ�ַ
����   1
*******************************************************************************/
uint8 IICreadByte(uint8 dev, uint8 reg, uint8 *data)
{
    *data=I2C_ReadOneByte(dev, reg);
    return 0;
}


/**************************ʵ�ֺ���********************************************
*����ԭ��:      unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data)
*��������:      д��ָ���豸 ָ���Ĵ���һ���ֽ�
����    dev  Ŀ���豸��ַ
        reg    �Ĵ�����ַ
        data  ��Ҫд����ֽ�
����   1
*******************************************************************************/
unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data)
{
    return IICwriteBytes(dev, reg, 1, &data);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      u8 IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data)
*��������:      �� �޸� д ָ���豸 ָ���Ĵ���һ���ֽ� �еĶ��λ
����    dev  Ŀ���豸��ַ
        reg    �Ĵ�����ַ
        bitStart  Ŀ���ֽڵ���ʼλ
        length   λ����
        data    ��Ÿı�Ŀ���ֽ�λ��ֵ
����   �ɹ� Ϊ1
        ʧ��Ϊ0
*******************************************************************************/
uint8 IICwriteBits(uint8 dev,uint8 reg,uint8 bitStart,uint8 length,uint8 data)
{
    uint8 b;
    if (IICreadByte(dev, reg, &b) != 0)
    {
        uint8 mask = (0xFF << (bitStart + 1)) | 0xFF >> ((8 - bitStart) + length - 1);
        data <<= (8 - length);
        data >>= (7 - bitStart);
        b &= mask;
        b |= data;
        return IICwriteByte(dev, reg, b);
    }
    else
    {
        return 0;
    }
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:      u8 IICwriteBit(u8 dev, u8 reg, u8 bitNum, u8 data)
*��������:      �� �޸� д ָ���豸 ָ���Ĵ���һ���ֽ� �е�1��λ
����    dev  Ŀ���豸��ַ
        reg    �Ĵ�����ַ
        bitNum  Ҫ�޸�Ŀ���ֽڵ�bitNumλ
        data  Ϊ0 ʱ��Ŀ��λ������0 ���򽫱���λ
����   �ɹ� Ϊ1
        ʧ��Ϊ0
*******************************************************************************/
uint8 IICwriteBit(uint8 dev, uint8 reg, uint8 bitNum, uint8 data)
{
    uint8 b;
    IICreadByte(dev, reg, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return IICwriteByte(dev, reg, b);
}

//------------------End of File----------------------------


