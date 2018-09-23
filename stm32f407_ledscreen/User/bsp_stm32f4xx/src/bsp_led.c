#include "bsp.h"
#include  <includes.h>
/*
	�ó��������ڰ�����STM32-X4��STM32-F4������

	�����������Ӳ�������޸�GPIO����� IsKeyDown1 - IsKeyDown8 ����

	����û���LEDָʾ�Ƹ���С��4�������Խ������LEDȫ������Ϊ�͵�1��LEDһ��������Ӱ�������
*/

#ifdef STM32_X3		/* ������ STM32-X4 ������ */
	/*
		������STM32-X4 LED���߷��䣺
			LED1       : PE2 (�͵�ƽ�������ߵ�ƽϨ��)
			LED2       : PE3 (�͵�ƽ�������ߵ�ƽϨ��)
			LED3       : PE4 (�͵�ƽ�������ߵ�ƽϨ��)
			LED4       : PE5 (�͵�ƽ�������ߵ�ƽϨ��)
	*/
	#define RCC_ALL_LED 	RCC_AHB1Periph_GPIOE	/* �����ڶ�Ӧ��RCCʱ�� */

	#define GPIO_PORT_LED1  GPIOE
	#define GPIO_PIN_LED1	GPIO_Pin_2

	#define GPIO_PORT_LED2  GPIOE
	#define GPIO_PIN_LED2	GPIO_Pin_3

	#define GPIO_PORT_LED3  GPIOE
	#define GPIO_PIN_LED3	GPIO_Pin_4

	#define GPIO_PORT_LED4  GPIOE
	#define GPIO_PIN_LED4	GPIO_Pin_5

#else	/* STM32_F4 */
	/*
		������STM32-V5 ������LED���߷��䣺
			LD1     : PI10/TP_NCS          (�͵�ƽ�������ߵ�ƽϨ��)
			LD2     : PF7/NRF24L01_CSN     (�͵�ƽ�������ߵ�ƽϨ��)
			LD3     : PF8/SF_CS            (�͵�ƽ�������ߵ�ƽϨ��)
			LD4     : PC2/NRF905_CSN/VS1053_XCS  (�͵�ƽ�������ߵ�ƽϨ��)
	*/

	/* �����ڶ�Ӧ��RCCʱ�� */
	#define RCC_ALL_LED 	(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE )


	
#if 0
	#define GPIO_PORT_LED1  	GPIOB
	#define GPIO_PIN_LED1		GPIO_Pin_6
	#define GPIO_PORT_LED2  	GPIOB
	#define GPIO_PIN_LED2		GPIO_Pin_7
	#define GPIO_PORT_LED3  	GPIOB
	#define GPIO_PIN_LED3		GPIO_Pin_8
	#define GPIO_PORT_LED4  	GPIOB
	#define GPIO_PIN_LED4		GPIO_Pin_2
	#define JXI5020_PORT   		GPIOC
	#define D7258_PORT 			GPIOA
	#define D7258_A				GPIO_Pin_5
	#define D7258_B				GPIO_Pin_6
	#define D7258_C				GPIO_Pin_7
	#define D7258_EN			GPIO_Pin_4
	#define UR13_SDO			GPIO_Pin_0
	#define UG13_SDO			GPIO_Pin_1
	#define UB13_SDO			GPIO_Pin_2
	#define UR14_SDO			GPIO_Pin_3
	#define UG14_SDO			GPIO_Pin_4
	#define UB14_SDO			GPIO_Pin_5
	#define JXI5020_SCK			GPIO_Pin_6
	#define JXI5020_LE			GPIO_Pin_7
	#define JXI5020_OE			GPIO_Pin_8
	#define JXI5020_GCLK		GPIO_Pin_9
	#define JXI5020_PORT_BSRR 	(* (volatile unsigned long *)(GPIOC_BASE + 0x18))
	#define D7258_PORT_BSRR 	(* (volatile unsigned long *)(GPIOA_BASE + 0x18))	
#else
	#define GPIO_PORT_LED1  	GPIOB
	#define GPIO_PIN_LED1		GPIO_Pin_4
	#define GPIO_PORT_LED2  	GPIOA
	#define GPIO_PIN_LED2		GPIO_Pin_15
	#define GPIO_PORT_LED3  	GPIOB
	#define GPIO_PIN_LED3		GPIO_Pin_5

	#define JXI5020_DATA_PORT   GPIOA
	#define D7258_PORT 			GPIOB
	#define D7258_A				GPIO_Pin_0
	#define D7258_B				GPIO_Pin_1
	#define D7258_C				GPIO_Pin_10
	#define D7258_EN			GPIO_Pin_11
	#define UR13_SDO			GPIO_Pin_0
	#define UG13_SDO			GPIO_Pin_1
	#define UB13_SDO			GPIO_Pin_2
	#define UR14_SDO			GPIO_Pin_3
	#define UG14_SDO			GPIO_Pin_4
	#define UB14_SDO			GPIO_Pin_5
	#define JXI5020_SCK_PORT   	GPIOC
	#define JXI5020_SCK			GPIO_Pin_4
	#define JXI5020_LE_PORT   	GPIOB	
	#define JXI5020_LE			GPIO_Pin_12
	
	//#define JXI5020_OE			GPIO_Pin_8
	#define JXI5020_GCLK_PORT   GPIOC	
	#define JXI5020_GCLK		GPIO_Pin_5
	//#define JXI5020_PORT_BSRR 	(* (volatile unsigned long *)(GPIOA_BASE + 0x18))
	#define D7258_PORT_BSRR 	(* (volatile unsigned long *)(GPIOB_BASE + 0x18))	
