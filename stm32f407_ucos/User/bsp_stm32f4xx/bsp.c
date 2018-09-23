/*
*********************************************************************************************************
*
*	模块名称 : BSP模块(For STM32F4XX)
*	文件名称 : bsp.c
*	版    本 : V1.1
*	说    明 : 这是硬件底层驱动程序的主文件。每个c文件可以 #include "bsp.h" 来包含所有的外设驱动模块。
*			   bsp = Borad surport packet 板级支持包
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-03-01  armfly   正式发布
*		V1.1    2013-06-20  armfly   规范注释，添加必要说明
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include <includes.h>
#include <sheep.h>
#include "vad.h"
#include "..\prink\printk.h"


void NVIC_Configuration(void);

/*
*********************************************************************************************************
*                                             REGISTERS
*********************************************************************************************************
*/

#define  BSP_REG_DEM_CR                       (*(CPU_REG32 *)0xE000EDFC)
#define  BSP_REG_DWT_CR                       (*(CPU_REG32 *)0xE0001000)
#define  BSP_REG_DWT_CYCCNT                   (*(CPU_REG32 *)0xE0001004)
#define  BSP_REG_DBGMCU_CR                    (*(CPU_REG32 *)0xE0042004)

/*
*********************************************************************************************************
*                                            REGISTER BITS
*********************************************************************************************************
*/

#define  BSP_DBGMCU_CR_TRACE_IOEN_MASK                   0x10
#define  BSP_DBGMCU_CR_TRACE_MODE_ASYNC                  0x00
#define  BSP_DBGMCU_CR_TRACE_MODE_SYNC_01                0x40
#define  BSP_DBGMCU_CR_TRACE_MODE_SYNC_02                0x80
#define  BSP_DBGMCU_CR_TRACE_MODE_SYNC_04                0xC0
#define  BSP_DBGMCU_CR_TRACE_MODE_MASK                   0xC0

#define  BSP_BIT_DEM_CR_TRCENA                    DEF_BIT_24

#define  BSP_BIT_DWT_CR_CYCCNTENA                 DEF_BIT_00

extern u16 		VcBuf1[VcBuf_Len];
extern u16 		VcBuf2[VcBuf_Len];



void DMA_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr)
{ 
 
	DMA_InitTypeDef  DMA_InitStructure;
	if((u32)DMA_Streamx>(u32)DMA2)//得到当前stream是属于DMA2还是DMA1
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 		
	}
	else 
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1时钟使能 
	}
	DMA_DeInit(DMA_Streamx);
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}//等待DMA可配置 
		
	/* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel = chx;  				//通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = par;			//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = mar;			//DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	//外设到存储器模式
	DMA_InitStructure.DMA_BufferSize = ndtr;				//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;						//使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;				//中等优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
	DMA_Init(DMA_Streamx, &DMA_InitStructure);//初始化DMA Stream
}
//开启一次DMA传输
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:数据传输量  
void DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr)
{
 
	DMA_Cmd(DMA_Streamx, DISABLE);                      //关闭DMA传输 
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//确保DMA可以被设置  	
	DMA_SetCurrDataCounter(DMA_Streamx, ndtr);          //数据传输量  
	DMA_Cmd(DMA_Streamx, ENABLE);                      //开启DMA传输 
}

void ADC_DMA_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
	TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
	TIM_OCInitTypeDef         TIM_OCInitStructure;

	//RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
	
	/* Enable peripheral clocks ------------------------------------------------*/
	/* Enable DMA1 clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA1时钟使能 

	/* Enable ADC1 and GPIOC clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_APB1Periph_TIM3, ENABLE);
	RCC_AHB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_ADC1, ENABLE);


	ADC_DeInit(); 
	/* Configure PA.01 (ADC Channel1) as analog input -------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;							//不带上下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;					//独立模式   
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//Sampling Time + Conversion Time = 15 + 5 cycles = 20cyc 
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; 			//DMA失能
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;					//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz   /* PCLK2 = HCLK / 2*/
	ADC_CommonInit(&ADC_CommonInitStructure);									//Conversion Time =21MHz / 20cyc = 1.05MSps.
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;						//12位模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;								//非扫描模式	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//关闭连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_CC1;		//ADC_ExternalTrigConv_T3_TRGO;		//使用T3 TRGO触发    ADC_ExternalTrigConv_T3_CC1
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;//触发边沿上升 ADC_ExternalTrigConvEdge_Rising
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//右对齐	
	ADC_InitStructure.ADC_NbrOfConversion = 1;									//1个转换在规则序列中 也就是只转换规则序列1 
	ADC_Init(ADC1, &ADC_InitStructure);	

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_15Cycles); //配置ADC转换通道

	ADC_Cmd(ADC1, ENABLE);														//开启AD转换器	 

	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 reset calibaration register */   
	//ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	//while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibaration */
	//ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	//while(ADC_GetCalibrationStatus(ADC1));

	/* Start ADC1 Software Conversion */ 
	//ADC_SoftwareStartConv(ADC1);		//使能指定的ADC1的软件转换启动功能	
	
