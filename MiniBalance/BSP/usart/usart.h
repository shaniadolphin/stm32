#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f10x.h"
  /**************************************************************************
作者：平衡小车之家
我的淘宝小店：http://shop114407458.taobao.com/
**************************************************************************/

void uart1_init(u32 baud);
void uart2_init(u32 baud);

void printk(char *Data, ...);
void printwifi(char *Data, ...);
#endif