#endif	
#endif
/*****
MBI5052
U13_R---1	 2---U13_G
U13_B---3	 4---GND  
U14_R---5	 6---U14_G  
U14_B---7	 8---GND  
A    ---9	10---B  
C    ---11	12---EN_L/EN_H 
DCLK ---13	14---LE
GCLK ---15	16---GND 

Total: 64(MBI5052x4)x32(D7529x4) ���°���ͬʱˢ
�ϰ���
N 1:		1----------------64
N 2:		1----------------64	
.
.
N16:		1----------------64
�°���	
N 1:		1----------------64		
N 2:		1----------------64	
.
.
N16:		1----------------64	
****/
//======================================================
// 128X64OLEDҺ���ײ�����[8X16]�����
// �����: Guokaiyi
// ��  ��: [8X16]�����ַ�����ģ���� (����ȡģ���λ)
// !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
//======================================================
const unsigned char F8X16[]=
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // -Space-  32
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x18,0x3C,0x3C,0x3C,0x18,0x18,  // -!- 33
	0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00,

	0x00,0x66,0x66,0x66,0x24,0x00,0x00,0x00,  // -"-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x6C,0x6C,0xFE,0x6C,0x6C,  // -#-
	0x6C,0xFE,0x6C,0x6C,0x00,0x00,0x00,0x00,

	0x18,0x18,0x7C,0xC6,0xC2,0xC0,0x7C,0x06,  // -$-
	0x86,0xC6,0x7C,0x18,0x18,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0xC2,0xC6,0x0C,0x18,  // -%-
	0x30,0x60,0xC6,0x86,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x6C,0x6C,0x38,0x76,0xDC,  // -&-
	0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,

	0x00,0x30,0x30,0x30,0x60,0x00,0x00,0x00,  // -'-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x0C,0x18,0x30,0x30,0x30,0x30,  // -(-
	0x30,0x30,0x18,0x0C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x30,0x18,0x0C,0x0C,0x0C,0x0C,  // -)-
	0x0C,0x0C,0x18,0x30,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x66,0x3C,0xFF,  // -*-
	0x3C,0x66,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x7E,  // -+-
	0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // -,-
	0x00,0x18,0x18,0x18,0x30,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,  // ---
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // -.-
	0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x02,0x06,0x0C,0x18,  // -/-
	0x30,0x60,0xC0,0x80,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0xCE,0xD6,0xD6,  // -0-
	0xE6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,  // -1-
	0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0x06,0x0C,0x18,0x30,  // -2-
	0x60,0xC0,0xC6,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0x06,0x06,0x3C,0x06,  // -3-
	0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,  // -4-
	0x0C,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0xC0,0xC0,0xC0,0xFC,0x0E,  // -5-
	0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x60,0xC0,0xC0,0xFC,0xC6,  // -6-
	0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0xC6,0x06,0x06,0x0C,0x18,  // -7-
	0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0xC6,  // -8-
	0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7E,0x06,  // -9-
	0x06,0x06,0x0C,0x78,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,  // -:-
	0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,  // -;-
	0x00,0x18,0x18,0x30,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x06,0x0C,0x18,0x30,0x60,  // -<-
	0x30,0x18,0x0C,0x06,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,  // -=-
	0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x60,0x30,0x18,0x0C,0x06,  // ->-
	0x0C,0x18,0x30,0x60,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0x0C,0x18,0x18,  // -?-
	0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x7C,0xC6,0xC6,0xDE,0xDE,  // -@-
	0xDE,0xDC,0xC0,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,  // -A-
	0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x66,  // -B-
	0x66,0x66,0x66,0xFC,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xC0,  // -C-
	0xC0,0xC2,0x66,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xF8,0x6C,0x66,0x66,0x66,0x66,  // -D-
	0x66,0x66,0x6C,0xF8,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,  // -E-
	0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,  // -F-
	0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xDE,  // -G-
	0xC6,0xC6,0x66,0x3A,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xFE,0xC6,  // -H-
	0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,  // -I-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,  // -J-
	0xCC,0xCC,0xCC,0x78,0x00,0x00,0x00,0x00,

	0x00,0x00,0xE6,0x66,0x6C,0x6C,0x78,0x78,  // -K-
	0x6C,0x66,0x66,0xE6,0x00,0x00,0x00,0x00,

	0x00,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,  // -L-
	0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,  // -M-
	0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,  // -N-
	0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x6C,0xC6,0xC6,0xC6,0xC6,  // -O-
	0xC6,0xC6,0x6C,0x38,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x60,  // -P-
	0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,  // -Q-
	0xC6,0xD6,0xDE,0x7C,0x0C,0x0E,0x00,0x00,

	0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x6C,  // -R-
	0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0x60,0x38,0x0C,  // -S-
	0x06,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7E,0x7E,0x5A,0x18,0x18,0x18,  // -T-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,  // -U-
	0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,  // -V-
	0xC6,0x6C,0x38,0x10,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xD6,  // -W-
	0xD6,0xFE,0x6C,0x6C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0x6C,0x6C,0x38,0x38,  // -X-
	0x6C,0x6C,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x66,0x66,0x66,0x66,0x3C,0x18,  // -Y-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0xC6,0x86,0x0C,0x18,0x30,  // -Z-
	0x60,0xC2,0xC6,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,  // -[-
	0x30,0x30,0x30,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x80,0xC0,0xE0,0x70,0x38,  // -\-
	0x1C,0x0E,0x06,0x02,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,  // -]-
	0x0C,0x0C,0x0C,0x3C,0x00,0x00,0x00,0x00,

	0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,  // -^-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // -_-
	0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,

	0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,  // -`-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x78,0x0C,0x7C,  // -a-
	0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,

	0x00,0x00,0xE0,0x60,0x60,0x78,0x6C,0x66,  // -b-
	0x66,0x66,0x66,0xDC,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC0,  // -c-
	0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x1C,0x0C,0x0C,0x3C,0x6C,0xCC,  // -d-
	0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xFE,  // -e-
	0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x6C,0x64,0x60,0xF0,0x60,  // -f-
	0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,  // -g-
	0xCC,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,0x00,

	0x00,0x00,0xE0,0x60,0x60,0x6C,0x76,0x66,  // -h-
	0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x18,0x18,0x00,0x38,0x18,0x18,  // -i-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x06,0x06,0x00,0x0E,0x06,0x06,  // -j-
	0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00,

	0x00,0x00,0xE0,0x60,0x60,0x66,0x6C,0x78,  // -k-
	0x78,0x6C,0x66,0xE6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,  // -l-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xEC,0xFE,0xD6,  // -m-
	0xD6,0xD6,0xD6,0xD6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,  // -n-
	0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC6,  // -o-
	0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,  // -p-
	0x66,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00,

	0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,  // -q-
	0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x1E,0x00,

	0x00,0x00,0x00,0x00,0x00,0xDC,0x76,0x62,  // -r-
	0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0x60,  // -s-
	0x38,0x0C,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x10,0x30,0x30,0xFC,0x30,0x30,  // -t-
	0x30,0x30,0x36,0x1C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0xCC,  // -u-
	0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,  // -v-
	0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,  // -w-
	0xD6,0xD6,0xFE,0x6C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xC6,0x6C,0x38,  // -x-
	0x38,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,  // -y-
	0xC6,0xC6,0xC6,0x7E,0x06,0x0C,0xF8,0x00,

	0x00,0x00,0x00,0x00,0x00,0xFE,0xCC,0x18,  // -z-
	0x30,0x60,0xC6,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0x0E,0x18,0x18,0x18,0x70,0x18,  // -{-
	0x18,0x18,0x18,0x0E,0x00,0x00,0x00,0x00,

	0x00,0x00,0x18,0x18,0x18,0x18,0x00,0x18,  // -|-
	0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,

	0x00,0x00,0x70,0x18,0x18,0x18,0x0E,0x18,  // -}-
	0x18,0x18,0x18,0x70,0x00,0x00,0x00,0x00,

	0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00,  // -~-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x10,0x38,0x6C,0xC6,  // --
	0xC6,0xC6,0xFE,0x00,0x00,0x00,0x00,0x00,
};

// ------------------  ������ģ�����ݽṹ���� ------------------------ //
struct  typFNT_GB16                 // ������ģ���ݽṹ 
{
       unsigned char  Index[3];               // ������������	
       unsigned char   Msk[32];                        // ���������� 
};

