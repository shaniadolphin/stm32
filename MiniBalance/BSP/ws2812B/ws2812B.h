#ifndef __WS2812B_H
#define	__WS2812B_H

#include "stm32f10x.h"

#define LEDS_N						6

#define WHITE						0xFFFF
#define BLACK						0x0000	  
#define BLUE						0x001F  
#define BRED						0XF81F
#define GRED						0XFFE0
#define GBLUE						0X07FF
#define RED							0xF800
#define MAGENTA						0xF81F
#define GREEN						0x07E0
#define CYAN						0x7FFF
#define YELLOW						0xFFE0
#define BROWN						0XBC40 //×ØÉ«
#define BRRED						0XFC07 //×ØºìÉ«
#define GRAY						0X8430 //»ÒÉ«
extern unsigned int colortmp;
//extern void ws2812TIMIrqHandler( void );
//extern void ws2812DMAIrqHandler( void );
extern void ws2812Init(void);
extern void WS2812_send(unsigned char (*color)[3], unsigned short len);
extern void ws2812Test(void);
//extern void WS2812_framedata_setPixel( unsigned char row, unsigned short column, unsigned char red, unsigned char green, unsigned char blue );
//extern void WS2812_sendbuf( unsigned int buffersize );
//void testWS2812(void);
#endif /* __LED_H */
