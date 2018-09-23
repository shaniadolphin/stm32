#include "./IIC/ioi2c.h"
#include "stm32f10x.h"
#include "./usart/usart.h"

#define I2C_TIMEOUT 5
#define I2CDEV_CLK_TS (1000000 / 100000)

#define Transmitter             0x00
#define Receiver                0x01

#define FALSE 0
#define TRUE 1

static vu8 Direction = Transmitter;
//static u16 SlaveADDR;
//static u32 TxLength,RxLength;

//static u32 DeviceOffset=0xffffffff;

//static u8* pTxBuffer1;
//static u8* pTxBuffer2;
//static u8* pRxBuffer1;
//static u8* pRxBuffer2;
//static u8 check_begin = FALSE;
vu8 MasterReceptionComplete = 0;
vu8 MasterTransitionComplete = 0; // to indicat master's send process
vu8 SlaveReceptionComplete =0;
vu8 SlaveTransitionComplete =0;

vu8 WriteComplete = 0; // to indicat target's internal write process

/*P-V operation on I2C1 or I2C2*/
static vu8 PV_flag_1;
static vu8 PV_flag_2;

__IO uint8_t Address;
__IO uint32_t I2CDirection = I2C_DIRECTION_TX;
__IO uint32_t NumbOfBytes1;
__IO uint32_t NumbOfBytes2;
/* Buffer of data to be received by I2C1 */
uint8_t* Buffer_Rx1;
/* Buffer of data to be transmitted by I2C1 */
uint8_t* Buffer_Tx1;
/* Buffer of data to be received by I2C1 */
uint8_t* Buffer_Rx2;
/* Buffer of data to be transmitted by I2C1 */
uint8_t* Buffer_Tx2;
__IO uint8_t Tx_Idx1 = 0, Rx_Idx1 = 0;
__IO uint8_t Tx_Idx2 = 0, Rx_Idx2 = 0;

I2C_STATE i2c_comm_state;

/**************************实现函数********************************************
*函数原型:		void IIC_Init(void)
*功　　能:		初始化I2C对应的接口引脚。
*******************************************************************************/
void IIC_Init(void)
{
#if 0	
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     //使能PB端口时钟
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;	//端口配置
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;          //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         //50M
  GPIO_Init(GPIOB, &GPIO_InitStructure);					//根据设定参数初始化GPIOB 
#else
  GPIO_InitTypeDef  GPIO_InitStructure; 
  uint32_t u32Temp;
  RCC_APB2PeriphClockCmd(AHRS_I2C_GPIO_CLK , ENABLE);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin =  AHRS_I2C_SclPin;
  GPIO_Init(AHRS_I2C_GPIO, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_InitStructure.GPIO_Pin =   AHRS_I2C_SdaPin;
  GPIO_Init(AHRS_I2C_GPIO, &GPIO_InitStructure);
  SCL_H();
  SDA_H();    //总线释放	
#endif
	
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
/**************************实现函数********************************************
*函数原型:		void IIC_Start(void)
*功　　能:		产生IIC起始信号
*******************************************************************************/
int IIC_Start(void)
{
#if 0
	SDA_OUT();     //sda线输出
	IIC_SDA=1;
	if(!READ_SDA)return 0;	
	IIC_SCL=1;
	delay_us(1);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	if(READ_SDA)return 0;
	delay_us(1);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
	return 1;
#else
    I2C_delay();    
    SCL_H(); 
    //if(SDA_read()  == 0)return IIC_STATE_BUSY;  //SDA线为低电平则总线忙,退出
    //if(SCL_read()  == 0)return IIC_STATE_BUSY;  //SDA线为低电平则总线忙,退出         
    SDA_L();
    //I2C_delay();    
    //SDA_H();
    //I2C_delay();    
    //SDA_L();									// I2C Start Condition    
    return IIC_STATE_OK;   
#endif	
}

int IIC_Restart(void)
{
    I2C_delay();    
    SCL_L();
    SDA_H();
    I2C_delay();    
    SCL_H();
    //if(SDA_read()  == 0)return IIC_STATE_BUSY;  //SDA线为低电平则总线忙,退出
    //if(SCL_read()  == 0)return IIC_STATE_BUSY;  //SDA线为低电平则总线忙,退出      
    I2C_delay();  
    SDA_L();  
    return IIC_STATE_OK;    
}

/**************************实现函数********************************************
*函数原型:		void IIC_Stop(void)
*功　　能:	    //产生IIC停止信号
*******************************************************************************/	  
void IIC_Stop(void)
{
#if 0
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(1);
	IIC_SCL=1; 
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(1);							   	
#else
    I2C_delay();
    SCL_L();
    SDA_L();
    I2C_delay();
    SCL_H();
    SDA_H();
#endif
}

/**************************实现函数********************************************
*函数原型:		u8 IIC_Wait_Ack(void)
*功　　能:	    等待应答信号到来 
//返回值：1，接收应答失败
//        0，接收应答成功
*******************************************************************************/
int IIC_Wait_Ack(void)
{
#if 0
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA=1;
	delay_us(1);	   
	IIC_SCL=1;
	delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>50)
		{
			IIC_Stop();
			return 0;
		}
	  delay_us(1);
	}
	IIC_SCL=0;//时钟输出0 	   
	return 1; 
#else
    SDA_H();					// SDA Input
    I2C_delay();      
    SCL_H();					// Test Acknowledge
    if(SDA_read())return 0;
    else return 1;				// return ok if got acknowledge from slave device
#endif	
} 

/**************************实现函数********************************************
*函数原型:		void IIC_Ack(void)
*功　　能:	    产生ACK应答
*******************************************************************************/
void IIC_Ack(void)
{
#if 0
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(1);
	IIC_SCL=1;
	delay_us(1);
	IIC_SCL=0;
#else
    SCL_L();
    I2C_delay();
    SDA_L();
    I2C_delay();
    SCL_H();
    I2C_delay();
    SCL_L();
    I2C_delay(); 	
#endif
}
	