/////////////////////////////////////////////////////////////////////////
// ������ģ��                                                          //
// ���ֿ�: ����16.dot,����ȡģ���λ,��������:�����Ҵ��ϵ���         //
/////////////////////////////////////////////////////////////////////////
 const struct  typFNT_GB16 GB1616[] =          // ���ݱ� 
{
"֧", 0x01,0x00,0x01,0x00,0x01,0x04,0xFF,0xFE,
      0x01,0x00,0x01,0x00,0x3F,0xF0,0x08,0x20,
      0x08,0x20,0x04,0x40,0x02,0x80,0x01,0x00,
      0x02,0x80,0x0C,0x60,0x30,0x1E,0xC0,0x04,

"��", 0x10,0x40,0x10,0x40,0x10,0x50,0x13,0xF8,
      0xFC,0x40,0x10,0x48,0x17,0xFC,0x18,0x10,
      0x30,0x14,0xD7,0xFE,0x12,0x10,0x11,0x10,
      0x11,0x10,0x10,0x10,0x50,0x50,0x20,0x20,

"��", 0x11,0x10,0x11,0x10,0x17,0xFC,0x11,0x10,
      0xFD,0x14,0x17,0xFE,0x30,0x48,0x3B,0xFC,
      0x56,0x48,0x53,0xF8,0x92,0x48,0x13,0xF8,
      0x10,0x00,0x11,0x10,0x13,0x0C,0x14,0x04,

"��", 0x04,0x00,0x25,0xFC,0x25,0x04,0x24,0x88,
      0x24,0x50,0x24,0x20,0x24,0x50,0x24,0x8E,
      0x07,0x04,0x01,0x10,0x3F,0xF8,0x08,0x20,
      0x08,0x20,0x04,0x44,0xFF,0xFE,0x00,0x00,

"��", 0x00,0x08,0x3F,0xFC,0x20,0x08,0x20,0x08,
      0x3F,0xF8,0x22,0x20,0x21,0x48,0x2F,0xFC,
      0x22,0x20,0x22,0x24,0x3F,0xFE,0x22,0x20,
      0x22,0x20,0x42,0x20,0x84,0x20,0x08,0x20,

"��", 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,       
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

"*",  0x00,0x00,0x00,0x00,0x00,0x80,0x03,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
      0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x03,0xC0,0x00,0x00,0x00,0x00,
			
"��", 0x00,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x05,0x00,0x01,0x00,0x01,0x00,0x01,0x00,
			0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,/*"��",0*/
			
"��", 0x00,0x00,0x00,0x00,0x07,0x80,0x08,0x40,0x10,0x20,0x00,0x20,0x00,0x20,0x00,0x40,
      0x00,0x80,0x01,0x00,0x02,0x00,0x04,0x00,0x08,0x00,0x1F,0xF0,0x00,0x00,0x00,0x00,/*"��",0*/
			
"��", 0x00,0x00,0x00,0x00,0x0F,0xF0,0x00,0x20,0x00,0x40,0x00,0x80,0x01,0xC0,0x00,0x20,
      0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x08,0x20,0x07,0xC0,0x00,0x00,0x00,0x00,/*"��",0*/

"��", 0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x00,0x04,0x00,0x04,0x80,0x08,0x80,0x08,0x80,
      0x10,0x80,0x10,0x80,0x3F,0xF0,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x00,0x00,0x00,/*"��",0*/

"��", 0x00,0x00,0x00,0x00,0x1F,0xC0,0x10,0x00,0x10,0x00,0x10,0x00,0x1F,0x00,0x00,0x80,
      0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x10,0x80,0x0F,0x00,0x00,0x00,0x00,0x00,/*"��",0*/

"��", 0x00,0x00,0x00,0x00,0x00,0x80,0x01,0x00,0x02,0x00,0x04,0x00,0x08,0x00,0x0F,0x80,
      0x10,0x40,0x10,0x20,0x10,0x20,0x10,0x20,0x08,0x40,0x07,0x80,0x00,0x00,0x00,0x00,/*"��",0*/

"��", 0x00,0x00,0x00,0x00,0x1F,0xF0,0x00,0x20,0x00,0x40,0x00,0x80,0x00,0x80,0x01,0x00,
      0x01,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,/*"��",0*/

"��", 0x00,0x00,0x00,0x00,0x07,0x80,0x08,0x40,0x10,0x20,0x10,0x20,0x08,0x40,0x07,0x80,
      0x08,0x40,0x10,0x20,0x10,0x20,0x10,0x20,0x08,0x40,0x07,0x80,0x00,0x00,0x00,0x00,/*"��",0*/

"��", 0x00,0x00,0x00,0x00,0x07,0x80,0x08,0x40,0x10,0x20,0x10,0x20,0x10,0x20,0x08,0x20,
      0x07,0xC0,0x00,0x40,0x00,0x80,0x01,0x00,0x02,0x00,0x04,0x00,0x00,0x00,0x00,0x00,/*"��",0*/

"��", 0x00,0x00,0x00,0x00,0x07,0xC0,0x08,0x20,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
      0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x08,0x20,0x07,0xC0,0x00,0x00,0x00,0x00,/*"��",0*/			
/*****
"��", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,
      0x06,0x60,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,

"��", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x0C,0x00,0x0D,0xC0,0x0E,0x60,
      0x0C,0x30,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,

"��", 0x00,0x00,0x00,0x00,0x0F,0xF0,0x0F,0xF0,0x08,0x30,0x00,0x60,0x00,0xC0,0x00,0xC0,
      0x01,0x80,0x01,0x80,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x00,0x00,0x00,0x00,

"��", 0x00,0x00,0x00,0x00,0x03,0xE0,0x06,0x30,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,
      0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x06,0x30,0x03,0xE0,0x00,0x00,0x00,0x00,

"��", 0x00,0x00,0x00,0x00,0x0F,0xF0,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0F,0xC0,0x0E,0x60,
      0x00,0x30,0x00,0x30,0x00,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,
 			
"��", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x00,0x30,0x00,0x60,0x01,0xC0,
      0x00,0x60,0x00,0x30,0x00,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,

"��", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x0C,0x30,0x00,0x30,0x00,0x30,
      0x00,0x60,0x00,0xC0,0x01,0x80,0x03,0x10,0x06,0x10,0x0F,0xF0,0x00,0x00,0x00,0x00,
****/
};
unsigned int leddisbuf[32*64] = {//RGB 8*3 R:0xff0000 G:0x00ff00 B:0x0000ff
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

};

unsigned char led_dis_cnt = 0;
unsigned char moving = 0;

extern unsigned char displayline;

void delay(unsigned short delay)
{
	while(delay)delay--;
}


