		  
#ifndef __LCD_DIS24_H__
#define __LCD_DIS24_H__

void LCD_Init(void);
void LCD_test(void);

void LCD_Clear_Screen(u16 color);
void TFT_SetPoint(u16 x,u16 y,u16 point);
void TFT_PutChar(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor);
void TFT_PutChar12x24(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor);
void GUI_Text(u16 x, u16 y, u8 *str, u16 len,u16 Color, u16 bkColor);
void GUI_Text12x24(u16 x, u16 y, u8 *str, u16 len,u16 Color, u16 bkColor);
void LCD_Color_Show(void);
void LCD_WRITE_CMD(u16 index);
void LCD_WRITE_DATA(u16 Data);

#endif

