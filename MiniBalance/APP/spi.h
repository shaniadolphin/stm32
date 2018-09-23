#ifndef _SPI_H_
#define _SPI_H_

#define Select_DA()     	GPIO_ResetBits(GPIOE, GPIO_Pin_0)
/* Deselect SPI FLASH: ChipSelect pin high */
#define NotSelect_DA()    	GPIO_SetBits(GPIOE, GPIO_Pin_0)

#define Select_RF()     	GPIO_ResetBits(GPIOB, GPIO_Pin_12)
/* Deselect SPI FLASH: ChipSelect pin high */
#define NotSelect_RF()    	GPIO_SetBits(GPIOB, GPIO_Pin_12)

#define Enable_RF()		GPIO_ResetBits(GPIOA, GPIO_Pin_8)
#define Disable_RF()	GPIO_SetBits(GPIOA, GPIO_Pin_8)

void SPI_DEV_Init(void);
unsigned short SPI_ReadData(unsigned short Dummy_Byte);
unsigned short SPI_SendData(unsigned short data);
unsigned char SPI_Send_Byte(unsigned char byte);


#endif
