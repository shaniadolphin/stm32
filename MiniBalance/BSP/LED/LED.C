#include "./led/led.h"

/**************************************************************************
函数功能：LED接口初始化
入口参数：无 
返回  值：无
**************************************************************************/
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(LED_CLOCK, ENABLE); 	//使能端口时钟
	GPIO_InitStructure.GPIO_Pin = LED_PIN;	          	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;     	//10M
	GPIO_Init(LED_PORT, &GPIO_InitStructure);					//根据设定参数初始化GPIOA 
}

/**************************************************************************
函数功能：LED闪烁
入口参数：闪烁频率 
返回  值：无
**************************************************************************/
void Led_Flash(u16 time)
{
  static int temp;
  if(0==time) 
  {  
	  GPIO_ResetBits(LED_PORT, LED_PIN);//LED=0;
  }
  else if(++temp==time)	
  {
	  //LED=~LED;
	  LED_PORT -> ODR ^= LED_PIN;
	  temp=0;
  }
}
/*----------------------------------------------------------------------------
 * @描述: 	LED指示灯函数		
 * @输入: 	N - 灯编号
 			STATUS - 灯状态 0表示IO口输出0,1表示IO口输出1
 * @返回: 	None 
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          创建模块	
----------------------------------------------------------------------------*/
void LED_N_STATUS(unsigned char num,unsigned char status)
{
	switch(num)
	{
		case 1:
			if(status==0)
			{
				GPIO_ResetBits(LED_PORT, LED_PIN);//LED=0;
			}
			else
			{
				GPIO_SetBits(LED_PORT, LED_PIN);//LED=0;			
			}
		break;
		case 2:
			if(status==0)
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_0);
			 	GPIO_SetBits(GPIOB, GPIO_Pin_1);
			}
			else
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_0);
			 	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
				GPIO_SetBits(GPIOA, GPIO_Pin_0);
				GPIO_ResetBits(GPIOA, GPIO_Pin_1);
				GPIO_SetBits(GPIOA, GPIO_Pin_2);			
			}
		break;
		case 3:
			if(status==0)
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_0);
			 	GPIO_SetBits(GPIOB, GPIO_Pin_1);
			}
			else
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_0);
			 	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
				GPIO_SetBits(GPIOA, GPIO_Pin_0);
				GPIO_SetBits(GPIOA, GPIO_Pin_1);
				GPIO_ResetBits(GPIOA, GPIO_Pin_2);			
			}
		break;

	}
}


