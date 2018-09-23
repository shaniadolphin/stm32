#if !defined( _BSP_H )
#define _BSP_H

#define KEY_SHORT_CNT		3
#define KEY_LONG_CNT		166
#define KEY_LONGLONG_CNT	416

void RCC_Configuration(void);

void GPIO_Configuration(void);

void NVIC_Configuration(void);

//CPU_INT32U  BSP_CPU_ClkFreq (void);

//INT32U  OS_CPU_SysTickClkFreq (void);

void  OS_CPU_SysTickInit(void);

void BSP_Init(void);

void sys_power_cmd(unsigned char onoff);

enum{
	M_STATE = 0,
	W_FAULT = 6,
	W_MIN_HUN = 7,
	W_MIN_TEN = 8,
	W_MIN_ONE = 9,
	W_SEC_TEN = 10,
	W_SEC_ONE = 11,
	B_FAULT = 12,
	B_MIN_HUN = 2,
	B_MIN_TEN = 3,
	B_MIN_ONE = 4,
	B_SEC_TEN = 5,
	B_SEC_ONE = 1,
};

typedef enum
{
	LED_OFF = 0,
	LED_ON,
	LED_SLOW_SHINE,
	LED_FAST_SHINE,
}LED_STATUS;

#endif
