#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"
  /**************************************************************************
作者：平衡小车之家
我的淘宝小店：http://shop114407458.taobao.com/
**************************************************************************/
//LED 端口定义
#define LED_CLOCK			RCC_APB2Periph_GPIOB
#define LED_PORT    		GPIOB
#define LED_PIN	 			GPIO_Pin_9

#define LED 	PBout(9) // PA12
void LED_Init(void);  //初始化
void Led_Flash(u16 time);
void LED_N_STATUS(unsigned char num,unsigned char status);
#endif