/**************************实现函数********************************************
*函数原型:		void IIC_NAck(void)
*功　　能:	    产生NACK应答
*******************************************************************************/	    
void IIC_NAck(void)
{
#if 0
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(1);
	IIC_SCL=1;
	delay_us(1);
	IIC_SCL=0;
#else
    SDA_H();
    I2C_delay();    
    SCL_H(); 	
#endif
}
/**************************实现函数********************************************
*函数原型:		void IIC_Send_Byte(u8 txd)
*功　　能:	    IIC发送一个字节
*******************************************************************************/		  
void IIC_Send_Byte(u8 txd)
{
#if 0	
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(1);   
		IIC_SCL=1;
		delay_us(1); 
		IIC_SCL=0;	
		delay_us(1);
    }	
#else
    unsigned char mask = 0x80;
    I2C_delay();    
    SCL_L();
    for (mask = 0x80; mask != 0; mask >>= 1)
    {

        if (txd & mask)
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
#endif	
} 	 
  
/**************************实现函数********************************************
*函数原型:		bool i2cWrite(uint8_t addr, uint8_t reg, uint8_t data)
*功　　能:		
*******************************************************************************/
int i2cWrite(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data)
{
#if 0
	int i;
    if (!IIC_Start())
        return 1;
    IIC_Send_Byte(addr << 1 );
    if (!IIC_Wait_Ack()) {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);
    IIC_Wait_Ack();
		for (i = 0; i < len; i++) {
        IIC_Send_Byte(data[i]);
        if (!IIC_Wait_Ack()) {
            IIC_Stop();
            return 0;
        }
    }
    IIC_Stop();
    return 0;
#else
    unsigned char i2c_error = 0;
	unsigned char i = 0;
	i2c_error = IIC_Start();			// write start sequence 
    IIC_Send_Byte(addr <<1);			// write ALS Address byte w/ WRITE command 
    i2c_error = IIC_Wait_Ack();
    if(i2c_error != IIC_STATE_OK){IIC_Stop(); return IIC_STATE_NOACK;}
    IIC_Send_Byte(reg);					// write ASL Address Byte 
    i2c_error = IIC_Wait_Ack();
    if(i2c_error != IIC_STATE_OK){IIC_Stop(); return IIC_STATE_NOACK;}
	for (i = 0; i < len; i++) 
	{
		IIC_Send_Byte(data[i]);
		i2c_error = IIC_Wait_Ack();
		if (i2c_error != IIC_STATE_OK) 
		{
			IIC_Stop();
			return IIC_STATE_NOACK;
		}
	}
    IIC_Stop();
    return IIC_STATE_OK;  
#endif	
}
/**************************实现函数********************************************
*函数原型:bool i2cWrite(uint8_t addr, uint8_t reg, uint8_t data)
*功　　能:		
*******************************************************************************/
int i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
#if 0	
    if (!IIC_Start())
        return 1;
    IIC_Send_Byte(addr << 1);
    if (!IIC_Wait_Ack()) {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte((addr << 1)+1);
    IIC_Wait_Ack();
    while (len) {
        if (len == 1)
            *buf = IIC_Read_Byte(0);
        else
            *buf = IIC_Read_Byte(1);
        buf++;
        len--;
    }
    IIC_Stop();
    return 0;
#else
    unsigned char i2c_error = 0;
	unsigned char i = 0;
	i2c_error = IIC_Start();			// write start sequence 
    IIC_Send_Byte(addr <<1);			// write ALS Address byte w/ WRITE command 
    i2c_error = IIC_Wait_Ack();
    if(i2c_error != IIC_STATE_OK){IIC_Stop(); return IIC_STATE_NOACK;}
    IIC_Send_Byte(reg);					// write ASL Address Byte 
    i2c_error = IIC_Wait_Ack();
    if(i2c_error != IIC_STATE_OK){IIC_Stop(); return IIC_STATE_NOACK;}
    
	i2c_error = IIC_Restart();					// write start sequence 
    
	IIC_Send_Byte((addr <<1) + 1);		// write ALS Address byte w/ READ command 
    i2c_error = IIC_Wait_Ack();
    if(i2c_error != IIC_STATE_OK){IIC_Stop(); return IIC_STATE_NOACK;}  
	while(len) 
	{
        *buf = IIC_Read_Byte(0);
        if (len == 1)
            IIC_NAck();
        else
            IIC_Ack();
        buf++;
        len--;
    }
    IIC_Stop();
    return IIC_STATE_OK;    
#endif
}


/**************************实现函数********************************************
*函数原型:		u8 IIC_Read_Byte(unsigned char ack)
*功　　能:	    //读1个字节，ack=1时，发送ACK，ack=0，发送nACK 
*******************************************************************************/  
u8 IIC_Read_Byte(unsigned char ack)
{
#if 0
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        delay_us(2);
		IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(2); 
    }					 
    if (ack)
        IIC_Ack(); //发送ACK 
    else
        IIC_NAck();//发送nACK  
    return receive;
#else
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
    //if (ack)
    //    IIC_Ack(); //发送ACK 
    //else
    //    IIC_NAck();//发送nACK  
    return TempData;  	
#endif
}

