#ifndef __RTC_H
#define __RTC_H	 
 
 //////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//RTC ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/5
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
//********************************************************************************
//�޸�˵��
//V1.1 20140726
//����:RTC_Get_Week����,���ڸ�����������Ϣ,�õ�������Ϣ.
////////////////////////////////////////////////////////////////////////////////// 

	
unsigned char My_RTC_Init(void);						//RTC��ʼ��
unsigned char RTC_Set_Time(unsigned char hour,unsigned char min,unsigned char sec,unsigned char ampm);			//RTCʱ������
unsigned char RTC_Set_Date(unsigned char year,unsigned char month,unsigned char date,unsigned char week); 		//RTC��������
void RTC_Set_AlarmA(unsigned char week,unsigned char hour,unsigned char min,unsigned char sec);		//��������ʱ��(����������,24Сʱ��)
void RTC_Set_WakeUp(unsigned int wksel,unsigned short cnt);					//�����Ի��Ѷ�ʱ������

#endif

















