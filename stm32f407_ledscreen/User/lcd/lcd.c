#include "lcd.h"
#include "FONT.h"
#include "GB1616.h"	//16*16汉字字模
#include "asc8x16.h"
/*
 * 函数名：LCD_GPIO_Config
 * 描述  ：根据FSMC配置LCD的I/O
 * 输入  ：无
 * 输出  ：无
 * 调用  ：内部调用        
 */
//串行外设接口SPI的初始化，SPI配置成主模式							  
//本例程选用SPI1对NRF24L01进行读写操作，先对SPI1进行初始化
void SPI1_Init(void)
{	 
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_AHB1Periph_GPIOA, ENABLE );	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	//SPI1口初始化
	/* Configure SPI1 pins: SCK, MISO and MOSI */
	RCC_AHB1PeriphClockCmd(SPI_LCD_CS_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = SPI_LCD_CS_PIN | SPI_LCD_DC_PIN | SPI_LCD_RST_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(SPI_LCD_CS_PORT, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(SPI_LCD_SCK_GPIO_CLK | SPI_LCD_MISO_GPIO_CLK | SPI_LCD_MOSI_GPIO_CLK,  ENABLE);
	RCC_APB2PeriphClockCmd(SPI_LCD_CLK, ENABLE); 

	GPIO_PinAFConfig(SPI_LCD_SCK_GPIO_PORT, SPI_LCD_SCK_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(SPI_LCD_MISO_GPIO_PORT, SPI_LCD_MISO_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(SPI_LCD_MOSI_GPIO_PORT, SPI_LCD_MOSI_SOURCE, GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Pin = SPI_LCD_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(SPI_LCD_SCK_GPIO_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = SPI_LCD_MISO_PIN;
	GPIO_Init(SPI_LCD_MISO_GPIO_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = SPI_LCD_MOSI_PIN;
	GPIO_Init(SPI_LCD_MOSI_GPIO_PORT, &GPIO_InitStructure);
	
	/* SPI1 configuration */                                            //初始化SPI1结构体
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI1设置为两线全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		                //设置SPI1为主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		            //SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		                    //串行时钟在不操作时，时钟为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	                    //第一个时钟沿开始采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		                    //NSS信号由软件（使用SSI位）管理
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;  //SPI波特率预分频值为8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	                //数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	                        //CRC值计算的多项式

	SPI_Init(SPI1, &SPI_InitStructure);                                 //根据SPI_InitStruct中指定的参数初始化外设SPI1寄存器
	
	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);                                              //使能SPI1外设

}  
/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte 
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
unsigned short SPI1_Send_Byte(unsigned short  byte)
{
	/* Loop while DR register in not emplty */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	/* Send byte through the SPI2 peripheral */
	//SPI_I2S_SendData(SPI2, data);
	SPI1->DR = byte;	
	/* Wait to receive*/
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);	
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);
}

volatile static void Delay(__IO u32 nCount)
{	
	volatile int i;
	for(i=0;i<720;i++)
    for(; nCount != 0; nCount--);
}  
  
void WriteComm(unsigned short CMD)
{			
	SPI_LCD_RS(0);
	SPI1_Send_Byte(CMD);
}
void WriteData(unsigned short tem_data)
{			
	SPI_LCD_RS(1);
	SPI1_Send_Byte(tem_data);
}

/**********************************************
Lcd初始化函数
Initial condition  (DB0-15,RS,CSB,WRD,RDB,RESETB="L") 
***********************************************/
void Lcd_Initialize(void)
{	
	SPI1_Init();
	SPI_LCD_RST(0);
	Delay(1000);
	SPI_LCD_RST(1);	
	SPI_LCD_CS(1);
	////////////////////HX8357C+TM3.2////////////
	WriteComm(0x0001);
	Delay(100);
	SPI_LCD_CS(0);
	WriteComm(0x0011);
	Delay(20);
	//************* Start Initial Sequence **********//
	WriteComm(0xC0);
	WriteData(0x0A); // P-Gamma level
	WriteData(0x0A); // N-Gamma level
	WriteComm(0xC1); // BT & VC Setting
	WriteData(0x41);
	WriteData(0x07); // VCI1 = 2.5V
	WriteComm(0xC2); // DC1.DC0 Setting
	WriteData(0x33);
	WriteComm(0xC5);
	WriteData(0x00);
	WriteData(0x42); // VCM Setting
	WriteData(0x80); // VCM Register Enable
	WriteComm(0xB1);
	WriteData(0xB0); // Frame Rate Setting
	WriteData(0x11);
	WriteComm(0xB4);
	WriteData(0x00); // Frame Rate Setting
	WriteComm(0xB6); // RM.DM Setting
	WriteData(0x00);
	WriteData(0x02);
	WriteData(0x3B);
	WriteComm(0xB7); // Entry Mode
	WriteData(0x07);
	WriteComm(0xF0); // Enter ENG , must be set before gamma setting
	WriteData(0x36);
	WriteData(0xA5);
	WriteData(0xD3);
	WriteComm(0xE5); // Open gamma function , must be set before gamma setting
	WriteData(0x80);
	WriteComm(0xE5); // Page 1
	WriteData(0x01);
	WriteComm(0XB3); // WEMODE=0(Page 1) , pixels over window setting will be ignored.
	WriteData(0x00);
	WriteComm(0xE5); // Page 0
	WriteData(0x00);
	WriteComm(0xF0); // Exit ENG , must be set before gamma setting
	WriteData(0x36);
	WriteData(0xA5);
	WriteData(0x53);
	WriteComm(0xE0); // Gamma setting
	WriteData(0x00);
	WriteData(0x35);
	WriteData(0x33);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x35);
	WriteData(0x33);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);
	WriteComm(0x36); // Color filter setting
	WriteData(0x08);
	WriteComm(0xEE);
	WriteData(0x00);
	WriteComm(0x3A); // interface setting
	WriteData(0x55);
	WriteComm(0x11); // Exit sleep mode
	WriteComm(0x20);
	WriteComm(0x29); // Display on 

	Delay(10);
	Lcd_ColorBox(0, 0, 480, 320, BLACK);
	{
		WriteComm(0xC0);   //****** *******
		WriteData(0x10);   //REV & SM & GS
		WriteComm(0x36);   //Top to Bottom / Left to Right / Row column exchange / LCD Refresh Top to Bottom / BGR mode 
		WriteData(0x28);
	}
}

void test_color(void)
{
	unsigned short i,j;
	BlockWrite(0,319,0,479);
	for(i=0;i<480;i++)
	{	for(j=0;j<320;j++)
		{
			if(j<40)
			{
				WriteData(BLACK>>8);
				WriteData(BLACK); 
			}
			else if(j<80)
			{
				WriteData(BLUE>>8);
				WriteData(BLUE);
			}
			else if(j<120)
			{
				WriteData(BRED>>8);
				WriteData(BRED);
			}
			else if(j<160)
			{
				WriteData(GRED>>8);
				WriteData(GRED);
			}
			else if(j<200)
			{
				WriteData(RED>>8);
				WriteData(RED);
			}
			else if(j<240)
			{
				WriteData(GREEN>>8);
				WriteData(GREEN);
			}
			else if(j<280)
			{
				WriteData(YELLOW>>8);
				WriteData(YELLOW);
			}
			else if(j<320)
			{
				WriteData(BROWN>>8);
				WriteData(BROWN);
			}
		}
	}
}

/**********************************************
函数名：Lcd块选函数
功能：选定Lcd上指定的矩形区域

注意：xStart、yStart、Xend、Yend随着屏幕的旋转而改变，位置是矩形框的四个角

入口参数：xStart x方向的起始点
          ySrart y方向的起始点
          Xend   y方向的终止点
          Yend   y方向的终止点
返回值：无
***********************************************/
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) 
{
	//HX8357-C

	WriteComm(0x2A);
	WriteData(Xstart>>8);
	WriteData(Xstart);
	WriteData(Xend>>8);
	WriteData(Xend);

	WriteComm(0x2B);
	WriteData(Ystart>>8);
	WriteData(Ystart);
	WriteData(Yend>>8);
	WriteData(Yend);

	WriteComm(0x2C);
}
/**********************************************
函数名：Lcd块选函数
功能：选定Lcd上指定的矩形区域

注意：xStart和 yStart随着屏幕的旋转而改变，位置是矩形框的四个角

入口参数：xStart x方向的起始点
          ySrart y方向的终止点
          xLong 要选定矩形的x方向长度
          yLong  要选定矩形的y方向长度
返回值：无
***********************************************/
void Lcd_ColorBox(u16 xStart,u16 yStart,u16 xLong,u16 yLong,u16 Color)
{
	u32 temp;

	BlockWrite(xStart,xStart+xLong-1,yStart,yStart+yLong-1);
	for (temp=0; temp<xLong*yLong; temp++)
	{
		WriteData(Color>>8);
		WriteData(Color);
	}
}

