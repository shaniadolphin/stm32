#include "main.h"
#include "stm32f4xx_hal.h"

#define JXI5020_DATA_PORT   UR13_SDO_GPIO_Port
#define D7258_PORT 			D7258_A_GPIO_Port
#define D7258_A				D7258_A_Pin
#define D7258_B				D7258_B_Pin
#define D7258_C				D7258_C_Pin
#define D7258_EN			D7258_EN_Pin
#define UR13_SDO			UR13_SDO_Pin
#define UG13_SDO			UG13_SDO_Pin
#define UB13_SDO			UB13_SDO_Pin
#define UR14_SDO			UR14_SDO_Pin
#define UG14_SDO			UG14_SDO_Pin
#define UB14_SDO			UB14_SDO_Pin
#define JXI5020_SCK_PORT   	JXI5020_SCK_GPIO_Port
#define JXI5020_SCK			JXI5020_SCK_Pin
#define JXI5020_LE_PORT   	JXI5020_LE_GPIO_Port
#define JXI5020_LE			JXI5020_LE_Pin

//#define JXI5020_OE			GPIO_Pin_8
#define JXI5020_GCLK_PORT   JXI5020_GCLK_GPIO_Port
#define JXI5020_GCLK		JXI5020_GCLK_Pin

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

Total: 64(MBI5052x4)x32(D7529x4) 上下半屏同时刷
上半屏
N 1:		1----------------64
N 2:		1----------------64	
.
.
N16:		1----------------64
下半屏	
N 1:		1----------------64		
N 2:		1----------------64	
.
.
N16:		1----------------64	
****/
//======================================================
// 128X64OLED液晶底层驱动[8X16]字体库
// 设计者: Guokaiyi
// 描  述: [8X16]西文字符的字模数据 (横向取模左高位)
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

// ------------------  汉字字模的数据结构定义 ------------------------ //
struct  typFNT_GB16                 // 汉字字模数据结构 
{
       unsigned char  Index[3];               // 汉字内码索引	
       unsigned char   Msk[32];                        // 点阵码数据 
};

