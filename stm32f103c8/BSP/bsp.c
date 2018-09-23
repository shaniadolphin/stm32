#include "..\App\includes.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_pwr.h"
#include "spi.h"
#include "Msg.h"

#define NEW_VERSION		1

const unsigned char table_week[12] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 }; //���������ݱ�

const unsigned char mon_table[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; //ƽ����·����ڱ�

const unsigned char display_bit_a[13] = {
	0xdf, //~0x20, //1  A�� Υ�� ʮλ
	0xef, //~0x10, //2  A�� Υ�� ��λ
	0xff, //~0x00, //3  A�� ��ʱ ��ʮλ
	0xff, //~0x00, //4  A�� ��ʱ �ָ�λ
	0x7f, //~0x80, //5  A�� ��ʱ ��ʮλ
	0xbf, //~0x40, //6  A�� ��ʱ ���λ
	0xff, //~0x00, //7  B�� Υ�� ʮλ
	0xff, //~0x00, //8  B�� Υ�� ��λ
	0xff, //~0x00, //9  �׷� ��ʱ ��ʮλ
	0xff, //~0x00, //10 �׷� ��ʱ �ָ�λ
	0xff, //~0x00, //11 �׷� ��ʱ ��ʮλ
	0xff, //~0x00, //12 �׷� ��ʱ ���λ
	~0x08, //~0x00, //13
};

const unsigned char display_bit_b[13] = {
	0xff, //~0x00, //1  A�� Υ�� ʮλ
	0xff, //~0x00, //2  A�� Υ�� ��λ
	0xfd, //~0x02, //3  A�� ��ʱ ��ʮλ
	0xfe, //~0x01, //4  A�� ��ʱ �ָ�λ
	0xff, //~0x00, //5  A�� ��ʱ ��ʮλ
	0xff, //~0x00, //6  A�� ��ʱ ���λ
	0x7f, //~0x80, //7  B�� Υ�� ʮλ
	0xbf, //~0x40, //8  B�� Υ�� ��λ
	0xdf, //~0x20, //9  �׷� ��ʱ ��ʮλ
	0xef, //~0x10, //10 �׷� ��ʱ �ָ�λ
	0xf7, //~0x08, //11 �׷� ��ʱ ��ʮλ
	0xfb, //~0x04, //12 �׷� ��ʱ ���λ
	0xff, //~0x04, //13
};

unsigned char display_dat_buf[13] = {
	0x3f, //1  A�� Υ�� ʮλ
	0x3f, //2  A�� Υ�� ��λ
	0x3f, //3  A�� ��ʱ ��ʮλ
	0x3f, //4  A�� ��ʱ �ָ�λ
	0x3f, //5  A�� ��ʱ ��ʮλ
	0x3f, //6  A�� ��ʱ ���λ
	0x3f, //7  B�� Υ�� ʮλ
	0x3f, //8  B�� Υ�� ��λ
	0x3f, //9  B�� ��ʱ ��ʮλ
	0x3f, //10 B�� ��ʱ �ָ�λ
	0x3f, //11 B�� ��ʱ ��ʮλ
	0x3f, //12
	0x3f, //13
};


unsigned char led_dis_cnt = 0;
unsigned int led_flash_flag = 0;
unsigned char led_flash_cnt = 0;
unsigned char white_led = LED_OFF;
unsigned char black_led = LED_OFF;
unsigned char state_led = LED_OFF;
unsigned short key_car_cnt = 0;
unsigned short key_horse_cnt = 0;
unsigned short key_phase_cnt = 0;
unsigned short key_queen_cnt = 0;
unsigned short key_black_cnt = 0;
unsigned short key_white_cnt = 0;
unsigned char usb_in_flag 	 = 0;
unsigned int Time_year   = 0;
unsigned char Time_month = 0;
unsigned char Time_date  = 0;
unsigned char Time_hour  = 0;
unsigned char Time_min   = 0;
unsigned char Time_sec   = 0;

extern void USART_Config(USART_TypeDef *USARTx, u32 baud);
extern void USART2_Config(u32 baud);
extern void USART_OUT(USART_TypeDef *USARTx, char *Data, ...);

ErrorStatus HSEStartUpStatus;
unsigned char RTC_Set(unsigned short syear, unsigned char smon, unsigned char sday, unsigned char hour, unsigned char min, unsigned char sec);
unsigned char RTC_Set_Unix(unsigned int unixtime);
extern void KeyChessRising(unsigned char key);
//unsigned char RTC_Get(void);
/*----------------------------------------------------------------------------
 * @����: RCC_Configuration
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
void RCC_Configuration(void)
{
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();
	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);
	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS)
	{
		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);

		/* PCLK2 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1);

		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);

		/* ADCCLK = PCLK2/4 */
		RCC_ADCCLKConfig(RCC_PCLK2_Div4);

		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		/* PLLCLK = 8MHz * 9 = 56 MHz */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

		/* Enable PLL */
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while (RCC_GetSYSCLKSource() != 0x08)
		{}
	}
}
void ADC_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
	/* Configure PC.04 (ADC Channel14) as analog input -------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  //����ģʽ  CR1
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;                            //ɨ��ģʽʹ��
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                  //����ת��ģʽʹ��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //����ת�����ⲿ�¼�--��  CR2
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              //ת����������Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;                             //ת����ͨ����Ϊ1
	ADC_Init(ADC1, &ADC_InitStructure);
	/* ADCCLK = PCLK2/4 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	/* ADC1 regular channel14 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);     //ͨ��10����ʱ��
																					//ADC_DMACmd(ADC1, ENABLE);		                          						  //����ADC1����DMA����
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);                                                        //ʹ��ADC1
	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC1);                                                   //����ADC1��λУ׼�Ĵ���
	/* Check the end of ADC1 reset calibration register */
	while (ADC_GetResetCalibrationStatus(ADC1));                      //���У׼�Ĵ����Ƿ�λ���
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);                                                   //����ADC1 У׼
	/* Check the end of ADC1 calibration */
	while (ADC_GetCalibrationStatus(ADC1));                              //���У׼�Ƿ����
	/* Start ADC1 Software Conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);                             //�����������ADC1ת��
}
/*----------------------------------------------------------------------------
 * @����: �رյ��Խӿڣ���GPIOʹ��
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
void UnableJTAG(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                     // enable clock for Alternate Function
	AFIO->MAPR &= ~(7UL << 24); // clear used bit
	AFIO->MAPR |=  (2UL << 24); // set used bits
}
/*----------------------------------------------------------------------------
 * @����: RTC_Configuration
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
void RTC_Configuration(void)
{
#if 0
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);

	/* Wait till LSE is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

#if 0
	/* Disable the Tamper Pin */
	BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper
								 functionality must be disabled */
	/* Enable RTC Clock Output on Tamper Pin */
	BKP_RTCCalibrationClockOutputCmd(ENABLE);
