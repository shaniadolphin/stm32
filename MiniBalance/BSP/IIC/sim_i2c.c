/*********************************Copyright (c)*********************************
**                               
**
**--------------File Info-------------------------------------------------------
** File Name:               sim_i2c.c
** Last modified Date:      
** Last Version:            
** Description:             ģ��I2C�ӿ�(Ĭ��100kbps)
**
**------------------------------------------------------------------------------
** Created By:              wanxuncpx
** Created date:            
** Version:                 
** Descriptions:            
**
*******************************************************************************/

/******************************************************************************
����˵��:
    
******************************************************************************/

/******************************************************************************
*********************************  Ӧ �� �� �� ********************************
******************************************************************************/

/******************************************************************************
********************************* �ļ����ò��� ********************************
******************************************************************************/
#include "sim_i2c.h"


/******************************************************************************
******************************* �Զ���������� ********************************
******************************************************************************/


/******************************************************************************
********************************* �� �� �� �� *********************************
******************************************************************************/
/*---------------------* 
*    IMPORT:�����ṩ   * 
*----------------------*/
//none

/*---------------------* 
*    EXPORT:�����ṩ   * 
*----------------------*/
//none

/******************************************************************************
********************************* �� �� �� �� *********************************
******************************************************************************/
char  test=0;
/*---------------------* 
*    IMPORT:�����ṩ   * 
*----------------------*/
//none

/*---------------------* 
*    EXPORT:�����ṩ   * 
*----------------------*/
//none



/******************************************************************************
*********************************  ����ʼ  **********************************
******************************************************************************/
/******************************************************************************
/ ��������:ms����ʱ����
/ �޸�����:none
/ �������:none
/ �������:none
/ ʹ��˵��:none
******************************************************************************/
void Delayms(uint32_t time)
{
   uint16_t i=0;  
   while(time--)
   {
      i=8000;
      while(i--);
   }  
}

/******************************************************************************
/ ��������:I2C�ӿ����ó���
/ �޸�����:none
/ �������:none
/ �������:none
/ ʹ��˵��:none
******************************************************************************/
void I2C_Ini(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 
    
  //�ر�JTAGʹ��SWD���ͷ�IO��
  uint32_t u32Temp;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB , ENABLE);
  u32Temp = AFIO->MAPR;
  u32Temp &= ~AFIO_MAPR_SWJ_CFG ;
  u32Temp |= AFIO_MAPR_SWJ_CFG_1;
  AFIO->MAPR = u32Temp;
  
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB , ENABLE);
  //GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);
  
  //����I2C��SCL���ź�SDA����
  RCC_APB2PeriphClockCmd(AHRS_I2C_GPIO_CLK , ENABLE);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin =  AHRS_I2C_SclPin;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_InitStructure.GPIO_Pin =   AHRS_I2C_SdaPin;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  SCL_H();SDA_H();    //�����ͷ�
  
  //��������IO?
//  RCC_APB2PeriphClockCmd(LED1_GPIO_CLK | LED2_GPIO_CLK , ENABLE);
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//  GPIO_InitStructure.GPIO_Pin =  LED1_GPIO_PIN;
//  GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);
//  GPIO_InitStructure.GPIO_Pin =  LED2_GPIO_PIN;
//  GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);
//  
//  //��������
//  RCC_APB2PeriphClockCmd(KEY_GPIO_CLK, ENABLE);
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
//  GPIO_InitStructure.GPIO_Pin =  KEY_GPIO_PIN;
//  GPIO_Init(KEY_GPIO_PORT, &GPIO_InitStructure);
}

/******************************************************************************
/ ��������:Simulation IIC Timing series delay
/ �޸�����:none
/ �������:none
/ �������:none
/ ʹ��˵��:none
******************************************************************************/
__inline void I2C_delay(void)
{
        
   u8 i = I2C_DELAY_VAL; //��������Ż��ٶ�   ����������͵�5����д��
   while(i) 
   { 
     i--; 
   }  
}


/******************************************************************************
/ ��������:��ʱ5msʱ��
/ �޸�����:none
/ �������:none
/ �������:none
/ ʹ��˵��:none
******************************************************************************/
void delay5ms(void)
{
   int i=5000;  
   while(i) 
   { 
     i--; 
   }  
}

