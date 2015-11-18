/*! @file
********************************************************************************
<PRE>
--------------------------------------------------------------------------------
�ļ���       : MyBus.h
�ļ�ʵ�ֹ��� : ���ݼ������
����         : ����ͦ
�汾         : V1.0
��ע         :
--------------------------------------------------------------------------------
�޸ļ�¼ :
�� ��        �汾     �޸���              �޸�����
2010/12/10   1.0      ����ͦ              ԭʼ�汾
--------------------------------------------------------------------------------
</PRE>
*******************************************************************************/

#ifndef __USERBUS_H__
#define __USERBUS_H__

#include "user_typedefine.h"

/// �����˱�ʾ����ֽ���,δ�����ʾС���ֽ���
//#define ENDIAN_BIG

#if 1
#ifdef ENDIAN_BIG
#define   ntohs(x)    (x)
#define   htons(x)    (x)
#define   ntohl(x)    (x)
#define   htonl(x)    (x)
#else
#define   ntohs(x)    (((x>>8) & 0xFF) | ((x & 0xFF)<<8))
#define   htons(x)    (((x>>8) & 0xFF) | ((x & 0xFF)<<8))
#define   ntohl(x)    ( ((x>>24) & 0xFF) | ((x>>8) & 0xFF00) | \
                        ((x & 0xFF00)<<8) | ((x & 0xFF)<<24)   \
                      )
#define   htonl(x)    ( ((x>>24) & 0xFF) | ((x>>8) & 0xFF00) | \
                        ((x & 0xFF00)<<8) | ((x & 0xFF)<<24)   \
                      )
#endif
#endif
typedef struct 
{
    uint8 bit0:1;
	uint8 bit1:1;
	uint8 bit2:1;
	uint8 bit3:1;
	uint8 bit4:1;
	uint8 bit5:1;
	uint8 bit6:1;
	uint8 bit7:1;
}stBIT8;

typedef union
{
    uint8 uch_Data;
	stBIT8 Bit;
}unBIT8;

typedef struct 
{
    uint16  bit0:1;
	uint16	bit1:1;
	uint16	bit2:1;
	uint16	bit3:1;
	uint16	bit4:1;
	uint16	bit5:1;
	uint16	bit6:1;
	uint16	bit7:1;
	uint16  bit8:1;
	uint16	bit9:1;
	uint16	bit10:1;
	uint16	bit11:1;
	uint16	bit12:1;
	uint16	bit13:1;
	uint16	bit14:1;
	uint16	bit15:1;
}stBIT16;

typedef union
{
    uint16 uin_Data;
	stBIT16 Bit;
}unBIT16;

#define MIN(a, b)   ((a) < (b)) ? (a) : (b)
#define MAX(a, b)   ((a) > (b)) ? (a) : (b)

void   Encodefp32(fp32 fValue, uint8 *p);
void   Encodeuint32(uint32 ulData, uint8 *p);
void   Encodeint32(int32 lData, uint8 *p);
void   Encodeuint16(uint16 uinData, uint8 *p);
void   Encodeint16(int16 inData, uint8 *p);
fp32   Decodefp32(uint8 *p);
uint32 Decodeuint32(uint8 *p);
int32  Decodeint32(uint8 *p);
uint16 Decodeuint16(uint8 *p);
int16  Decodeint16(uint8 *p);

#endif//__USERBUS_H__

