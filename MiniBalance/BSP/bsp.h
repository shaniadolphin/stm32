#if !defined( _BSP_H )
#define _BSP_H

#include "includes.h"

void RCC_Configuration(void);

void GPIO_Configuration(void);

void NVIC_Configuration(void);

CPU_INT32U  BSP_CPU_ClkFreq (void);

//INT32U  OS_CPU_SysTickClkFreq (void);

void  OS_CPU_SysTickInit(void);

void BSP_Init(void);
void keyProcess(void);
void PulseSetting(unsigned short pulse1_time, unsigned short pulse2_time);
void pulseSetting(unsigned short ch, unsigned short pulsetime);
#endif
