#include "..\App\includes.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"

#include "spi.h"

#include "./key/key.h"
#include "./led/led.h"
#include "./motor/motor.h"
#include "./IIC/IOI2C.h"
#include "./MPU6050/mpu6050.h"
#include "./adc/adc.h"
#include "./ws2812B/ws2812B.h"
#include "./usart/usart.h"
#include "./rtc/rtc.h"
#include "./ENCODER/encoder.h"

ErrorStatus HSEStartUpStatus;
/*
*********************************************************************************************************

*/

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{		
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();	
	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);	
	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();	
	if(HSEStartUpStatus == SUCCESS)
	{
		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 
		
		/* PCLK2 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1); 
		
		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);
		
		/* ADCCLK = PCLK2/4 */
		RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
		
		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		
		/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		
		/* Enable PLL */ 
		RCC_PLLCmd(ENABLE);
		
		/* Wait till PLL is ready */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		
		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		
		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}
	/* Enable peripheral clocks --------------------------------------------------*/
	/* Enable DMA clock */
  	//SystemInit(); 

}
//�������·���ʵ��ִ�л��ָ��WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//�ر������ж�
__asm void INTX_DISABLE(void)
{
	CPSID I;		  
}
//���������ж�
__asm void INTX_ENABLE(void)
{
	CPSIE I;		  
}
//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

//�������ģʽ	  
void Sys_Standby(void)
{
	SCB->SCR|=1<<2;//ʹ��SLEEPDEEPλ (SYS->CTRL)	   
	RCC->APB1ENR|=1<<28;     //ʹ�ܵ�Դʱ��	    
 	PWR->CSR|=1<<8;          //����WKUP���ڻ���
	PWR->CR|=1<<2;           //���Wake-up ��־
	PWR->CR|=1<<1;           //PDDS��λ		  
	WFI_SET();				 //ִ��WFIָ��		 
}

//ϵͳ��λ   
void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 	

void UnableJTAG(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                     // enable clock for Alternate Function
	AFIO->MAPR &= ~(7UL << 24); // clear used bit
	AFIO->MAPR |=  (2UL << 24); // set used bits
}


void Motor_Poweron(unsigned char on)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	if(on)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_3);//PCout(8) = 1;
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_3);//PCout(8) = 0;
	}
}
/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : PB5: LED1 (mini and V3)
                   PD6��LED2 (only V3)
				   PD3��LED3 (only V3)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Configuration(void)
{
	//GPIO_InitTypeDef GPIO_InitStructure;
	//EXTI_InitTypeDef EXTI_InitStructure;
	/*�Կ���LEDָʾ�Ƶ�IO�ڽ����˳�ʼ�������˿�����Ϊ������������������ٶ�Ϊ50Mhz��PA9,PA10�˿ڸ���Ϊ����1��TX��RX��
	������ĳ������ʱ������Ӧ�������ڵĶ˿ڵ�ʱ�ӽ���ʹ�ܡ������޷����óɹ��������õ��˶˿�B��D,E�� ���Ҫ���⼸���˿ڵ�ʱ��
	����ʹ�ܣ�ͬʱ�����õ�����IO�ڹ����������ô��ڡ���˻�Ҫʹ��AFIO�����ù���IO��ʱ�ӡ�*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE); 
}



/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
NVIC_PriorityGroup_1:
��0�飺����4λ����ָ����Ӧ���ȼ�
��1�飺���1λ����ָ����ռʽ���ȼ������3λ����ָ����Ӧ���ȼ�
��2�飺���2λ����ָ����ռʽ���ȼ������2λ����ָ����Ӧ���ȼ�
��3�飺���3λ����ָ����ռʽ���ȼ������1λ����ָ����Ӧ���ȼ�
��4�飺����4λ����ָ����ռʽ���ȼ�
*******************************************************************************/
void NVIC_Configuration(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Configure one bit for preemption priority */
	#if defined (VECT_TAB_RAM)
		/* Set the Vector Table base location at 0x20000000 */ 
		NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
	#elif defined(VECT_TAB_FLASH_IAP)
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2000);
	#else  /* VECT_TAB_FLASH  */
		/* Set the Vector Table base location at 0x08000000 */ 
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
	#endif 	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			     	//���ô���1�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Connect Key Button EXTI Line to Key Button GPIO Pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource12);	
	/* Configure Key Button EXTI Line to generate an interrupt on falling edge */  
	EXTI_InitStructure.EXTI_Line = EXTI_Line12;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);	
	/* Enable the EXTI15_10 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Connect Key Button EXTI Line to Key Button GPIO Pin */
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);          // Enable the DMA Interrupt 
}

void BSP_Init(void)
{
	/* System Clocks Configuration --72M*/
	RCC_Configuration(); 
	UnableJTAG();	
	GPIO_Configuration();	
	RTC_Configuration();
	//Sys_Poweron(1);
	Motor_Poweron(1);
	LED_Init();                     //=====��ʼ���� LED ���ӵ�Ӳ���ӿ�
	KEY_Init();                     //=====������ʼ��
	MiniBalance_PWM_Init(3600-1, 0);//=====��ʼ��PWM 10KHZ������������� �����ʼ������ӿ� 
	MiniBalance_Motor_Init();
	/* NVIC configuration */
	/*Ƕ�������жϿ����� 
	  ˵����USART1��ռ���ȼ�����0�����1λ�� ���������ȼ�����0�����7λ�� */ 
	NVIC_Configuration();		
	uart1_init(115200);
	
	Encoder_Init_TIM2();
	Encoder_Init_TIM4();
	//SPI_DEV_Init();
	//ADC1_Configuration();
	ws2812Init();
	IIC_Init();                     //=====IIC��ʼ��
	ICM20602_initialize();           //=====MPU6050��ʼ��	
	//MiniBalance_EXTI_Init();        //=====MPU6050 5ms��ʱ�жϳ�ʼ��

}



////SYSTICK ���ã� 10msһ��systick�жϣ�����ucos ���Ƚ��ģ� 1KHz
void  OS_CPU_SysTickInit(void)
{
	RCC_ClocksTypeDef  rcc_clocks;
	INT32U         cnts;
	RCC_GetClocksFreq(&rcc_clocks);
	cnts = (INT32U)rcc_clocks.HCLK_Frequency/OS_TICKS_PER_SEC;
	SysTick_Config(cnts);  
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
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