/**************************实现函数********************************************
*函数原型:		unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr)
*功　　能:	    读取指定设备 指定寄存器的一个值
输入	I2C_Addr  目标设备地址
		addr	   寄存器地址
返回   读出来的值
*******************************************************************************/ 
int I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr, unsigned char *value)
{
#if 0
	unsigned char res=0;	
	IIC_Start();	
	IIC_Send_Byte(I2C_Addr);	   //发送写命令
	res++;
	IIC_Wait_Ack();
	IIC_Send_Byte(addr); res++;  //发送地址
	IIC_Wait_Ack();	  
	//IIC_Stop();//产生一个停止条件	
	IIC_Start();
	IIC_Send_Byte(I2C_Addr+1); res++;          //进入接收模式			   
	IIC_Wait_Ack();
	res=IIC_Read_Byte(0);	   
    IIC_Stop();//产生一个停止条件
	return res;
#else
    unsigned char i2c_error = 0;  
    i2c_error = IIC_Start();					// write start sequence 
    //if(i2c_error != IIC_STATE_OK){IIC_Stop(); return i2c_error;}
    IIC_Send_Byte(I2C_Addr <<1);					//write ALS Address byte w/ WRITE command 
    i2c_error = IIC_Wait_Ack();
    //if(i2c_error != IIC_STATE_OK){IIC_Stop(); return IIC_STATE_NOACK;}  
    IIC_Send_Byte(addr);							//set ALS's I2C address 
    i2c_error = IIC_Wait_Ack();
    //if(i2c_error != IIC_STATE_OK){IIC_Stop(); return IIC_STATE_NOACK;}  
    i2c_error = IIC_Restart();		
    //write start sequence 
    IIC_Send_Byte((I2C_Addr <<1) +1);			//write ALS Address byte w/ READ command 
    i2c_error = IIC_Wait_Ack();
    //if(i2c_error != IIC_STATE_OK){IIC_Stop(); return IIC_STATE_NOACK;}  
    *value = IIC_Read_Byte(0);					//read 8 bit data
    IIC_NAck();								//send NON-ACK to Slave device
    IIC_Stop();
    return IIC_STATE_OK;
#endif	
}


/**************************实现函数********************************************
*函数原型:		u8 IICreadBytes(u8 dev, u8 reg, u8 length, u8 *data)
*功　　能:	    读取指定设备 指定寄存器的 length个值
输入	dev  目标设备地址
		reg	  寄存器地址
		length 要读的字节数
		*data  读出的数据将要存放的指针
返回   读出来的字节数量
*******************************************************************************/ 
u8 IICreadBytes(u8 dev, u8 reg, u8 length, u8 *data)
{
    u8 count = 0;	
	IIC_Start();
	IIC_Send_Byte(dev << 1);	   //发送写命令
	IIC_Wait_Ack();
	IIC_Send_Byte(reg);   //发送地址
    IIC_Wait_Ack();	  
	IIC_Restart();
	IIC_Send_Byte((dev << 1) + 1);  //进入接收模式	
	IIC_Wait_Ack();
    for(count=0;count<length;count++)
	{	 
	    if(count!=length-1)
		{
			data[count]=IIC_Read_Byte(1);  //带ACK的读数据
			IIC_Ack(); //发送ACK 
		} 	
		else  
		{
			data[count]=IIC_Read_Byte(0);	 //最后一个字节NACK
			IIC_NAck(); //发送NACK 
		}
	}
    IIC_Stop();//产生一个停止条件
    return count;
}

/**************************实现函数********************************************
*函数原型:		u8 IICwriteBytes(u8 dev, u8 reg, u8 length, u8* data)
*功　　能:	    将多个字节写入指定设备 指定寄存器
输入	dev  目标设备地址
		reg	  寄存器地址
		length 要写的字节数
		*data  将要写的数据的首地址
返回   返回是否成功
*******************************************************************************/ 
u8 IICwriteBytes(u8 dev, u8 reg, u8 length, u8* data){
  
 	u8 count = 0;
	IIC_Start();
	IIC_Send_Byte(dev << 1);	   //发送写命令
	IIC_Wait_Ack();
	IIC_Send_Byte(reg);   //发送地址
    IIC_Wait_Ack();	  
	for(count=0;count<length;count++)
	{
		IIC_Send_Byte(data[count]); 
		IIC_Wait_Ack(); 
	 }
	IIC_Stop();//产生一个停止条件

    return 1; //status == 0;
}

/**************************实现函数********************************************
*函数原型:		u8 IICreadByte(u8 dev, u8 reg, u8 *data)
*功　　能:	    读取指定设备 指定寄存器的一个值
输入	dev  目标设备地址
		reg	   寄存器地址
		*data  读出的数据将要存放的地址
返回   1
*******************************************************************************/ 
u8 IICreadByte(u8 dev, u8 reg, u8 *data)
{
	//I2C_ReadOneByte(dev, reg, data);   
    return IICreadBytes(dev, reg, 1, data);
}

/**************************实现函数********************************************
*函数原型:		unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data)
*功　　能:	    写入指定设备 指定寄存器一个字节
输入	dev  目标设备地址
		reg	   寄存器地址
		data  将要写入的字节
返回   1
*******************************************************************************/ 
u8 IICwriteByte(u8  dev, u8 reg, u8 data)
{
    return IICwriteBytes(dev, reg, 1, &data);
}

/**************************实现函数********************************************
*函数原型:		u8 IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data)
*功　　能:	    读 修改 写 指定设备 指定寄存器一个字节 中的多个位
输入	dev  目标设备地址
		reg	   寄存器地址
		bitStart  目标字节的起始位
		length   位长度
		data    存放改变目标字节位的值
返回   成功 为1 
 		失败为0
*******************************************************************************/ 
u8 IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data)
{

    u8 b;
    if (IICreadByte(dev, reg, &b) != 0) 
    {
        u8 mask = (0xFF << (bitStart + 1)) | 0xFF >> ((8 - bitStart) + length - 1);
        data <<= (8 - length);
        data >>= (7 - bitStart);
        b &= mask;
        b |= data;
        return IICwriteByte(dev, reg, b);
    } 
    else 
    {
        return 0;
    }
}