/******************************************
函数名：Lcd图像填充100*100
功能：向Lcd指定位置填充图像
入口参数：Index 要寻址的寄存器地址
          ConfigTemp 写入的数据或命令值
******************************************/
void LCD_Fill_Pic(u16 x, u16 y,u16 pic_H, u16 pic_V, const unsigned char* pic)
{
	unsigned long i;
	//unsigned int j;

// 	WriteComm(0x36); //Set_address_mode
// 	WriteData(0x0a); //横屏，从左下角开始，从左到右，从下到上
	BlockWrite(x,x+pic_H-1,y,y+pic_V-1);
	
	for (i = 0; i < pic_H*pic_V*2; i++)
	{
// 		WriteData(pic[i]);
		SPI1_Send_Byte(1);	
		SPI_I2S_SendData(SPI1, pic[i]);
	}
// 	WriteComm(0x36); //Set_address_mode
// 	WriteData(0xaa);
}

void DrawPixel(u16 x, u16 y, u16 Color)
{
	BlockWrite(x,x,y,y);
// 	WriteComm(0x200);   
// 	WriteData(x);
// 	WriteComm(0x201);   
// 	WriteData(y);
// 	WriteComm(0x202);
	WriteData(Color>>8);
	WriteData(Color & 0x00ff);
// 	*(__IO u16 *) (Bank1_LCD_D) = Color;
}