#if 1
	DMA_Config(DMA2_Stream0, DMA_Channel_0, (u32)&(ADC1->DR),(u32)VcBuf1, VcBuf_Len);//
	DMA_Enable(DMA2_Stream0, VcBuf_Len);
	//DMA_Cmd(DMA2_Stream0, ENABLE);                      				//开启DMA传输 
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE); 		
#else
	DMA_DeInit(DMA2_Stream0);
	/* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  					//通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(ADC1->DR);		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)VcBuf;					//DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;				//存储器到外设模式
	DMA_InitStructure.DMA_BufferSize = 10;//VcBuf_Len;					//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据长度:16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	//存储器数据长度:16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;						// 使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;					//中等优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;			//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;	//外设突发单次传输
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);							//初始化DMA Stream
	DMA_Cmd(DMA2_Stream0, ENABLE);                      				//开启DMA传输 
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE); 	
	//DMA_ClearFlag(DMA2_Stream0, 0);
#endif
#if 1
	/* TIM1 configuration ------------------------------------------------------*/ 
	/*Tout=((Perio+1)*(TIM_Prescaler+1))/Freq_timer us.*****/
	/* Time Base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Period = 999;      					//自动重装载值  168000000/100/210=8000    
	TIM_TimeBaseStructure.TIM_Prescaler = 99;       				//定时器分频  (PCLK/2)/8000000 - 1
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    				
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	/* TIM1 channel1 configuration in PWM mode */

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; 				//选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;		//比较输出使能                
	TIM_OCInitStructure.TIM_Pulse = 50; 	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     	//输出极性:TIM输出比较极性低    
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM3, ENABLE);	
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_OC1);//定时器触发输出	
	//TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
	TIM_Cmd(TIM3, ENABLE); 	//开启定时器，开始信号采集	
#endif	

#if 0
	TIM_ARRPreloadConfig(TIM3, ENABLE);				   
	/* TIM1 counter enable */
	//TIM_Cmd(TIM1, ENABLE); 
	/* TIM1 main Output Enable */
	//TIM_CtrlPWMOutputs(TIM1, ENABLE); 
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_OC1);//定时器触发输出

	TIM_Cmd(TIM3, ENABLE); 	//开启定时器，开始信号采集	
#endif	
}


static void ADC3_CH13_DMA_Config(void)
{
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef       ADC_InitStructure;
    DMA_InitTypeDef  DMA_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	GPIO_InitTypeDef GPIO_InitStructure;   
    //时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
{    
    //DMA设置
    DMA_InitStructure.DMA_Channel = DMA_Channel_0; //选择通道号
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR); //外围设备地址,ADC_DR_DATA规则数据寄存器
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(u16 *)VcBuf1; //DMA存储器地址,自己设置的缓存地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory; //传输方向,外设->存储器
    DMA_InitStructure.DMA_BufferSize = VcBuf_Len; //DMA缓存大小,数据传输量32*8
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设是否为增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //存储器是否为增量模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //外设数据长度半个字(16位)
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //存储器数据长度半字(16位)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA模式,普通模式、循环模式,还有双缓冲模式,需要特殊设置
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA优先级,高优先级
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //不使用FIFO
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; //FIFO阈值
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; //存储器突发,单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; //外设突发,单次传输
    DMA_Init(DMA2_Stream0, &DMA_InitStructure); //初始化DMA2_Stream0,对应为ADC3

    //双缓冲模式设置
    //DMA_DoubleBufferModeConfig(DMA2_Stream0,(uint32_t)(u16 *)(VcBuf2),DMA_Memory_0);    //DMA_Memory_0首先被传输
    //DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);
    
    //设置DMA中断
    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC); //清除中断标志   
    DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, DISABLE); //传输完成中断                                       
    DMA_Cmd(DMA2_Stream0, ENABLE); //使能DMA		
}