/**************************实现函数********************************************
*函数原型:		u8 IICwriteBit(u8 dev, u8 reg, u8 bitNum, u8 data)
*功　　能:	    读 修改 写 指定设备 指定寄存器一个字节 中的1个位
输入	dev  目标设备地址
		reg	   寄存器地址
		bitNum  要修改目标字节的bitNum位
		data  为0 时，目标位将被清0 否则将被置位
返回   成功 为1 
 		失败为0
*******************************************************************************/ 
u8 IICwriteBit(u8 dev, u8 reg, u8 bitNum, u8 data)
{
    u8 b;
    IICreadByte(dev, reg, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return IICwriteByte(dev, reg, b);
}

/*----------------------------------------------------------------------------
 * @描述: 读AP3220寄存器电压值
 * @输入: reg,data
 * @返回: ret
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2015.04.21     zxjun          创建模块		
----------------------------------------------------------------------------*/
unsigned char IIC_Reg_Set(unsigned char dev, unsigned char reg, unsigned char data)
{
	unsigned char ret = 1;
	int i;
    unsigned char tmp;
	for (i=0; (i < 5) && ret; i++)//重试2次
	{
		ret = IICwriteByte(dev, reg, data);	  /* power up and enable ALS sensor and disable PS sensor */
#if 1		
        printk("W:reg0x%x=%d return %d\r\n",reg, data, ret);
        ret = IICreadByte(dev, reg, &tmp);
        printk("R:reg0x%x=%d return %d\r\n",reg, tmp, ret);
        if(tmp != data)ret =1;
        else ret =0;
#endif                
	}
	return ret;
}

//------------------End of File----------------------------
static void i2cdevRuffLoopDelay(uint32_t us)
{
  volatile uint32_t delay;

  for(delay = I2CDEV_LOOPS_PER_US * us; delay > 0; delay--);
}

static void i2cdevResetBusI2c2(void)
{
  /* Make sure the bus is free by clocking it until any slaves release the bus. */
  GPIO_InitTypeDef  GPIO_InitStructure;
  /* Reset the I2C block */
  I2C_DeInit(I2C2);

  /* I2C1 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
  /* I2C1 SDA configuration */
  GPIO_InitStructure.GPIO_Pin = I2CDEV_I2C2_PIN_SDA;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  /* I2C1 SCL configuration */
  GPIO_InitStructure.GPIO_Pin = I2CDEV_I2C2_PIN_SCL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_SetBits(GPIOB, I2CDEV_I2C2_PIN_SDA);
  /* Check SDA line to determine if slave is asserting bus and clock out if so */
  while(GPIO_ReadInputDataBit(GPIOB, I2CDEV_I2C2_PIN_SDA) == Bit_RESET)
  {
    /* Set clock high */
    GPIO_SetBits(GPIOB, I2CDEV_I2C2_PIN_SCL);
    /* Wait for any clock stretching to finish. */
    GPIO_WAIT_LOW(GPIOB, I2CDEV_I2C1_PIN_SCL, 10 * I2CDEV_LOOPS_PER_MS);
    i2cdevRuffLoopDelay(I2CDEV_CLK_TS);

    /* Generate a clock cycle */
    GPIO_ResetBits(GPIOB, I2CDEV_I2C2_PIN_SCL);
    i2cdevRuffLoopDelay(I2CDEV_CLK_TS);
    GPIO_SetBits(GPIOB, I2CDEV_I2C2_PIN_SCL);
    i2cdevRuffLoopDelay(I2CDEV_CLK_TS);
  }

  /* Generate a start then stop condition */
  GPIO_SetBits(GPIOB, I2CDEV_I2C2_PIN_SCL);
  i2cdevRuffLoopDelay(I2CDEV_CLK_TS);
  GPIO_ResetBits(GPIOB, I2CDEV_I2C2_PIN_SDA);
  i2cdevRuffLoopDelay(I2CDEV_CLK_TS);
  GPIO_ResetBits(GPIOB, I2CDEV_I2C2_PIN_SDA);
  i2cdevRuffLoopDelay(I2CDEV_CLK_TS);

  /* Set data and clock high and wait for any clock stretching to finish. */
  GPIO_SetBits(GPIOB, I2CDEV_I2C2_PIN_SDA);
  GPIO_SetBits(GPIOB, I2CDEV_I2C2_PIN_SCL);
  GPIO_WAIT_LOW(GPIOB, I2CDEV_I2C2_PIN_SCL, 10 * I2CDEV_LOOPS_PER_MS);
  /* Wait for data to be high */
  GPIO_WAIT_HIGH(GPIOB, I2CDEV_I2C2_PIN_SDA, 10 * I2CDEV_LOOPS_PER_MS);

}
void  I2C2_Comm_Init(u32 I2C_Speed, u16 I2C_Addr)
{
	/******* GPIO configuration and clock enable *********/
	GPIO_InitTypeDef  GPIO_InitStructure; 
	I2C_InitTypeDef  I2C_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	i2cdevResetBusI2c2();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_AHBPeriph_FSMC, DISABLE);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10| GPIO_Pin_11;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/*********** I2C periphral configuration **********/
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C; // fixed
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;  // fixed
	I2C_InitStructure.I2C_OwnAddress1 = I2C_Addr;  // user parameter
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable; // fixed
#ifdef SLAVE_10BIT_ADDRESS  
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit;  // user define
#else
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
#endif
	I2C_InitStructure.I2C_ClockSpeed = I2C_Speed; // user parameter
	I2C_Cmd(I2C2, ENABLE);
	I2C_Init(I2C2, &I2C_InitStructure);

	/************** I2C NVIC configuration *************************/  
	/* 1 bit for pre-emption priority, 3 bits for subpriority */
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = I2C2_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = I2C2_ER_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#define I2C_BUSY 0x20
  if (I2C2->SR2 & I2C_BUSY)
  {
    /* Reset the I2C block */
    I2C_SoftwareResetCmd(I2C2, ENABLE);
    I2C_SoftwareResetCmd(I2C2, DISABLE);
  }
}



/**
 * @brief  Initializes DMA channel used by the I2C Write/read routines.
 * @param  None.
 * @retval None.
 */
