#ifndef __KEY_H
#define __KEY_H	 

#include "includes.h"

#define KEY_SHORT_CNT		3
#define KEY_LONG_CNT		166
#define KEY_LONGLONG_CNT	416

#define KEY_CLOCK	RCC_APB2Periph_GPIOC
#define KEY_PORT    GPIOC
#define KEY_PIN	 	GPIO_Pin_4

#define KEY 		(KEY_PORT->IDR & KEY_PIN)

void KEY_Init(void);          //������ʼ��
u8 click_N_Double (u8 time);  //��������ɨ���˫������ɨ��
u8 click(void);               //��������ɨ��
u8 Long_Press(void);           //����ɨ��  
#endif  
