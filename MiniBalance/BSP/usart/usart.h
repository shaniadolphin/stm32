#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f10x.h"
  /**************************************************************************
���ߣ�ƽ��С��֮��
�ҵ��Ա�С�꣺http://shop114407458.taobao.com/
**************************************************************************/

void uart1_init(u32 baud);
void uart2_init(u32 baud);

void printk(char *Data, ...);
void printwifi(char *Data, ...);
#endif