void I2C_DMAConfig(I2C_TypeDef* I2Cx, uint8_t* pBuffer, u32 BufferSize,
    u32 Direction)
{
    DMA_InitTypeDef I2CDMA_InitStructure;
  /* Initialize the DMA with the new parameters */
  if (Direction == I2C_DIRECTION_TX)
  {
    /* Configure the DMA Tx Channel with the buffer address and the buffer size */
    I2CDMA_InitStructure.DMA_MemoryBaseAddr = (u32) pBuffer;
    I2CDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    I2CDMA_InitStructure.DMA_BufferSize = (u32) BufferSize;

    if (I2Cx == I2C1)
    {
      I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (u32) I2C1_DR_Address;
      DMA_Cmd(I2C1_DMA_CHANNEL_TX, DISABLE);
      DMA_Init(I2C1_DMA_CHANNEL_TX, &I2CDMA_InitStructure);
      DMA_Cmd(I2C1_DMA_CHANNEL_TX, ENABLE);
    }
    else
    {
      I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (u32) I2C2_DR_Address;
      DMA_Cmd(I2C2_DMA_CHANNEL_TX, DISABLE);
      DMA_Init(I2C2_DMA_CHANNEL_TX, &I2CDMA_InitStructure);
      DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
      DMA_Cmd(I2C2_DMA_CHANNEL_TX, ENABLE);
    }
  }
  else /* Reception */
  {
    /* Configure the DMA Rx Channel with the buffer address and the buffer size */
    I2CDMA_InitStructure.DMA_MemoryBaseAddr = (u32) pBuffer;
    I2CDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    I2CDMA_InitStructure.DMA_BufferSize = (u32) BufferSize;
    if (I2Cx == I2C1)
    {

      I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (u32) I2C1_DR_Address;
      DMA_Cmd(I2C1_DMA_CHANNEL_RX, DISABLE);
      DMA_Init(I2C1_DMA_CHANNEL_RX, &I2CDMA_InitStructure);
      DMA_Cmd(I2C1_DMA_CHANNEL_RX, ENABLE);
    }

    else
    {
      I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (u32) I2C2_DR_Address;
      DMA_Cmd(I2C2_DMA_CHANNEL_RX, DISABLE);
      DMA_Init(I2C2_DMA_CHANNEL_RX, &I2CDMA_InitStructure);
      DMA_Cmd(I2C2_DMA_CHANNEL_RX, ENABLE);
    }

  }
}

/**
 * @brief  Reads buffer of bytes  from the slave.
 * @param pBuffer: Buffer of bytes to be read from the slave.
 * @param NumByteToRead: Number of bytes to be read by the Master.
 * @param Mode: Polling or DMA or Interrupt having the highest priority in the application.
 * @param SlaveAddress: The address of the slave to be addressed by the Master.
 * @retval : None.
 */
uint8_t I2C_Master_BufferRead(I2C_TypeDef* I2Cx, uint8_t* pBuffer,
    uint32_t NumByteToRead, I2C_ProgrammingModel Mode, uint8_t SlaveAddress,
    uint32_t timeoutMs)

{
  __IO uint32_t temp = 0;
  __IO uint32_t Timeout = 0;

  /* Enable I2C errors interrupts (used in all modes: Polling, DMA and Interrupts */
  I2Cx->CR2 |= I2C_IT_ERR;

  if (Mode == DMA) /* I2Cx Master Reception using DMA */
  {
    /* Configure I2Cx DMA channel */
    I2C_DMAConfig(I2Cx, pBuffer, NumByteToRead, I2C_DIRECTION_RX);
    /* Set Last bit to have a NACK on the last received byte */
    I2Cx->CR2 |= CR2_LAST_Set;
    /* Enable I2C DMA requests */
    I2Cx->CR2 |= CR2_DMAEN_Set;
    Timeout = 0xFFFF;
    /* Send START condition */
    I2Cx->CR1 |= CR1_START_Set;
    /* Wait until SB flag is set: EV5  */
    while ((I2Cx->SR1 & 0x0001) != 0x0001)
    {
      if (Timeout-- == 0)
        return 1;
    }
    Timeout = 0xFFFF;
    /* Send slave address */
    /* Set the address bit0 for read */
    SlaveAddress |= OAR1_ADD0_Set;
    Address = SlaveAddress;
    /* Send the slave address */
    I2Cx->DR = Address;
    /* Wait until ADDR is set: EV6 */
    while ((I2Cx->SR1 & 0x0002) != 0x0002)
    {
      if (Timeout-- == 0)
        return 1;
    }
    /* Clear ADDR flag by reading SR2 register */
    temp = I2Cx->SR2;
    if (I2Cx == I2C1)
    {
      /* Wait until DMA end of transfer */
      while (!DMA_GetFlagStatus(DMA1_FLAG_TC7));
//      xSemaphoreTake(i2cdevDmaEventI2c1, M2T(timeoutMs));
      /* Disable DMA Channel */
      DMA_Cmd(I2C1_DMA_CHANNEL_RX, DISABLE);
      /* Clear the DMA Transfer Complete flag */
      DMA_ClearFlag(DMA1_FLAG_TC7);
    }
    else /* I2Cx = I2C2*/
    {
      /* Wait until DMA end of transfer */
      while (!DMA_GetFlagStatus(DMA1_FLAG_TC5))
//      xSemaphoreTake(i2cdevDmaEventI2c2, M2T(timeoutMs));
      /* Disable DMA Channel */
      DMA_Cmd(I2C2_DMA_CHANNEL_RX, DISABLE);
      /* Clear the DMA Transfer Complete flag */
      DMA_ClearFlag(DMA1_FLAG_TC5);
    }
    /* Program the STOP */
    I2Cx->CR1 |= CR1_STOP_Set;
    /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
    while ((I2Cx->CR1 & 0x200) == 0x200)
      ;
  }
  /* I2Cx Master Reception using Interrupts with highest priority in an application */
  else
  {
    /* Enable EVT IT*/
    I2Cx->CR2 |= I2C_IT_EVT;
    /* Enable BUF IT */
    I2Cx->CR2 |= I2C_IT_BUF;
    /* Set the I2C direction to reception */
    I2CDirection = I2C_DIRECTION_RX;
    if (I2Cx == I2C1)
    	Buffer_Rx1 = pBuffer;
    else
	Buffer_Rx2 = pBuffer;
    SlaveAddress |= OAR1_ADD0_Set;
    Address = SlaveAddress;
    if (I2Cx == I2C1)
      NumbOfBytes1 = NumByteToRead;
    else
      NumbOfBytes2 = NumByteToRead;
    /* Send START condition */
    I2Cx->CR1 |= CR1_START_Set;
    Timeout = timeoutMs * I2CDEV_LOOPS_PER_MS;
    /* Wait until the START condition is generated on the bus: START bit is cleared by hardware */
    while ((I2Cx->CR1 & 0x100) == 0x100 && Timeout)
    {
      Timeout--;
    }
    /* Wait until BUSY flag is reset (until a STOP is generated) */
    while ((I2Cx->SR2 & 0x0002) == 0x0002 && Timeout)
    {
      Timeout--;
    }
    /* Enable Acknowledgement to be ready for another reception */
    I2Cx->CR1 |= CR1_ACK_Set;

    if (Timeout == 0)
      return 1;
  }

  return 0;
//  temp++; //To avoid GCC warning!
}