#endif

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
#endif
	//����ǲ��ǵ�һ������ʱ��
	//u8 temp=0;
	if (BKP->DR1 != 0x5050) //��һ������
	{
		RCC->APB1ENR	|=	1 << 28;     //ʹ�ܵ�Դʱ��
		RCC->APB1ENR	|=	1 << 27;     //ʹ�ܱ���ʱ��
		PWR->CR				|=	1 << 8;      //ȡ��������д����
		RCC->BDCR			|=	1 << 16;     //����������λ
		RCC->BDCR			&=	~(1 << 16);  //����������λ����
		RCC->BDCR			|=	1 << 0;      //�����ⲿ��������
		while ((!(RCC->BDCR & 0x02))) //�ȴ��ⲿʱ�Ӿ���	 &&temp<250
		{
			//temp++;
			//delay_ms(10);
		};
		//if(temp>=250)return 1;//��ʼ��ʱ��ʧ��,����������

		//RCC->BDCR|=1<<8; //LSI��ΪRTCʱ��
		RCC->BDCR 	|= 	RCC_RTCCLKSource_LSE;
		RCC->BDCR		|=	1 << 15; //RTCʱ��ʹ��
		while (!(RTC->CRL & (1 << 5))) //�ȴ�RTC�Ĵ����������
		{};
		while (!(RTC->CRL & (1 << 3))) //�ȴ�RTC�Ĵ���ͬ��
		{};
		RTC->CRH		|=	0x01;         //�������ж�
		while (!(RTC->CRL & (1 << 5))); //�ȴ�RTC�Ĵ����������
		RTC->CRL		|= 	1 << 4;              //��������

		RTC->PRLH		=		0x0000;
		RTC->PRLL		=		32767;          //ʱ����������(�д��۲�,���Ƿ�������?)����ֵ��32767

		RTC_Set(2012, 9, 7, 13, 16, 55);  //����ʱ��

		RTC->CRL		&=	~(1 << 4);           //���ø���
		while (!(RTC->CRL & (1 << 5)));   //�ȴ�RTC�Ĵ����������
		BKP->DR1		=		0x5050;

	}
	else //ϵͳ������ʱ
	{
		while (!(RTC->CRL & (1 << 3))); //�ȴ�RTC�Ĵ���ͬ��
		RTC->CRH	|=	0x01;         //�������ж�
		while (!(RTC->CRL & (1 << 5))); //�ȴ�RTC�Ĵ����������
	}
	//RTC_Get();//����ʱ��
	//return 0; //ok
}
/*----------------------------------------------------------------------------
 * @����: GPIO_Configuration
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	UnableJTAG();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	/* Configure Key Button GPIO Pin as input floating (Key Button EXTI Line) */
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	//GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* Configure Key Button GPIO Pin as input floating (Key Button EXTI Line) */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//USB insert detect PA.15
	//Key K_B POWER ON PA.6
	//KEY K_W          PA.5
	//KEY 4KEYS  			 PA.4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;                    //RST
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIOA->BRR = GPIO_Pin_7; //SYSTEM POWER OFF;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_9;                    //RST
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_9);  //SDI = 1;
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);      //OE = 0;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;                   //RST
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_11);  //WIFI RST

	//ant select
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIOB->BRR = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8; //GPIO_ResetBits(GPIOB, GPIO_Pin_1);      //OE = 0;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;                               //USB
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_11);  //WIFI RST
}
/*----------------------------------------------------------------------------
 * @����: NVIC_Configuration
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
void NVIC_Configuration(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
#if 0
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);
#else
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x3000);
#endif
	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;                   //���ô���1�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Connect Key Button EXTI Line to Key Button GPIO Pin *
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource12);
	Configure Key Button EXTI Line to generate an interrupt on falling edge */
	EXTI_InitStructure.EXTI_Line = EXTI_Line12;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	/* Enable the EXTI15_10 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Connect Key Button EXTI Line to Key Button GPIO Pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	/* Configure Key Button EXTI Line to generate an interrupt on falling edge */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);
	/* Enable the EXTI15_10 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the USB interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the TIM4 interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void TIM4_init(void)
{
/**
	TIM4->PSC = 71;
	TIM4->ARR = 1000 - 1;//΢��
	TIM4->CR1|=(1<<3);
	TIM4->CR1|= 0x01;
	TIM4->CNT = 0;
	//while((TIM4->SR & 0x01)==0);
	//TIM4->SR = 0;
**/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_DeInit(TIM4);//ʹ��ȱʡֵ��ʼ��TIM����Ĵ���
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;//�Զ���װ�ؼĴ���ֵΪ1  ARR  1000 * (1/1000000) = 1ms
	TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1);//ʱ��Ԥ��Ƶ��Ϊ36000  720000000/72 = 1000000
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//������Ƶ����1
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//����ģʽ
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM4,TIM_FLAG_Update);//������±�־λ
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);//ʹ���ж�
	TIM_Cmd(TIM4, ENABLE);
}
/*----------------------------------------------------------------------------
 * @����: BSP_Init
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
void BSP_Init(void)
{
	/* System Clocks Configuration --72M*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);
	RCC_Configuration();
	RTC_Configuration();
	GPIO_Configuration();
	/* NVIC configuration */
	/*Ƕ�������жϿ�����
	  ˵����USART1��ռ���ȼ�����0�����1λ�� ���������ȼ�����0�����7λ�� */
	NVIC_Configuration();
	//USART_Config(USART1,115200);		  //����1��ʼ��
	USART2_Config(115200);        //����2��ʼ��
	SPI_DEV_Init();
	ADC_Configuration();
	//TIM4_init();

}
/*----------------------------------------------------------------------------
 * @����: OS_CPU_SysTickInit
		  OS_TICKS_PER_SEC Ϊÿ���TICK������Ĭ��Ϊ1000����1msһ���ж�
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
void  OS_CPU_SysTickInit(void)
{
	RCC_ClocksTypeDef  rcc_clocks;
	INT32U         cnts;
	RCC_GetClocksFreq(&rcc_clocks);
	cnts = (INT32U)rcc_clocks.HCLK_Frequency / OS_TICKS_PER_SEC;
	SysTick_Config(cnts);
}
/*----------------------------------------------------------------------------
 * @����: Is_Leap_Year
		  �ж��Ƿ�Ϊ����
 * @����: year
 * @����: 1�����ꣻ0����������
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
unsigned char Is_Leap_Year(unsigned short year)
{
	if (year % 4 == 0) //�����ܱ�4����
	{
		if (year % 100 == 0)
		{
			if (year % 400 == 0) return 1; //�����00��β,��Ҫ�ܱ�400����
			else return 0;
		}
		else return 1;
	}
	else return 0;
}
/*----------------------------------------------------------------------------
 * @����: RTC_Get
		  �ж��Ƿ�Ϊ����
 * @����: ��
 * @����: 0����ɸ��£��������������
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
unsigned char RTC_Get(unsigned int timecount)
{
	static unsigned short daycnt = 0;
	//unsigned int timecount=0;
	unsigned int temp = 0;
	unsigned short temp1 = 0;

	//timecount   = RTC->CNTH;//�õ��������е�ֵ(������)
	//timecount <<= 16;
	//timecount  += RTC->CNTL;

	temp = timecount / 86400;   //�õ�����(��������Ӧ��)
	if (daycnt != temp) //����һ����
	{
		daycnt = temp;
		temp1 = 1970;   //��1970�꿪ʼ
		while (temp >= 365)
		{
			if (Is_Leap_Year(temp1)) //������
			{
				if (temp >= 366) temp -= 366; //�����������
				else
				{
					temp1++;
					break;
				}
			}
			else temp -= 365;      //ƽ��
			temp1++;
		}
		Time_year = temp1; //�õ����
		temp1 = 0;
		while (temp >= 28) //������һ����
		{
			if (Is_Leap_Year(Time_year) && temp1 == 1) //�����ǲ�������/2�·�
			{
				if (temp >= 29) temp -= 29; //�����������
				else break;
			}
			else
			{
				if (temp >= mon_table[temp1]) temp -= mon_table[temp1]; //ƽ��
				else break;
			}
			temp1++;
		}
		Time_month = temp1 + 1; //�õ��·�
		Time_date  = temp  + 1;     //�õ�����
	}
	temp      = timecount % 86400;          //�õ�������
	Time_hour = temp / 3600;        //Сʱ
	temp = temp % 3600;
	Time_min  = temp / 60;  //����
	Time_sec  = temp % 60;  //����

	return 0;
}
/*----------------------------------------------------------------------------
 * @����: RTC_Set
		  ����ʱ��
 * @����: syear��smon��sday��hour��min��sec
 * @����: 0����ɳɹ����������������
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
unsigned char RTC_Set(unsigned short syear, unsigned char smon, unsigned char sday, unsigned char hour, unsigned char min, unsigned char sec)
{
	unsigned short t;
	unsigned int seccount = 0;
	if (syear < 1970 || syear > 2099) return 1;
	for (t = 1970; t < syear; t++) //��������ݵ��������
	{
		if (Is_Leap_Year(t)) seccount += 31622400; //�����������
		else seccount += 31536000;              //ƽ���������
	}
	smon -= 1;
	for (t = 0; t < smon; t++)    //��ǰ���·ݵ����������
	{
		seccount += (u32)mon_table[t] * 86400; //�·����������
		if (Is_Leap_Year(syear) && t == 1) seccount += 86400; //����2�·�����һ���������
	}
	seccount += (u32)(sday - 1) * 86400; //��ǰ�����ڵ����������
	seccount += (u32)hour * 3600;           //Сʱ������
	seccount += (u32)min * 60;                //����������
	seccount += sec;                              //�������Ӽ���ȥ

	//����ʱ��
	RCC->APB1ENR |= 1 << 28; //ʹ�ܵ�Դʱ��
	RCC->APB1ENR |= 1 << 27; //ʹ�ܱ���ʱ��
	PWR->CR |= 1 << 8;      //ȡ��������д����
							//���������Ǳ����!
	RTC->CRL |= 1 << 4;     //��������
	RTC->CNTL = seccount & 0xffff;
	RTC->CNTH = seccount >> 16;
	RTC->CRL &= ~(1 << 4); //���ø���
	while (!(RTC->CRL & (1 << 5))); //�ȴ�RTC�Ĵ����������
									//RTC_Get();//������֮�����һ������
	return 0;
}
unsigned char RTC_Set_Unix(unsigned int unixtime)
{
	//����ʱ��
	RCC->APB1ENR |= 1 << 28; //ʹ�ܵ�Դʱ��
	RCC->APB1ENR |= 1 << 27; //ʹ�ܱ���ʱ��
	PWR->CR |= 1 << 8;      //ȡ��������д����
							//���������Ǳ����!
	RTC->CRL |= 1 << 4;     //��������
	RTC->CNTL = unixtime & 0xffff;
	RTC->CNTH = unixtime >> 16;
	RTC->CRL &= ~(1 << 4); //���ø���
	while (!(RTC->CRL & (1 << 5))); //�ȴ�RTC�Ĵ����������
									//RTC_Get();//������֮�����һ������
	return 0;
}
/*----------------------------------------------------------------------------
 * @����: dispaly_process
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
void dispaly_process(void) //��7us
{
	unsigned char i, j;
	unsigned char data[3];
	unsigned char wdata;
	uint16_t gpio_input_temp;
	GPIOB->BRR = GPIO_Pin_2;
	GPIOB->BRR = GPIO_Pin_1;
	//GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	gpio_input_temp = GPIOA->IDR;
	data[0] = display_bit_a[led_dis_cnt]; //led_dis_cnt
	data[1] = display_bit_b[led_dis_cnt];
	data[2] = display_dat_buf[led_dis_cnt];
	//data[2] = display_dat_buf[0];
	if (led_dis_cnt < 4)
	{
		if (white_led == LED_ON)
		{
			data[0] &= ~0x01;
		}
		else if (white_led == LED_SLOW_SHINE)
		{
			if (led_flash_cnt < 30) data[0] &= ~0x01;
		}
		else if (white_led == LED_FAST_SHINE)
		{
			if ((led_flash_cnt % 10) < 5) data[0] &= ~0x01;
		}
	}
	else if(led_dis_cnt < 8)
	{
		if (black_led == LED_ON)
		{
			data[0] &= ~0x02;
		}
		else if (black_led == LED_SLOW_SHINE)
		{
			if (led_flash_cnt < 30) data[0] &= ~0x02;
		}
		else if (black_led == LED_FAST_SHINE)
		{
			if ((led_flash_cnt % 10) < 5) data[0] &= ~0x02;
		}
	}
	else
	{
		if (state_led == LED_ON)
		{
			data[0] &= ~0x04;
		}
		else if (state_led == LED_SLOW_SHINE)
		{
			if (led_flash_cnt < 30) data[0] &= ~0x04;
		}
		else if (state_led == LED_FAST_SHINE)
		{
			if ((led_flash_cnt % 10) < 5) data[0] &= ~0x04;
		}
	}

	if ((led_flash_flag & (1 << led_dis_cnt)))
	{
		if (led_flash_cnt < 30) data[2] = 0x00;   //flash freq 0.12 *60 = 0.072s

	}
	//get key value
	switch (led_dis_cnt)
	{
		case 1:
		#if NEW_VERSION
		if ((gpio_input_temp & GPIO_Pin_4) == GPIO_Pin_4)
		#else
		if ((gpio_input_temp & GPIO_Pin_4) == 0)
		#endif
		{
			if (key_horse_cnt < KEY_LONG_CNT)
			{
				key_horse_cnt++;
				if (key_horse_cnt == KEY_LONG_CNT)
				{
					SetMsg(KEY_LONG_HORSE);
					KeyChessRising(KEY_LONG_HORSE);
				}
			}
			else if (key_horse_cnt < KEY_LONGLONG_CNT)
			{
				key_horse_cnt++;
				if (key_horse_cnt == KEY_LONGLONG_CNT)
				{
					SetMsg(KEY_LONGLONG_HORSE);
				}
			}
		}
		else if (key_horse_cnt != 0)
		{
			ClearMsg(KEY_LONGLONG_HORSE);
			if (key_horse_cnt > KEY_SHORT_CNT && key_horse_cnt < KEY_LONG_CNT)
			{
				SetMsg(KEY_SHORT_HORSE);
			}
			key_horse_cnt = 0;
		}
		break;
	case 2:
		#if NEW_VERSION
		if ((gpio_input_temp & GPIO_Pin_4) == GPIO_Pin_4)
		#else
		if ((gpio_input_temp & GPIO_Pin_4) == 0)
		#endif
		{
			if (key_car_cnt < KEY_LONG_CNT)
			{
				key_car_cnt++;
				if (key_car_cnt == KEY_LONG_CNT)
				{
					SetMsg(KEY_LONG_CAR);
					KeyChessRising(KEY_LONG_CAR);
				}
			}
			else if (key_car_cnt < KEY_LONGLONG_CNT)
			{
				key_car_cnt++;
				if (key_car_cnt == KEY_LONGLONG_CNT)
				{
					SetMsg(KEY_LONGLONG_CAR);
				}
			}
		}
		else if (key_car_cnt != 0)
		{
			ClearMsg(KEY_LONGLONG_CAR);
			if (key_car_cnt > KEY_SHORT_CNT && key_car_cnt < KEY_LONG_CNT)
			{
				SetMsg(KEY_SHORT_CAR);
			}
			key_car_cnt = 0;
		}
		break;
	case 5:
		#if NEW_VERSION
		if ((gpio_input_temp & GPIO_Pin_4) == GPIO_Pin_4)
		#else
		if ((gpio_input_temp & GPIO_Pin_4) == 0)
		#endif
		{
			if (key_queen_cnt < KEY_LONG_CNT)
			{
				key_queen_cnt++;
				if (key_queen_cnt == KEY_LONG_CNT)
				{
					SetMsg(KEY_LONG_QUEEN);
					KeyChessRising(KEY_LONG_QUEEN);
				}
			}
			else if (key_queen_cnt < KEY_LONGLONG_CNT)
			{
				key_queen_cnt++;
				if (key_queen_cnt == KEY_LONGLONG_CNT)
				{
					SetMsg(KEY_LONGLONG_QUEEN);
				}
			}
		}
		else if (key_queen_cnt != 0)
		{
			ClearMsg(KEY_LONGLONG_QUEEN);
			if (key_queen_cnt > KEY_SHORT_CNT && key_queen_cnt < KEY_LONG_CNT)
			{
				SetMsg(KEY_SHORT_QUEEN);
			}
			key_queen_cnt = 0;
		}
		break;
	case 6:
		#if NEW_VERSION
		if ((gpio_input_temp & GPIO_Pin_4) == GPIO_Pin_4)
		#else
		if ((gpio_input_temp & GPIO_Pin_4) == 0)
		#endif
		{
			if (key_phase_cnt < KEY_LONG_CNT)
			{
				key_phase_cnt++;
				if (key_phase_cnt == KEY_LONG_CNT)
				{
					SetMsg(KEY_LONG_PHASE);
					KeyChessRising(KEY_LONG_PHASE);
				}
			}
			else if (key_phase_cnt < KEY_LONGLONG_CNT)
			{
				key_phase_cnt++;
				if (key_phase_cnt == KEY_LONGLONG_CNT)
				{
					SetMsg(KEY_LONGLONG_PHASE);
				}
			}
		}
		else if (key_phase_cnt != 0)
		{
			ClearMsg(KEY_LONGLONG_PHASE);
			if (key_phase_cnt > KEY_SHORT_CNT && key_phase_cnt < KEY_LONG_CNT)
			{
				SetMsg(KEY_SHORT_PHASE);
			}
			key_phase_cnt = 0;
		}
		break;
	case 7:
		if ((gpio_input_temp & GPIO_Pin_5) == 0)
		{
			if (key_black_cnt < KEY_LONG_CNT)
			{
				key_black_cnt++;
				if (key_black_cnt == KEY_LONG_CNT)
				{
					SetMsg(KEY_LONG_BLACK);
				}
			}
		}
		else if (key_black_cnt != 0)
		{
			if (key_black_cnt > KEY_SHORT_CNT && key_black_cnt < KEY_LONG_CNT)
			{
				SetMsg(KEY_SHORT_BLACK);
			}
			key_black_cnt = 0;
		}
		break;
	case 8:
		if ((gpio_input_temp & GPIO_Pin_6) == 0)
		{
			if (key_white_cnt < KEY_LONGLONG_CNT)
			{
				key_white_cnt++;
				if (key_white_cnt == KEY_LONGLONG_CNT)
				{
					SetMsg(KEY_LONGLONG_WHITE);
				}
			}
		}
		else if (key_white_cnt != 0)
		{
			if (key_white_cnt > KEY_SHORT_CNT && key_white_cnt < KEY_LONG_CNT)
			{
				SetMsg(KEY_SHORT_WHITE);
			}
			else if (key_white_cnt >= KEY_LONG_CNT && key_white_cnt < KEY_LONGLONG_CNT)
			{
				SetMsg(KEY_LONG_WHITE);
			}
			key_white_cnt = 0;
		}

		if ((gpio_input_temp & GPIO_Pin_15) == 0)
		{
			usb_in_flag++;
			SetMsg(MSG_USB_ON);
			if (usb_in_flag > KEY_LONG_CNT)
			{
			}
		}
		else if (usb_in_flag != 0)
		{
			ClearMsg(MSG_USB_ON);
			usb_in_flag = 0;
		}
		break;
	}

	for (i = 0; i < 3; i++)
	{
		wdata = data[i];
		for (j = 0; j < 8; j++)
		{
			if (wdata & 0x80) //MSB First
				GPIOB->BSRR = GPIO_Pin_0;
			//GPIO_SetBits(GPIOB, GPIO_Pin_0);  //SDI = 1;
			else GPIOB->BRR = GPIO_Pin_0;
			//GPIO_ResetBits(GPIOB, GPIO_Pin_0);//SDI = 0;
			GPIOB->BSRR = GPIO_Pin_3;
			//GPIO_SetBits(GPIOB, GPIO_Pin_3);		//SCK = 1;
			wdata <<= 1;
			GPIOB->BRR = GPIO_Pin_3;
			//GPIO_ResetBits(GPIOB, GPIO_Pin_3);  //SCK = 0;
		}
	}

	GPIOB->BSRR = GPIO_Pin_2;
	led_dis_cnt++;
	if (led_dis_cnt == 13)
	{
		led_dis_cnt = 0;
		led_flash_cnt++;
		if (led_flash_cnt > 60) led_flash_cnt = 0;
	}

}
/*----------------------------------------------------------------------------
 * @����: dispaly_process
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------
 1.0       2015.11.15     zxjun          ����ģ��
----------------------------------------------------------------------------*/
void sys_power_cmd(unsigned char onoff)
{
	if (onoff)
	{
		GPIOA->BSRR = GPIO_Pin_7; //on
	}
	else
	{
		GPIOA->BRR  = GPIO_Pin_7; //off
	}
}


#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8 *file, u32 line)
{
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{}
}
#endif

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