/*******************************************************************************
* Function Name  : I2C_Start
* Description    : Master Start Simulation IIC Communication
* Input          : None
* Output         : None
* Return         : Wheather  Start
* ��SCL�Ǹߵ�ƽʱSDA�ߵ�ƽ��͵�ƽ�л�
****************************************************************************** */
unsigned char I2C_Start(void)
{
    I2C_delay();    
    SCL_H(); 
    if(SDA_read()  == 0)return IIC_STATE_BUSY;  //SDA��Ϊ�͵�ƽ������æ,�˳�
    if(SCL_read()  == 0)return IIC_STATE_BUSY;  //SDA��Ϊ�͵�ƽ������æ,�˳�         
    SDA_L();
    I2C_delay();    
    SDA_H();
    I2C_delay();    
    SDA_L();		// I2C Start Condition    
    return 0;   
}
/*******************************************************************************
* Function Name  : I2C_Start
* Description    : Master Start Simulation IIC Communication
* Input          : None
* Output         : None
* Return         : Wheather  Start
* ��SCL�Ǹߵ�ƽʱSDA�ߵ�ƽ��͵�ƽ�л�
****************************************************************************** */
unsigned char I2C_Restart(void)
{
    I2C_delay();    
    SCL_L();
    SDA_H();
    I2C_delay();    
    SCL_H();
    if(SDA_read()  ==0 )return IIC_STATE_BUSY;  //SDA��Ϊ�͵�ƽ������æ,�˳�
    if(SCL_read()  ==0 )return IIC_STATE_BUSY;  //SDA��Ϊ�͵�ƽ������æ,�˳�      
    I2C_delay();  
    SDA_L();  
    return 0;    
}
/*******************************************************************************
* Function Name  : I2C_Stop
* Description    : Master Stop Simulation IIC Communication
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_Stop(void)
{
    I2C_delay();      
    SCL_L();
    SDA_L();
    I2C_delay();    
    SCL_H();
    SDA_H();  
} 
/*******************************************************************************
* Function Name  : I2C_Ack
* Description    : Master Send Acknowledge Single
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_Ack(void)
{   
	SCL_L();
    I2C_delay();
    SDA_L();
    I2C_delay();
    SCL_H();
    I2C_delay();
    SCL_L();
    I2C_delay();   
}   
/*******************************************************************************
* Function Name  : I2C_NoAck
* Description    : Master Send No Acknowledge Single
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_NoAck(void)
{   
    SDA_H();
    I2C_delay();    
    SCL_H(); 
} 
/*******************************************************************************
* Function Name  : I2C_WaitAck
* Description    : Master Reserive Slave Acknowledge Single
* Input          : None
* Output         : None
* Return         : Wheather  Reserive Slave Acknowledge Single
****************************************************************************** */
unsigned char I2C_WaitAck(void)   //����Ϊ:=1��ACK,=0��ACK
{

    SDA_H();					// SDA Input
    I2C_delay();      
    SCL_H();					// Test Acknowledge
    if(SDA_read())return 0;
    else return 1;				// return ok if got acknowledge from slave device    
}
/*******************************************************************************
* Function Name  : I2C_SendByte
* Description    : Master Send a Byte to Slave
* Input          : Will Send Date
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_SendByte(u8 SendByte) //���ݴӸ�λ����λ//
{
    unsigned char mask = 0x80;
    I2C_delay();    
    SCL_L();
    for (mask = 0x80; mask != 0; mask >>= 1)
    {

        if (SendByte & mask)
        {
            SDA_H();
        }
        else
        {
            SDA_L();
        }
        I2C_delay();
        SCL_H();
        I2C_delay();
        SCL_L();
    }
}  
/*******************************************************************************
* Function Name  : I2C_RadeByte
* Description    : Master Reserive a Byte From Slave
* Input          : None
* Output         : None
* Return         : Date From Slave 
****************************************************************************** */
uint8_t I2C_ReadByte(void)  //���ݴӸ�λ����λ//
{ 
    unsigned char TempData = 0;
    unsigned char i;

    SDA_H(); 
    I2C_delay();
    SCL_L();
    for (i=0; i<8; i++)
    {
        I2C_delay();       
        SCL_H();
        TempData <<= 1;
        if (SDA_read())
        {
            TempData++;
        }
        I2C_delay();   
        SCL_L();
    }
    return TempData;  
} 
/******************************************************************************
/ ��������:���ֽ�д��
/ �޸�����:none
/ �������:
/   @arg SlaveAddress   ��������ַ
/   @arg REG_Address    �Ĵ�����ַ
/   @arg REG_data       ��д����ֽ�����
/ �������: �������ֽ�����
/ ʹ��˵��:��ʱһ�������ĵ��ֽڶ�ȡ����
******************************************************************************/
unsigned char Single_Write(uint8_t slave_addr,uint8_t word_addr,uint8_t pword_value)
{
    unsigned char i2c_error = 0;
	i2c_error = I2C_Start();					// write start sequence 
    I2C_SendByte(slave_addr <<1);			// write ALS Address byte w/ WRITE command 
    i2c_error = I2C_WaitAck();
    if(i2c_error ==0){I2C_Stop(); return IIC_STATE_NOACK;}
    I2C_SendByte(word_addr);			// write ASL Address Byte 
    i2c_error = I2C_WaitAck();
    if(i2c_error ==0){I2C_Stop(); return IIC_STATE_NOACK;}
    I2C_SendByte(pword_value);			// Data byte
    i2c_error = I2C_WaitAck();
    if(i2c_error ==0){I2C_Stop(); return IIC_STATE_NOACK;}    
    I2C_Stop(); 
    return 0;
}

