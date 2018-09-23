/*******************************************

				  2.4�� QVGA��ʾ��������







**********************************************/

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "fsmc_sram.h"
#include "ili9320_font.h"

#include "ASCII12X24.h"
#include "ASCII8X16.h"

#define Bank1_LCD_D    ((uint32_t)0x60010000)    //disp Data ADDR
#define Bank1_LCD_C    ((uint32_t)0x60000000)	 //disp Reg ADDR

unsigned long color1=0;
//void MUC_Init();
void LCD_Init(void);
void LCD_WR_REG(unsigned int index);
void LCD_WR_CMD(unsigned int index,unsigned int val);

void LCD_WR_Data(unsigned int val);
void LCD_WR_Data_8(unsigned int val);
void LCD_test(void);
void LCD_clear(unsigned int p);
void LCD_Clear_Screen(u16 color);
void LCD_Set_Area(u16 xstart,u16 ystart,u16 xend,u16 yend);
void lcd_wr_zf(unsigned int a, unsigned int b, unsigned int a1,unsigned int b1, unsigned int d,unsigned int e, unsigned char g, unsigned char *f); 
void lcd_wr_pixel(unsigned int a, unsigned int b, unsigned int e);
void LCD_Color_Show(void);
unsigned int color[]={0xf800,0x07e0,0x001f,0xffe0,0x0000,0xffff,0x07ff,0xf81f};

unsigned int LCD_RD_data(void);
extern void lcd_rst(void);
extern void Delay(__IO uint32_t nCount);


static void delay(u16 cnt) 
{
  while (cnt--);
}
static void delay_ms(u16 cnt)
{
	do{delay(10);}while(cnt--);	
}

//д�Ĵ�����ַ����
void LCD_WR_REG(unsigned int index)
{
	*(__IO uint8_t *) (Bank1_LCD_C)= index;

}

void LCD_WRITE_CMD(u16 index)		   	//д���TFT
{
	*(__IO uint8_t *) (Bank1_LCD_C)=(index>>8);
	*(__IO uint8_t *) (Bank1_LCD_C)=(index);	
}

void LCD_WRITE_DATA(u16 Data)			//д���ݵ�TFT
{
	*(__IO uint8_t *) (Bank1_LCD_D)=(Data>>8);
	*(__IO uint8_t *) (Bank1_LCD_D)=(Data);	
}

//д�Ĵ������ݺ���
//���룺dbw ����λ����1Ϊ16λ��0Ϊ8λ��
void LCD_WR_CMD(unsigned int index,unsigned int val)
{
	LCD_WRITE_CMD(index);
	LCD_WRITE_DATA(val);
//	*(__IO uint8_t *) (Bank1_LCD_C)= index>>8;
//	*(__IO uint8_t *) (Bank1_LCD_C)= index;	
//	*(__IO uint8_t *) (Bank1_LCD_D)= val>>8;
//	*(__IO uint8_t *) (Bank1_LCD_D)= val;
}

unsigned int LCD_RD_data(void)
{
	unsigned int a=0;
	a=(*(__IO uint16_t *) (Bank1_LCD_D)); 	//Dummy
	a=*(__IO uint16_t *) (Bank1_LCD_D); //L

	return(a);	
}

//д16λ���ݺ���

void    LCD_WR_Data(unsigned int val)
{ 
	*(__IO uint16_t *) (Bank1_LCD_D)= val>>8; 
	*(__IO uint16_t *) (Bank1_LCD_D)= val; 
}

void LCD_WR_Data_8(unsigned int val)
{
	*(__IO uint16_t *) (Bank1_LCD_D)= val;
}