/////////////////////////////////////////////////////////////////////////
// 汉字字模表                                                          //
// 汉字库: 宋体16.dot,横向取模左高位,数据排列:从左到右从上到下         //
/////////////////////////////////////////////////////////////////////////
 const struct  typFNT_GB16 GB1616[] =          // 数据表 
{
	{{"支"}, 
	{0x01,0x00,0x01,0x00,0x01,0x04,0xFF,0xFE,
	0x01,0x00,0x01,0x00,0x3F,0xF0,0x08,0x20,
	0x08,0x20,0x04,0x40,0x02,0x80,0x01,0x00,
	0x02,0x80,0x0C,0x60,0x30,0x1E,0xC0,0x04}},
	
	{{"持"},
	{0x10,0x40,0x10,0x40,0x10,0x50,0x13,0xF8,
	0xFC,0x40,0x10,0x48,0x17,0xFC,0x18,0x10,
	0x30,0x14,0xD7,0xFE,0x12,0x10,0x11,0x10,
	0x11,0x10,0x10,0x10,0x50,0x50,0x20,0x20}},

	{{"横"},
	{0x11,0x10,0x11,0x10,0x17,0xFC,0x11,0x10,
	0xFD,0x14,0x17,0xFE,0x30,0x48,0x3B,0xFC,
	0x56,0x48,0x53,0xF8,0x92,0x48,0x13,0xF8,
	0x10,0x00,0x11,0x10,0x13,0x0C,0x14,0x04}},
	
	{{"竖"},
	{0x04,0x00,0x25,0xFC,0x25,0x04,0x24,0x88,
	0x24,0x50,0x24,0x20,0x24,0x50,0x24,0x8E,
	0x07,0x04,0x01,0x10,0x3F,0xF8,0x08,0x20,
	0x08,0x20,0x04,0x44,0xFF,0xFE,0x00,0x00}},

	{{"屏"},
	{0x00,0x08,0x3F,0xFC,0x20,0x08,0x20,0x08,
	0x3F,0xF8,0x22,0x20,0x21,0x48,0x2F,0xFC,
	0x22,0x20,0x22,0x24,0x3F,0xFE,0x22,0x20,
	0x22,0x20,0x42,0x20,0x84,0x20,0x08,0x20}},

	{{"　"},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
#if 0	
"*",  0x00,0x00,0x00,0x00,0x00,0x80,0x03,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
      0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x03,0xC0,0x00,0x00,0x00,0x00,
			
"１", 0x00,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x05,0x00,0x01,0x00,0x01,0x00,0x01,0x00,
			0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,/*"１",0*/
			
"２", 0x00,0x00,0x00,0x00,0x07,0x80,0x08,0x40,0x10,0x20,0x00,0x20,0x00,0x20,0x00,0x40,
      0x00,0x80,0x01,0x00,0x02,0x00,0x04,0x00,0x08,0x00,0x1F,0xF0,0x00,0x00,0x00,0x00,/*"２",0*/
			
"３", 0x00,0x00,0x00,0x00,0x0F,0xF0,0x00,0x20,0x00,0x40,0x00,0x80,0x01,0xC0,0x00,0x20,
      0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x08,0x20,0x07,0xC0,0x00,0x00,0x00,0x00,/*"３",0*/

"４", 0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x00,0x04,0x00,0x04,0x80,0x08,0x80,0x08,0x80,
      0x10,0x80,0x10,0x80,0x3F,0xF0,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x00,0x00,0x00,/*"４",0*/

"５", 0x00,0x00,0x00,0x00,0x1F,0xC0,0x10,0x00,0x10,0x00,0x10,0x00,0x1F,0x00,0x00,0x80,
      0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x10,0x80,0x0F,0x00,0x00,0x00,0x00,0x00,/*"５",0*/

"６", 0x00,0x00,0x00,0x00,0x00,0x80,0x01,0x00,0x02,0x00,0x04,0x00,0x08,0x00,0x0F,0x80,
      0x10,0x40,0x10,0x20,0x10,0x20,0x10,0x20,0x08,0x40,0x07,0x80,0x00,0x00,0x00,0x00,/*"６",0*/

"７", 0x00,0x00,0x00,0x00,0x1F,0xF0,0x00,0x20,0x00,0x40,0x00,0x80,0x00,0x80,0x01,0x00,
      0x01,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,/*"７",0*/

"８", 0x00,0x00,0x00,0x00,0x07,0x80,0x08,0x40,0x10,0x20,0x10,0x20,0x08,0x40,0x07,0x80,
      0x08,0x40,0x10,0x20,0x10,0x20,0x10,0x20,0x08,0x40,0x07,0x80,0x00,0x00,0x00,0x00,/*"８",0*/

"９", 0x00,0x00,0x00,0x00,0x07,0x80,0x08,0x40,0x10,0x20,0x10,0x20,0x10,0x20,0x08,0x20,
      0x07,0xC0,0x00,0x40,0x00,0x80,0x01,0x00,0x02,0x00,0x04,0x00,0x00,0x00,0x00,0x00,/*"９",0*/

"０", 0x00,0x00,0x00,0x00,0x07,0xC0,0x08,0x20,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
      0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x08,0x20,0x07,0xC0,0x00,0x00,0x00,0x00/*"０",0*/			
/*****
"８", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,
      0x06,0x60,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,

"６", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x0C,0x00,0x0D,0xC0,0x0E,0x60,
      0x0C,0x30,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,

"７", 0x00,0x00,0x00,0x00,0x0F,0xF0,0x0F,0xF0,0x08,0x30,0x00,0x60,0x00,0xC0,0x00,0xC0,
      0x01,0x80,0x01,0x80,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x00,0x00,0x00,0x00,

"０", 0x00,0x00,0x00,0x00,0x03,0xE0,0x06,0x30,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,
      0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x06,0x30,0x03,0xE0,0x00,0x00,0x00,0x00,

"５", 0x00,0x00,0x00,0x00,0x0F,0xF0,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0F,0xC0,0x0E,0x60,
      0x00,0x30,0x00,0x30,0x00,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,
 			
"３", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x00,0x30,0x00,0x60,0x01,0xC0,
      0x00,0x60,0x00,0x30,0x00,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,

"２", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x0C,0x30,0x00,0x30,0x00,0x30,
      0x00,0x60,0x00,0xC0,0x01,0x80,0x03,0x10,0x06,0x10,0x0F,0xF0,0x00,0x00,0x00,0x00,
****/
#endif
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

//extern unsigned char displayline;

void delay(unsigned short delay)
{
	while(delay)delay--;
}


/*****************************************************************************
 函 数 名  : LED_P6x8Char
 功能描述  : 显示一个6x8标准ASCII字符
 输入参数  : UCHAR8 ucIdxX  显示的行坐标0-64
             UCHAR8 ucIdxY  页范围列坐标0-32
             UCHAR8 ucData  显示的字符
 输出参数  : NONE
 返 回 值  : NONE
*****************************************************************************/
void LED_P8x16Char(unsigned char ucIdxX, unsigned char ucIdxY, unsigned char ucData,unsigned int uccolor,unsigned int ucbgcolor)
{
	unsigned char i, j, ucDataTmp;     
	unsigned int pos;
	unsigned char ucasciitmp;
    //ucIdxY <<= 1;   
	ucDataTmp = ucData - 32;//ascii转换为码表索引 ，码表索引0对应ascii32(SPACE)
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
	for (k=0;k<1;k++) 
	 { //64标示自建汉字库中的个数，循环查询内码
		if (((GB1616[k].Index[0])==c[0])&&((GB1616[k].Index[1])==c[1]))
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

#define set_gpio_0(gpioport,gpiopin)  gpioport -> BSRR = gpiopin
#define set_gpio_1(gpioport,gpiopin)  gpioport -> BSRR = (uint32_t)gpiopin << 16U
#define set_gpio_n(gpioport,gpiopin)  gpioport -> BSRR = gpiopin
/****
  if(PinState != GPIO_PIN_RESET)
  {
    GPIOx->BSRR = GPIO_Pin; //高电平
  }
  else
  {
    GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U; //低电平
  }
****/

void trans_cmd(unsigned char cmd)
{
	unsigned char cmdcnt;
	//JXI5020_LE_PORT -> BSRR = (uint32_t)JXI5020_LE << 16U;  //BSRRH 表示BSRR寄存器高16位（BRy），哪一个BRy置1，引脚输出低电平；
	//JXI5020_SCK_PORT -> BSRR = (uint32_t)JXI5020_SCK << 16U;
	set_gpio_0(JXI5020_LE_PORT, JXI5020_LE);
	set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
	delay(1);
	//JXI5020_LE_PORT -> BSRR = JXI5020_LE;  //BSRRL 表示BSRR寄存器低16位（BSy），哪一个BRy置1，引脚输出高电平；
	set_gpio_1(JXI5020_LE_PORT, JXI5020_LE);
	switch(cmd)
	{
		case 0://停止错误侦测
			for(cmdcnt = 1;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRR = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRR = (uint32_t)JXI5020_SCK << 16U;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;
		case 1://数据栓锁
			for(cmdcnt = 1;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRR = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;	
		case 2://垂直同步
			for(cmdcnt = 3;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRR = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;	
		case 3://写状态寄存器
			for(cmdcnt = 4;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;
		case 4://读状态寄存器
			for(cmdcnt = 5;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;	
		case 5://执行LED开路错误侦测
			for(cmdcnt = 7;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;	
		case 6://热启动 软复位
			for(cmdcnt = 10;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;	
		case 7://写状态寄存器之前的前置设定指定
			for(cmdcnt = 14;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;	
		case 8://离开信道输出延迟模式
			for(cmdcnt = 23;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;
		case 9://进入信道输出延迟模式
			for(cmdcnt = 26;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;	
		default://错误指令，复位操作
			for(cmdcnt = 10;cmdcnt > 0;cmdcnt --)
			{
				//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
				delay(0);	
			}				
		break;
	}
	//JXI5020_LE_PORT -> BSRRH = JXI5020_LE;
	set_gpio_0(JXI5020_LE_PORT, JXI5020_LE);
	//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;	
	set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
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
	//GPIO_PORT_LED1->ODR ^= GPIO_PIN_LED1;
	
	//1写入状态寄存器
	//a写状态寄存器之前的前置设定指定
		updateflag = 1;
		//JXI5020_LE_PORT -> BSRRL = JXI5020_LE;//LE置为高电平 	
		//JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;
		set_gpio_1(JXI5020_LE_PORT, JXI5020_LE);
		set_gpio_0(JXI5020_GCLK_PORT, JXI5020_GCLK);
		for(sck_cnt = 14;sck_cnt > 0;sck_cnt --)
		{
			//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
			set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
			delay(2);
			//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
			set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
			delay(1);	
		}
		//JXI5020_LE_PORT -> BSRRH = JXI5020_LE;//LE置为低电平 
		set_gpio_0(JXI5020_LE_PORT, JXI5020_LE);
		//b写状态寄存器
		for(j = 0;j < 4;j++)//每个通道由4片MBI5052级联，每个通道锁存一次数据
		{
			for(sck_cnt = 0;sck_cnt < 16;sck_cnt ++)
			{
				//JXI5020_DATA_PORT -> BSRRL &= ~(UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO);
				//JXI5020_DATA_PORT -> BSRRH &= ~(UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO);
				mask = 0x8000 >> sck_cnt;
				if(j == 3)
					if(sck_cnt == 12)
						set_gpio_1(JXI5020_LE_PORT, JXI5020_LE);//JXI5020_LE_PORT -> BSRRL = JXI5020_LE;//LE置为高电平 	
				if(state_reg & mask)
					set_gpio_1(JXI5020_DATA_PORT, UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO);//JXI5020_DATA_PORT -> BSRRL  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
				else 
					set_gpio_0(JXI5020_DATA_PORT, UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO);//JXI5020_DATA_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
				delay(1);
				set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);//JXI5020_SCK_PORT -> BSRRL = JXI5020_SCK;
				delay(1);
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
				//delay(1);
			}
		}
		set_gpio_0(JXI5020_LE_PORT, JXI5020_LE);//JXI5020_LE_PORT -> BSRRH = JXI5020_LE;//LE置为低电平 	
		//3发送至少50个GCLK 最后3个CLK拉高LE，发送VSYNC 
		set_gpio_0(JXI5020_LE_PORT, JXI5020_LE);//JXI5020_LE_PORT -> BSRRH = JXI5020_LE;//LE置为低电平 		
		for(sck_cnt = 0;sck_cnt < 100;sck_cnt ++)
		{
			if(sck_cnt == 100 - 3)
				set_gpio_1(JXI5020_LE_PORT, JXI5020_LE);//JXI5020_LE_PORT -> BSRRL = JXI5020_LE;	//LE置为高电平 	
			//if(state_reg & mask)JXI5020_PORT ->BSRRL  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
			//JXI5020_DATA_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;	
			set_gpio_0(JXI5020_DATA_PORT, UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO);	
			delay(1);
			//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;	//置为高电平 
			set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
			//JXI5020_GCLK_PORT ->BSRRL = JXI5020_GCLK;//置为高电平 
			set_gpio_1(JXI5020_GCLK_PORT, JXI5020_GCLK);
			delay(1);
			if(sck_cnt < 100 - 1)
			{
				//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;	//最后一个不要拉低	
				set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
			}
			if(sck_cnt < 100 -40)
			{
				//JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;	//最后一个不要拉低	
				set_gpio_0(JXI5020_GCLK_PORT, JXI5020_GCLK);
			}
		}
		//JXI5020_LE_PORT -> BSRRH = JXI5020_LE;		//LE置为低电平 	
		set_gpio_0(JXI5020_LE_PORT, JXI5020_LE);
		//JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;	//GCLK置为低电平 
		set_gpio_0(JXI5020_GCLK_PORT, JXI5020_GCLK);
		delay(2);//LE下降沿与GCLK上升沿满足要求	
		//4消隐
		for(sck_cnt = 0;sck_cnt < 1024;sck_cnt ++)
		{
			//JXI5020_DATA_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
			set_gpio_0(JXI5020_DATA_PORT, UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO);
			//JXI5020_SCK_PORT ->BSRRL = JXI5020_SCK;
			//JXI5020_PORT ->BSRRL = JXI5020_GCLK;
			delay(1);
			//JXI5020_PORT -> BSRRH = JXI5020_SCK;
		}
		//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;//SCK置为低电平 	
		set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
		//JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;	//GCLK置为低电平 
		set_gpio_0(JXI5020_GCLK_PORT, JXI5020_GCLK);
		//5开始显示	
#if 0
		//2写入16*2*16的数据
		for(m = 0;m < 16;m++)//共有16行 两个半屏一起写入数据
		{
			for(i = 0;i < 16;i++)//每个MBI5052有16个通道，
			//for(i = 0;i < 1;i++)
			{	
				for(j = 0;j < 4;j++)//每个通道由4片MBI5052级联，每个通道锁存一次数据
				//for(j = 0;j < 1;j++)
				{
	#if 0
					bufaddA = 0x0000ff00;	//0x00ff0000 red
											//0x0000ff00 green
											//0x000000ff blue
					bufaddB = 0x0000ff00;;//8位原始数据对应到16位PWM数据
	#else				
					bufaddA = leddisbuf[((m+1)*64-1) - j*16 - i];
					bufaddB = leddisbuf[((m+1)*64-1) - j*16 - i + 64*16];//8位原始数据对应到16位PWM数据
	#endif
	#if 0				
					if(m == displayline)//m == 4 && (i == 0)  && (j == 1)
					{	
						red1 =   (bufaddA & 0xff0000) >> 8;//2 * (leddisbuf[i*64 + j] & 0xff0000>>16)上半屏数据
						green1 = (bufaddA & 0x00ff00) >> 0;//2 * (leddisbuf[i*64 + j] & 0x00ff00>>8)上半屏数据
						blue1 =  (bufaddA & 0x0000ff) << 8;//上半屏数据
						red2 =   (bufaddB & 0xff0000) >> 8;//上半屏数据
						green2 = (bufaddB & 0x00ff00) >> 0;//上半屏数据
						blue2 =  (bufaddB & 0x0000ff) << 8;//上半屏数据	
					}
					else 
					{	
						red1 =   0x0000;
						green1 = 0x0000;
						blue1 =  0x0000;//(leddisbuf[bufaddA] & 0x0000ff) << 1;//上半屏数据
						red2 =   0x0000;//(leddisbuf[bufaddB] & 0xff0000) >> 15;//上半屏数据
						green2 = 0x0000;//(leddisbuf[bufaddB] & 0x00ff00) >> 7;//上半屏数据
						blue2 =  0x0000;//(leddisbuf[bufaddB] & 0x0000ff) << 1;//上半屏数据
					}
	#else
					red1 =   (bufaddA & 0xff0000) >> 8;//2 * (leddisbuf[i*64 + j] & 0xff0000>>16)上半屏数据0x0000;//
					green1 =  (bufaddA & 0x00ff00) >> 0;//2 * (leddisbuf[i*64 + j] & 0x00ff00>>8)上半屏数据0x0000;//
					blue1 =  (bufaddA & 0x0000ff) << 8;//上半屏数据 0xffff;//
					red2 =    (bufaddB & 0xff0000) >> 8;//上半屏数据
					green2 =  (bufaddB & 0x00ff00) >> 0;//上半屏数据
					blue2 =   (bufaddB & 0x0000ff) << 8;//上半屏数据	
	#endif				
					
					//if(m == 0)red1 = 255;
					for(k = 0;k < 16;k++)
					{
						mask = 0x8000 >> k;
						if(j == 3)
							if(k == 15)JXI5020_PORT -> BSRRL = JXI5020_LE;//最后一个位LE置为高电平 
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
				JXI5020_PORT -> BSRRH = JXI5020_LE;//LE置为低电平 		
			}
		}

	}
	if(1)//else//	
	{
		//3发送至少50个GCLK 最后3个CLK拉高LE，发送VSYNC 
		JXI5020_PORT -> BSRRH = JXI5020_LE;//LE置为低电平 		
		for(sck_cnt = 0;sck_cnt < 100;sck_cnt ++)
		{
			if(sck_cnt == 100 - 3)JXI5020_PORT -> BSRRL = JXI5020_LE;	//LE置为高电平 	
			//if(state_reg & mask)JXI5020_PORT ->BSRRL  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
			JXI5020_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
			delay(1);
			JXI5020_PORT ->BSRRL = JXI5020_SCK;	//置为高电平 
			JXI5020_PORT ->BSRRL = JXI5020_GCLK;//置为高电平 
			delay(1);
			if(sck_cnt < 100 - 1)
			{
				JXI5020_PORT -> BSRRH = JXI5020_SCK;	//最后一个不要拉低	
			}
			if(sck_cnt < 100 -40)
			{
				JXI5020_PORT -> BSRRH = JXI5020_GCLK;	//最后一个不要拉低	
			}
		}
		JXI5020_PORT -> BSRRH = JXI5020_LE;		//LE置为低电平 	
		JXI5020_PORT -> BSRRH = JXI5020_GCLK;	//GCLK置为低电平 
		delay(2);//LE下降沿与GCLK上升沿满足要求	
		//4消隐
		for(sck_cnt = 0;sck_cnt < 1024;sck_cnt ++)
		{
			JXI5020_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;		
			JXI5020_PORT ->BSRRL = JXI5020_SCK;
			//JXI5020_PORT ->BSRRL = JXI5020_GCLK;
			delay(1);
			//JXI5020_PORT -> BSRRH = JXI5020_SCK;
		}
		JXI5020_PORT -> BSRRH = JXI5020_SCK;//SCK置为低电平 	
		JXI5020_PORT -> BSRRH = JXI5020_GCLK;	//GCLK置为低电平 
		//5开始显示	
		for(i = 0;i < 16; i++)
		{
			#if 1
			//设置行电平
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
				for(sck_cnt = 0;sck_cnt < 1024 + 50;sck_cnt ++)//发送1024个脉冲，第1024个脉冲时停止 占时应约为1000/60/16=1.25ms 最后等待50个脉冲的消隐时间
				{		
					JXI5020_PORT ->BSRRL = JXI5020_GCLK;
					delay(3);
					if(sck_cnt < 1024-1)JXI5020_PORT -> BSRRH = JXI5020_GCLK;//只发送1023个
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

//0复位
#else	
		//2写入16*2*16的数据
		for(m = 0;m < 16;m++)//共有16行 两个半屏一起写入数据
		{
			if(m < 8) 
				set_gpio_1(D7258_PORT, D7258_EN);//D7258_PORT -> BSRRL = D7258_EN;
			else 
				set_gpio_0(D7258_PORT, D7258_EN);//D7258_PORT -> BSRRH = D7258_EN;
			switch(m)
			{
				case 7:
				case 15:
					//D7258_PORT_BSRR = (D7258_A | D7258_B | D7258_C)<<16;
					set_gpio_n(D7258_PORT, (D7258_A | D7258_B | D7258_C)<<16);
				break;
				case 6:
				case 14:
					//D7258_PORT_BSRR = D7258_A | ((D7258_B | D7258_C)<<16);
					set_gpio_n(D7258_PORT, D7258_A | ((D7258_B | D7258_C)<<16));
				break;	
				case 5:
				case 13:
					//D7258_PORT_BSRR = D7258_B | ((D7258_A | D7258_C)<<16);		
					set_gpio_n(D7258_PORT, D7258_B | ((D7258_A | D7258_C)<<16));
				break;
				case 4:
				case 12:
					//D7258_PORT_BSRR = D7258_A | D7258_B |((D7258_C)<<16);		
					set_gpio_n(D7258_PORT, D7258_A | D7258_B |((D7258_C)<<16));
				break;		
				case 3:
				case 11:
					//D7258_PORT_BSRR = D7258_C | ((D7258_A | D7258_B)<<16);
					set_gpio_n(D7258_PORT, D7258_C | ((D7258_A | D7258_B)<<16)); 
				break;	
				case 2:
				case 10:
					//D7258_PORT_BSRR = D7258_A | D7258_C | (D7258_B<<16);	
					set_gpio_n(D7258_PORT, D7258_A | D7258_C | (D7258_B<<16));
				break;	
				case 1:
				case 9:
					//D7258_PORT_BSRR = D7258_B | D7258_C | (D7258_A<<16);
					set_gpio_n(D7258_PORT, D7258_B | D7258_C | (D7258_A<<16));
				break;	
				case 0:
				case 8:	
					//D7258_PORT_BSRR = D7258_A | D7258_B | D7258_C;
					set_gpio_n(D7258_PORT, D7258_A | D7258_B | D7258_C);
				break;	
				default:
					//D7258_PORT -> BSRRL = D7258_EN;
					set_gpio_1(D7258_PORT, D7258_EN);
					//D7258_PORT_BSRR = (D7258_A | D7258_B | D7258_C)<<16;
					set_gpio_n(D7258_PORT, D7258_A | D7258_B | D7258_C);
				break;			
			}			
			for(i = 0;i < 16;i++)//每个MBI5052有16个通道，  16*4*16 = 1024
			{	
				for(j = 0;j < 4;j++)//每个通道由4片MBI5052级联，每个通道锁存一次数据
				{
	#if 0
					bufaddA = 0x0000ff00;	//0x00ff0000 red
											//0x0000ff00 green
											//0x000000ff blue
					bufaddB = 0x0000ff00;//8位原始数据对应到16位PWM数据
	#else				
					bufaddA = leddisbuf[((m+1)*64-1) - j*16 - i];
					bufaddB = leddisbuf[((m+1)*64-1) - j*16 - i + 64*16];//8位原始数据对应到16位PWM数据
	#endif
	#if 0				
					if(m == displayline)//m == 4 && (i == 0)  && (j == 1)
					{	
						red1 =   (bufaddA & 0xff0000) >> 8;//2 * (leddisbuf[i*64 + j] & 0xff0000>>16)上半屏数据
						green1 = (bufaddA & 0x00ff00) >> 0;//2 * (leddisbuf[i*64 + j] & 0x00ff00>>8)上半屏数据
						blue1 =  (bufaddA & 0x0000ff) << 8;//上半屏数据
						red2 =   (bufaddB & 0xff0000) >> 8;//上半屏数据
						green2 = (bufaddB & 0x00ff00) >> 0;//上半屏数据
						blue2 =  (bufaddB & 0x0000ff) << 8;//上半屏数据	
					}
					else 
					{	
						red1 =   0x0000;
						green1 = 0x0000;
						blue1 =  0x0000;//(leddisbuf[bufaddA] & 0x0000ff) << 1;//上半屏数据
						red2 =   0x0000;//(leddisbuf[bufaddB] & 0xff0000) >> 15;//上半屏数据
						green2 = 0x0000;//(leddisbuf[bufaddB] & 0x00ff00) >> 7;//上半屏数据
						blue2 =  0x0000;//(leddisbuf[bufaddB] & 0x0000ff) << 1;//上半屏数据
					}
	#else
					red1 =   (bufaddA & 0xff0000) >> 8;//2 * (leddisbuf[i*64 + j] & 0xff0000>>16)上半屏数据0x0000;//
					green1 =  (bufaddA & 0x00ff00) >> 0;//2 * (leddisbuf[i*64 + j] & 0x00ff00>>8)上半屏数据0x0000;//
					blue1 =  (bufaddA & 0x0000ff) << 8;//上半屏数据 0xffff;//
					red2 =    (bufaddB & 0xff0000) >> 8;//上半屏数据
					green2 =  (bufaddB & 0x00ff00) >> 0;//上半屏数据
					blue2 =   (bufaddB & 0x0000ff) << 8;//上半屏数据	
	#endif				
					
					//if(m == 0)red1 = 255;
					for(k = 0;k < 16;k++)
					{
						mask = 0x8000 >> k;
						if(j == 3)
							if(k == 15)
								set_gpio_1(JXI5020_LE_PORT, JXI5020_LE);//JXI5020_LE_PORT -> BSRRL = JXI5020_LE;//最后一个位LE置为高电平 
						if(red1 & mask)
							set_gpio_1(JXI5020_DATA_PORT, UR14_SDO);//JXI5020_DATA_PORT -> BSRRL = UR14_SDO;
						else 
							set_gpio_0(JXI5020_DATA_PORT, UR14_SDO);//JXI5020_DATA_PORT -> BSRRH = UR14_SDO;
						
						if(green1 & mask)
							set_gpio_1(JXI5020_DATA_PORT, UG14_SDO);//JXI5020_DATA_PORT -> BSRRL = UG14_SDO;
						else 
							set_gpio_0(JXI5020_DATA_PORT, UG14_SDO);//JXI5020_DATA_PORT -> BSRRH = UG14_SDO;
						
						if(blue1 & mask)
							set_gpio_1(JXI5020_DATA_PORT, UB14_SDO);//JXI5020_DATA_PORT -> BSRRL = UB14_SDO;
						else 
							set_gpio_0(JXI5020_DATA_PORT, UB14_SDO);//JXI5020_DATA_PORT -> BSRRH = UB14_SDO;	
						
						if(red2 & mask)
							set_gpio_1(JXI5020_DATA_PORT, UR13_SDO);//JXI5020_DATA_PORT -> BSRRL = UR13_SDO;
						else 
							set_gpio_0(JXI5020_DATA_PORT, UR13_SDO);//JXI5020_DATA_PORT -> BSRRH = UR13_SDO;
						
						if(green2 & mask)
							set_gpio_1(JXI5020_DATA_PORT, UG13_SDO);//JXI5020_DATA_PORT -> BSRRL = UG13_SDO;
						else 
							set_gpio_0(JXI5020_DATA_PORT, UG13_SDO);//JXI5020_DATA_PORT -> BSRRH = UG13_SDO;
						
						if(blue2 & mask)
							set_gpio_1(JXI5020_DATA_PORT, UB13_SDO);//JXI5020_DATA_PORT -> BSRRL = UB13_SDO;
						else 
							set_gpio_0(JXI5020_DATA_PORT, UB13_SDO);//JXI5020_DATA_PORT -> BSRRH = UB13_SDO;	
						
						delay(1);
						//JXI5020_SCK_PORT -> BSRRL = JXI5020_SCK;
						set_gpio_1(JXI5020_SCK_PORT, JXI5020_SCK);
						//JXI5020_GCLK_PORT -> BSRRL = JXI5020_GCLK;//m == 15
						set_gpio_1(JXI5020_GCLK_PORT, JXI5020_GCLK);
						delay(1);
						//delayins++;
						//JXI5020_SCK_PORT -> BSRRH = JXI5020_SCK;
						set_gpio_0(JXI5020_SCK_PORT, JXI5020_SCK);
						//JXI5020_GCLK_PORT -> BSRRH = JXI5020_GCLK;//m == 15	
						set_gpio_0(JXI5020_GCLK_PORT, JXI5020_GCLK);
					}		
				}
				//JXI5020_LE_PORT -> BSRRH = JXI5020_LE;//LE置为低电平 		
			}
			for(sck_cnt = 0;sck_cnt < 50;sck_cnt ++)//发送1024个脉冲，第1024个脉冲时停止 占时应约为1000/60/16=1.25ms 最后等待50个脉冲的消隐时间
			{		
				//JXI5020_GCLK_PORT ->BSRRH = JXI5020_GCLK;
				set_gpio_0(JXI5020_GCLK_PORT, JXI5020_GCLK);
				delay(1);
			}
		}
#endif	
	
#if 0
//3发送至少50个GCLK 最后3个CLK拉高LE，发送VSYNC 
	for(sck_cnt = 0;sck_cnt < 60;sck_cnt ++)
	{
		if(sck_cnt == 60 - 3)
			set_gpio_1(JXI5020_PORT, JXI5020_LE);//JXI5020_PORT -> BSRRL = JXI5020_LE;//LE置为高电平 	
		//if(state_reg & mask)JXI5020_PORT ->BSRRL  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;
		//JXI5020_PORT ->BSRRH  = UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO;	
		set_gpio_0(JXI5020_PORT, UR14_SDO | UG14_SDO | UB14_SDO | UR13_SDO | UG13_SDO | UB13_SDO);
		delay(1);
		//JXI5020_PORT ->BSRRL = JXI5020_SCK;
		set_gpio_1(JXI5020_PORT, JXI5020_SCK);
		delay(1);
		if(sck_cnt < 59)
			set_gpio_0(JXI5020_PORT, JXI5020_SCK);//JXI5020_PORT -> BSRRH = JXI5020_SCK;//最后一个不要拉低
	}
	//JXI5020_PORT -> BSRRH = JXI5020_LE;//LE置为低电
	set_gpio_0(JXI5020_PORT, JXI5020_LE);
#endif	
}

#if 0
/*----------------------------------------------------------------------------
 * @描述: dispaly_process
 * @输入: 无
 * @返回: 无
 * @历史:
 版本        日期          作者           改动内容和原因
 ------    -----------	 ---------	 ----------------------	
 1.0       2015.11.15     zxjun          创建模块	
----------------------------------------------------------------------------*/
void dispaly_process(void) //共7us
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

#endif