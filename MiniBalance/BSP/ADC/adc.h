#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"

#define Battery_Ch 		8
#define DISTANCE_CH1 	0
#define DISTANCE_CH2 	1
#define DISTANCE_CH3	2
#define DISTANCE_CH4	3
#define BATT_CH			4

#define N 50 //每通道采50次
#define M 5 //为12个通道

void Adc_Init(void);
void ADC1_Configuration(void);
u16 Get_Adc(u8 ch);
int Get_battery_volt(void);   
void adc_irqhandler_process(void);

#endif 