{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //使能ADC3时钟
    
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);      //ADC3复位
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE); //复位结束     

	/* Configure PA.01 (ADC Channel1) as analog input -------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;							//不带上下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; //两个采样阶段之间的延迟5个时钟(三重模式或双重模式下使用)
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能(对于多个ADC模式)
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化
    
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;//扫描模式(多通道ADC采集要用扫描模式)    
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//关闭连续转换
	//ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //软件触发
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //上升沿触发
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;  //定时器事件2触发ADC
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐    
    ADC_InitStructure.ADC_NbrOfConversion = 1;    //8个转换在规则序列中
    ADC_Init(ADC1, &ADC_InitStructure);//ADC初始化
    
    //连续模式下,通道的配置
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_15Cycles);  //PF3,VIN1,通道9,UA,rank=1,表示连续转换中第一个转换的通道,采样时间15个周期(如果数据不稳定可以延时采样时间)
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_14, 2, ADC_SampleTime_15Cycles); //PF4,VIN2,通道14,UB
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_15, 3, ADC_SampleTime_15Cycles); //PF5,VIN3,通道15,AC220
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_4, 4, ADC_SampleTime_15Cycles);     //PF6,VIN4,通道4,I0
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 5, ADC_SampleTime_15Cycles);     //PF7,VIN5,通道5,IA
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_6, 6, ADC_SampleTime_15Cycles);     //PF8,VIN6,通道6,IB
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_7, 7, ADC_SampleTime_15Cycles);     //PF9,VIN7,通道7,IC
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_8, 8, ADC_SampleTime_15Cycles);     //PF10,VIN8,通道8,暂时未接入信号

    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE); //连续使能DMA
    ADC_DMACmd(ADC1, ENABLE);  //使能ADC_DMA
    ADC_Cmd(ADC1, ENABLE);//开启AD转换器    
	
}

{ 
    //时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);          

    //失能定时器
    TIM_Cmd(TIM2, DISABLE);
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //初始化定时器
    
    //定时器设置,T=(84 * 625) / 84M = 625us = 20ms/32,50Hz信号,每周期采样32个点,采样频率1600
    TIM_TimeBaseStructure.TIM_Prescaler = 84-1; //分频系数,TIM2时钟在分频系数不为1时,时钟是APB1时钟的两倍84M
    TIM_TimeBaseStructure.TIM_Period = 1000000/fs - 1; //周期值1M/32=31250,CurrentFreq单位为0.1hz,因此需扩大10倍
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //时钟分频因子
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ; //向上计数模式
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //初始化定时器2
    
    //使能定时器中断    
    TIM_ARRPreloadConfig(TIM2, ENABLE); //允许TIM2定时重载
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);  //选择TIM2的UPDATA事件更新为触发源
	//TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //配置TIM2中断类型
    
    //使能TIM2
    //TIM_Cmd(TIM2, ENABLE); 
}
}
/*
*********************************************************************************************************
*	函 数 名: bsp_Init
*	功能说明: 初始化硬件设备
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Init(void)
{
	NVIC_Configuration();  /* 中断优先级分组配置 */
	
	bsp_InitUart(); 	   /* 初始化串口 */
	bsp_InitLed(); 		   /* 初始LED指示灯端口 */
	bsp_InitKey(); 	       /* 按键初始化 */	
	//Lcd_Initialize();
	//ADC_DMA_Init();
	ADC3_CH13_DMA_Config();
	//Adc_Init();
	//TIM3_Int_Init();
#ifdef TRACE_EN                                                 /* See project / compiler preprocessor options.         */
    BSP_CPU_REG_DBGMCU_CR |=  BSP_DBGMCU_CR_TRACE_IOEN_MASK;    /* Enable tracing (see Note #2).                        */
    BSP_CPU_REG_DBGMCU_CR &= ~BSP_DBGMCU_CR_TRACE_MODE_MASK;    /* Clr trace mode sel bits.                             */
    BSP_CPU_REG_DBGMCU_CR |=  BSP_DBGMCU_CR_TRACE_MODE_SYNC_04; /* Cfg trace mode to synch 4-bit.                       */
#endif
	
}