/**
 * @brief  Writes buffer of bytes.
 * @param pBuffer: Buffer of bytes to be sent to the slave.
 * @param NumByteToWrite: Number of bytes to be sent by the Master.
 * @param Mode: Polling or DMA or Interrupt having the highest priority in the application.
 * @param SlaveAddress: The address of the slave to be addressed by the Master.
 * @retval : None.
 */
uint8_t I2C_Master_BufferWrite(I2C_TypeDef* I2Cx, uint8_t* pBuffer,
    uint32_t NumByteToWrite, I2C_ProgrammingModel Mode, uint8_t SlaveAddress,
    uint32_t timeoutMs)
{

  __IO uint32_t temp = 0;
  __IO uint32_t Timeout = 0;

  /* Enable Error IT (used in all modes: DMA, Polling and Interrupts */
  I2Cx->CR2 |= I2C_IT_ERR;
  if (Mode == DMA) /* I2Cx Master Transmission using DMA */
  {
    Timeout = 0xFFFF;
    /* Configure the DMA channel for I2Cx transmission */
    I2C_DMAConfig(I2Cx, pBuffer, NumByteToWrite, I2C_DIRECTION_TX);
    /* Enable the I2Cx DMA requests */
    I2Cx->CR2 |= CR2_DMAEN_Set;
    /* Send START condition */
    I2Cx->CR1 |= CR1_START_Set;
    /* Wait until SB flag is set: EV5 */
    while ((I2Cx->SR1 & 0x0001) != 0x0001)
    {
      if (Timeout-- == 0)
        return 1;
    }
    Timeout = 0xFFFF;
    /* Send slave address */
    /* Reset the address bit0 for write */
    SlaveAddress &= OAR1_ADD0_Reset;
    Address = SlaveAddress;
    /* Send the slave address */
    I2Cx->DR = Address;
    /* Wait until ADDR is set: EV6 */
    while ((I2Cx->SR1 & 0x0002) != 0x0002)
    {
      if (Timeout-- == 0)
        return 1;
    }

    /* Clear ADDR flag by reading SR2 register */
    temp = I2Cx->SR2;
    if (I2Cx == I2C1)
    {
      /* Wait until DMA end of transfer */
      while (!DMA_GetFlagStatus(DMA1_FLAG_TC6));
//      xSemaphoreTake(i2cdevDmaEventI2c1, M2T(5));
      /* Disable the DMA1 Channel 6 */
      DMA_Cmd(I2C1_DMA_CHANNEL_TX, DISABLE);
      /* Clear the DMA Transfer complete flag */
      DMA_ClearFlag(DMA1_FLAG_TC6);
    }
    else /* I2Cx = I2C2 */
    {
      /* Wait until DMA end of transfer */
      while (!DMA_GetFlagStatus(DMA1_FLAG_TC4))
//      xSemaphoreTake(i2cdevDmaEventI2c2, M2T(5));
      /* Disable the DMA1 Channel 4 */
      DMA_Cmd(I2C2_DMA_CHANNEL_TX, DISABLE);
      /* Clear the DMA Transfer complete flag */
      DMA_ClearFlag(DMA1_FLAG_TC4);
    }

    /* EV8_2: Wait until BTF is set before programming the STOP */
    while ((I2Cx->SR1 & 0x00004) != 0x000004)
      ;
    /* Program the STOP */
    I2Cx->CR1 |= CR1_STOP_Set;
    /* Make sure that the STOP bit is cleared by Hardware */
    while ((I2Cx->CR1 & 0x200) == 0x200)
      ;
  }
  /* I2Cx Master Transmission using Interrupt with highest priority in the application */
  else
  {
    /* Enable EVT IT*/
    I2Cx->CR2 |= I2C_IT_EVT;
    /* Enable BUF IT */
    I2Cx->CR2 |= I2C_IT_BUF;
    /* Set the I2C direction to Transmission */
    I2CDirection = I2C_DIRECTION_TX;
    if (I2Cx == I2C1)
    	Buffer_Tx1 = pBuffer;
    else
	Buffer_Tx2 = pBuffer;
    SlaveAddress &= OAR1_ADD0_Reset;
    Address = SlaveAddress;
    if (I2Cx == I2C1)
      NumbOfBytes1 = NumByteToWrite;
    else
      NumbOfBytes2 = NumByteToWrite;
    /* Send START condition */
    I2Cx->CR1 |= CR1_START_Set;
    Timeout = timeoutMs * I2CDEV_LOOPS_PER_MS;
    /* Wait until the START condition is generated on the bus: the START bit is cleared by hardware */
    while ((I2Cx->CR1 & 0x100) == 0x100 && Timeout)
    {
      Timeout--;
    }
    /* Wait until BUSY flag is reset: a STOP has been generated on the bus signaling the end
     of transmission */
    while ((I2Cx->SR2 & 0x0002) == 0x0002 && Timeout)
    {
      Timeout--;
    }

    if (Timeout == 0)
      return 1;
  }
  return 0;

//  temp++; //To avoid GCC warning!
}

uint8_t i2cdevRead(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
               uint16_t len, uint8_t *data)
{
	uint8_t status = 0;
	if (memAddress != I2CDEV_NO_MEM_ADDR)
	{
        status = I2C_Master_BufferWrite(I2Cx, &memAddress,  1, INTERRUPT, devAddress << 1, I2C_TIMEOUT);
	}
	if (!status)
	{
        //TODO: Fix DMA transfer if more then 3 bytes
        status = I2C_Master_BufferRead(I2Cx, (uint8_t*)data,  len, INTERRUPT, devAddress << 1, I2C_TIMEOUT);
	}
	return status;
}

