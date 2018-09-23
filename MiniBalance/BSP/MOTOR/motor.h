#ifndef __MOTOR_H
#define __MOTOR_H
#include <stm32f10x.h>	

  /**************************************************************************
作者：平衡小车之家
我的淘宝小店：http://shop114407458.taobao.com/
**************************************************************************/
#define PWMA   TIM3->CCR4  //PA8
#define AIN1_CLOCK		RCC_APB2Periph_GPIOC
#define AIN1_PORT    	GPIOC
#define AIN1_PIN	 	GPIO_Pin_10	
#define AIN1(a)	\
						if (a)	\
						GPIO_SetBits(AIN1_PORT, AIN1_PIN);	\
						else		\
						GPIO_ResetBits(AIN1_PORT, AIN1_PIN)						
//#define AIN2   PCout(10)						
//#define AIN1   PCout(11)
#define AIN2_CLOCK		RCC_APB2Periph_GPIOC
#define AIN2_PORT    	GPIOC
#define AIN2_PIN	 	GPIO_Pin_11	
#define AIN2(a)	\
						if (a)	\
						GPIO_SetBits(AIN2_PORT, AIN2_PIN);	\
						else		\
						GPIO_ResetBits(AIN2_PORT, AIN2_PIN)							
//#define BIN1   PBout(3)
//#define BIN2   PBout(4)
#define BIN1_CLOCK		RCC_APB2Periph_GPIOB
#define BIN1_PORT    	GPIOB
#define BIN1_PIN	 	GPIO_Pin_3	
#define BIN1(a)	\
						if (a)	\
						GPIO_SetBits(BIN1_PORT, BIN1_PIN);	\
						else		\
						GPIO_ResetBits(BIN1_PORT, BIN1_PIN)		
#define BIN2_CLOCK		RCC_APB2Periph_GPIOB
#define BIN2_PORT    	GPIOB
#define BIN2_PIN	 	GPIO_Pin_4	
#define BIN2(a)	\
						if (a)	\
						GPIO_SetBits(BIN2_PORT, BIN2_PIN);	\
						else		\
						GPIO_ResetBits(BIN2_PORT, BIN2_PIN)								
#define PWMB   TIM3->CCR3  //PA11
void MiniBalance_PWM_Init(u16 arr,u16 psc);
void MiniBalance_Motor_Init(void);
#endif