/****************************************************************************
* ��    �ƣ�u16 ili9320_BGR2RGB(u16 c)
* ��    �ܣ�RRRRRGGGGGGBBBBB ��Ϊ BBBBBGGGGGGRRRRR ��ʽ
* ��ڲ�����c      BRG ��ɫֵ
* ���ڲ�����RGB ��ɫֵ
* ˵    �����ڲ���������
* ���÷�����
****************************************************************************/
u16 ili9320_BGR2RGB(u16 c)
{
	u16  r, g, b;
	b = (c>>0)  & 0x1f;
	g = (c>>5)  & 0x3f;
	r = (c>>11) & 0x1f;
	return( (b<<11) + (g<<5) + (r<<0) );
}
/*******************************************************************************/
void LCD_Color_Show(void)
{
	LCD_Set_Area(0,0,239,319);
	LCD_Clear_Screen(0x001F);
	delay_ms(40000);
	delay_ms(40000);
	delay_ms(40000);
	LCD_Clear_Screen(0xFFE0);
	delay_ms(40000);
	delay_ms(40000);
	delay_ms(40000);
	LCD_Clear_Screen(0xF800);
	delay_ms(40000);
	delay_ms(40000);
	delay_ms(40000);
	LCD_Clear_Screen(0x0000);
	delay_ms(40000);
	delay_ms(40000);
}

//��ʼ������
void LCD_Init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ; 	 //LCD-RST
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);  
 
    GPIO_SetBits(GPIOE, GPIO_Pin_1);		 	 
	delay_ms(15000);
	GPIO_ResetBits(GPIOE, GPIO_Pin_1);
    delay_ms(15000);					   				   
    GPIO_SetBits(GPIOE, GPIO_Pin_1);		 	 
	delay_ms(400);
	GPIO_ResetBits(GPIOE, GPIO_Pin_1);
    delay_ms(1200);					   
    GPIO_SetBits(GPIOE, GPIO_Pin_1);		 	 
	delay_ms(1200);
		