void PutGB1616(unsigned short x, unsigned short  y, unsigned char c[2], unsigned int fColor,unsigned int bColor,unsigned char flag)
{
	unsigned char i,j,k,m;
	for (k=0;k<64;k++) 
	{ //64标示自建汉字库中的个数，循环查询内码
		if ((codeGB_16[k].Index[0]==c[0])&&(codeGB_16[k].Index[1]==c[1]))
		{ 
			for(i=0;i<32;i++) 
			{
				m=codeGB_16[k].Msk[i];
				for(j=0;j<8;j++) 
				{		
					if((m&0x80)==0x80) 
					{
						DrawPixel(x+j,y,fColor);
					}
					else 
					{
						if(flag) DrawPixel(x+j,y,bColor);
					}
					m=m<<1;
				} 
				if(i%2)
				{
					y++;
					x=x-8;
				}
				else 
					x=x+8;
			}
		}  
	}	
}
/*****************************************************************************
 函 数 名  : LED_P8x16Str
 功能描述  : 写入一组8x16标准ASCII字符串
 输入参数  :  x       为显示的横坐标0~480
             y       为页范围0～320
             ch  要显示的字符
						fColor 
 输出参数  : NONE
 返 回 值  : NONE
*****************************************************************************/
void PutAsc0816(unsigned short x, unsigned short  y, unsigned char ch, unsigned int fColor,unsigned int bColor,unsigned char flag)
{
	unsigned char i,j,m;
	unsigned char ucDataTmp;
	ucDataTmp = ch - 32;
#if 0
	for(i=0;i<16;i++) 
	{
		m = nAsciiDot[ucDataTmp * 16 + i];
		for(j = 0; j < 8; j++) 
		{		
			if((m & 0x80) == 0x80) 
			{
				DrawPixel(x + j, y, fColor);
			}
			else 
			{
				if(flag) DrawPixel(x + j, y, bColor);
			}
			m = m<<1;
		} 
		y++;
	} 
#else
	//Lcd_ColorBox(x, y, 8, 16, fColor);
	BlockWrite(x, x + 8 - 1, y, y + 16 - 1);
	//BlockWrite(x, y, 8, 16);
	for(i=0;i<16;i++) 
	{
		m = nAsciiDot[ucDataTmp * 16 + i];
		for(j = 0; j < 8; j++) 
		{	
			if((m & 0x80) == 0x80) 
			{
				WriteData(fColor>>8);
				WriteData(fColor & 0x00ff);
			}
			else 
			{
				if(flag)
				{					
					WriteData(bColor>>8);
					WriteData(bColor & 0x00ff);
				}
				else
				{
					WriteData(BLACK>>8);
					WriteData(BLACK & 0x00ff);					
				}
			}
			m = m<<1;
		} 
		y++;
	} 
#endif	
}

