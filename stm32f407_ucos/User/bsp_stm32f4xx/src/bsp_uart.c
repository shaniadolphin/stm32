/*
*********************************************************************************************************
*	                                  
*	模块名称 : 串口驱动模块    
*	文件名称 : bsp_uart.c
*	版    本 : V1.0
*	说    明 : 实现printf和scanf函数重定向到串口1，即支持printf信息到USART1
*				实现重定向，只需要添加2个函数:
*				int fputc(int ch, FILE *f);
*				int fgetc(FILE *f);
*				对于KEIL MDK编译器，编译选项中需要在MicorLib前面打钩，否则不会有数据打印到USART1。
*	修改记录 :
*		版本号  日期         作者     说明
*		v1.0    2012-12-17  armfly  ST固件库V1.0.2版本。
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "stm32f4xx.h"
#include "stdarg.h"
#include "string.h"
/*
*********************************************************************************************************
*	函 数 名: bsp_InitUart
*	功能说明: 初始化CPU的USART1串口硬件设备。未启用中断。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitUart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* 串口1 TX = PA9   RX = PA10 */

	/* 第1步： 配置GPIO */

	/* 打开 GPIO 时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

	/* 打开 UART 时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	/* 将 PA9 映射为 USART1_TX */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);

	/* 将 PA10 映射为 USART1_RX */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = 115200;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART6, &USART_InitStructure); //初始化串口1
	
	USART_Cmd(USART6, ENABLE);  //使能串口1 

	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(USART6, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */   
}

/******************************************************
		整形数据转字符串函数
		char *itoa(int value, char *string, int radix)
		radix=10 标示是10进制	非十进制，转换结果为0;

		例：d=-379;
		执行	itoa(d, buf, 10); 后

		buf="-379"
**********************************************************/
char* itoa(int value, char *string, int radix)
{
	int i = 0, j = 0;
	int data = value;
	char temp[16];
	char MinusFlag;
	int pos;

	if (radix != 16 && radix != 10)
	{
		string[j] = 0;
		return 0;
	}

	MinusFlag = (radix == 10 && value < 0);
	if (MinusFlag)
	{
		data = -data;
	}
	else
	{
		data = value;
	}

	do
	{
		pos = data % radix;
		data /= radix;
		if (pos < 10)
		{
			temp[i] = pos + '0';
		}
		else
		{
			temp[i] = pos + 'a' - 10;
		}
		i++;
	}while (data > 0);

	if (MinusFlag)
	{
		temp[i++] = '-';
	}
	temp[i] = 0;
	i--;

	while (i >= 0)
	{
		string[j] = temp[i];
		j++;
		i--;
	}
	string[j] = 0;
	return string;

} /* NCL_Itoa */
/*----------------------------------------------------------------------------
 * @描述: 	浮点数转换成字符串
			char *ftoa(double data, char *string)
			例：d=-3.79
			执行 ftoa(d, buf, 10) 后
			buf="-3.79000000"			
 * @输入: 	value    - 双精度数据
 			*string  - 存储value转换后对应字符串的地址
 * @返回: 	string
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          创建模块	
----------------------------------------------------------------------------*/
char *ftoa(double data, char *string)
{
	int i,j,k,t;
	long temp,tempoten;
	unsigned char dotslength = 6;
	static unsigned char intpart[20], dotpart[20]; //数的整数部分和小数部分
	double datatmp;
	int cntp;
	int flag;								//为1表示该数为小数

	//1.确定符号位，并转换为科学计数法n.xxxxxEpppp
	if(data < 0) 
	{
		string[0]='-';
		data=-data;
	}
	else 
		string[0]='+';
	cntp = 0;
	flag = 0;
	datatmp = data;
#if 1
	if(datatmp != 0.0)
	{
		while(datatmp > 10)
		{
			flag = 0;
			datatmp = datatmp / 10;
			cntp ++;
		}
		while(datatmp < 1)
		{
			flag = 1;
			datatmp = datatmp * 10;
			cntp ++;
		}	
	}	
#endif
	//2.确定整数部分
	temp = (long)datatmp;
	//printk("aa%daa",temp);

	i = 0;
	tempoten = temp/10;
	while(tempoten != 0)
	{
		intpart[i] = temp - 10*tempoten + 48; //to ascii code
		temp = tempoten;
		tempoten = temp/10;
		i++;
	}
	intpart[i] = temp + 48;

	//3.确定小数部分,取了12位小数
	data = datatmp - (long)datatmp;
	for(j=0; j<dotslength; j++)
	{
		dotpart[j] = (int)(data*10) + 48;
		data = data * 10.0;
		data = data - (long)data;
	}
	//4.数据组装
	for(k=1; k<=i+1; k++) 
		string[k] = intpart[i+1-k];
	string[i+2] = '.';
	for(k=i+3; k<i+j+3; k++) 
		string[k] = dotpart[k-i-3];
	t = i + j + 3;
	string[t++] = 'E';
	if(flag == 0)string[t++] = '+';
	else string[t++] = '-';

	
	//5.确定指数部分
	i = 0;
	temp = cntp;
	tempoten = cntp/10;
	while(tempoten != 0)
	{
		string[t + i] = cntp - 10*tempoten + 48; //to ascii code
		temp = tempoten;
		tempoten = temp/10;
		i++;
	}
	string[t + i] = temp + 48;
	string[t + i + 1] = '\0';
	return string;
}
void USART_TX(USART_TypeDef *USARTx, unsigned char ch)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
	USART_SendData(USARTx, ch);
}