//#ifdef FSMC_16
//	LCD_WR_CMD(0x00E3, 0x3008); // Set internal timing
//	LCD_WR_CMD(0x00E7, 0x0012); // Set internal timing
//	LCD_WR_CMD(0x00EF, 0x1231); // Set internal timing
//	LCD_WR_CMD(0x0000, 0x0001); // Start Oscillation
//	LCD_WR_CMD(0x0001, 0x0100); // set SS and SM bit
//	LCD_WR_CMD(0x0002, 0x0700); // set 1 line inversion
//
//	LCD_WR_CMD(0x0003, 0x1018); // set GRAM write direction and BGR=0,262K colors,1 transfers/pixel.
//	LCD_WR_CMD(0x0004, 0x0000); // Resize register
//	LCD_WR_CMD(0x0008, 0x0202); // set the back porch and front porch
//	LCD_WR_CMD(0x0009, 0x0000); // set non-display area refresh cycle ISC[3:0]
//	LCD_WR_CMD(0x000A, 0x0000); // FMARK function
//	LCD_WR_CMD(0x000C, 0x0000); // RGB interface setting
//	LCD_WR_CMD(0x000D, 0x0000); // Frame marker Position
//	LCD_WR_CMD(0x000F, 0x0000); // RGB interface polarity
////Power On sequence 
//	LCD_WR_CMD(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
//	LCD_WR_CMD(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
//	LCD_WR_CMD(0x0012, 0x0000); // VREG1OUT voltage
//	LCD_WR_CMD(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
//	Delay(200); // Dis-charge capacitor power voltage
//	LCD_WR_CMD(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
//	LCD_WR_CMD(0x0011, 0x0227); // R11h=0x0221 at VCI=3.3V, DC1[2:0], DC0[2:0], VC[2:0]
//	Delay(50); // Delay 50ms
//	LCD_WR_CMD(0x0012, 0x001C); // External reference voltage= Vci;
//	Delay(50); // Delay 50ms
//	LCD_WR_CMD(0x0013, 0x1800); // R13=1200 when R12=009D;VDV[4:0] for VCOM amplitude
//	LCD_WR_CMD(0x0029, 0x001C); // R29=000C when R12=009D;VCM[5:0] for VCOMH
//	LCD_WR_CMD(0x002B, 0x000D); // Frame Rate = 91Hz
//	Delay(50); // Delay 50ms
//	LCD_WR_CMD(0x0020, 0x0000); // GRAM horizontal Address
//	LCD_WR_CMD(0x0021, 0x0000); // GRAM Vertical Address
//// ----------- Adjust the Gamma Curve ----------//
//	LCD_WR_CMD(0x0030, 0x0007);
//	LCD_WR_CMD(0x0031, 0x0302);
//	LCD_WR_CMD(0x0032, 0x0105);
//	LCD_WR_CMD(0x0035, 0x0206);
//	LCD_WR_CMD(0x0036, 0x0808);
//	LCD_WR_CMD(0x0037, 0x0206);
//	LCD_WR_CMD(0x0038, 0x0504);
//	LCD_WR_CMD(0x0039, 0x0007);
//	LCD_WR_CMD(0x003C, 0x0105);
//	LCD_WR_CMD(0x003D, 0x0808);
////------------------ Set GRAM area ---------------//
//	LCD_WR_CMD(0x0050, 0x0000); // Horizontal GRAM Start Address
//	LCD_WR_CMD(0x0051, 0x00EF); // Horizontal GRAM End Address
//	LCD_WR_CMD(0x0052, 0x0000); // Vertical GRAM Start Address
//	LCD_WR_CMD(0x0053, 0x013F); // Vertical GRAM Start Address
//	LCD_WR_CMD(0x0060, 0xA700); // Gate Scan Line
//	LCD_WR_CMD(0x0061, 0x0001); // NDL,VLE, REV
//	LCD_WR_CMD(0x006A, 0x0000); // set scrolling line
////-------------- Partial Display Control ---------//
//	LCD_WR_CMD(0x0080, 0x0000);
//	LCD_WR_CMD(0x0081, 0x0000);
//	LCD_WR_CMD(0x0082, 0x0000);
//	LCD_WR_CMD(0x0083, 0x0000);
//	LCD_WR_CMD(0x0084, 0x0000);
//	LCD_WR_CMD(0x0085, 0x0000);
////-------------- Panel Control -------------------//
//	LCD_WR_CMD(0x0090, 0x0010);
//	LCD_WR_CMD(0x0092, 0x0000);
//	LCD_WR_CMD(0x0093, 0x0003);
//	LCD_WR_CMD(0x0095, 0x0110);
//	LCD_WR_CMD(0x0097, 0x0000);
//	LCD_WR_CMD(0x0098, 0x0000);
//	LCD_WR_CMD(0x0007, 0x0133); // 262K color and display ON
//	LCD_WR_CMD(32, 0);
//    LCD_WR_CMD(33, 0x013F);
//	*(__IO uint16_t *) (Bank1_LCD_C)= 34;
//#else
	//------------------------------------Display Control Setting--------------------------------------//
	LCD_WRITE_CMD(0x0001);  
	LCD_WRITE_DATA(0x0000);	// Output Direct 0100  0000|0 SM 0 SS|0000|0000		   SM=0 SS=0
	LCD_WRITE_CMD(0x0002);
	LCD_WRITE_DATA(0x0700); 	// Line Inversion
	
	LCD_WRITE_CMD(0x0003);
	LCD_WRITE_DATA(0x10B0);	// Entry Mode (65K, BGR)//0x1030   
	LCD_WRITE_CMD(0x0004);
	LCD_WRITE_DATA(0x0000);	// Set Scaling function off		
	LCD_WRITE_CMD(0x0008);
	LCD_WRITE_DATA(0x0200);	// Porch Setting202	
	LCD_WRITE_CMD(0x0009);
	LCD_WRITE_DATA(0x0000);	// Scan Cycle	
	LCD_WRITE_CMD(0x000a);
	LCD_WRITE_DATA(0x0000);	// FMARK off
