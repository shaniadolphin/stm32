#include "./usart/usart.h"
#include "stdarg.h"	  
#include "string.h"

#define CR2_LINEN_Set              ((uint16_t)0x4000)  /*!< USART LIN Enable Mask */
#define CR2_LINEN_Reset            ((uint16_t)0xBFFF)  /*!< USART LIN Disable Mask */
#define CR3_SCEN_Set              ((uint16_t)0x0020)  /*!< USART SC Enable Mask */
#define CR3_SCEN_Reset            ((uint16_t)0xFFDF)  /*!< USART SC Disable Mask */
#define CR3_IREN_Set              ((uint16_t)0x0002)  /*!< USART IrDA Enable Mask */
#define CR3_IREN_Reset            ((uint16_t)0xFFFD)  /*!< USART IrDA Disable Mask */
//////////////////////////////////////////////////////////////////
u8 Usart2_Receive;
u8 mode_data[8];
u8 six_data_stop[3]={0x59,0x59,0x59};  //停止数据样本
u8 six_data_start[3]={0x58,0x58,0x58};  //启动数据样本

void uart1_init(u32 baud)
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING;		  //浮空输入模式	   
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 
	
	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			  		//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	USART_InitStructure.USART_BaudRate = baud;						//速率115200bps
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//数据位8位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//停止位1位
	USART_InitStructure.USART_Parity = USART_Parity_No;				//无校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	
	/*Configure USART1 */
	USART_Init(USART1, &USART_InitStructure);						//????????
	
	/*DISABLE USART1 Receive and Transmit interrupts */
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);                 //??????
	//USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);				    //?????????
	
	/*Enable the USART1 */
	USART_Cmd(USART1, ENABLE);	
	
	USART_ClearFlag(USART1, USART_FLAG_TXE);     /* ???????,Transmission Complete flag */
}




/**************************************************************************
函数功能：串口2初始化
入口参数： bound:波特率
返回  值：无
**************************************************************************/
void uart2_init(u32 baud)
{
	NVIC_InitTypeDef NVIC_InitStructure;
#if 0
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//使能UGPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);		//使能USART3时钟
	//USART2_TX  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 					//PB.10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;				//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//USART2_RX	  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;					//PB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;					//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART2, &USART_InitStructure);     				//初始化串口
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);				//开启串口接受中断
	USART_Cmd(USART2, ENABLE);                    				//使能串口
#else
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp		=	(float)(36*1000000)/(baud * 16);//??USARTDIV
	mantissa	=	temp;				 //??????
	fraction	=	(temp-mantissa)*16; //??????	 
	mantissa	<<=	4;
	mantissa	+=	fraction; 
	RCC->APB2ENR|=	1<<2;   //??PORTA???  
	RCC->APB1ENR|=	1<<17;  //?????? 
	GPIOA->CRL	&=	0xFFFF00FF;//IO????
	GPIOA->CRL	|=	0x00008B00;//IO????
		  
	RCC->APB1RSTR	|=	1<<17;   //????2
	RCC->APB1RSTR	&=	~(1<<17);//????	   	   
	//?????
 	USART2->BRR	=	mantissa; // ?????	 
	USART2->CR1	|=	0x200C;  //1???,????.
	//USART2 -> SR;
	USART_ClearFlag(USART2, USART_FLAG_TXE);     /* 清发送完成标志，Transmission Complete flag */
#endif
	//Usart NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;		//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);								//根据指定的参数初始化VIC寄存器
}

