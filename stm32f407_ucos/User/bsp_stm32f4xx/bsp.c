/*
*********************************************************************************************************
*
*	ģ������ : BSPģ��(For STM32F4XX)
*	�ļ����� : bsp.c
*	��    �� : V1.1
*	˵    �� : ����Ӳ���ײ�������������ļ���ÿ��c�ļ����� #include "bsp.h" ���������е���������ģ�顣
*			   bsp = Borad surport packet �弶֧�ְ�
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-03-01  armfly   ��ʽ����
*		V1.1    2013-06-20  armfly   �淶ע�ͣ���ӱ�Ҫ˵��
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
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
	if((u32)DMA_Streamx>(u32)DMA2)//�õ���ǰstream������DMA2����DMA1
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2ʱ��ʹ�� 		
	}
	else 
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1ʱ��ʹ�� 
	}
	DMA_DeInit(DMA_Streamx);
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}//�ȴ�DMA������ 
		
	/* ���� DMA Stream */
	DMA_InitStructure.DMA_Channel = chx;  				//ͨ��ѡ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = par;			//DMA�����ַ
	DMA_InitStructure.DMA_Memory0BaseAddr = mar;			//DMA �洢��0��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	//���赽�洢��ģʽ
	DMA_InitStructure.DMA_BufferSize = ndtr;				//���ݴ����� 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//���������ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//�洢������ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//�������ݳ���:8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;//�洢�����ݳ���:8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;						//ʹ����ͨģʽ 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;				//�е����ȼ�
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//�洢��ͻ�����δ���
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//����ͻ�����δ���
	DMA_Init(DMA_Streamx, &DMA_InitStructure);//��ʼ��DMA Stream
}
//����һ��DMA����
//DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:���ݴ�����  
void DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr)
{
 
	DMA_Cmd(DMA_Streamx, DISABLE);                      //�ر�DMA���� 
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//ȷ��DMA���Ա�����  	
	DMA_SetCurrDataCounter(DMA_Streamx, ndtr);          //���ݴ�����  
	DMA_Cmd(DMA_Streamx, ENABLE);                      //����DMA���� 
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
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA1ʱ��ʹ�� 

	/* Enable ADC1 and GPIOC clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_APB1Periph_TIM3, ENABLE);
	RCC_AHB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_ADC1, ENABLE);


	ADC_DeInit(); 
	/* Configure PA.01 (ADC Channel1) as analog input -------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//ģ������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;							//����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;					//����ģʽ   
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//Sampling Time + Conversion Time = 15 + 5 cycles = 20cyc 
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; 			//DMAʧ��
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;					//Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz   /* PCLK2 = HCLK / 2*/
	ADC_CommonInit(&ADC_CommonInitStructure);									//Conversion Time =21MHz / 20cyc = 1.05MSps.
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;						//12λģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;								//��ɨ��ģʽ	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//�ر�����ת��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_CC1;		//ADC_ExternalTrigConv_T3_TRGO;		//ʹ��T3 TRGO����    ADC_ExternalTrigConv_T3_CC1
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;//������������ ADC_ExternalTrigConvEdge_Rising
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//�Ҷ���	
	ADC_InitStructure.ADC_NbrOfConversion = 1;									//1��ת���ڹ��������� Ҳ����ֻת����������1 
	ADC_Init(ADC1, &ADC_InitStructure);	

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_15Cycles); //����ADCת��ͨ��

	ADC_Cmd(ADC1, ENABLE);														//����ADת����	 

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
	//ADC_SoftwareStartConv(ADC1);		//ʹ��ָ����ADC1�����ת����������	
	
#if 1
	DMA_Config(DMA2_Stream0, DMA_Channel_0, (u32)&(ADC1->DR),(u32)VcBuf1, VcBuf_Len);//
	DMA_Enable(DMA2_Stream0, VcBuf_Len);
	//DMA_Cmd(DMA2_Stream0, ENABLE);                      				//����DMA���� 
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE); 		
#else
	DMA_DeInit(DMA2_Stream0);
	/* ���� DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  					//ͨ��ѡ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(ADC1->DR);		//DMA�����ַ
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)VcBuf;					//DMA �洢��0��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;				//�洢��������ģʽ
	DMA_InitStructure.DMA_BufferSize = 10;//VcBuf_Len;					//���ݴ����� 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//���������ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//�洢������ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//�������ݳ���:16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	//�洢�����ݳ���:16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;						// ʹ����ͨģʽ 
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;					//�е����ȼ�
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;			//�洢��ͻ�����δ���
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;	//����ͻ�����δ���
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);							//��ʼ��DMA Stream
	DMA_Cmd(DMA2_Stream0, ENABLE);                      				//����DMA���� 
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE); 	
	//DMA_ClearFlag(DMA2_Stream0, 0);
#endif
#if 1
	/* TIM1 configuration ------------------------------------------------------*/ 
	/*Tout=((Perio+1)*(TIM_Prescaler+1))/Freq_timer us.*****/
	/* Time Base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Period = 999;      					//�Զ���װ��ֵ  168000000/100/210=8000    
	TIM_TimeBaseStructure.TIM_Prescaler = 99;       				//��ʱ����Ƶ  (PCLK/2)/8000000 - 1
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    				
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	/* TIM1 channel1 configuration in PWM mode */

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; 				//ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;		//�Ƚ����ʹ��                
	TIM_OCInitStructure.TIM_Pulse = 50; 	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     	//�������:TIM����Ƚϼ��Ե�    
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM3, ENABLE);	
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_OC1);//��ʱ���������	
	//TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
	TIM_Cmd(TIM3, ENABLE); 	//������ʱ������ʼ�źŲɼ�	