void UsartPrint(USART_TypeDef* USARTx, char *Data, va_list ap)
{
	const char *s;
	int d;
	double f;
	char buf[100];
	unsigned char prefix = 0x20;

	while (*Data != 0)
	{                                         //判断是否到达字符串结束符
		if (*Data == 0x5c)                       //'\'
		{
			switch (*++Data)
			{
				case 'r':	
					while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);					//回车符
					USART_SendData(USARTx, 0x0d);
					Data++;
				break;
				case 'n':							          //换行符
					while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
					USART_SendData(USARTx, 0x0a);
					Data++;
				break;
				case 0x22://" " "
					while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
					USART_SendData(USARTx, 0x22);
					Data++;
				break;
				case 0x27://" ' "
					while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
					USART_SendData(USARTx, 0x27);
					Data++;
				break;				
				case 0x5c://" / "	
					while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
					USART_SendData(USARTx, 0x5c);
					Data++;
				break;				
				default:
					Data++;
				break;
			}
		}
		else if (*Data == '%')
		{
			switch (*++Data)
			{
				case 's':       //字符串
					s = va_arg(ap, const char*);
					for (; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
					break;
				case 'd':   //十进制
					d = va_arg(ap, int);
					itoa(d, buf, 10);
					for (s = buf; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
					break;
				case 'f':	//十进制
				case 'F':
					f = va_arg(ap, double);
					ftoa(f, buf);
					for (s = buf; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
				break;					
				case 'x':       //十六进制
				case 'X':       //十六进制
					d = va_arg(ap, int);
					itoa(d, buf, 16);
					for (s = buf; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
					break;
				case '0':
					prefix = '0';
					Data++;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
					int outlen = *Data - '0';
					int len;
					int radix;
					int i;

					Data++;
					if (*Data == 'd' || *Data == 'x' || *Data == 'X')
					{
						if (*Data == 'd')
						{
							radix = 10;
						}
						else
						{
							radix = 16;
						}
						d = va_arg(ap, int);
						itoa(d, buf, radix);
						len = strlen(buf);
						s = buf;
						if (outlen >= len)
						{
							for (i = 0; i < outlen - len; i++)
							{
								USART_TX(USARTx, prefix);
							}
						}
						else
						{
							for (i = 0; i < len - outlen; i++)
							{
								s++;
							}
						}
						for (; *s; s++)
						{
							USART_TX(USARTx, *s);
						}
						prefix = 0x20;
					}
					Data++;
				}
					break;
				default:
					Data++;
					break;
			}
		}
		else
		{
			USART_TX(USARTx, *Data++);
		}
	}
}