/*
*********************************************************************************************************
*	函 数 名: NVIC_Configuration
*	功能说明: 中断优先级配置，这里先把优先级分组分配好，然后在
*			  具体的模块里面实现优先级配置 
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void NVIC_Configuration(void)
{	
	NVIC_InitTypeDef   NVIC_InitStructure;	
	/* 设置NVIC优先级分组为Group2：0-3抢占式优先级，0-3的响应式优先级 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	

    //定时器中断设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //定时器TIM2中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;  //抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;        //子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;            //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);    //根据指定的参数初始化NVIC寄存器    
    
    //DMA中断设置
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;  //DMA2_Stream0中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;  //抢占优先级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;        //子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;            //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);    //根据指定的参数初始化NVIC寄存器    
}

#if 0
u16 *CurrentBuffPtr = NULL;        //当前缓存指针
void DMA2_Stream0_IRQHandler(void)  
{
    u16 period = 0;
    
	OSIntEnter(); //进入中断 
    if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))  //判断DMA传输完成中断  
    {
         DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
//         DMA_Cmd(DMA2_Stream0, ENABLE); //使能DMA,连续模式下不需要该使能
         //数据转移程序
         if(DMA_GetCurrentMemoryTarget(DMA2_Stream0) == DMA_Memory_0)
         {
             CurrentBuffPtr = VcBuf1;
         }
         else
         {
             CurrentBuffPtr = VcBuf2;
         }
         //遥测任务等待该信号量
         //OSSemPost(SemAD);
         
         //判断是否更新TIM2
         //if(UpdataTIM)
         //{
         //    period = 312500/CurrentFreq - 1;        //计算采样频率1M/32=31250;32为采样点数,31250/freq=采样周期
         //    TIM_ARRPreloadConfig(TIM2, DISABLE); //关闭TIM2定时重载
         //    TIM2->ARR = period ;        //更新定时器周期
         //    TIM_ARRPreloadConfig(TIM2, ENABLE);     //打开TIM2定时重载
         //}
    }
    OSIntExit();  //退出中断  
}
#endif
/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    RCC_ClocksTypeDef  rcc_clocks;


    RCC_GetClocksFreq(&rcc_clocks);
    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}


/*
*********************************************************************************************************
*                                            BSP_Tick_Init()
*
* Description : Initialize all the peripherals that required OS Tick services (OS initialized)
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void BSP_Tick_Init (void)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    
    cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */
    //printf("cpu_clk_freq = %d\r\n", cpu_clk_freq);
#if (OS_VERSION >= 30000u)
    cnts  = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;       /* Determine nbr SysTick increments.                    */
#else
    cnts  = cpu_clk_freq / (CPU_INT32U)OS_TICKS_PER_SEC;        /* Determine nbr SysTick increments.                    */
#endif
    
    OS_CPU_SysTickInit(cnts);                                 /* 这里默认的是最高优先级，根据实际情况修改             */
