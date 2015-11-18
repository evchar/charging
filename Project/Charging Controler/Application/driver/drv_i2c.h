
#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

void IIC_Init(void);                //3?��??��IIC��?IO?��				 
void IIC_Start(void);				//����?��IIC?a��?D?o?
void IIC_Stop(void);	  			//����?��IIC����?1D?o?
void IIC_Send_Byte(uint8 txd);			//IIC����?����???��??��
uint8 IIC_Read_Byte(unsigned char ack);//IIC?����?��???��??��
uint8 IIC_Wait_Ack(void); 				//IIC�̨���yACKD?o?
void IIC_Ack(void);					//IIC����?��ACKD?o?
void IIC_NAck(void);				//IIC2?����?��ACKD?o?

void IIC_Write_One_Byte(uint8 daddr,uint8 addr,uint8 data);
uint8 IIC_Read_One_Byte(uint8 daddr,uint8 addr);	 
unsigned char I2C_Readkey(unsigned char I2C_Addr);

unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr);
unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data);
uint8 IICwriteBytes(uint8 dev, uint8 reg, uint8 length, uint8* data);
uint8 IICwriteBits(uint8 dev,uint8 reg,uint8 bitStart,uint8 length,uint8 data);
uint8 IICwriteBit(uint8 dev,uint8 reg,uint8 bitNum,uint8 data);
uint8 IICreadBytes(uint8 dev, uint8 reg, uint8 length, uint8 *data);
uint8 SMT480TreadBytes(uint8 dev, uint8 command, uint8 reg, uint8 length, uint8 *data);
#endif // __FILE_BSP_I2C_H__