/*----------------------------------------------------------------------------
 * @描述: 	整形数据转字符串函数
			char *itoa(int value, char *string, int radix) 
			radix=10 表示是10进制
			例：d=-379
			执行 itoa(d, buf, 10) 后
			buf="-379"			
 * @输入: 	value - 整形数据
 			*string - 存储value转换后对应字符串的地址
			radix - 转换进制，仅10 和 16 有效
 * @返回: 	string - 
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          创建模块	
----------------------------------------------------------------------------*/
char *itoa(int value, char *string, int radix)
{
	const unsigned char char_hex_array[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }
    if(radix == 10)
    {
        /* if this is a negative value insert the minus sign. */
        if (value < 0)
        {
            *ptr++ = '-';
    
            /* Make the value positive. */
            value *= -1;
        }
        for (i = 10000; i > 0; i /= 10)
        {
            d = value / i;
    
            if (d || flag)
            {
                *ptr++ = (char)(d + 0x30);
                value -= (d * i);
                flag = 1;
            }
        }
    }
    else if(radix == 16)
    {
        /* if this is a negative value insert the minus sign. */
        if (value < 0)
        {
            value *= -1;/* Make the value positive. */
        }
        *ptr++ = char_hex_array[ (value & 0xf000) >> 12];
        *ptr++ = char_hex_array[ (value & 0x0f00) >> 8 ];
        *ptr++ = char_hex_array[ (value & 0x00f0) >> 4 ];
        *ptr++ = char_hex_array[ (value & 0x000f)      ];
    }    

    /* Null terminate the string. */
    *ptr = 0;

    return string;
} 
// 浮点数转换成字符串
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
/*----------------------------------------------------------------------------
 * @描述: 格式化串口输出函数
			"\r"	回车符	   USART_OUT(USART1, "abcdefg\r")  
			"\n"	换行符	   USART_OUT(USART1, "abcdefg\r\n")
			"%s"	字符串	   USART_OUT(USART1, "字符串是：%s","abcdefg")
			"%d"	十进制	   USART_OUT(USART1, "a=%d",10)
			"%x"	十六进制   USART_OUT(USART1, "a=%x",10) 		
 * @输入: USARTx-串口号,Data-格式化串口输出数据
 * @返回: 无
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          创建模块	
----------------------------------------------------------------------------*/
void USART_TX(USART_TypeDef* USARTx, char *Data, va_list ap)
{
	const char *s;
	int d;
	double f;
	char buf[50];
	unsigned char prefix = 0x20;

	while(*Data!=0)
	{				                          	//判断是否到达字符串结束符
		if(*Data==0x5c)						  	//'\'
		{
			switch (*++Data)					//转义字符
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
		else if(*Data=='%')
		{
			switch (*++Data){
				case 's':		//字符串
					s = va_arg(ap, const char *);
					for ( ; *s; s++)
					{
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
						USART_SendData(USARTx, *s);
					}
					Data++;
				break;
				case 'd':	//十进制
				case 'D':
					d = va_arg(ap, int);
					itoa(d, buf, 10);
					for (s = buf; *s; s++)
					{
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
						USART_SendData(USARTx, *s);
					}
					Data++;
				break;
				case 'f':	//十进制
				case 'F':
					f = va_arg(ap, double);
					ftoa(f, buf);
					for (s = buf; *s; s++)
					{
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
						USART_SendData(USARTx, *s);
					}
					Data++;
				break;
				case 'x':		//十六进制
				case 'X':		//十六进制
					d = va_arg(ap, int);
					itoa(d, buf, 16);
					for (s = buf; *s; s++)
					{
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
						USART_SendData(USARTx, *s);
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
							d = va_arg(ap, int);
							itoa(d, buf, radix);
						}
						else if(*Data == 'x' || *Data == 'X')
						{
							radix = 16;
							d = va_arg(ap, int);
							itoa(d, buf, radix);
						}					
						len = strlen(buf);
						s = buf;
						if (outlen >= len)
						{
							for (i=0; i<outlen-len; i++)
							{
								while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
								USART_SendData(USARTx, prefix);
							}
						}
						else
						{
							for (i=0; i<len-outlen; i++)
							{
								s++;
							}
						}
						for (; *s; s++)
						{
							while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
							USART_SendData(USARTx,*s);
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
			while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
			USART_SendData(USARTx, *Data++);
		}
	}
}

void printk(char *Data, ...)
{
	va_list ap;
	va_start(ap, Data);
	USART_TX(USART1, Data, ap);
	va_end(ap);
}

void printwifi(char *Data, ...)
{
	va_list ap;
	va_start(ap, Data);
	USART_TX(USART2, Data, ap);
	va_end(ap);
}

/******************************************************
		?????????
		"\r"	???	   USART_OUT(USART1, "abcdefg\r")
		"\n"	???	   USART_OUT(USART1, "abcdefg\r\n")
		"%s"	???	   USART_OUT(USART1, "????:%s","abcdefg")
		"%d"	???	   USART_OUT(USART1, "a=%d",10)
**********************************************************/
void USART_OUT(USART_TypeDef *USARTx, char *Data, ...)
{
	va_list ap;

	va_start(ap, Data);
	USART_TX(USARTx, Data, ap);
	va_end(ap);
}