//-----------------------------------End Display Control setting-----------------------------------------//
//-------------------------------- Power Control Registers Initial --------------------------------------//
	LCD_WRITE_CMD(0x0010);
	LCD_WRITE_DATA(0x0000);//Power Control1  07 90	
	LCD_WRITE_CMD(0x0011);
	LCD_WRITE_DATA(0x0007);//Power Control2	
	LCD_WRITE_CMD(0x0012);
	LCD_WRITE_DATA(0x0000);//Power Control3	
	LCD_WRITE_CMD(0x0013);
	LCD_WRITE_DATA(0x1300);//Power Control4  00 04
//---------------------------------End Power Control Registers Initial -------------------------------//
	delay_ms(60);
//--------------------------------- Power Supply Startup 1 Setting------------------------------------//
	LCD_WRITE_CMD(0x0010);
	LCD_WRITE_DATA(0x1290); //Power Control1  	1490
	delay_ms(100);	
	LCD_WRITE_CMD(0x0011);
	LCD_WRITE_DATA(0x0527); //Power Control2  		0227
	delay_ms(100);
//--------------------------------- End Power Supply Startup 1 Setting------------------------------//
	LCD_WRITE_CMD(0x0012);
	LCD_WRITE_DATA(0x0018); //Power Control3  		 1C
	LCD_WRITE_CMD(0x0013);
	LCD_WRITE_DATA(0x1000); //Power Control4  1b 00	
	LCD_WRITE_CMD(0x0029);
	LCD_WRITE_DATA(0x001E); //VCOMH setting   00 22	   1E
	LCD_WRITE_CMD(0x002b);
	LCD_WRITE_DATA(0x000D); 						//	0b
	LCD_WRITE_CMD(0x0020);
	LCD_WRITE_DATA(0x0000); 
	LCD_WRITE_DATA(0x0000); 
	LCD_WRITE_CMD(0x0021);
	LCD_WRITE_DATA(0x0000); 
	LCD_WRITE_DATA(0x0000); 
//--------------------------------- End Power Supply Startup 2 Setting------------------------------//
	delay_ms(100);
//-------------------------------------Gamma Cluster Setting-------------------------------------------//
	LCD_WRITE_CMD(0x0030);
	LCD_WRITE_DATA(0x0000);	
	LCD_WRITE_CMD(0x0031);
	LCD_WRITE_DATA(0x0407);	
	LCD_WRITE_CMD(0x0032);
	LCD_WRITE_DATA(0x0601);	
	LCD_WRITE_CMD(0x0035);
	LCD_WRITE_DATA(0x0104);	
	LCD_WRITE_CMD(0x0036);
	LCD_WRITE_DATA(0x0e06);	
	LCD_WRITE_CMD(0x0037);
	LCD_WRITE_DATA(0x0106);	
	LCD_WRITE_CMD(0x0038);
	LCD_WRITE_DATA(0x0704);	
	LCD_WRITE_CMD(0x0039);
	LCD_WRITE_DATA(0x0300);	
	LCD_WRITE_CMD(0x003c);
	LCD_WRITE_DATA(0x0401);	
	LCD_WRITE_CMD(0x003d);
	LCD_WRITE_DATA(0x060e);
	
//---------------------------------------End Gamma Setting---------------------------------------------//
//----------------------------------Display Windows 240 X 320----------------------------------------//
	LCD_WRITE_CMD(0x0050);
	LCD_WRITE_DATA(0x0000); 	// Horizontal Address Start Position
	
	LCD_WRITE_CMD(0x0051);
	LCD_WRITE_DATA(0x00ef);	// Horizontal Address End Position
	
	LCD_WRITE_CMD(0x0052);
	LCD_WRITE_DATA(0x0000);	// Vertical Address Start Position
	
	LCD_WRITE_CMD(0x0053);
	LCD_WRITE_DATA(0x013f);	// Vertical Address End Position
//----------------------------------End Display Windows 240 X 320----------------------------------//
	
	LCD_WRITE_CMD(0x0060);
	LCD_WRITE_DATA(0xa700);	// Gate scan control	GS=0
	
	LCD_WRITE_CMD(0x0061);	//
	LCD_WRITE_DATA(0x0001);	// Non-display Area setting
	LCD_WRITE_CMD(0x006a);
	LCD_WRITE_DATA(0x0000);	