uint8_t i2cdevWrite(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
                uint16_t len, uint8_t *data)
{
  uint8_t status;
  static uint8_t buffer[17];
  int i;

  if (memAddress != I2CDEV_NO_MEM_ADDR)
  {
    // Sorry ...
    if (len > 16) len = 16;

    if(len == 0) return 1;

    buffer[0] = memAddress;
    for(i = 0; i < len ; i++)
      buffer[i + 1] = data[i];

    status = I2C_Master_BufferWrite(I2Cx, buffer,  len + 1, INTERRUPT, devAddress << 1, I2C_TIMEOUT);
  }
  else
  {
    status = I2C_Master_BufferWrite(I2Cx, data,  len, INTERRUPT, devAddress << 1, I2C_TIMEOUT);
  }
  return status;
}

/**
 * @brief  This function handles I2C1 Event interrupt request.
 * @param  None
 * @retval : None
 */
void i2cInterruptHandlerI2c1(void)
{

  __IO uint32_t SR1Register = 0;
  __IO uint32_t SR2Register = 0;

  /* Read the I2C1 SR1 and SR2 status registers */
  SR1Register = I2C1->SR1;
  SR2Register = I2C1->SR2;

  /* If SB = 1, I2C1 master sent a START on the bus: EV5) */
  if ((SR1Register & 0x0001) == 0x0001)
  {
    /* Send the slave address for transmssion or for reception (according to the configured value
     in the write master write routine */
    I2C1->DR = Address;
    SR1Register = 0;
    SR2Register = 0;
  }
  /* If I2C1 is Master (MSL flag = 1) */

  if ((SR2Register & 0x0001) == 0x0001)
  {
    /* If ADDR = 1, EV6 */
    if ((SR1Register & 0x0002) == 0x0002)
    {
      /* Write the first data in case the Master is Transmitter */
      if (I2CDirection == I2C_DIRECTION_TX)
      {
        /* Initialize the Transmit counter */
        Tx_Idx1 = 0;
        /* Write the first data in the data register */I2C1->DR =
            Buffer_Tx1[Tx_Idx1++];
        /* Decrement the number of bytes to be written */
        NumbOfBytes1--;
        /* If no further data to be sent, disable the I2C BUF IT
         in order to not have a TxE  interrupt */
        if (NumbOfBytes1 == 0)
        {
          I2C1->CR2 &= (uint16_t) ~I2C_IT_BUF;
        }
      }
      /* Master Receiver */
      else
      {
        /* Initialize Receive counter */
        Rx_Idx1 = 0;
        /* At this stage, ADDR is cleared because both SR1 and SR2 were read.*/
        /* EV6_1: used for single byte reception. The ACK disable and the STOP
         Programming should be done just after ADDR is cleared. */
        if (NumbOfBytes1 == 1)
        {
          /* Clear ACK */
          I2C1->CR1 &= CR1_ACK_Reset;
          /* Program the STOP */
          I2C1->CR1 |= CR1_STOP_Set;
        }
      }
      SR1Register = 0;
      SR2Register = 0;
    }
    /* Master transmits the remaing data: from data2 until the last one.  */
    /* If TXE is set */
    if ((SR1Register & 0x0084) == 0x0080)
    {
      /* If there is still data to write */
      if (NumbOfBytes1 != 0)
      {
        /* Write the data in DR register */
        I2C1->DR = Buffer_Tx1[Tx_Idx1++];
        /* Decrment the number of data to be written */
        NumbOfBytes1--;
        /* If  no data remains to write, disable the BUF IT in order
         to not have again a TxE interrupt. */
        if (NumbOfBytes1 == 0)
        {
          /* Disable the BUF IT */
          I2C1->CR2 &= (uint16_t) ~I2C_IT_BUF;
        }
      }
      SR1Register = 0;
      SR2Register = 0;
    }
    /* If BTF and TXE are set (EV8_2), program the STOP */
    if ((SR1Register & 0x0084) == 0x0084)
    {
      /* Program the STOP */
      I2C1->CR1 |= CR1_STOP_Set;
      /* Disable EVT IT In order to not have again a BTF IT */
      I2C1->CR2 &= (uint16_t) ~I2C_IT_EVT;
      SR1Register = 0;
      SR2Register = 0;
    }
    /* If RXNE is set */
    if ((SR1Register & 0x0040) == 0x0040)
    {
      /* Read the data register */
      Buffer_Rx1[Rx_Idx1++] = I2C1->DR;
      /* Decrement the number of bytes to be read */
      NumbOfBytes1--;
      /* If it remains only one byte to read, disable ACK and program the STOP (EV7_1) */
      if (NumbOfBytes1 == 1)
      {
        /* Clear ACK */
        I2C1->CR1 &= CR1_ACK_Reset;
        /* Program the STOP */
        I2C1->CR1 |= CR1_STOP_Set;
      }
      SR1Register = 0;
      SR2Register = 0;
    }
  }
}

/**
 * @brief  This function handles I2C1 Error interrupt request.
 * @param  None
 * @retval : None
 */
void i2cErrorInterruptHandlerI2c1(void)
{

  __IO uint32_t SR1Register = 0;

  /* Read the I2C1 status register */
  SR1Register = I2C1->SR1;
  /* If AF = 1 */
  if ((SR1Register & 0x0400) == 0x0400)
  {
    I2C1->SR1 &= 0xFBFF;
    SR1Register = 0;
  }
  /* If ARLO = 1 */
  if ((SR1Register & 0x0200) == 0x0200)
  {
    I2C1->SR1 &= 0xFBFF;
    SR1Register = 0;
  }
  /* If BERR = 1 */
  if ((SR1Register & 0x0100) == 0x0100)
  {
    I2C1->SR1 &= 0xFEFF;
    SR1Register = 0;
  }
  /* If OVR = 1 */
  if ((SR1Register & 0x0800) == 0x0800)
  {
    I2C1->SR1 &= 0xF7FF;
    SR1Register = 0;
  }
}

