/*
*********************************************************************************************************
*	                                  
*	ģ������ : ��������ģ��    
*	�ļ����� : bsp_uart.c
*	��    �� : V1.0
*	˵    �� : ʵ��printf��scanf�����ض��򵽴���1����֧��printf��Ϣ��USART1
*				ʵ���ض���ֻ��Ҫ���2������:
*				int fputc(int ch, FILE *f);
*				int fgetc(FILE *f);
*				����KEIL MDK������������ѡ������Ҫ��MicorLibǰ��򹳣����򲻻������ݴ�ӡ��USART1��
*	�޸ļ�¼ :
*		�汾��  ����         ����     ˵��
*		v1.0    2012-12-17  armfly  ST�̼���V1.0.2�汾��
*
*	Copyright (C), 2012-2013, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "stm32f4xx.h"
#include "stdarg.h"
#include "string.h"
/*
*********************************************************************************************************
*	�� �� ��: bsp_InitUart
*	����˵��: ��ʼ��CPU��USART1����Ӳ���豸��δ�����жϡ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitUart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* ����1 TX = PA9   RX = PA10 */

	/* ��1���� ����GPIO */

	/* �� GPIO ʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

	/* �� UART ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	/* �� PA9 ӳ��Ϊ USART1_TX */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);

	/* �� PA10 ӳ��Ϊ USART1_RX */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOC,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = 115200;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART6, &USART_InitStructure); //��ʼ������1
	
	USART_Cmd(USART6, ENABLE);  //ʹ�ܴ���1 

	/* CPU��Сȱ�ݣ��������úã����ֱ��Send�����1���ֽڷ��Ͳ���ȥ
		�����������1���ֽ��޷���ȷ���ͳ�ȥ������ */
	USART_ClearFlag(USART6, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */   
}

/******************************************************
		��������ת�ַ�������
		char *itoa(int value, char *string, int radix)
		radix=10 ��ʾ��10����	��ʮ���ƣ�ת�����Ϊ0;

		����d=-379;
		ִ��	itoa(d, buf, 10); ��

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
 * @����: 	������ת�����ַ���
			char *ftoa(double data, char *string)
			����d=-3.79
			ִ�� ftoa(d, buf, 10) ��
			buf="-3.79000000"			
 * @����: 	value    - ˫��������
 			*string  - �洢valueת�����Ӧ�ַ����ĵ�ַ
 * @����: 	string
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
char *ftoa(double data, char *string)
{
	int i,j,k,t;
	long temp,tempoten;
	unsigned char dotslength = 6;
	static unsigned char intpart[20], dotpart[20]; //�����������ֺ�С������
	double datatmp;
	int cntp;
	int flag;								//Ϊ1��ʾ����ΪС��

	//1.ȷ������λ����ת��Ϊ��ѧ������n.xxxxxEpppp
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
	//2.ȷ����������
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

	//3.ȷ��С������,ȡ��12λС��
	data = datatmp - (long)datatmp;
	for(j=0; j<dotslength; j++)
	{
		dotpart[j] = (int)(data*10) + 48;
		data = data * 10.0;
		data = data - (long)data;
	}
	//4.������װ
	for(k=1; k<=i+1; k++) 
		string[k] = intpart[i+1-k];
	string[i+2] = '.';
	for(k=i+3; k<i+j+3; k++) 
		string[k] = dotpart[k-i-3];
	t = i + j + 3;
	string[t++] = 'E';
	if(flag == 0)string[t++] = '+';
	else string[t++] = '-';

	
	//5.ȷ��ָ������
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
	{                                         //�ж��Ƿ񵽴��ַ���������
		if (*Data == 0x5c)                       //'\'
		{
			switch (*++Data)
			{
				case 'r':	
					while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);					//�س���
					USART_SendData(USARTx, 0x0d);
					Data++;
				break;
				case 'n':							          //���з�
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
				case 's':       //�ַ���
					s = va_arg(ap, const char*);
					for (; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
					break;
				case 'd':   //ʮ����
					d = va_arg(ap, int);
					itoa(d, buf, 10);
					for (s = buf; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
					break;
				case 'f':	//ʮ����
				case 'F':
					f = va_arg(ap, double);
					ftoa(f, buf);
					for (s = buf; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
				break;					
				case 'x':       //ʮ������
				case 'X':       //ʮ������
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