/******************************************************************************
/ ��������:���ֽ�д��
/ �޸�����:none
/ �������:
/   @arg SlaveAddress   ��������ַ
/   @arg REG_Address    �Ĵ�����ַ
/ �������: �������ֽ�����
/ ʹ��˵��:��ʱһ�������ĵ��ֽڶ�ȡ����
******************************************************************************/
unsigned char Single_Read(unsigned char slave_addr, unsigned char word_addr, unsigned char *pword_value)
{   
    unsigned char i2c_error = 0;  

    i2c_error = I2C_Start();					// write start sequence 
    //if(i2c_error !=0){I2C_Stop(); return i2c_error;}
    I2C_SendByte(slave_addr <<1);			// write ALS Address byte w/ WRITE command 
    i2c_error = I2C_WaitAck();
    if(i2c_error ==0){I2C_Stop(); return IIC_STATE_NOACK;}  
    I2C_SendByte(word_addr);			// set ALS's I2C address 
    i2c_error = I2C_WaitAck();
    if(i2c_error ==0){I2C_Stop(); return IIC_STATE_NOACK;}  
    i2c_error = I2C_Restart();	
    //i2c_error = I2C_Start();	
    // write start sequence 
    I2C_SendByte((slave_addr <<1) +1);		// write ALS Address byte w/ READ command 
    i2c_error = I2C_WaitAck();
    if(i2c_error ==0){I2C_Stop(); return IIC_STATE_NOACK;}  
    *pword_value = I2C_ReadByte();		// read 8 bit data
    I2C_NoAck();			// send NON-ACK to Slave device
    I2C_Stop();
    return 0;  
}

/******************************************************************************
/ ��������:���ֽڶ�������
/ �޸�����:none
/ �������:
/   @arg SlaveAddress   ��������ַ
/   @arg REG_Address    �Ĵ�����ַ
/   @arg ptChar         �������
/   @arg size           ���������ݸ���,size�������=1
/ �������: �ɹ�ʧ�ܱ��
/ ʹ��˵��:none
******************************************************************************/
unsigned char Mult_Read(uint8_t slave_addr,uint8_t word_addr,uint8_t *ptChar,uint8_t size)
{
    unsigned char i2c_error = 0;
	unsigned char i = 0;
	i2c_error = I2C_Start();					// write start sequence 
    I2C_SendByte(slave_addr <<1);			// write ALS Address byte w/ WRITE command 
    i2c_error = I2C_WaitAck();
    if(i2c_error ==0){I2C_Stop(); return IIC_STATE_NOACK;}
    I2C_SendByte(word_addr);			// write ASL Address Byte 
    i2c_error = I2C_WaitAck();
    if(i2c_error ==0){I2C_Stop(); return IIC_STATE_NOACK;}
    
	i2c_error = I2C_Restart();					// write start sequence 
    
	I2C_SendByte((slave_addr <<1) +1);		// write ALS Address byte w/ READ command 
    i2c_error = I2C_WaitAck();
    if(i2c_error ==0){I2C_Stop(); return IIC_STATE_NOACK;}  
    
    //��������ax,ay,az����
    //for(i=1; i < size; i++)
    //{
    //    *ptChar++ = I2C_ReadByte();
    //    I2C_Ack();
    //}
	 while (size) 
	 {
        *ptChar = I2C_ReadByte();
        if (size == 1)
            I2C_NoAck();
        else
            I2C_Ack();
        ptChar++;
        size--;
    }
    //*ptChar++ = I2C_ReadByte();
    //I2C_NoAck();
    I2C_Stop();
    return 0;    
}




