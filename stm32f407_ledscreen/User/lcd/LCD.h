#include "includes.h"

//屏幕开始时显示方式，注意：当IDelay时显示第一幅画面是逐像素刷新的
//此时必须手动在刷新结束后加上  LCD_WR_REG(0x0007,0x0173);才能显示
//当Immediately时没有被注释掉是不需要此过程

/* 选择BANK1-BORSRAM1 连接 TFT，地址范围为0X60000000~0X63FFFFFF
 * FSMC_A16 接LCD的DC(寄存器/数据选择)脚
 * 16 bit => FSMC[24:0]对应HADDR[25:1]
 * 寄存器基地址 = 0X60000000
 * RAM基地址 = 0X60020000 = 0X60000000+2^16*2 = 0X60000000 + 0X20000 = 0X60020000
 * 当选择不同的地址线时，地址要重新计算。
 */
//#define Bank1_LCD_D    ((u32)0x60020000)    //Disp Data ADDR
//#define Bank1_LCD_C    ((u32)0x60000000)	   //Disp Reg ADDR


#define SPI_LCD 					SPI1 
#define SPI_LCD_CLK 				RCC_APB2Periph_SPI1
#define SPI_LCD_SCK_PIN				GPIO_Pin_3
#define SPI_LCD_SCK_GPIO_PORT		GPIOB
#define SPI_LCD_SCK_GPIO_CLK		RCC_AHB1Periph_GPIOB
#define SPI_LCD_SCK_SOURCE			GPIO_PinSource3
#define SPI_LCD_MISO_PIN			GPIO_Pin_4
#define SPI_LCD_MISO_GPIO_PORT		GPIOB
#define SPI_LCD_MISO_GPIO_CLK		RCC_AHB1Periph_GPIOB
#define SPI_LCD_MISO_SOURCE			GPIO_PinSource4
#define SPI_LCD_MOSI_PIN			GPIO_Pin_5
#define SPI_LCD_MOSI_GPIO_PORT		GPIOB
#define SPI_LCD_MOSI_GPIO_CLK		RCC_AHB1Periph_GPIOB
#define SPI_LCD_MOSI_SOURCE			GPIO_PinSource5

//GPIO_SetBits(SPI_LCD_CS_PORT,SPI_LCD_CS_PIN);//GPIO_ResetBits(SPI_LCD_CS_PORT,SPI_LCD_CS_PIN)
#define SPI_LCD_CS_PORT    			GPIOA
#define SPI_LCD_CS_PIN	   			GPIO_Pin_4
#define SPI_LCD_CS_GPIO_CLK			RCC_AHB1Periph_GPIOA
#define SPI_LCD_CS(a)	\
									if (a)	\
											SPI_LCD_CS_PORT->BSRRL = SPI_LCD_CS_PIN;\
									else		\
											SPI_LCD_CS_PORT->BSRRH = SPI_LCD_CS_PIN;
//GPIO_SetBits(SPI_LCD_DC_PORT,SPI_LCD_DC_PIN);
//GPIO_ResetBits(SPI_LCD_DC_PORT,SPI_LCD_DC_PIN)							
#define SPI_LCD_DC_PORT    GPIOA
#define SPI_LCD_DC_PIN	   GPIO_Pin_5
#define SPI_LCD_RS(a)	\
									if (a)	\
											SPI_LCD_DC_PORT->BSRRL = SPI_LCD_DC_PIN;\
									else		\
											SPI_LCD_DC_PORT->BSRRH = SPI_LCD_DC_PIN;	
						
#define SPI_LCD_RST_PORT    	GPIOA
#define SPI_LCD_RST_PIN	   		GPIO_Pin_3
#define SPI_LCD_RST(a)	\
									if (a)	\
											SPI_LCD_RST_PORT->BSRRL = SPI_LCD_RST_PIN;\
									else		\
											SPI_LCD_RST_PORT->BSRRH = SPI_LCD_RST_PIN;	

//Lcd初始化及其低级控制函数
void Lcd_Configuration(void);					
void DataToWrite(u16 data);
void WriteComm(unsigned short CMD);
void WriteData(unsigned short tem_data);
void Lcd_Initialize(void);
void LCD_WR_REG(u16 Index,u16 CongfigTemp);
void Lcd_WR_Start(void);
//Lcd高级控制函数
void Lcd_ColorBox(u16 x,u16 y,u16 xLong,u16 yLong,u16 Color);
void DrawPixel(u16 x, u16 y, u16 Color);
void LCD_PutString(unsigned short x, unsigned short y, char *s, unsigned int fColor, unsigned int bColor,unsigned char flag);
u16 ssd1289_GetPoint(u16 x,u8 y);
void LCD_Fill_Pic(u16 x, u16 y,u16 pic_H, u16 pic_V, const unsigned char* pic);
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend);
char Tiky_Button(char *filename,u16 x,u16 y);
char display_picture(char *filename);
void test_color(void);
void clear_full_screen(unsigned int color);
void LCD_Printint(unsigned short ucIdxX, unsigned short ucIdxY, int cData, unsigned int fColor, unsigned int bColor,unsigned char flag);
void LCD_P8x16Str(unsigned short x, unsigned short y, char *s, unsigned int fColor, unsigned int bColor,unsigned char flag);
/*定义常见颜色*/
// #define red 0x001f
// #define blue 0xf800
// #define green 0x07e0
// #define black 0x0000
// #define white 0xffff
// #define yellow 0x07ff
// #define orange 0x05bf
// #define Chocolate4 0x4451
// #define Grey 0xefbd//灰色

#define White          0xFFFF
#define Black          0x0000
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0

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
#define BROWN						0XBC40 //棕色
#define BRRED						0XFC07 //棕红色
#define GRAY						0X8430 //灰色
