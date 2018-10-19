/*
**************************************************************************************************
*
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.h
*	版    本 : V1.0
*	说    明 : 头文件
************************************************************************************************/

#ifndef __LED_DISPLAY_H
#define __LED_DISPLAY_H

void display_full(void);
void LED_P8x16Char(unsigned char ucIdxX, unsigned char ucIdxY, unsigned char ucData,unsigned int uccolor,unsigned int ucbgcolor);
void LED_P8x16Str(unsigned char ucIdxX, unsigned char ucIdxY, unsigned char ucDataStr[],unsigned int uccolor,unsigned int ucbgcolor);
void LED_GB1616(unsigned char ucIdxX, unsigned char  ucIdxY, unsigned char c[2], unsigned int uccolor, unsigned int ucbgcolor);
#endif
