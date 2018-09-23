/*
*********************************************************************************************************
*
*	ģ������ : LEDָʾ������ģ��
*	�ļ����� : bsp_led.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_LED_H
#define __BSP_LED_H

/* ���ⲿ���õĺ������� */
void bsp_InitLed(void);
void bsp_LedOn(uint8_t _no);
void bsp_LedOff(uint8_t _no);
void bsp_LedToggle(uint8_t _no);
uint8_t bsp_IsLedOn(uint8_t _no);
void dispaly_process(void);
void display_full(void);
void LED_P8x16Char(unsigned char ucIdxX, unsigned char ucIdxY, unsigned char ucData,unsigned int uccolor,unsigned int ucbgcolor);
void LED_P8x16Str(unsigned char ucIdxX, unsigned char ucIdxY, unsigned char ucDataStr[],unsigned int uccolor,unsigned int ucbgcolor);
void LED_GB1616(unsigned char ucIdxX, unsigned char  ucIdxY, unsigned char c[2], unsigned int uccolor, unsigned int ucbgcolor);
#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
