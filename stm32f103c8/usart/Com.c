#include "includes.h"


void USART_Config(USART_TypeDef* USARTx,u32 baud)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//PA9,PA10 复用IO口功能用于配置串口。因此要使能AFIO（复用功能IO）时钟。
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	//usart_init----------------------------------------------------
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		  //浮空输入模式	   
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 
	
	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			  //复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	USART_InitStructure.USART_BaudRate =baud;						//速率115200bps
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//数据位8位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//停止位1位
	USART_InitStructure.USART_Parity = USART_Parity_No;				//无校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
	
	/* Configure USART1 */
	USART_Init(USARTx, &USART_InitStructure);							//配置串口参数函数
	
	
	/* Enable USART1 Receive and Transmit interrupts */
	USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);                    //使能接收中断
	//USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);				    //使能发送缓冲空中断
	
	
	/* Enable the USART1 */
	USART_Cmd(USARTx, ENABLE);	
	
	USART1 -> SR;
	
	//USART_ClearFlag(USARTx, USART_FLAG_TXE);     /* 清发送完成标志，Transmission Complete flag */
}

void USART2_Config(u32 baud)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(36*1000000)/(baud * 16);//??USARTDIV
	mantissa=temp;				 //??????
	fraction=(temp-mantissa)*16; //??????	 
  mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //??PORTA???  
	RCC->APB1ENR|=1<<17;  //?????? 
	GPIOA->CRL&=0XFFFF00FF;//IO????
	GPIOA->CRL|=0X00008B00;//IO????
		  
	RCC->APB1RSTR|=1<<17;   //????2
	RCC->APB1RSTR&=~(1<<17);//????	   	   
	//?????
 	USART2->BRR=mantissa; // ?????	 
	USART2->CR1|=0X200C;  //1???,????.
	USART2 -> SR;
} 