//	SysTick_Config(cnts);   //这里默认的是最低优先级
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                          CPU_TS_TmrInit()
*
* Description : Initialize & start CPU timestamp timer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_TS_Init().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but MUST NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrInit() is an application/BSP function that MUST be defined by the developer 
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR' 
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater 
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR' 
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be 
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets 
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple 
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the 
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                   (c) When applicable, timer period SHOULD be less than the typical measured time 
*                       but MUST be less than the maximum measured time; otherwise, timer resolution 
*                       inadequate to measure desired times.
*
*                   See also 'CPU_TS_TmrRd()  Note #2'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
void  CPU_TS_TmrInit (void)
{
    CPU_INT32U  fclk_freq;


    fclk_freq = BSP_CPU_ClkFreq();

    BSP_REG_DEM_CR     |= (CPU_INT32U)BSP_BIT_DEM_CR_TRCENA;    /* Enable Cortex-M4's DWT CYCCNT reg.                   */
    BSP_REG_DWT_CYCCNT  = (CPU_INT32U)0u;
    BSP_REG_DWT_CR     |= (CPU_INT32U)BSP_BIT_DWT_CR_CYCCNTENA;

    CPU_TS_TmrFreqSet((CPU_TS_TMR_FREQ)fclk_freq);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           CPU_TS_TmrRd()
*
* Description : Get current CPU timestamp timer count value.
*
* Argument(s) : none.
*
* Return(s)   : Timestamp timer count (see Notes #2a & #2b).
*
* Caller(s)   : CPU_TS_Init(),
*               CPU_TS_Get32(),
*               CPU_TS_Get64(),
*               CPU_IntDisMeasStart(),
*               CPU_IntDisMeasStop().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but SHOULD NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrRd() is an application/BSP function that MUST be defined by the developer 
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR' 
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater 
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR' 
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be 
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets 
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple 
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the 
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                       (1) If timer is a 'down' counter whose values decrease with each time count,
*                           then the returned timer value MUST be ones-complemented.
*
*                   (c) (1) When applicable, the amount of time measured by CPU timestamps is 
*                           calculated by either of the following equations :
*
*                           (A) Time measured  =  Number timer counts  *  Timer period
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured 
*                                       Timer period            Timer's period in some units of 
*                                                                   (fractional) seconds
*                                       Time measured           Amount of time measured, in same 
*                                                                   units of (fractional) seconds 
*                                                                   as the Timer period
*
*                                                  Number timer counts
*                           (B) Time measured  =  ---------------------
*                                                    Timer frequency
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured
*                                       Timer frequency         Timer's frequency in some units 
*                                                                   of counts per second
*                                       Time measured           Amount of time measured, in seconds
*
*                       (2) Timer period SHOULD be less than the typical measured time but MUST be less 
*                           than the maximum measured time; otherwise, timer resolution inadequate to 
*                           measure desired times.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_TS_TMR  CPU_TS_TmrRd (void)
{
    CPU_TS_TMR  ts_tmr_cnts;

                                                                
    ts_tmr_cnts = (CPU_TS_TMR)BSP_REG_DWT_CYCCNT;

    return (ts_tmr_cnts);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         CPU_TSxx_to_uSec()
*
* Description : Convert a 32-/64-bit CPU timestamp from timer counts to microseconds.
*
* Argument(s) : ts_cnts   CPU timestamp (in timestamp timer counts [see Note #2aA]).
*
* Return(s)   : Converted CPU timestamp (in microseconds           [see Note #2aD]).
*
* Caller(s)   : Application.
*
*               This function is an (optional) CPU module application programming interface (API) 
*               function which MAY be implemented by application/BSP function(s) [see Note #1] & 
*               MAY be called by application function(s).
*
* Note(s)     : (1) CPU_TS32_to_uSec()/CPU_TS64_to_uSec() are application/BSP functions that MAY be 
*                   optionally defined by the developer when either of the following CPU features is 
*                   enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) The amount of time measured by CPU timestamps is calculated by either of 
*                       the following equations :
*
*                                                                        10^6 microseconds
*                       (1) Time measured  =   Number timer counts   *  -------------------  *  Timer period
*                                                                            1 second
*
*                                              Number timer counts       10^6 microseconds
*                       (2) Time measured  =  ---------------------  *  -------------------
*                                                Timer frequency             1 second
*
*                               where
*
*                                   (A) Number timer counts     Number of timer counts measured
*                                   (B) Timer frequency         Timer's frequency in some units 
*                                                                   of counts per second
*                                   (C) Timer period            Timer's period in some units of 
*                                                                   (fractional)  seconds
*                                   (D) Time measured           Amount of time measured, 
*                                                                   in microseconds
*
*                   (b) Timer period SHOULD be less than the typical measured time but MUST be less 
*                       than the maximum measured time; otherwise, timer resolution inadequate to 
*                       measure desired times.
*
*                   (c) Specific implementations may convert any number of CPU_TS32 or CPU_TS64 bits 
*                       -- up to 32 or 64, respectively -- into microseconds.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_32_EN == DEF_ENABLED)
CPU_INT64U  CPU_TS32_to_uSec (CPU_TS32  ts_cnts)
{
    CPU_INT64U  ts_us;
    CPU_INT64U  fclk_freq;

    
    fclk_freq = BSP_CPU_ClkFreq();    
    ts_us     = ts_cnts / (fclk_freq / DEF_TIME_NBR_uS_PER_SEC);

    return (ts_us);
}
#endif


#if (CPU_CFG_TS_64_EN == DEF_ENABLED)
CPU_INT64U  CPU_TS64_to_uSec (CPU_TS64  ts_cnts)
{
    CPU_INT64U  ts_us;
    CPU_INT64U  fclk_freq;
    

    fclk_freq = BSP_CPU_ClkFreq();    
    ts_us     = ts_cnts / (fclk_freq / DEF_TIME_NBR_uS_PER_SEC);

    return (ts_us);
}
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