void LCD_PutString(unsigned short x, unsigned short y, char *s, unsigned int fColor, unsigned int bColor,unsigned char flag) 
{
	unsigned char l=0;
	while(*s) 
	{
		PutGB1616(x+l*8,y,(unsigned char*)s,fColor,bColor,flag);
		s+=2;
		l+=2;
	}
}

void LCD_P8x16Str(unsigned short x, unsigned short y, char *s, unsigned int fColor, unsigned int bColor,unsigned char flag) 
{
	unsigned char l = 0;
	unsigned char len = 0;
	unsigned char data[1];
	while(s[len] != '\0') 
	{
		data[0] = s[len];
		PutAsc0816(x + l * 8, y, data[0], fColor, bColor, flag);
		len ++;
		l ++;
	}
}


void LCD_Printint(unsigned short ucIdxX, unsigned short ucIdxY, int cData, unsigned int fColor, unsigned int bColor,unsigned char flag)
{
    unsigned int i, j, k, l, m;
    unsigned int usData;

    if(cData < 0)
    {
        //LED_P6x8Char(ucIdxX, ucIdxY, '-');
        PutAsc0816(ucIdxX, ucIdxY, '-', fColor, bColor, flag);
        usData = (unsigned int)(-cData);  
    }
    else
    {
        PutAsc0816(ucIdxX, ucIdxY, ' ', fColor, bColor, flag);
        usData = (unsigned int)cData;
    }
    l = usData / 10000;
    m = (usData % 10000) /1000;
    i = (usData % 1000) / 100;
    j = (usData % 100) / 10;
    k = usData % 10;
#if 1  
    if(l ==0)
    {   
        if(m ==0)
        {
            if(i ==0)
            {
                if(j ==0) PutAsc0816(ucIdxX+32, ucIdxY, ' ', fColor, bColor, flag);//zero_cnt |=0x08;
                else PutAsc0816(ucIdxX+32, ucIdxY, j+48, fColor, bColor, flag);
                PutAsc0816(ucIdxX+24, ucIdxY, ' ', fColor, bColor, flag);//zero_cnt |=0x04;
            } 
            else
            {
                PutAsc0816(ucIdxX+24, ucIdxY, i+48, fColor, bColor, flag);  
                PutAsc0816(ucIdxX+32, ucIdxY, j+48, fColor, bColor, flag);
            }
            PutAsc0816(ucIdxX+16, ucIdxY, ' ', fColor, bColor, flag);//zero_cnt |= 0x02;
        }
        else 
        {
            PutAsc0816(ucIdxX+16, ucIdxY, m+48, fColor, bColor, flag);
            PutAsc0816(ucIdxX+24, ucIdxY, i+48, fColor, bColor, flag);  
            PutAsc0816(ucIdxX+32, ucIdxY, j+48, fColor, bColor, flag);            
        }
        PutAsc0816(ucIdxX+8, ucIdxY, ' ', fColor, bColor, flag);//zero_cnt |= 0x01;
    }
    else 
    {      
        PutAsc0816(ucIdxX+8, ucIdxY, l+48, fColor, bColor, flag);
        PutAsc0816(ucIdxX+16, ucIdxY, m+48, fColor, bColor, flag);
        PutAsc0816(ucIdxX+24, ucIdxY, i+48, fColor, bColor, flag);  
        PutAsc0816(ucIdxX+32, ucIdxY, j+48, fColor, bColor, flag); 
    }
    PutAsc0816(ucIdxX+40, ucIdxY, k+48, fColor, bColor, flag);
#else
    if(l ==0)
    {   
        if(m ==0)
        {
            if(i ==0)
            {
                if(j ==0) zero_cnt |=0x08;
                zero_cnt |=0x04;
            } 
            zero_cnt |= 0x02;
        }
        zero_cnt |= 0x01;
    }
    else LCD_P8x16Char(ucIdxX+8, ucIdxY, l+48);
    LCD_P8x16Char(ucIdxX+40, ucIdxY, k+48);    
    if(zero_cnt & 0x01)LCD_P8x16Char(ucIdxX+8, ucIdxY, ' ');
    else LCD_P8x16Char(ucIdxX+8, ucIdxY, l+48);
    if(zero_cnt & 0x02)LCD_P8x16Char(ucIdxX+16, ucIdxY, ' ');
    else LCD_P8x16Char(ucIdxX+16, ucIdxY, m+48);
    if(zero_cnt & 0x04)LCD_P8x16Char(ucIdxX+24, ucIdxY, ' ');
    else LCD_P8x16Char(ucIdxX+24, ucIdxY, i+48);  
    if(zero_cnt & 0x08)LCD_P8x16Char(ucIdxX+32, ucIdxY, ' ');
    else LCD_P8x16Char(ucIdxX+32, ucIdxY, j+48);
    LCD_P8x16Char(ucIdxX+40, ucIdxY, k+48);
#endif
    return;
}

