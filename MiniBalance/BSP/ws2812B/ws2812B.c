#include "./ws2812B/ws2812B.h"
#include "includes.h"
/* Buffer that holds one complete DMA transmission
 * 
 * Ensure that this buffer is big enough to hold
 * all data bytes that need to be sent
 * 
 * The buffer size can be calculated as follows:
 * number of LEDs * 24 bytes + 42 bytes
 * 
 * This leaves us with a maximum string length of
 * (2^16 bytes per DMA stream - 42 bytes)/24 bytes per LED = 2728 LEDs
 */
#define TIM3_CCR3_Address 	(0x40000400 + 0x3C) 	// physical memory address of Timer 3 CCR3 register
#define TIM4_CCR3_Address 	(0x40000800 + 0x3C) 	// physical memory address of Timer 4 CCR3 register
#define TIM4_CCR4_Address 	(0x40000800 + 0x40) 	// physical memory address of Timer 4 CCR3 register
#define TIM1_CCR1_Address 	(0x40012C00 + 0x34)
//#define TIM3_CCR1_Address 0x40000434	// physical memory address of Timer 3 CCR1 register
	
#define TIMING_ONE  		60  //1000/72 * 50 ns= 694.4ns 
#define TIMING_ZERO 		30  //1000/72 * 25 ns= 347.2ns
#define TIMING_RESET 		0
#define TIMING_HIGH 		90 

unsigned short LED_BYTE_Buffer[LEDS_N * 24 +1];
unsigned int colortmp = 0;

//---------------------------------------------------------------//
void Delay(__IO uint32_t nCount)
{ 
  while(nCount--);
}

void ws2812Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
#ifdef	USE_TIME3	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	/* GPIOA Configuration: TIM3 Channel 1 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	/* Compute the prescaler value */
	//PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 90-1; // 800kHz 			 //当定时器从0计数到89，即为90次，为一个定时周期72M/90=800K
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					//设置预分频：不预分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				//设置时钟分频系数：不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;			//配置为PWM模式1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;   						//设置跳变值，当计数器计数到这个值时，电平发生跳变
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;	//当定时器计数值小于CCR3_Val时为高电平
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
		
	/* configure DMA */
	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	/* DMA1 Channel6 Config */
	DMA_DeInit(DMA1_Channel3);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)TIM3_CCR3_Address;	//DMA外设基址 TIM3_CCR3_Address 
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)LED_BYTE_Buffer;		//DMA内存基址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						//DST外设作为数据传送目的地址,SRC则作为源地址
	DMA_InitStructure.DMA_BufferSize = LEDS_N * 24 +1;						//DMA缓存大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址寄存器不增加
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//内存地址寄存器不增加
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外存字宽16bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;		//内存字宽16bit
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// stop DMA feed after buffer size is reached
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//DMA置为高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							//不置为内存到内存传输
	
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	/* TIM3 CC1 DMA Request enable */
	TIM_DMACmd(TIM3, TIM_DMA_Update, ENABLE);
#else
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* TIM1 Peripheral Configuration */ 
	TIM_DeInit(TIM1);
	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0; //预分频18，即72MHz/18=4MHz.
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 90-1;//计数周期为40000,输出频率为100Hz.
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);

	/* Channel 1, 2,3 and 4 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;                  
	TIM_OCInitStructure.TIM_Pulse = 0; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;         
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;          

	TIM_OC1Init(TIM1,&TIM_OCInitStructure);
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
#if 0	
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

	/* TIM1 counter enable */
	TIM_Cmd(TIM1,ENABLE);

	/* Main Output Enable */
	
	//当CCRx>TIM_CNT时，输出“1”
	TIM1->CCR1=45-1;	//TIMx->CCRy
#else	
		/* DMA1 Channel6 Config */
	DMA_DeInit(DMA1_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&TIM1->CCR1);//(uint32_t)TIM1_CCR1_Address;	//DMA外设基址 TIM3_CCR3_Address 
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)LED_BYTE_Buffer;		//DMA内存基址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						//DST外设作为数据传送目的地址,SRC则作为源地址
	DMA_InitStructure.DMA_BufferSize = LEDS_N * 24 +1;						//DMA缓存大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址寄存器不增加
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//内存地址寄存器不增加
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外存字宽16bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;		//内存字宽16bit
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// stop DMA feed after buffer size is reached
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//DMA置为高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							//不置为内存到内存传输
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
	/* TIM3 CC1 DMA Request enable */
	TIM_DMACmd(TIM1, TIM_DMA_CC2, ENABLE);
	//DMA_Cmd(DMA1_Channel3, DISABLE); 					//disable DMA channel 6