//--------------- OTM3225 Panel Image Control -----------------//
	LCD_WRITE_CMD(0x80);   
	LCD_WRITE_DATA( 0x0000); // Set Partial Display 1
	LCD_WRITE_CMD(0x81);   
	LCD_WRITE_DATA( 0x0000); // Set Partial Display 1
	LCD_WRITE_CMD(0x82);   
	LCD_WRITE_DATA( 0x0000); // Set Partial Display 1
	LCD_WRITE_CMD(0x83);   
	LCD_WRITE_DATA( 0x0000); // Set Partial Display 2
	LCD_WRITE_CMD(0x84);   
	LCD_WRITE_DATA( 0x0000); // Set Partial Display 2
	LCD_WRITE_CMD(0x85);   
	LCD_WRITE_DATA( 0x0000); // Set Partial Display 2
	
	LCD_WRITE_CMD(0x0090);
	LCD_WRITE_DATA(0x0010);	// RTNI setting
	LCD_WRITE_CMD(0x0092);
	LCD_WRITE_DATA(0x0600);
	LCD_WRITE_CMD(0x0093);
	LCD_WRITE_DATA(0x0003);		
	LCD_WRITE_CMD(0x0095);
	LCD_WRITE_DATA(0x0110);
	LCD_WRITE_CMD(0x0097);
	LCD_WRITE_DATA(0x0000);
	LCD_WRITE_CMD(0x0098);
	LCD_WRITE_DATA(0x0000);										
	LCD_WRITE_CMD(0x0007);
	LCD_WRITE_DATA(0x0173);	// Display Control1
	delay_ms(100);		
}

void LCD_Set_Location(u16 i,u16 j)		//����X��Y���꣬����дRAM��׼��
{
	LCD_WRITE_CMD(0x0050);
	LCD_WRITE_DATA(i); 				//����X����λ��	
	LCD_WRITE_CMD(0x0052);
	LCD_WRITE_DATA(j);					//����Y����λ��
	LCD_WRITE_CMD(0x0022);				//ָ��RAM�Ĵ�����׼��д���ݵ�RAM
}

void LCD_Set_Area(u16 xstart,u16 ystart,u16 xend,u16 yend)
{
	LCD_WRITE_CMD(0x0052);
	LCD_WRITE_DATA(xstart); // xstart
	
	LCD_WRITE_CMD(0x0053);
	LCD_WRITE_DATA(xend); // xend
	
	LCD_WRITE_CMD(0x0050);
	LCD_WRITE_DATA(ystart); // ystart
	LCD_WRITE_CMD(0x0051);
	LCD_WRITE_DATA( yend); // yend
	
//	LCD_WRITE_CMD(0x0020);    
//	LCD_WRITE_DATA(xstart); // xstart
//	
//	LCD_WRITE_CMD(0x0021);
//	LCD_WRITE_DATA(ystart); // ystart
	
	LCD_WRITE_CMD(0x0022);
}
/****************************************************************************
* ��    �ƣ�void ili9320_Clear(u16 dat)
* ��    �ܣ�����Ļ����ָ������ɫ��������������� 0xffff
* ��ڲ�����dat      ���ֵ
* ���ڲ�������
* ˵    ����
* ���÷�����ili9320_Clear(0xffff);
****************************************************************************/
void TFT_Clear(u16 dat)
{
//	u32 i;
//	LCD_WR_CMD(0x0003,0x1018);   //������
//	LCD_WR_CMD(0x0050, 0); // Horizontal GRAM Start Address
//	LCD_WR_CMD(0x0051, 239); // Horizontal GRAM End Address
//	LCD_WR_CMD(0x0052, 0); // Vertical GRAM Start Address
//	LCD_WR_CMD(0x0053, 319); // Vertical GRAM Start Address	 
//	LCD_WR_CMD(32, 0);
//	LCD_WR_CMD(33, 0);
//	//*(__IO uint16_t *) (Bank1_LCD_C)= 34;
//	LCD_WR_REG(34);
//	for(i=0;i<76800;i++) LCD_WR_Data(dat); 
	unsigned long j;
	LCD_Set_Area(0,0,240-1,320-1);
	for(j=0;j<320*240;j++)	
		LCD_WRITE_DATA(dat); 
}
/****************************************************************************
* ��    �ƣ�void TFT_SetPoint(u16 x,u16 y,u16 point)
* ��    �ܣ���ָ�����껭��
* ��ڲ�����x      ������
*           y      ������
*           point  �����ɫ
* ���ڲ�������
* ˵    ����
* ���÷�����TFT_SetPoint(10,10,0x0fe0);
****************************************************************************/
void TFT_SetPoint(u16 x,u16 y,u16 point)
{	
	LCD_WRITE_CMD(0x52);
	LCD_WRITE_DATA(x);
	LCD_WRITE_CMD(0x50);
	LCD_WRITE_DATA(y);
	LCD_WRITE_CMD(0x22);
	//LCD_WR_Data(0xff);//����	
	LCD_WRITE_DATA(point); 
}



