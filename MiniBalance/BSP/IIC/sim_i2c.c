/*********************************Copyright (c)*********************************
**                               
**
**--------------File Info-------------------------------------------------------
** File Name:               sim_i2c.c
** Last modified Date:      
** Last Version:            
** Description:             模拟I2C接口(默认100kbps)
**
**------------------------------------------------------------------------------
** Created By:              wanxuncpx
** Created date:            
** Version:                 
** Descriptions:            
**
*******************************************************************************/

/******************************************************************************
更新说明:
    
******************************************************************************/

/******************************************************************************
*********************************  应 用 资 料 ********************************
******************************************************************************/

/******************************************************************************
********************************* 文件引用部分 ********************************
******************************************************************************/
#include "sim_i2c.h"


/******************************************************************************
******************************* 自定义参数配置 ********************************
******************************************************************************/


/******************************************************************************
********************************* 数 据 声 明 *********************************
******************************************************************************/
/*---------------------* 
*    IMPORT:由外提供   * 
*----------------------*/
//none

/*---------------------* 
*    EXPORT:向外提供   * 
*----------------------*/
//none

/******************************************************************************
********************************* 函 数 声 明 *********************************
******************************************************************************/
char  test=0;
/*---------------------* 
*    IMPORT:由外提供   * 
*----------------------*/
//none

/*---------------------* 
*    EXPORT:向外提供   * 
*----------------------*/
//none



/******************************************************************************
*********************************  程序开始  **********************************
******************************************************************************/
/******************************************************************************
/ 函数功能:ms级延时函数
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
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
/ 函数功能:I2C接口配置程序
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void I2C_Ini(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 
    
  //关闭JTAG使能SWD以释放IO口
  uint32_t u32Temp;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB , ENABLE);
  u32Temp = AFIO->MAPR;
  u32Temp &= ~AFIO_MAPR_SWJ_CFG ;
  u32Temp |= AFIO_MAPR_SWJ_CFG_1;
  AFIO->MAPR = u32Temp;
  
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB , ENABLE);
  //GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);
  
  //配置I2C的SCL引脚和SDA引脚
  RCC_APB2PeriphClockCmd(AHRS_I2C_GPIO_CLK , ENABLE);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin =  AHRS_I2C_SclPin;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_InitStructure.GPIO_Pin =   AHRS_I2C_SdaPin;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  SCL_H();SDA_H();    //总线释放
  
  //配置其他IO?
//  RCC_APB2PeriphClockCmd(LED1_GPIO_CLK | LED2_GPIO_CLK , ENABLE);
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//  GPIO_InitStructure.GPIO_Pin =  LED1_GPIO_PIN;
//  GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);
//  GPIO_InitStructure.GPIO_Pin =  LED2_GPIO_PIN;
//  GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);
//  
//  //配置输入
//  RCC_APB2PeriphClockCmd(KEY_GPIO_CLK, ENABLE);
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
//  GPIO_InitStructure.GPIO_Pin =  KEY_GPIO_PIN;
//  GPIO_Init(KEY_GPIO_PORT, &GPIO_InitStructure);
}

/******************************************************************************
/ 函数功能:Simulation IIC Timing series delay
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
__inline void I2C_delay(void)
{
        
   u8 i = I2C_DELAY_VAL; //这里可以优化速度   ，经测试最低到5还能写入
   while(i) 
   { 
     i--; 
   }  
}


/******************************************************************************
/ 函数功能:延时5ms时间
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
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
* 在SCL是高电平时SDA高电平向低电平切换
****************************************************************************** */
unsigned char I2C_Start(void)
{
    I2C_delay();    
    SCL_H(); 
    if(SDA_read()  == 0)return IIC_STATE_BUSY;  //SDA线为低电平则总线忙,退出
    if(SCL_read()  == 0)return IIC_STATE_BUSY;  //SDA线为低电平则总线忙,退出         
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
* 在SCL是高电平时SDA高电平向低电平切换
****************************************************************************** */
unsigned char I2C_Restart(void)
{
    I2C_delay();    
    SCL_L();
    SDA_H();
    I2C_delay();    
    SCL_H();
    if(SDA_read()  ==0 )return IIC_STATE_BUSY;  //SDA线为低电平则总线忙,退出
    if(SCL_read()  ==0 )return IIC_STATE_BUSY;  //SDA线为低电平则总线忙,退出      
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
unsigned char I2C_WaitAck(void)   //返回为:=1有ACK,=0无ACK
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
void I2C_SendByte(u8 SendByte) //数据从高位到低位//
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
uint8_t I2C_ReadByte(void)  //数据从高位到低位//
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
/ 函数功能:单字节写入
/ 修改日期:none
/ 输入参数:
/   @arg SlaveAddress   从器件地址
/   @arg REG_Address    寄存器地址
/   @arg REG_data       欲写入的字节数据
/ 输出参数: 读出的字节数据
/ 使用说明:这时一个完整的单字节读取函数
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
/ 函数功能:单字节写入
/ 修改日期:none
/ 输入参数:
/   @arg SlaveAddress   从器件地址
/   @arg REG_Address    寄存器地址
/ 输出参数: 读出的字节数据
/ 使用说明:这时一个完整的单字节读取函数
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
/ 函数功能:多字节读出函数
/ 修改日期:none
/ 输入参数:
/   @arg SlaveAddress   从器件地址
/   @arg REG_Address    寄存器地址
/   @arg ptChar         输出缓冲
/   @arg size           读出的数据个数,size必须大于=1
/ 输出参数: 成功失败标记
/ 使用说明:none
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
    
    //连续读出ax,ay,az数据
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




