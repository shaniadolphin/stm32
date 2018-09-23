/*

                         \\\|///
                       \\  - -  //
                        (  @ @  )
+---------------------oOOo-(_)-oOOo-------------------------+
|                 �ܶ���STM32�������������                 |
|                      DFU FirmWare                         |
|                          Sun68                            |
|                        2009.12.8                          |
|                 �ù̼�֧��USB���߱��                     |
|                �ܶ�STM32Ƕ��ʽ����������                  |
|                  http://OutSTM.5d6d.com 				    |             
|				   QQ: 9191274	 							|
|                              Oooo                         |
+-----------------------oooO--(   )-------------------------+
                       (   )   ) /
                        \ (   (_/
                         \_)     


*/
/* Includes ------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "dfu_mal.h"
#include "hw_config.h"
#include "platform_config.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"

/* Private typedef -----------------------------------------------------------*/
typedef  void (*pFunction)(void);
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
typedef enum
{
	LED_OFF = 0x00,
	LED_ON = 0xFF,
	LED_SLOW_SHINE = 0x33,
	LED_FAST_SHINE = 0x55,
	LED_SLOW_FAST_SHINE = 0x53,
}LED_STATUS;

uint8_t DeviceState;
uint8_t DeviceStatus[6];
pFunction Jump_To_Application;
uint32_t JumpAddress;

unsigned char delayNMicroSeconds(unsigned short us);
/* Private functions ---------------------------------------------------------*/
void putcc(u8 ch)
{      
	while((USART1->SR & 0x40) == 0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
}

void UnableJTAG(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                     // enable clock for Alternate Function
	AFIO->MAPR &= ~(7UL << 24); // clear used bit
	AFIO->MAPR |=  (2UL << 24); // set used bit
}
/*******************************************************************************
* Function Name  : main.
* Description    : main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int main(void)
{
	unsigned char i;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	UnableJTAG();
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);

	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);
	
	
	//PWR_BackupAccessCmd(ENABLE);//�����޸�RTC �ͺ󱸼Ĵ���
	//RCC_LSICmd(DISABLE);//�ر��ⲿ�����ⲿʱ���źŹ��� ��PC14 PC15 �ſ��Ե���ͨIO�á� 
	//BKP_TamperPinCmd(DISABLE);//�ر����ּ�⹦�ܣ�Ҳ���� PC13��Ҳ���Ե���ͨIO ʹ��	
	//for(i=0;i<10;i++)
	//{
	//	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	//	delayNMicroSeconds(2000);	
	//	GPIO_SetBits(GPIOC, GPIO_Pin_13);
	//	delayNMicroSeconds(2000);	
	//}
	/* Check if the Key push-button on STM3210x-EVAL Board is pressed */
	if(((GPIOB->IDR & GPIO_Pin_12) == 0)) //&& ((GPIOC->IDR & GPIO_Pin_2) == 0))
	{ /* Test if user code is programmed starting from address 0x8003000 */
		/* Enter DFU mode */
		DeviceState = STATE_dfuERROR;
		DeviceStatus[0] = STATUS_ERRFIRMWARE;
		DeviceStatus[4] = DeviceState;
		Set_System();
		Set_USBClock();
		USB_Init();
		//ta6932_changestate(1, 0, 0, 5);		
		while (1)
		{
			switch(DeviceState)
			{
				case STATE_appIDLE:					
				break;
				case STATE_appDETACH:				
				break;	
				case STATE_dfuIDLE:					
				break;	
				case STATE_dfuDNLOAD_SYNC:					
				break;	
				case STATE_dfuDNBUSY:		
				break;
				case STATE_dfuDNLOAD_IDLE:				
				break;	
				case STATE_dfuMANIFEST_SYNC:				
				break;	
				case STATE_dfuMANIFEST:				
				break;	
				case STATE_dfuMANIFEST_WAIT_RESET:				
				break;	
				case STATE_dfuUPLOAD_IDLE:				
				break;	
				case STATE_dfuERROR:				
				break;
				default:
				break;
			}				
			//display_dat_buf[6] = segment_data[DeviceState%16];
			//display_dat_buf[12] = segment_data[bDeviceState%16];
			
			delayNMicroSeconds(2000);	
			//dispaly_process();			
			//if(bDeviceState == SUSPENDED)break;
		}
	} /* Otherwise enters DFU mode to allow user to program his application */	
	//if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
	{ /* Jump to user application */

		JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);

		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) ApplicationAddress);
		Jump_To_Application = (pFunction) JumpAddress;
		Jump_To_Application();
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
 2.0       2016.08.22     zxjun          ������ʾ��ʽ������TA6932��ʾ����
                                         �������ж��Ƴ������Ӻ������
 3.0	   2016.08.22     zxjun			 ����LED����˸��ʽ����LED����˸�ֽ��8��:
										 0 1 0 1 0 1 0 1
										 0x00��ʾϨ��
										 0x55��ʾ����
										 0x33��ʾ����
										 0x53��ʾ����+����
----------------------------------------------------------------------------*/

unsigned char delayNMicroSeconds(unsigned short us)
{
	TIM3->PSC = 71;
	TIM3->ARR = us;
	TIM3->CR1|=(1<<3);
	TIM3->CNT = 0;
	TIM3->CR1|= 0x01;
	while((TIM3->SR & 0x01)==0)
	{
	}
	TIM3->SR = 0;
	return 0;
}


#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