/*****************************************************************************
 �� �� ��  : LED_P6x8Char
 ��������  : ��ʾһ��6x8��׼ASCII�ַ�
 �������  : UCHAR8 ucIdxX  ��ʾ��������0-64
             UCHAR8 ucIdxY  ҳ��Χ������0-32
             UCHAR8 ucData  ��ʾ���ַ�
 �������  : NONE
 �� �� ֵ  : NONE
*****************************************************************************/
void LED_P8x16Char(unsigned char ucIdxX, unsigned char ucIdxY, unsigned char ucData,unsigned int uccolor,unsigned int ucbgcolor)
{
	unsigned char i, j, ucDataTmp;     
	unsigned int pos;
	unsigned char ucasciitmp;
    //ucIdxY <<= 1;   
	ucDataTmp = ucData - 32;//asciiת��Ϊ������� ���������0��Ӧascii32(SPACE)
	for(i = 0; i < 16; i++) 
	{
		ucasciitmp = F8X16[(ucDataTmp << 4) + i];
		for (j = 0;j < 8; j++)
		{
			pos = (ucIdxY * 64 + ucIdxX) + 64 * i + j;			
			if ((0x80 >> j) & ucasciitmp)
			{
				leddisbuf[pos] = uccolor;
			}
			else
			{	
				leddisbuf[pos] = ucbgcolor;
			}
		}
	}

}
void LED_P8x16Str(unsigned char ucIdxX, unsigned char ucIdxY, unsigned char ucDataStr[],unsigned int uccolor,unsigned int ucbgcolor)
{
    unsigned char i, j, k, ucDataTmp;
		unsigned int pos;
		unsigned char ucasciitmp;
    //ucIdxY <<= 1;
    
    for (k = 0; ucDataStr[k] != '\0'; k++)
    {    
        ucDataTmp = ucDataStr[k] - 32;
        if(ucIdxX > 64)
        {
            ucIdxX = 0;
            ucIdxY += 16;
        }
				for(i = 0; i < 16; i++) 
				{
					ucasciitmp = F8X16[(ucDataTmp << 4) + i];
					for (j = 0;j < 8; j++)
					{
						pos = (ucIdxY * 64 + ucIdxX) + 64 * i + j;			
						if ((0x80 >> j) & ucasciitmp)
						{
							leddisbuf[pos] = uccolor;
						}
						else
						{	
							leddisbuf[pos] = ucbgcolor;
						}
					}
				}
        ucIdxX += 8;
    }

    return;
}
void LED_GB1616(unsigned char ucIdxX, unsigned char ucIdxY, unsigned char c[2], unsigned int uccolor, unsigned int ucbgcolor)
{
	unsigned char i,j,k,m;
	unsigned int pos;
	for (k=0;k<64;k++) 
	{ //64��ʾ�Խ����ֿ��еĸ�����ѭ����ѯ����
		if ((GB1616[k].Index[0]==c[0])&&(GB1616[k].Index[1]==c[1]))
		{ 
			for(i=0;i<16;i++) 
			{
				m = GB1616[k].Msk[2*i];	
				for (j = 0;j < 8; j++)
				{
					pos = (ucIdxY * 64 + ucIdxX) + 64 * i + j;			
					if ((0x80 >> j) & m)
					{
						leddisbuf[pos] = uccolor;
					}
					else
					{	
						leddisbuf[pos] = ucbgcolor;
					}
				}
				m = GB1616[k].Msk[2*i+1];		
				for (j = 0;j < 8; j++)
				{
					pos = (ucIdxY * 64 + ucIdxX) + 64 * i + 8 + j;			
					if ((0x80 >> j) & m)
					{
						leddisbuf[pos] = uccolor;
					}
					else
					{	
						leddisbuf[pos] = ucbgcolor;
					}
				}	
				//i++;
				//if(i%2)
				//{
				//	ucIdxY++;
				//	ucIdxX = ucIdxX-8;
				//}
				//else 
				//	ucIdxX = ucIdxX+8;
			}
		}  
	}	
}
/*
*********************************************************************************************************
*	�� �� ��: bsp_InitLed
*	����˵��: ����LEDָʾ����ص�GPIO,  �ú����� bsp_Init() ���á�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	/*
		�������е�LEDָʾ��GPIOΪ�������ģʽ
		���ڽ�GPIO����Ϊ���ʱ��GPIO����Ĵ�����ֵȱʡ��0����˻�����LED����.
		�����Ҳ�ϣ���ģ�����ڸı�GPIOΪ���ǰ���ȹر�LEDָʾ��
	*/
	//bsp_LedOff(1);
	//bsp_LedOff(2);
	//bsp_LedOff(3);
	//bsp_LedOff(4);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED1;
	GPIO_Init(GPIO_PORT_LED1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED2;
	GPIO_Init(GPIO_PORT_LED2, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED3;
	GPIO_Init(GPIO_PORT_LED3, &GPIO_InitStructure);

	//GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED4;
	//GPIO_Init(GPIO_PORT_LED4, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = UR13_SDO|UG13_SDO|UB13_SDO|UR14_SDO|UG14_SDO|UB14_SDO;
	GPIO_Init(JXI5020_DATA_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = JXI5020_SCK;
	GPIO_Init(JXI5020_SCK_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = JXI5020_GCLK;
	GPIO_Init(JXI5020_GCLK_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = JXI5020_LE;
	GPIO_Init(JXI5020_LE_PORT, &GPIO_InitStructure);
	//JXI5020_PORT->BSRRL = UR13_SDO|UG13_SDO|UB13_SDO|UR14_SDO|UG13_SDO|UB13_SDO|JXI5020_SCK|JXI5020_LE;
	//JXI5020_PORT->BSRRL = JXI5020_OE;

	GPIO_InitStructure.GPIO_Pin = D7258_A|D7258_B|D7258_C|D7258_EN;
	GPIO_Init(D7258_PORT, &GPIO_InitStructure);	
	//D7258_PORT->BSRRL 	= D7258_A|D7258_B|D7258_C|D7258_EN;
	
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedOn
*	����˵��: ����ָ����LEDָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_LedOff(uint8_t _no)
{
	_no--;

	if (_no == 0)
	{
		GPIO_PORT_LED1->BSRRH = GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED2->BSRRH = GPIO_PIN_LED2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED3->BSRRH = GPIO_PIN_LED3;
	}
	//else if (_no == 3)
	//{
	//	GPIO_PORT_LED4->BSRRH = GPIO_PIN_LED4;
	//}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedOff
*	����˵��: Ϩ��ָ����LEDָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_LedOn(uint8_t _no)
{
	_no--;

	if (_no == 0)
	{
		GPIO_PORT_LED1->BSRRL = GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED2->BSRRL = GPIO_PIN_LED2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED3->BSRRL = GPIO_PIN_LED3;
	}
	//else if (_no == 3)
	//{
	//	GPIO_PORT_LED4->BSRRL = GPIO_PIN_LED4;
	//}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedToggle
*	����˵��: ��תָ����LEDָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
void bsp_LedToggle(uint8_t _no)
{
	if (_no == 1)
	{
		GPIO_PORT_LED1->ODR ^= GPIO_PIN_LED1;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED2->ODR ^= GPIO_PIN_LED2;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED3->ODR ^= GPIO_PIN_LED3;
	}
	//else if (_no == 4)
	//{
	//	GPIO_PORT_LED4->ODR ^= GPIO_PIN_LED4;
	//}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_IsLedOn
*	����˵��: �ж�LEDָʾ���Ƿ��Ѿ�������
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: 1��ʾ�Ѿ�������0��ʾδ����
*********************************************************************************************************
*/
uint8_t bsp_IsLedOn(uint8_t _no)
{
	if (_no == 1)
	{
		if ((GPIO_PORT_LED1->ODR & GPIO_PIN_LED1) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 2)
	{
		if ((GPIO_PORT_LED2->ODR & GPIO_PIN_LED2) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 3)
	{
		if ((GPIO_PORT_LED3->ODR & GPIO_PIN_LED3) == 0)
		{
			return 1;
		}
		return 0;
	}
	//else if (_no == 4)
	//{
	//	if ((GPIO_PORT_LED4->ODR & GPIO_PIN_LED4) == 0)
	//	{
	//		return 1;
	//	}
	//	return 0;
	//}
	//
	return 0;
}

void trans_cmd(unsigned char cmd)
{
	unsigned char cmdcnt;
	JXI5020_LE_PORT -> BSRRH = JXI5020_LE;
	JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
	delay(1);
	JXI5020_LE_PORT -> BSRRL = JXI5020_LE;	
	switch(cmd)
	{
		case 0://ֹͣ�������
			for(cmdcnt = 1;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;
		case 1://����˨��
			for(cmdcnt = 1;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;	
		case 2://��ֱͬ��
			for(cmdcnt = 3;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;	
		case 3://д״̬�Ĵ���
			for(cmdcnt = 4;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;
		case 4://��״̬�Ĵ���
			for(cmdcnt = 5;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;	
		case 5://ִ��LED��·�������
			for(cmdcnt = 7;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;	
		case 6://������ ��λ
			for(cmdcnt = 10;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;	
		case 7://д״̬�Ĵ���֮ǰ��ǰ���趨ָ��
			for(cmdcnt = 14;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;	
		case 8://�뿪�ŵ�����ӳ�ģʽ
			for(cmdcnt = 23;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;
		case 9://�����ŵ�����ӳ�ģʽ
			for(cmdcnt = 26;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;	
		default://����ָ���λ����
			for(cmdcnt = 10;cmdcnt > 0;cmdcnt --)
			{
				JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				delay(0);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				delay(0);	
			}				
		break;
	}
	JXI5020_LE_PORT -> BSRRH = JXI5020_LE;
	JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;	
}

#define GCLK_FROM_DCLK		13	
#define LINE_16_NUMS		8
#define PWM_GCLKS16BIT		7
#define GCLK_DOUBLE			6
#define UP_DIS				14
#define DOWN_DIS			15
unsigned char updateflag = 0;
void display_full(void)
{
	unsigned int sck_cnt;
	unsigned short i,j,k,m;
	unsigned short state_reg=(1<<UP_DIS)|(1<<DOWN_DIS)|(0<<GCLK_FROM_DCLK)|(15<<LINE_16_NUMS)|(0<<PWM_GCLKS16BIT)|(0<<GCLK_DOUBLE)|0x2b;
	unsigned short red1;
	unsigned short green1;
	unsigned short blue1;
	unsigned short red2;
	unsigned short green2;
	unsigned short blue2;
	unsigned int mask;
	unsigned int bufaddA,bufaddB;
	GPIO_PORT_LED1->ODR ^= GPIO_PIN_LED1;
	
	//1д��״̬�Ĵ���
	//aд״̬�Ĵ���֮ǰ��ǰ���趨ָ��
		updateflag = 1;
		JXI5020_LE_PORT -> BSRRL = JXI5020_LE;//LE��Ϊ�ߵ�ƽ 	
		JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;
		for(sck_cnt = 14;sck_cnt > 0;sck_cnt --)
		{
			JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
			delay(2);
			JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
			delay(1);	
		}
		JXI5020_LE_PORT -> BSRRH = JXI5020_LE;//LE��Ϊ�͵�ƽ 		
	//bд״̬�Ĵ���
		for(j = 0;j < 4;j++)//ÿ��ͨ����4ƬMBI5052������ÿ��ͨ������һ������
		{
			for(sck_cnt = 0;sck_cnt < 16;sck_cnt ++)
			{
				//JXI5020_DATA_PORT -> BSRRL &= ~(UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO);
				//JXI5020_DATA_PORT -> BSRRH &= ~(UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO);
				mask = 0x8000 >> sck_cnt;
				if(j == 3)
					if(sck_cnt == 12)JXI5020_LE_PORT -> BSRRL = JXI5020_LE;//LE��Ϊ�ߵ�ƽ 	
				if(state_reg & mask)JXI5020_DATA_PORT -> BSRRL  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
				else JXI5020_DATA_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
				delay(1);
				JXI5020_SCK_PORT -> BSRRL = JXI5020_SCK;
				delay(1);
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				//delay(1);
			}
		}
		JXI5020_LE_PORT -> BSRRH = JXI5020_LE;//LE��Ϊ�͵�ƽ 	
		//3��������50��GCLK ���3��CLK����LE������VSYNC 
		JXI5020_LE_PORT -> BSRRH = JXI5020_LE;//LE��Ϊ�͵�ƽ 		
		for(sck_cnt = 0;sck_cnt < 100;sck_cnt ++)
		{
			if(sck_cnt == 100 - 3)JXI5020_LE_PORT -> BSRRL = JXI5020_LE;	//LE��Ϊ�ߵ�ƽ 	
			//if(state_reg & mask)JXI5020_PORT ->BSRRL  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
			JXI5020_DATA_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
			delay(1);
			JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;	//��Ϊ�ߵ�ƽ 
			JXI5020_GCLK_PORT ->BSRRL = JXI5020_GCLK;//��Ϊ�ߵ�ƽ 
			delay(1);
			if(sck_cnt < 100 - 1)
			{
				JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;	//���һ����Ҫ����	
			}
			if(sck_cnt < 100 -40)
			{
				JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;	//���һ����Ҫ����	
			}
		}
		JXI5020_LE_PORT -> BSRRH = JXI5020_LE;		//LE��Ϊ�͵�ƽ 	
		JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;	//GCLK��Ϊ�͵�ƽ 
		delay(2);//LE�½�����GCLK����������Ҫ��	
		//4����
		for(sck_cnt = 0;sck_cnt < 1024;sck_cnt ++)
		{
			JXI5020_DATA_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
			JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
			//JXI5020_PORT ->BSRRL = JXI5020_GCLK;
			delay(1);
			//JXI5020_PORT -> BSRRH = JXI5020_SCK;
		}
		JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;//SCK��Ϊ�͵�ƽ 	
		JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;	//GCLK��Ϊ�͵�ƽ 
		//5��ʼ��ʾ	
#if 0
		//2д��16*2*16������
		for(m = 0;m < 16;m++)//����16�� ��������һ��д������
		{
			for(i = 0;i < 16;i++)//ÿ��MBI5052��16��ͨ����
			//for(i = 0;i < 1;i++)
			{	
				for(j = 0;j < 4;j++)//ÿ��ͨ����4ƬMBI5052������ÿ��ͨ������һ������
				//for(j = 0;j < 1;j++)
				{
	#if 0
					bufaddA = 0x0000ff00;	//0x00ff0000 red
											//0x0000ff00 green
											//0x000000ff blue
					bufaddB = 0x0000ff00;;//8λԭʼ���ݶ�Ӧ��16λPWM����
	#else				
					bufaddA = leddisbuf[((m+1)*64-1) - j*16 - i];
					bufaddB = leddisbuf[((m+1)*64-1) - j*16 - i + 64*16];//8λԭʼ���ݶ�Ӧ��16λPWM����
	#endif
	#if 0				
					if(m == displayline)//m == 4 && (i == 0)  && (j == 1)
					{	
						red1 =   (bufaddA & 0xff0000) >> 8;//2 * (leddisbuf[i*64 + j] & 0xff0000>>16)�ϰ�������
						green1 = (bufaddA & 0x00ff00) >> 0;//2 * (leddisbuf[i*64 + j] & 0x00ff00>>8)�ϰ�������
						blue1 =  (bufaddA & 0x0000ff) << 8;//�ϰ�������
						red2 =   (bufaddB & 0xff0000) >> 8;//�ϰ�������
						green2 = (bufaddB & 0x00ff00) >> 0;//�ϰ�������
						blue2 =  (bufaddB & 0x0000ff) << 8;//�ϰ�������	
					}
					else 
					{	
						red1 =   0x0000;
						green1 = 0x0000;
						blue1 =  0x0000;//(leddisbuf[bufaddA] & 0x0000ff) << 1;//�ϰ�������
						red2 =   0x0000;//(leddisbuf[bufaddB] & 0xff0000) >> 15;//�ϰ�������
						green2 = 0x0000;//(leddisbuf[bufaddB] & 0x00ff00) >> 7;//�ϰ�������
						blue2 =  0x0000;//(leddisbuf[bufaddB] & 0x0000ff) << 1;//�ϰ�������
					}
	#else
					red1 =   (bufaddA & 0xff0000) >> 8;//2 * (leddisbuf[i*64 + j] & 0xff0000>>16)�ϰ�������0x0000;//
					green1 =  (bufaddA & 0x00ff00) >> 0;//2 * (leddisbuf[i*64 + j] & 0x00ff00>>8)�ϰ�������0x0000;//
					blue1 =  (bufaddA & 0x0000ff) << 8;//�ϰ������� 0xffff;//
					red2 =    (bufaddB & 0xff0000) >> 8;//�ϰ�������
					green2 =  (bufaddB & 0x00ff00) >> 0;//�ϰ�������
					blue2 =   (bufaddB & 0x0000ff) << 8;//�ϰ�������	
	#endif				
					
					//if(m == 0)red1 = 255;
					for(k = 0;k < 16;k++)
					{
						mask = 0x8000 >> k;
						if(j == 3)
							if(k == 15)JXI5020_PORT -> BSRRL = JXI5020_LE;//���һ��λLE��Ϊ�ߵ�ƽ 
	#if 1
						if(red1 & mask)JXI5020_PORT -> BSRRL = UR14_SDO;
						else JXI5020_PORT -> BSRRH = UR14_SDO;
						
						if(green1 & mask)JXI5020_PORT -> BSRRL = UG14_SDO;
						else JXI5020_PORT -> BSRRH = UG14_SDO;
						
						if(blue1 & mask)JXI5020_PORT -> BSRRL = UB14_SDO;
						else JXI5020_PORT -> BSRRH = UB14_SDO;	
						
						if(red2 & mask)JXI5020_PORT -> BSRRL = UR13_SDO;
						else JXI5020_PORT -> BSRRH = UR13_SDO;
						
						if(green2 & mask)JXI5020_PORT -> BSRRL = UG13_SDO;
						else JXI5020_PORT -> BSRRH = UG13_SDO;
						
						if(blue2 & mask)JXI5020_PORT -> BSRRL = UB13_SDO;
						else JXI5020_PORT -> BSRRH = UB13_SDO;	
						
						delay(1);
						JXI5020_PORT -> BSRRL = JXI5020_SCK;
						JXI5020_PORT -> BSRRL = JXI5020_GCLK;//m == 15
	#else
						JXI5020_PORT -> BSRRL = UR14_SDO | UG14_SDO;					
	#endif					
						delay(1);
						//delayins++;
						JXI5020_PORT -> BSRRH = JXI5020_SCK;		
						JXI5020_PORT -> BSRRH = JXI5020_GCLK;//m == 15					
					}		
				}
				JXI5020_PORT -> BSRRH = JXI5020_LE;//LE��Ϊ�͵�ƽ 		
			}
		}

	}
	if(1)//else//	
	{
		//3��������50��GCLK ���3��CLK����LE������VSYNC 
		JXI5020_PORT -> BSRRH = JXI5020_LE;//LE��Ϊ�͵�ƽ 		
		for(sck_cnt = 0;sck_cnt < 100;sck_cnt ++)
		{
			if(sck_cnt == 100 - 3)JXI5020_PORT -> BSRRL = JXI5020_LE;	//LE��Ϊ�ߵ�ƽ 	
			//if(state_reg & mask)JXI5020_PORT ->BSRRL  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
			JXI5020_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
			delay(1);
			JXI5020_PORT ->BSRRL = JXI5020_SCK;	//��Ϊ�ߵ�ƽ 
			JXI5020_PORT ->BSRRL = JXI5020_GCLK;//��Ϊ�ߵ�ƽ 
			delay(1);
			if(sck_cnt < 100 - 1)
			{
				JXI5020_PORT -> BSRRH = JXI5020_SCK;	//���һ����Ҫ����	
			}
			if(sck_cnt < 100 -40)
			{
				JXI5020_PORT -> BSRRH = JXI5020_GCLK;	//���һ����Ҫ����	
			}
		}
		JXI5020_PORT -> BSRRH = JXI5020_LE;		//LE��Ϊ�͵�ƽ 	
		JXI5020_PORT -> BSRRH = JXI5020_GCLK;	//GCLK��Ϊ�͵�ƽ 
		delay(2);//LE�½�����GCLK����������Ҫ��	
		//4����
		for(sck_cnt = 0;sck_cnt < 1024;sck_cnt ++)
		{
			JXI5020_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
			JXI5020_PORT ->BSRRL = JXI5020_SCK;
			//JXI5020_PORT ->BSRRL = JXI5020_GCLK;
			delay(1);
			//JXI5020_PORT -> BSRRH = JXI5020_SCK;
		}
		JXI5020_PORT -> BSRRH = JXI5020_SCK;//SCK��Ϊ�͵�ƽ 	
		JXI5020_PORT -> BSRRH = JXI5020_GCLK;	//GCLK��Ϊ�͵�ƽ 
		//5��ʼ��ʾ	
		for(i = 0;i < 16; i++)
		{
			#if 1
			//�����е�ƽ
			if(i < 8) D7258_PORT -> BSRRL = D7258_EN;
			else D7258_PORT -> BSRRH = D7258_EN;
			switch(i)
			{
				case 7:
				case 15:
					D7258_PORT_BSRR = (D7258_A | D7258_B | D7258_C)<<16;
					//D7258_PORT -> BSRRH = D7258_A | D7258_B | D7258_C;
					//D7258_PORT -> BSRRL = 0;
				break;
				case 6:
				case 14:
					D7258_PORT_BSRR = D7258_A | ((D7258_B | D7258_C)<<16);
					//D7258_PORT -> BSRRH = D7258_B | D7258_C;
					//D7258_PORT -> BSRRL = D7258_A;
				break;	
				case 5:
				case 13:
					D7258_PORT_BSRR = D7258_B | ((D7258_A | D7258_C)<<16);		
					//D7258_PORT -> BSRRH = D7258_A | D7258_C;
					//D7258_PORT -> BSRRL = D7258_B;
				break;
				case 4:
				case 12:
					D7258_PORT_BSRR = D7258_A | D7258_B |((D7258_C)<<16);		
					//D7258_PORT -> BSRRH = D7258_C;
					//D7258_PORT -> BSRRL = D7258_A | D7258_B;			
				break;		
				case 3:
				case 11:
					D7258_PORT_BSRR = D7258_C | ((D7258_A | D7258_B)<<16);
					//D7258_PORT -> BSRRH = D7258_A | D7258_B;
					//D7258_PORT -> BSRRL = D7258_C;
				break;	
				case 2:
				case 10:
					D7258_PORT_BSRR = D7258_A | D7258_C | (D7258_B<<16);	
					//D7258_PORT -> BSRRH = D7258_B;
					//D7258_PORT -> BSRRL = D7258_A | D7258_C;
				break;	
				case 1:
				case 9:
					D7258_PORT_BSRR = D7258_B | D7258_C | (D7258_A<<16);
					//D7258_PORT -> BSRRH = D7258_A;
					//D7258_PORT -> BSRRL = D7258_B | D7258_C;
				break;	
				case 0:
				case 8:	
					D7258_PORT_BSRR = D7258_A | D7258_B | D7258_C;
					//D7258_PORT -> BSRRH = 0;
					//D7258_PORT -> BSRRL = D7258_A | D7258_B | D7258_C;
				break;	
				default:
					D7258_PORT -> BSRRL = D7258_EN;
					D7258_PORT_BSRR = (D7258_A | D7258_B | D7258_C)<<16;
				break;			
			}
			#endif	
			//if(i < 16)
			{
				for(sck_cnt = 0;sck_cnt < 1024 + 50;sck_cnt ++)//����1024�����壬��1024������ʱֹͣ ռʱӦԼΪ1000/60/16=1.25ms ���ȴ�50�����������ʱ��
				{		
					JXI5020_PORT ->BSRRL = JXI5020_GCLK;
					delay(3);
					if(sck_cnt < 1024-1)JXI5020_PORT -> BSRRH = JXI5020_GCLK;//ֻ����1023��
					delay(2);
				}
			}				
			//for(sck_cnt = 0;sck_cnt < 50;sck_cnt ++)//
			//{	
			//	JXI5020_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
			//	JXI5020_PORT ->BSRRL = JXI5020_SCK;
			//	delay(1);
			//	JXI5020_PORT -> BSRRH = JXI5020_SCK;
			//}
			
			JXI5020_PORT -> BSRRH = JXI5020_GCLK;
		}
	}

//0��λ
#else	
		//2д��16*2*16������
		for(m = 0;m < 16;m++)//����16�� ��������һ��д������
		{
			if(m < 8) D7258_PORT -> BSRRL = D7258_EN;
			else D7258_PORT -> BSRRH = D7258_EN;
			switch(m)
			{
				case 7:
				case 15:
					D7258_PORT_BSRR = (D7258_A | D7258_B | D7258_C)<<16;
					//D7258_PORT -> BSRRH = D7258_A | D7258_B | D7258_C;
					//D7258_PORT -> BSRRL = 0;
				break;
				case 6:
				case 14:
					D7258_PORT_BSRR = D7258_A | ((D7258_B | D7258_C)<<16);
					//D7258_PORT -> BSRRH = D7258_B | D7258_C;
					//D7258_PORT -> BSRRL = D7258_A;
				break;	
				case 5:
				case 13:
					D7258_PORT_BSRR = D7258_B | ((D7258_A | D7258_C)<<16);		
					//D7258_PORT -> BSRRH = D7258_A | D7258_C;
					//D7258_PORT -> BSRRL = D7258_B;
				break;
				case 4:
				case 12:
					D7258_PORT_BSRR = D7258_A | D7258_B |((D7258_C)<<16);		
					//D7258_PORT -> BSRRH = D7258_C;
					//D7258_PORT -> BSRRL = D7258_A | D7258_B;			
				break;		
				case 3:
				case 11:
					D7258_PORT_BSRR = D7258_C | ((D7258_A | D7258_B)<<16);
					//D7258_PORT -> BSRRH = D7258_A | D7258_B;
					//D7258_PORT -> BSRRL = D7258_C;
				break;	
				case 2:
				case 10:
					D7258_PORT_BSRR = D7258_A | D7258_C | (D7258_B<<16);	
					//D7258_PORT -> BSRRH = D7258_B;
					//D7258_PORT -> BSRRL = D7258_A | D7258_C;
				break;	
				case 1:
				case 9:
					D7258_PORT_BSRR = D7258_B | D7258_C | (D7258_A<<16);
					//D7258_PORT -> BSRRH = D7258_A;
					//D7258_PORT -> BSRRL = D7258_B | D7258_C;
				break;	
				case 0:
				case 8:	
					D7258_PORT_BSRR = D7258_A | D7258_B | D7258_C;
					//D7258_PORT -> BSRRH = 0;
					//D7258_PORT -> BSRRL = D7258_A | D7258_B | D7258_C;
				break;	
				default:
					D7258_PORT -> BSRRL = D7258_EN;
					D7258_PORT_BSRR = (D7258_A | D7258_B | D7258_C)<<16;
				break;			
			}			
			for(i = 0;i < 16;i++)//ÿ��MBI5052��16��ͨ����  16*4*16 = 1024
			{	
				for(j = 0;j < 4;j++)//ÿ��ͨ����4ƬMBI5052������ÿ��ͨ������һ������
				{
	#if 0
					bufaddA = 0x0000ff00;	//0x00ff0000 red
											//0x0000ff00 green
											//0x000000ff blue
					bufaddB = 0x0000ff00;//8λԭʼ���ݶ�Ӧ��16λPWM����
	#else				
					bufaddA = leddisbuf[((m+1)*64-1) - j*16 - i];
					bufaddB = leddisbuf[((m+1)*64-1) - j*16 - i + 64*16];//8λԭʼ���ݶ�Ӧ��16λPWM����
	#endif
	#if 0				
					if(m == displayline)//m == 4 && (i == 0)  && (j == 1)
					{	
						red1 =   (bufaddA & 0xff0000) >> 8;//2 * (leddisbuf[i*64 + j] & 0xff0000>>16)�ϰ�������
						green1 = (bufaddA & 0x00ff00) >> 0;//2 * (leddisbuf[i*64 + j] & 0x00ff00>>8)�ϰ�������
						blue1 =  (bufaddA & 0x0000ff) << 8;//�ϰ�������
						red2 =   (bufaddB & 0xff0000) >> 8;//�ϰ�������
						green2 = (bufaddB & 0x00ff00) >> 0;//�ϰ�������
						blue2 =  (bufaddB & 0x0000ff) << 8;//�ϰ�������	
					}
					else 
					{	
						red1 =   0x0000;
						green1 = 0x0000;
						blue1 =  0x0000;//(leddisbuf[bufaddA] & 0x0000ff) << 1;//�ϰ�������
						red2 =   0x0000;//(leddisbuf[bufaddB] & 0xff0000) >> 15;//�ϰ�������
						green2 = 0x0000;//(leddisbuf[bufaddB] & 0x00ff00) >> 7;//�ϰ�������
						blue2 =  0x0000;//(leddisbuf[bufaddB] & 0x0000ff) << 1;//�ϰ�������
					}
	#else
					red1 =   (bufaddA & 0xff0000) >> 8;//2 * (leddisbuf[i*64 + j] & 0xff0000>>16)�ϰ�������0x0000;//
					green1 =  (bufaddA & 0x00ff00) >> 0;//2 * (leddisbuf[i*64 + j] & 0x00ff00>>8)�ϰ�������0x0000;//
					blue1 =  (bufaddA & 0x0000ff) << 8;//�ϰ������� 0xffff;//
					red2 =    (bufaddB & 0xff0000) >> 8;//�ϰ�������
					green2 =  (bufaddB & 0x00ff00) >> 0;//�ϰ�������
					blue2 =   (bufaddB & 0x0000ff) << 8;//�ϰ�������	
	#endif				
					
					//if(m == 0)red1 = 255;
					for(k = 0;k < 16;k++)
					{
						mask = 0x8000 >> k;
						if(j == 3)
							if(k == 15)JXI5020_LE_PORT -> BSRRL = JXI5020_LE;//���һ��λLE��Ϊ�ߵ�ƽ 
						if(red1 & mask)JXI5020_DATA_PORT -> BSRRL = UR14_SDO;
						else JXI5020_DATA_PORT -> BSRRH = UR14_SDO;
						
						if(green1 & mask)JXI5020_DATA_PORT -> BSRRL = UG14_SDO;
						else JXI5020_DATA_PORT -> BSRRH = UG14_SDO;
						
						if(blue1 & mask)JXI5020_DATA_PORT -> BSRRL = UB14_SDO;
						else JXI5020_DATA_PORT -> BSRRH = UB14_SDO;	
						
						if(red2 & mask)JXI5020_DATA_PORT -> BSRRL = UR13_SDO;
						else JXI5020_DATA_PORT -> BSRRH = UR13_SDO;
						
						if(green2 & mask)JXI5020_DATA_PORT -> BSRRL = UG13_SDO;
						else JXI5020_DATA_PORT -> BSRRH = UG13_SDO;
						
						if(blue2 & mask)JXI5020_DATA_PORT -> BSRRL = UB13_SDO;
						else JXI5020_DATA_PORT -> BSRRH = UB13_SDO;	
						
						delay(1);
						JXI5020_SCK_PORT -> BSRRL = JXI5020_SCK;
						JXI5020_GCLK_PORT -> BSRRL = JXI5020_GCLK;//m == 15
				
						delay(1);
						//delayins++;
						JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;		
						JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;//m == 15					
					}		
				}
				JXI5020_LE_PORT -> BSRRH = JXI5020_LE;//LE��Ϊ�͵�ƽ 		
			}
			for(sck_cnt = 0;sck_cnt < 50;sck_cnt ++)//����1024�����壬��1024������ʱֹͣ ռʱӦԼΪ1000/60/16=1.25ms ���ȴ�50�����������ʱ��
			{		
				JXI5020_GCLK_PORT ->BSRRH = JXI5020_GCLK;
				delay(1);
			}
		}
#endif	
	
#if 0
//3��������50��GCLK ���3��CLK����LE������VSYNC 
	for(sck_cnt = 0;sck_cnt < 60;sck_cnt ++)
	{
		if(sck_cnt == 60 - 3)JXI5020_PORT -> BSRRL = JXI5020_LE;//LE��Ϊ�ߵ�ƽ 	
		//if(state_reg & mask)JXI5020_PORT ->BSRRL  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
		JXI5020_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
		delay(1);
		JXI5020_PORT ->BSRRL = JXI5020_SCK;
		delay(1);
		if(sck_cnt < 59)JXI5020_PORT -> BSRRH = JXI5020_SCK;//���һ����Ҫ����
	}
	JXI5020_PORT -> BSRRH = JXI5020_LE;//LE��Ϊ�͵�
#endif	
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
	unsigned char j;
	//unsigned char rdata1[64];
	//unsigned short portdata = (JXI5020_PORT -> ODR) | (~(JXI5020_SCK | JXI5020_LE | UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO));
	//unsigned short seldata = (D7258_PORT -> ODR) | (~(D7258_EN | D7258_A | D7258_B | D7258_C));
	//unsigned int wdata1;
	//unsigned int wdata2;
	for (j = 0; j < 64; j++)
	{
		JXI5020_DATA_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
		JXI5020_SCK_PORT ->BSRRH = JXI5020_SCK;
		delay(2);
		JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
		delay(1);
	}	
	JXI5020_LE_PORT -> BSRRL = JXI5020_LE;
	delay(2);
	JXI5020_LE_PORT -> BSRRH  = JXI5020_LE;	
	//JXI5020_PORT -> BSRR = JXI5020_OE;
	if(led_dis_cnt < 8) D7258_PORT -> BSRRL = D7258_EN;
	else D7258_PORT -> BSRRH = D7258_EN;
	switch(led_dis_cnt)
	{
		case 7:
		case 15:
			D7258_PORT_BSRR = (D7258_A | D7258_B | D7258_C)<<16;
			//JXI5020_PORT ->BSRRH  = UR13_SDO|UG13_SDO|UB13_SDO|UR14_SDO|UG14_SDO|UB14_SDO|JXI5020_SCK|JXI5020_GCLK|JXI5020_LE|JXI5020_OE;
			//D7258_PORT -> BSRRH = D7258_A | D7258_B | D7258_C;
			//D7258_PORT -> BSRRL = 0;
			//D7258_PORT -> ODR |= (D7258_A | D7258_B | D7258_C);
		break;
		case 6:
		case 14:
			D7258_PORT_BSRR = D7258_A | ((D7258_B | D7258_C)<<16);
			//D7258_PORT -> BSRRH = D7258_B | D7258_C;
			//D7258_PORT -> BSRRL = D7258_A;
			//JXI5020_PORT ->BSRRL  = UR13_SDO|UG13_SDO|UB13_SDO|UR14_SDO|UG14_SDO|UB14_SDO|JXI5020_SCK|JXI5020_GCLK|JXI5020_LE|JXI5020_OE;

		break;	
		case 5:
		case 13:
			D7258_PORT_BSRR = D7258_B | ((D7258_A | D7258_C)<<16);		
			//D7258_PORT -> BSRRH = D7258_A | D7258_C;
			//D7258_PORT -> BSRRL = D7258_B;
		break;
		case 4:
		case 12:
			D7258_PORT_BSRR = D7258_A | D7258_B |((D7258_C)<<16);		
			//D7258_PORT -> BSRRH = D7258_C;
			//D7258_PORT -> BSRRL = D7258_A | D7258_B;			
		break;		
		case 3:
		case 11:
			D7258_PORT_BSRR = D7258_C | ((D7258_A | D7258_B)<<16);
			//D7258_PORT -> BSRRH = D7258_A | D7258_B;
			//D7258_PORT -> BSRRL = D7258_C;
		break;	
		case 2:
		case 10:
			D7258_PORT_BSRR = D7258_A | D7258_C | (D7258_B<<16);	
			//D7258_PORT -> BSRRH = D7258_B;
			//D7258_PORT -> BSRRL = D7258_A | D7258_C;
		break;	
		case 1:
		case 9:
			D7258_PORT_BSRR = D7258_B | D7258_C | (D7258_A<<16);
			//D7258_PORT -> BSRRH = D7258_A;
			//D7258_PORT -> BSRRL = D7258_B | D7258_C;
		break;	
		case 0:
		case 8:	
			D7258_PORT_BSRR = D7258_A | D7258_B | D7258_C;
			//D7258_PORT -> BSRRH = 0;
			//D7258_PORT -> BSRRL = D7258_A | D7258_B | D7258_C;
		break;			
	}
#if 0
	for (j = 0; j < 64; j++)
	{
			if(led_dis_cnt != paoma)JXI5020_PORT ->BSRR  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
			else JXI5020_PORT ->BRR  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
			JXI5020_PORT ->BSRR = JXI5020_SCK;
			JXI5020_PORT -> BRR = JXI5020_SCK;
	}
#endif
#if 1	
	for (j = 64; j > 0; j--)
	{
		if(leddisbuf[64*led_dis_cnt + j - 1] & 0x01)JXI5020_DATA_PORT ->BSRRL = UR14_SDO;
		else JXI5020_DATA_PORT ->BSRRH = UR14_SDO;
		if(leddisbuf[64*led_dis_cnt + j - 1] & 0x02)JXI5020_DATA_PORT ->BSRRL = UG14_SDO;
		else JXI5020_DATA_PORT ->BSRRH = UG14_SDO;	
		if(leddisbuf[64*led_dis_cnt + j - 1] & 0x04)JXI5020_DATA_PORT ->BSRRL = UB14_SDO;
		else JXI5020_DATA_PORT ->BSRRH = UB14_SDO;				
		//xia
		if(leddisbuf[64*(16+led_dis_cnt) + j - 1] & 0x01)JXI5020_DATA_PORT ->BSRRL = UR13_SDO;
		else JXI5020_DATA_PORT ->BSRRH = UR13_SDO;
		if(leddisbuf[64*(16+led_dis_cnt) + j - 1] & 0x02)JXI5020_DATA_PORT ->BSRRL = UG13_SDO;
		else JXI5020_DATA_PORT ->BSRRH = UG13_SDO;	
		if(leddisbuf[64*(16+led_dis_cnt) + j - 1] & 0x04)JXI5020_DATA_PORT ->BSRRL = UB13_SDO;
		else JXI5020_DATA_PORT ->BSRRH = UB13_SDO;	
		JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
		//JXI5020_PORT ->BSRRH = JXI5020_SCK;
		delay(0);
		JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
	}
#endif		
	//JXI5020_PORT ->BSRRH = JXI5020_SCK;
	//JXI5020_PORT -> BSRRL = JXI5020_SCK;	
	//JXI5020_PORT -> BSRRH = JXI5020_LE;				
	//JXI5020_PORT -> BSRRL  = JXI5020_LE;	
	//JXI5020_PORT -> BRR  = JXI5020_OE;	
	led_dis_cnt++;
	if (led_dis_cnt == 16)
	{
		led_dis_cnt = 0;
	}

}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