#endif
#endif
}

/* This function sends data bytes out to a string of WS2812s
 * The first argument is a pointer to the first RGB triplet to be sent
 * The seconds argument is the number of LEDs in the chain
 * 
 * This will result in the RGB triplet passed by argument 1 being sent to 
 * the LED that is the furthest away from the controller (the point where
 * data is injected into the chain)
 */
void WS2812_send(uint8_t (*color)[3], uint16_t len)
{
	uint8_t i;
	uint16_t memaddr;
	uint16_t index;
	uint16_t buffersize;
	buffersize = (len * 24) + 1;	// number of bytes needed is #LEDs * 24 bytes + 42 trailing bytes for RESET CODE
	memaddr = 0;
	index = 0;
	while (len)
	{	
		for(i=0; i<8; i++) // GREEN data
		{
			LED_BYTE_Buffer[memaddr] = ((color[index + 0][1]<<i) & 0x0080) ? TIMING_ONE:TIMING_ZERO;
			memaddr++;
		}
		for(i=0; i<8; i++) // RED
		{
			LED_BYTE_Buffer[memaddr] = ((color[index + 0][0]<<i) & 0x0080) ? TIMING_ONE:TIMING_ZERO;
			memaddr++;
		}
		for(i=0; i<8; i++) // BLUE
		{
			LED_BYTE_Buffer[memaddr] = ((color[index + 0][2]<<i) & 0x0080) ? TIMING_ONE:TIMING_ZERO;
			memaddr++;
		}
		len--;
		index++;
	}
	//===================================================================//	
	//bug：最后一个周期波形不知道为什么全是高电平，故增加一个波形
	//LED_BYTE_Buffer[memaddr] = TIMING_HIGH;//((color[0][2]<<8) & 0x0080) ? TIMING_ONE:TIMING_ZERO;
	//===================================================================//	
	//memaddr++;	
	while(memaddr < buffersize)
	{
		LED_BYTE_Buffer[memaddr] = 0;
		memaddr++;
	}
#ifdef	USE_TIME3
	DMA_SetCurrDataCounter(DMA1_Channel3, buffersize); 	//load number of bytes to be transferred
	DMA_Cmd(DMA1_Channel3, ENABLE); 					//enable DMA channel 6
	TIM_Cmd(TIM3, ENABLE); 								//enable Timer 3
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC3)) ; 			//wait until transfer complete
	TIM_Cmd(TIM3, DISABLE); 							//disable Timer 3
	DMA_Cmd(DMA1_Channel3, DISABLE); 					//disable DMA channel 6
	DMA_ClearFlag(DMA1_FLAG_TC3); 						//clear DMA1 Channel 6 transfer complete flag
#else
	DMA_SetCurrDataCounter(DMA1_Channel3, buffersize); 	//load number of bytes to be transferred
	DMA_Cmd(DMA1_Channel3, ENABLE); 					//enable DMA channel 6
	//TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_Cmd(TIM1, ENABLE); 								//enable Timer 3
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC3)) ; 			//wait until transfer complete
	//OSTimeDlyHMSM(0, 0, 0, 10);
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);
	TIM_Cmd(TIM1, DISABLE); 							//disable Timer 3
	DMA_Cmd(DMA1_Channel3, DISABLE); 					//disable DMA channel 6
	DMA_ClearFlag(DMA1_FLAG_TC3); 						//clear DMA1 Channel 6 transfer complete flag	
#endif
}

