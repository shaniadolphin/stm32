#include "stm32f10x.h"

//��Ļ��ʼʱ��ʾ��ʽ��ע�⣺��IDelayʱ��ʾ��һ��������������ˢ�µ�
//��ʱ�����ֶ���ˢ�½��������  LCD_WR_REG(0x0007,0x0173);������ʾ
//��Immediatelyʱû�б�ע�͵��ǲ���Ҫ�˹���

/* ѡ��BANK1-BORSRAM1 ���� TFT����ַ��ΧΪ0X60000000~0X63FFFFFF
 * FSMC_A16 ��LCD��DC(�Ĵ���/����ѡ��)��
 * 16 bit => FSMC[24:0]��ӦHADDR[25:1]
 * �Ĵ�������ַ = 0X60000000
 * RAM����ַ = 0X60020000 = 0X60000000+2^16*2 = 0X60000000 + 0X20000 = 0X60020000
 * ��ѡ��ͬ�ĵ�ַ��ʱ����ַҪ���¼��㡣
 */
#define Bank1_LCD_D    ((u32)0x60020000)    //Disp Data ADDR
#define Bank1_LCD_C    ((u32)0x60000000)	//Disp Reg ADDR

#define LCD_CS_CLOCK	RCC_APB2Periph_GPIOB
#define LCD_CS_PORT    	GPIOB
#define LCD_CS_PIN	 	GPIO_Pin_12
#define LCD_RS_CLOCK	RCC_APB2Periph_GPIOC
#define LCD_RS_PORT    	GPIOC
#define LCD_RS_PIN	 	GPIO_Pin_6
#define LCD_RST_CLOCK	RCC_APB2Periph_GPIOC
#define LCD_RST_PORT    GPIOC
#define LCD_RST_PIN	 	GPIO_Pin_7

#define SPI_CS(a)	\
						if (a)	\
						GPIO_SetBits(LCD_CS_PORT, LCD_CS_PIN);	\
						else		\
						GPIO_ResetBits(LCD_CS_PORT, LCD_CS_PIN)
#define LCD_RS(a)	\
						if (a)	\
						GPIO_SetBits(LCD_RS_PORT, LCD_RS_PIN);	\
						else		\
						GPIO_ResetBits(LCD_RS_PORT, LCD_RS_PIN)
#define LCD_RST(a)	\
						if (a)	\
						GPIO_SetBits(LCD_RST_PORT, LCD_RST_PIN);	\
						else		\
						GPIO_ResetBits(LCD_RST_PORT, LCD_RST_PIN)

//Lcd��ʼ������ͼ����ƺ���
void Lcd_Configuration(void);
void LCD_GPIO_Config(void);						
void DataToWrite(u16 data);
void WriteComm(u8 CMD);
void WriteData(u8 tem_data);
void Lcd_Initialize(void);
void LCD_WR_REG(u16 Index,u16 CongfigTemp);
void Lcd_WR_Start(void);
//Lcd�߼����ƺ���
void Lcd_ColorBox(u16 x,u16 y,u16 xLong,u16 yLong,u16 Color);
void DrawPixel(u16 x, u16 y, u16 Color);
void LCD_PutString(unsigned short x, unsigned short y, char *s, unsigned int fColor, unsigned int bColor,unsigned char flag);
u16 ssd1289_GetPoint(u16 x,u8 y);
void LCD_Fill_Pic(u16 x, u16 y,u16 pic_H, u16 pic_V, const unsigned char* pic);
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend);
char display_picture(char *filename);
void test_color(void);
void clear_full_screen(unsigned int color);
void PutAsc0816(unsigned short x, unsigned short  y, unsigned char ch, unsigned int fColor,unsigned int bColor,unsigned char flag);
void LCD_Printint(unsigned short ucIdxX, unsigned short ucIdxY, int cData, unsigned int fColor, unsigned int bColor,unsigned char flag);
void LCD_P8x16Str(unsigned short x, unsigned short y, char *s, unsigned int fColor, unsigned int bColor,unsigned char flag);
/*���峣����ɫ*/
// #define red 0x001f
// #define blue 0xf800
// #define green 0x07e0
// #define black 0x0000
// #define white 0xffff
// #define yellow 0x07ff
// #define orange 0x05bf
// #define Chocolate4 0x4451
// #define Grey 0xefbd//��ɫ

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
#define BROWN						0XBC40 //��ɫ
#define BRRED						0XFC07 //�غ�ɫ
#define GRAY						0X8430 //��ɫ
