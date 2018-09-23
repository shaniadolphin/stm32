#include "stm32f4xx.h"
#include <stdio.h>

#pragma import(__use_no_semihosting)          
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout; 

//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
/*
*********************************************************************************************************
*	函 数 名: fputc
*	功能说明: 重定义putc函数，这样可以使用printf函数从串口1打印输出
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{
	/* 等待发送结束 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}	
	/* 写一个字节到USART1 */
	USART_SendData(USART1, (uint8_t) ch);
	return ch;
}

/*
*********************************************************************************************************
*	函 数 名: fgetc
*	功能说明: 重定义getc函数，这样可以使用scanff函数从串口1输入数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int fgetc(FILE *f)
{
	/* 等待串口1输入数据 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

	return (int)USART_ReceiveData(USART1);
}
