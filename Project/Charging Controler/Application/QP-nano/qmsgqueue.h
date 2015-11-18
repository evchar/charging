#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#define MSG_BUF_LEN 4
#define GUI_MSG_NUM 8

enum emMSG_TIMER
{
    MSG_TIMER0,//�������ҳ��ʱ����ʱ�л�ҳ��
    MSG_TIMER1,//�����涨ʱ��ѯ�¶�ֵ
    MSG_TIMER2,
    MSG_TIMER3,
    MSG_TIMER4,
    MSG_TIMER5,//ˢ���������¶ȵ�����Ϣ�Ķ�ʱ��
    MSG_TIMER6,//����������ģ��ʱ����ʾ��ʱ��
    MSG_TIMER7,//��ʾ����ʧʱ�䶨ʱ��
    MSG_TIMER_END
};

void InitQMsgqueue(void);
void QPushmsg(uint8 uch_Msg);
uint8 QPullmsg(void);
void StartMsgTimer(uint8 uch_No, uint8 uch_Msg, uint16 uch_Timeout, uint16 uch_Interval);
void StopMsgTimer(uint8 uch_No);
void ProcessMsgTimer(void);

#endif//__MSG_QUEUE_H__