#endif	

#if 0
	TIM_ARRPreloadConfig(TIM3, ENABLE);				   
	/* TIM1 counter enable */
	//TIM_Cmd(TIM1, ENABLE); 
	/* TIM1 main Output Enable */
	//TIM_CtrlPWMOutputs(TIM1, ENABLE); 
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_OC1);//��ʱ���������

	TIM_Cmd(TIM3, ENABLE); 	//������ʱ������ʼ�źŲɼ�	
#endif	
}


static void ADC3_CH13_DMA_Config(void)
{
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef       ADC_InitStructure;
    DMA_InitTypeDef  DMA_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	GPIO_InitTypeDef GPIO_InitStructure;   
    //ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
{    
    //DMA����
    DMA_InitStructure.DMA_Channel = DMA_Channel_0; //ѡ��ͨ����
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR); //��Χ�豸��ַ,ADC_DR_DATA�������ݼĴ���
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(u16 *)VcBuf1; //DMA�洢����ַ,�Լ����õĻ����ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory; //���䷽��,����->�洢��
    DMA_InitStructure.DMA_BufferSize = VcBuf_Len; //DMA�����С,���ݴ�����32*8
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //�����Ƿ�Ϊ����ģʽ
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //�洢���Ƿ�Ϊ����ģʽ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //�������ݳ��Ȱ����(16λ)
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //�洢�����ݳ��Ȱ���(16λ)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMAģʽ,��ͨģʽ��ѭ��ģʽ,����˫����ģʽ,��Ҫ��������
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA���ȼ�,�����ȼ�
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //��ʹ��FIFO
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; //FIFO��ֵ
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; //�洢��ͻ��,���δ���
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; //����ͻ��,���δ���
    DMA_Init(DMA2_Stream0, &DMA_InitStructure); //��ʼ��DMA2_Stream0,��ӦΪADC3

    //˫����ģʽ����
    //DMA_DoubleBufferModeConfig(DMA2_Stream0,(uint32_t)(u16 *)(VcBuf2),DMA_Memory_0);    //DMA_Memory_0���ȱ�����
    //DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);
    
    //����DMA�ж�
    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC); //����жϱ�־   
    DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, DISABLE); //��������ж�                                       
    DMA_Cmd(DMA2_Stream0, ENABLE); //ʹ��DMA		
}