void ws2812display(unsigned int *disbuf, unsigned char length)
{
	uint8_t i;
	uint16_t memaddr;
	uint16_t index;
	uint16_t buffersize;
	unsigned int databyte;
	buffersize = (length * 24) + 1;	// number of bytes needed is #LEDs * 24 bytes + 42 trailing bytes for RESET CODE
	memaddr = 0;
	index = 0;
	while (length)
	{	
		databyte = disbuf[index] & 0x000000ff;
		for(i=0; i<8; i++) // GREEN data
		{
			LED_BYTE_Buffer[memaddr] = (databyte<<i & 0x0080) ? TIMING_ONE:TIMING_ZERO;
			memaddr++;
		}
		databyte = (disbuf[index] & 0x0000ff00)>>8;
		for(i=0; i<8; i++) // RED
		{
			LED_BYTE_Buffer[memaddr] = (databyte<<i & 0x0080) ? TIMING_ONE:TIMING_ZERO;
			memaddr++;
		}
		databyte = (disbuf[index] & 0x00ff0000)>>16;
		for(i=0; i<8; i++) // BLUE
		{
			LED_BYTE_Buffer[memaddr] = (databyte<<i & 0x0080) ? TIMING_ONE:TIMING_ZERO;
			memaddr++;
		}
		length--;
		index++;
	}
	//===================================================================//	
	//bug：最后一个周期波形不知道为什么全是高电平，故增加一个波形
	//LED_BYTE_Buffer[memaddr] = TIMING_HIGH;//((color[0][2]<<8) & 0x0080) ? TIMING_ONE:TIMING_ZERO;
	//===================================================================//	
	//memaddr++;	
	while(memaddr < buffersize)
	{
		LED_BYTE_Buffer[memaddr] = 0;
		memaddr++;
	}
	//DMA_SetCurrDataCounter(DMA1_Channel3, buffersize); 	//load number of bytes to be transferred
	DMA_Cmd(DMA1_Channel3, ENABLE); 					//enable DMA channel 6
	TIM_Cmd(TIM1, ENABLE); 								//enable Timer 3
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC3)) ; 			//wait until transfer complete
	TIM_Cmd(TIM1, DISABLE); 							//disable Timer 3
	DMA_Cmd(DMA1_Channel3, DISABLE); 					//disable DMA channel 6
	DMA_ClearFlag(DMA1_FLAG_TC3); 						//clear DMA1 Channel 6 transfer complete flag	
}
/*----------------------------------------------------------------------------
 * @描述: 	main 创建任务函数		
 * @输入: 				
 * @返回: 	
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          创建模块	
----------------------------------------------------------------------------*/
void ws2812Test(void)
{ 
	unsigned short i;
	unsigned short k;
	unsigned short j;
	unsigned char inc;
	unsigned char redtmp,greentmp,bluetmp;
	unsigned char ledbit[LEDS_N][3] ={0};
	unsigned int colortmp = 0;
	i = 0;
	k = 0;
	inc = 0;
	while (1)
	{			
		colortmp += 10;
		if(colortmp > 0x00ffffff)colortmp = 0;
		for(j = 0;j < LEDS_N;j++)
		{
			if(j == k)
			{
				ledbit[j][0] = redtmp;//RED (colortmp & 0x000000ff);
				ledbit[j][1] = greentmp;//GREEN(colortmp & 0x0000ff00) >> 8;//
				ledbit[j][2] = bluetmp;	//(colortmp & 0x00ff0000) >> 16;//
			}
			else
			{
				ledbit[j][0] = 0;//RED
				ledbit[j][1] = 0;//GREEN
				ledbit[j][2] = 0;					
			}
		}

#if 1		
		if(inc == 0)
		{
			k ++;
			if(k==LEDS_N)
			{
				inc = 1;
				if(i%3 == 0){redtmp=100;greentmp=0;bluetmp=0;}
				else if(i%3 == 1){redtmp=0;greentmp=100;bluetmp=0;}
				else if(i%3 == 2){redtmp=0;greentmp=0;bluetmp=100;}
				i++;
				if(i==3)i=0;
			}
		}
		else 
		{
			k --;
			if(k==0)
			{
				inc = 0;
				//if(i%3 == 0){redtmp=255;greentmp=0;bluetmp=0;}
				//else if(i%3 == 1){redtmp=0;greentmp=255;bluetmp=0;}
				//else if(i%3 == 2){redtmp=0;greentmp=0;bluetmp=255;}
				//i++;
				//if(i==3)i=0;
			}
		}
#else

		k ++;
		if(k==LEDS_N)
		{
			inc = 1;
			if(i%3 == 0){redtmp=255;greentmp=0;bluetmp=0;}
			else if(i%3 == 1){redtmp=0;greentmp=255;bluetmp=0;}
			else if(i%3 == 2){redtmp=0;greentmp=0;bluetmp=255;}
			i++;
			if(i==3)i=0;
			k = 0;
		}
#endif		
		WS2812_send(&ledbit[0], LEDS_N);
		
		OSTimeDlyHMSM(0, 0, 0, 100);
	}
}