void LCD_Clear_Screen(u16 color)
{
	//��ʾ��ɫ���ݵ�LCD
	unsigned long j;
	LCD_Set_Area(0,0,320-1,240-1);
	for(j=0;j<320*240;j++)	
		LCD_WRITE_DATA(color);
}
void LCD_SetPixel(u16 x, u16 y, u16 color)
{
	LCD_Set_Location(x,y);
	LCD_WRITE_DATA( color);
}
/****************************************************************************
* ��    �ƣ�void TFT_PutChar(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
* ��    �ܣ���ָ��������ʾһ��8x16�����ascii�ַ�
* ��ڲ�����x          ������
*           y          ������
*           charColor  �ַ�����ɫ
*           bkColor    �ַ�������ɫ
* ���ڲ�������
* ˵    ������ʾ��Χ�޶�Ϊ����ʾ��ascii��
* ���÷�����TFT_PutChar(10,10,'a',0x0000,0xffff);
****************************************************************************/
void TFT_PutChar(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
{
	u16 i=0;
	u16 j=0;  
	u8 tmp_char=0;
	
	for (i=0;i<16;i++)
	{
		tmp_char=ascii_8x16[((c-0x20)*16)+i];
		for (j=0;j<8;j++)
		{
			TFT_SetPoint(x+j,y+i,0xffff);		//����������ԭ��������
			if ( (tmp_char >> 7-j) & 0x01 == 0x01)
			{				
				TFT_SetPoint(x+j,y+i,charColor); // �ַ���ɫ
			}
			else
			{
				TFT_SetPoint(x+j,y+i,bkColor); 	// ������ɫ
			}
		}
	}
}

/****************************************************************************
* ��    �ƣ�void TFT_PutChar12x24(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
* ��    �ܣ���ָ��������ʾһ��12x24�����ascii�ַ�
* ��ڲ�����x          ������
*           y          ������
*           charColor  �ַ�����ɫ
*           bkColor    �ַ�������ɫ
* ���ڲ�������
* ˵    ������ʾ��Χ�޶�Ϊ����ʾ��ascii��
* ���÷�����TFT_PutChar(10,10,'a',0x0000,0xffff);
****************************************************************************/
void TFT_PutChar12x24(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
{
	u16 i=0;
	u16 j=0;  
	u8 tmp_char=0;
	
	for (i=0;i<48;)
	{
		tmp_char=nAscii12x24Dot[((c-0x20)*48)+i];
		for (j=0;j<8;j++)
		{
			TFT_SetPoint(x+j,y+(i>>1),0xffff);	  		//����������ԭ��������
			if ( (tmp_char >> 7-j) & 0x01 == 0x01)
			{	
				TFT_SetPoint(x+j,y+(i>>1),charColor); 	// �ַ���ɫ
			}
			else
			{
				TFT_SetPoint(x+j,y+(i>>1),bkColor); 	// ������ɫ
			}
		}
		tmp_char=nAscii12x24Dot[((c-0x20)*48)+i+1];
		for (j=0;j<4;j++)
		{
			TFT_SetPoint(x+8+j,y+(i>>1),0xffff);	  		//����������ԭ��������
			if ( (tmp_char >> 7-j) & 0x01 == 0x01)
			{
				TFT_SetPoint(x+8+j,y+(i>>1),charColor); // �ַ���ɫ
			}
			else
			{
				TFT_SetPoint(x+8+j,y+(i>>1),bkColor); 	// ������ɫ
			}
		}
		i=i+2;
	}
}
/****************************************************************************
* ��    �ƣ�void GUI_Text(u16 x, u16 y, u8 *str, u16 len,u16 Color, u16 bkColor)
* ��    �ܣ���ָ��������ʾ�ַ���
* ��ڲ�����x      ������
*           y      ������
*           *str   �ַ���
*           len    �ַ�������
*           Color  �ַ���ɫ
*           bkColor�ַ�������ɫ
* ���ڲ�������
* ˵    ����
* ���÷�����GUI_Text(0,0,"0123456789",10,0x0000,0xffff);
****************************************************************************/
void GUI_Text(u16 x, u16 y, u8 *str, u16 len,u16 Color, u16 bkColor)
{
	u8 i;	
	for (i=0;i<len;i++)
	{
		TFT_PutChar((x+8*i),y,*str++,Color,bkColor);
	}
}
/****************************************************************************
* ��    �ƣ�void GUI_Text12x24(u16 x, u16 y, u8 *str, u16 len,u16 Color, u16 bkColor)
* ��    �ܣ���ָ��������ʾ�ַ���
* ��ڲ�����x      ������
*           y      ������
*           *str   �ַ���
*           len    �ַ�������
*           Color  �ַ���ɫ
*           bkColor�ַ�������ɫ
* ���ڲ�������
* ˵    ����
* ���÷�����GUI_Text(0,0,"0123456789",10,0x0000,0xffff);
****************************************************************************/
void GUI_Text12x24(u16 x, u16 y, u8 *str, u16 len,u16 Color, u16 bkColor)
{
	u8 i;	
	for (i=0;i<len;i++)
	{
		TFT_PutChar12x24((x+12*i),y,*str++,Color,bkColor);
	}
}


//��ʾ����
void LCD_test(void)
{
	unsigned int tmp;
	
	//ili9320_Clear(0xffff);

	GUI_Text(0,0,"Hello world.",12,0x0000,0xf800);
	//GUI_Text12x24(0,50,"Hello world.",12,0x0000,0x001f);
	tmp=color1/10000;
	TFT_PutChar(0,100,tmp+0x30,0x0000,0xffff);
	TFT_PutChar12x24(0,150,tmp+0x30,0x0000,0xffff);
	  
	tmp=(color1%10000)/1000;
	TFT_PutChar(8,100,tmp+0x30,0x0000,0xffff);
	TFT_PutChar12x24(12,150,tmp+0x30,0x0000,0xffff);
 
	tmp=((color1%10000)%1000)/100;
	TFT_PutChar(16,100,tmp+0x30,0x0000,0xffff);
	TFT_PutChar12x24(24,150,tmp+0x30,0x0000,0xffff);
 
	tmp=(((color1%10000)%1000)%100)/10;
	TFT_PutChar(24,100,tmp+0x30,0x0000,0xffff);
	TFT_PutChar12x24(36,150,tmp+0x30,0x0000,0xffff);
	tmp=color1%10;
	TFT_PutChar(32,100,tmp+0x30,0x0000,0xffff); 
	TFT_PutChar12x24(48,150,tmp+0x30,0x0000,0xffff);
	color1++; 
	if(color1==65536) color1=0;  			
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