{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //ʹ��ADC3ʱ��
    
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);      //ADC3��λ
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE); //��λ����     

	/* Configure PA.01 (ADC Channel1) as analog input -------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//ģ������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;							//����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//����ģʽ
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; //���������׶�֮����ӳ�5��ʱ��(����ģʽ��˫��ģʽ��ʹ��)
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMAʧ��(���ڶ��ADCģʽ)
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz 
    ADC_CommonInit(&ADC_CommonInitStructure);//��ʼ��
    
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12λģʽ
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;//ɨ��ģʽ(��ͨ��ADC�ɼ�Ҫ��ɨ��ģʽ)    
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//�ر�����ת��
	//ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //�������
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //�����ش���
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;  //��ʱ���¼�2����ADC
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//�Ҷ���    
    ADC_InitStructure.ADC_NbrOfConversion = 1;    //8��ת���ڹ���������
    ADC_Init(ADC1, &ADC_InitStructure);//ADC��ʼ��
    
    //����ģʽ��,ͨ��������
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_15Cycles);  //PF3,VIN1,ͨ��9,UA,rank=1,��ʾ����ת���е�һ��ת����ͨ��,����ʱ��15������(������ݲ��ȶ�������ʱ����ʱ��)
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_14, 2, ADC_SampleTime_15Cycles); //PF4,VIN2,ͨ��14,UB
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_15, 3, ADC_SampleTime_15Cycles); //PF5,VIN3,ͨ��15,AC220
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_4, 4, ADC_SampleTime_15Cycles);     //PF6,VIN4,ͨ��4,I0
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 5, ADC_SampleTime_15Cycles);     //PF7,VIN5,ͨ��5,IA
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_6, 6, ADC_SampleTime_15Cycles);     //PF8,VIN6,ͨ��6,IB
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_7, 7, ADC_SampleTime_15Cycles);     //PF9,VIN7,ͨ��7,IC
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_8, 8, ADC_SampleTime_15Cycles);     //PF10,VIN8,ͨ��8,��ʱδ�����ź�

    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE); //����ʹ��DMA
    ADC_DMACmd(ADC1, ENABLE);  //ʹ��ADC_DMA
    ADC_Cmd(ADC1, ENABLE);//����ADת����    
	
}

{ 
    //ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);          

    //ʧ�ܶ�ʱ��
    TIM_Cmd(TIM2, DISABLE);
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //��ʼ����ʱ��
    
    //��ʱ������,T=(84 * 625) / 84M = 625us = 20ms/32,50Hz�ź�,ÿ���ڲ���32����,����Ƶ��1600
    TIM_TimeBaseStructure.TIM_Prescaler = 84-1; //��Ƶϵ��,TIM2ʱ���ڷ�Ƶϵ����Ϊ1ʱ,ʱ����APB1ʱ�ӵ�����84M
    TIM_TimeBaseStructure.TIM_Period = 1000000/fs - 1; //����ֵ1M/32=31250,CurrentFreq��λΪ0.1hz,���������10��
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //ʱ�ӷ�Ƶ����
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ; //���ϼ���ģʽ
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //��ʼ����ʱ��2
    
    //ʹ�ܶ�ʱ���ж�    
    TIM_ARRPreloadConfig(TIM2, ENABLE); //����TIM2��ʱ����
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);  //ѡ��TIM2��UPDATA�¼�����Ϊ����Դ
	//TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //����TIM2�ж�����
    
    //ʹ��TIM2
    //TIM_Cmd(TIM2, ENABLE); 
}
}
/*
*********************************************************************************************************
*	�� �� ��: bsp_Init
*	����˵��: ��ʼ��Ӳ���豸
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_Init(void)
{
	NVIC_Configuration();  /* �ж����ȼ��������� */
	
	bsp_InitUart(); 	   /* ��ʼ������ */
	bsp_InitLed(); 		   /* ��ʼLEDָʾ�ƶ˿� */
	bsp_InitKey(); 	       /* ������ʼ�� */	
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
*	�� �� ��: NVIC_Configuration
*	����˵��: �ж����ȼ����ã������Ȱ����ȼ��������ã�Ȼ����
*			  �����ģ������ʵ�����ȼ����� 
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void NVIC_Configuration(void)
{	
	NVIC_InitTypeDef   NVIC_InitStructure;	
	/* ����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	

    //��ʱ���ж�����
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //��ʱ��TIM2�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;  //��ռ���ȼ�0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;        //�����ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;            //IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);    //����ָ���Ĳ�����ʼ��NVIC�Ĵ���    
    
    //DMA�ж�����
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;  //DMA2_Stream0�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;  //��ռ���ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;        //�����ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;            //IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);    //����ָ���Ĳ�����ʼ��NVIC�Ĵ���    
}

#if 0
u16 *CurrentBuffPtr = NULL;        //��ǰ����ָ��
void DMA2_Stream0_IRQHandler(void)  
{
    u16 period = 0;
    
	OSIntEnter(); //�����ж� 
    if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))  //�ж�DMA��������ж�  
    {
         DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
//         DMA_Cmd(DMA2_Stream0, ENABLE); //ʹ��DMA,����ģʽ�²���Ҫ��ʹ��
         //����ת�Ƴ���
         if(DMA_GetCurrentMemoryTarget(DMA2_Stream0) == DMA_Memory_0)
         {
             CurrentBuffPtr = VcBuf1;
         }
         else
         {
             CurrentBuffPtr = VcBuf2;
         }
         //ң������ȴ����ź���
         //OSSemPost(SemAD);
         
         //�ж��Ƿ����TIM2
         //if(UpdataTIM)
         //{
         //    period = 312500/CurrentFreq - 1;        //�������Ƶ��1M/32=31250;32Ϊ��������,31250/freq=��������
         //    TIM_ARRPreloadConfig(TIM2, DISABLE); //�ر�TIM2��ʱ����
         //    TIM2->ARR = period ;        //���¶�ʱ������
         //    TIM_ARRPreloadConfig(TIM2, ENABLE);     //��TIM2��ʱ����
         //}
    }
    OSIntExit();  //�˳��ж�  
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
    
    OS_CPU_SysTickInit(cnts);                                 /* ����Ĭ�ϵ���������ȼ�������ʵ������޸�             */
//	SysTick_Config(cnts);   //����Ĭ�ϵ���������ȼ�
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

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