typedef __packed struct
{
	u8  pic_head[2];				//1
	u16 pic_size_l;			    //2
	u16 pic_size_h;			    //3
	u16 pic_nc1;				    //4
	u16 pic_nc2;				    //5
	u16 pic_data_address_l;	    //6
	u16 pic_data_address_h;		//7	
	u16 pic_message_head_len_l;	//8
	u16 pic_message_head_len_h;	//9
	u16 pic_w_l;					//10
	u16 pic_w_h;				    //11
	u16 pic_h_l;				    //12
	u16 pic_h_h;				    //13	
	u16 pic_bit;				    //14
	u16 pic_dip;				    //15
	u16 pic_zip_l;			    //16
	u16 pic_zip_h;			    //17
	u16 pic_data_size_l;		    //18
	u16 pic_data_size_h;		    //19
	u16 pic_dipx_l;			    //20
	u16 pic_dipx_h;			    //21	
	u16 pic_dipy_l;			    //22
	u16 pic_dipy_h;			    //23
	u16 pic_color_index_l;	    //24
	u16 pic_color_index_h;	    //25
	u16 pic_other_l;			    //26
	u16 pic_other_h;			    //27
	u16 pic_color_p01;		    //28
	u16 pic_color_p02;		    //29
	u16 pic_color_p03;		    //30
	u16 pic_color_p04;		    //31
	u16 pic_color_p05;		    //32
	u16 pic_color_p06;		    //33
	u16 pic_color_p07;		    //34
	u16 pic_color_p08;			//35			
}BMP_HEAD;

BMP_HEAD bmp;