void i2cInterruptHandlerI2c2(void)
{

  __IO uint32_t SR1Register = 0;
  __IO uint32_t SR2Register = 0;

  /* Read the I2C2 SR1 and SR2 status registers */
  SR1Register = I2C2->SR1;
  SR2Register = I2C2->SR2;

  /* If SB = 1, I2C2 master sent a START on the bus: EV5) */
  if ((SR1Register & 0x0001) == 0x0001)
  {
    /* Send the slave address for transmssion or for reception (according to the configured value
     in the write master write routine */
    I2C2->DR = Address;
    SR1Register = 0;
    SR2Register = 0;
  }
  /* If I2C2 is Master (MSL flag = 1) */

  if ((SR2Register & 0x0001) == 0x0001)
  {
    /* If ADDR = 1, EV6 */
    if ((SR1Register & 0x0002) == 0x0002)
    {
      /* Write the first data in case the Master is Transmitter */
      if (I2CDirection == I2C_DIRECTION_TX)
      {
        /* Initialize the Transmit counter */
        Tx_Idx2 = 0;
        /* Write the first data in the data register */
		I2C2->DR = Buffer_Tx2[Tx_Idx2++];
        /* Decrement the number of bytes to be written */
        NumbOfBytes2--;
        /* If no further data to be sent, disable the I2C BUF IT
         in order to not have a TxE  interrupt */
        if (NumbOfBytes2 == 0)
        {
          I2C2->CR2 &= (uint16_t) ~I2C_IT_BUF;
        }
      }
      /* Master Receiver */
      else
      {
        /* Initialize Receive counter */
        Rx_Idx2 = 0;
        /* At this stage, ADDR is cleared because both SR1 and SR2 were read.*/
        /* EV6_1: used for single byte reception. The ACK disable and the STOP
         Programming should be done just after ADDR is cleared. */
        if (NumbOfBytes2 == 1)
        {
          /* Clear ACK */
          I2C2->CR1 &= CR1_ACK_Reset;
          /* Program the STOP */
          I2C2->CR1 |= CR1_STOP_Set;
        }
      }
      SR1Register = 0;
      SR2Register = 0;
    }
    /* Master transmits the remaing data: from data2 until the last one.  */
    /* If TXE is set */
    if ((SR1Register & 0x0084) == 0x0080)
    {
      /* If there is still data to write */
      if (NumbOfBytes2 != 0)
      {
        /* Write the data in DR register */
        I2C2->DR = Buffer_Tx2[Tx_Idx2++];
        /* Decrment the number of data to be written */
        NumbOfBytes2--;
        /* If  no data remains to write, disable the BUF IT in order
         to not have again a TxE interrupt. */
        if (NumbOfBytes2 == 0)
        {
          /* Disable the BUF IT */
          I2C2->CR2 &= (uint16_t) ~I2C_IT_BUF;
        }
      }
      SR1Register = 0;
      SR2Register = 0;
    }
    /* If BTF and TXE are set (EV8_2), program the STOP */
    if ((SR1Register & 0x0084) == 0x0084)
    {
      /* Program the STOP */
      I2C2->CR1 |= CR1_STOP_Set;
      /* Disable EVT IT In order to not have again a BTF IT */
      I2C2->CR2 &= (uint16_t) ~I2C_IT_EVT;
      SR1Register = 0;
      SR2Register = 0;
    }
    /* If RXNE is set */
    if ((SR1Register & 0x0040) == 0x0040)
    {
      /* Read the data register */
      Buffer_Rx2[Rx_Idx2++] = I2C2->DR;
      /* Decrement the number of bytes to be read */
      NumbOfBytes2--;
      /* If it remains only one byte to read, disable ACK and program the STOP (EV7_1) */
      if (NumbOfBytes2 == 1)
      {
        /* Clear ACK */
        I2C2->CR1 &= CR1_ACK_Reset;
        /* Program the STOP */
        I2C2->CR1 |= CR1_STOP_Set;
      }
      SR1Register = 0;
      SR2Register = 0;
    }
  }
}

/**
 * @brief  This function handles I2C1 Error interrupt request.
 * @param  None
 * @retval : None
 */
void i2cErrorInterruptHandlerI2c2(void)
{

  __IO uint32_t SR1Register = 0;

  /* Read the I2C1 status register */
  SR1Register = I2C2->SR1;
  /* If AF = 1 */
  if ((SR1Register & 0x0400) == 0x0400)
  {
    I2C1->SR2 &= 0xFBFF;
    SR1Register = 0;
  }
  /* If ARLO = 1 */
  if ((SR1Register & 0x0200) == 0x0200)
  {
    I2C2->SR1 &= 0xFBFF;
    SR1Register = 0;
  }
  /* If BERR = 1 */
  if ((SR1Register & 0x0100) == 0x0100)
  {
    I2C2->SR1 &= 0xFEFF;
    SR1Register = 0;
  }
  /* If OVR = 1 */
  if ((SR1Register & 0x0800) == 0x0800)
  {
    I2C2->SR1 &= 0xF7FF;
    SR1Register = 0;
  }
}

void i2cDmaInterruptHandlerI2c1(void)
{
  if(DMA_GetITStatus(DMA1_IT_TC6))
  {
    DMA_ClearITPendingBit(DMA1_IT_TC6);

//    xSemaphoreGive(i2cdevDmaEventI2c1);
  }
  if(DMA_GetITStatus(DMA1_IT_TC7))
  {
    DMA_ClearITPendingBit(DMA1_IT_TC7);

//    xSemaphoreGive(i2cdevDmaEventI2c1);
  }
}

void i2cDmaInterruptHandlerI2c2(void)
{
  if(DMA_GetITStatus(DMA1_IT_TC4))
  {
    DMA_ClearITPendingBit(DMA1_IT_TC4);

//    xSemaphoreGive(i2cdevDmaEventI2c2);
  }
  if(DMA_GetITStatus(DMA1_IT_TC5))
  {
    DMA_ClearITPendingBit(DMA1_IT_TC5);

 //   xSemaphoreGive(i2cdevDmaEventI2c2);
  }
}

