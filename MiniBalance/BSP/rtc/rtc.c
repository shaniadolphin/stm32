#include "./rtc/rtc.h"

const unsigned char table_week[12] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 }; //月修正数据表
const unsigned char mon_table[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; //平年的月份日期表
unsigned int Time_year;
unsigned char Time_month;
unsigned char Time_date;
unsigned char Time_hour;
unsigned char Time_min;
unsigned char Time_sec;
unsigned int UnixTime;
/*----------------------------------------------------------------------------
 * @描述: Is_Leap_Year
		  判断是否为润年
 * @输入: year
 * @返回: 1：润年；0：不是润年
 * @历史:
 版本        日期          作者           改动内容和原因
 ------    -----------	 ---------	 ----------------------	
 1.0       2015.11.15     zxjun          创建模块	
----------------------------------------------------------------------------*/
unsigned char Is_Leap_Year(unsigned short year)
{
	if (year % 4 == 0) //必须能被4整除
	{
		if (year % 100 == 0)
		{
			if (year % 400 == 0) return 1; //如果以00结尾,还要能被400整除
			else return 0;
		}
		else return 1;
	}
	else return 0;
}
/*----------------------------------------------------------------------------
 * @描述: RTC_Get
		  判断是否为润年
 * @输入: 空
 * @返回: 0：完成更新；其它：错误代码
 * @历史: 
 版本        日期          作者           改动内容和原因
 ------    -----------	 ---------	 ----------------------	
 1.0       2015.11.15     zxjun          创建模块	
----------------------------------------------------------------------------*/
unsigned char RTC_Get(unsigned int timecount)
{
	static unsigned short daycnt = 0;
	//unsigned int timecount=0;
	unsigned int temp = 0;
	unsigned short temp1 = 0;

	//timecount   = RTC->CNTH;//得到计数器中的值(秒钟数)
	//timecount <<= 16;
	//timecount  += RTC->CNTL;

	temp = timecount / 86400;   //得到天数(秒钟数对应的)
	if (daycnt != temp) //超过一天了
	{
		daycnt = temp;
		temp1 = 1970;   //从1970年开始
		while (temp >= 365)
		{
			if (Is_Leap_Year(temp1)) //是闰年
			{
				if (temp >= 366) temp -= 366; //闰年的秒钟数
				else
				{
					temp1++;
					break;
				}
			}
			else temp -= 365;      //平年
			temp1++;
		}
		Time_year = temp1; //得到年份
		temp1 = 0;
		while (temp >= 28) //超过了一个月
		{
			if (Is_Leap_Year(Time_year) && temp1 == 1) //当年是不是闰年/2月份
			{
				if (temp >= 29) temp -= 29; //闰年的秒钟数
				else break;
			}
			else
			{
				if (temp >= mon_table[temp1]) temp -= mon_table[temp1]; //平年
				else break;
			}
			temp1++;
		}
		Time_month = temp1 + 1; //得到月份
		Time_date  = temp  + 1;     //得到日期
	}
	temp      = timecount % 86400;          //得到秒钟数
	Time_hour = temp / 3600;        //小时
	temp = temp % 3600;
	Time_min  = temp / 60;  //分钟
	Time_sec  = temp % 60;  //秒钟

	return 0;
}
/*----------------------------------------------------------------------------
 * @描述: RTC_Set
		  设置时间
 * @输入: syear，smon，sday，hour，min，sec
 * @返回: 0：完成成功；其它：错误代码
 * @历史: 
 版本        日期          作者           改动内容和原因
 ------    -----------	 ---------	 ----------------------	
 1.0       2015.11.15     zxjun          创建模块	
----------------------------------------------------------------------------*/
unsigned char RTC_Set(unsigned short syear, unsigned char smon, unsigned char sday, unsigned char hour, unsigned char min, unsigned char sec)
{
	unsigned short t;
	unsigned int seccount = 0;
	if (syear < 1970 || syear > 2099) return 1;
	for (t = 1970; t < syear; t++) //把所有年份的秒钟相加
	{
		if (Is_Leap_Year(t)) seccount += 31622400; //闰年的秒钟数
		else seccount += 31536000;              //平年的秒钟数
	}
	smon -= 1;
	for (t = 0; t < smon; t++)    //把前面月份的秒钟数相加
	{
		seccount += (u32)mon_table[t] * 86400; //月份秒钟数相加
		if (Is_Leap_Year(syear) && t == 1) seccount += 86400; //闰年2月份增加一天的秒钟数
	}
	seccount += (u32)(sday - 1) * 86400; //把前面日期的秒钟数相加
	seccount += (u32)hour * 3600;           //小时秒钟数
	seccount += (u32)min * 60;                //分钟秒钟数
	seccount += sec;                              //最后的秒钟加上去

	//设置时钟
	RCC->APB1ENR |= 1 << 28; //使能电源时钟
	RCC->APB1ENR |= 1 << 27; //使能备份时钟
	PWR->CR |= 1 << 8;      //取消备份区写保护
							//上面三步是必须的!
	RTC->CRL |= 1 << 4;     //允许配置
	RTC->CNTL = seccount & 0xffff;
	RTC->CNTH = seccount >> 16;
	RTC->CRL &= ~(1 << 4); //配置更新
	while (!(RTC->CRL & (1 << 5))); //等待RTC寄存器操作完成
									//RTC_Get();//设置完之后更新一下数据
	return 0;
}
unsigned char RTC_Set_Unix(unsigned int unixtime)
{
	//设置时钟
	RCC->APB1ENR |= 1 << 28; //使能电源时钟
	RCC->APB1ENR |= 1 << 27; //使能备份时钟
	PWR->CR |= 1 << 8;      //取消备份区写保护
							//上面三步是必须的!
	RTC->CRL |= 1 << 4;     //允许配置
	RTC->CNTL = unixtime & 0xffff;
	RTC->CNTH = unixtime >> 16;
	RTC->CRL &= ~(1 << 4); //配置更新
	while (!(RTC->CRL & (1 << 5))); //等待RTC寄存器操作完成
									//RTC_Get();//设置完之后更新一下数据
	return 0;
}
/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void rtcirqprocess(void)
{
  if(RTC->CRL & 0x0001)//秒钟中断
	{							
	  UnixTime   = ((unsigned int)RTC->CNTH<<16)|RTC->CNTL;//得到计数器中的值(秒钟数)
#if 0
	  //rtc_time_counter = RTC_GetCounter();
		Time_sec ++;
		if(Time_sec == 60)
		{
			Time_sec = 0;
			Time_min ++;
			if(Time_min == 60)
			{
				Time_min = 0;
				Time_hour ++;
				if(Time_hour == 24)
				{
					RTC_Get();	
				}
			}
			
		}
#endif
		//USART_OUT(USART1,"\r\n%d:%d:%d\r\n", Time_hour,Time_min,Time_sec);
 	}
	if(RTC->CRL & 0x0002)//闹钟中断
	{
		RTC->CRL &= ~(0x0002);		//清闹钟中断	  
		//USART_OUT(USART1,"\r\n产生一次ALARM中断\r\n");	   
	} 				  								 
	RTC->CRL &= 0x0FFA;         //清除溢出，秒钟中断标志
	while(!(RTC->CRL & (1<<5)));//等待RTC寄存器操作完成	  
}

/*----------------------------------------------------------------------------
 * @描述: RTC_Configuration
 * @输入: 无
 * @返回: 无
 * @历史:
 版本        日期          作者           改动内容和原因
 ------    -----------	 ---------	 ----------------------	
 1.0       2015.11.15     zxjun          创建模块	
----------------------------------------------------------------------------*/
void RTC_Configuration(void)
{
#if 0
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);

	/* Wait till LSE is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

#if 0
	/* Disable the Tamper Pin */
	BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper
								 functionality must be disabled */
	/* Enable RTC Clock Output on Tamper Pin */
	BKP_RTCCalibrationClockOutputCmd(ENABLE);
#endif

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
#endif
	//检查是不是第一次配置时钟
	//u8 temp=0;
	if (BKP->DR1 != 0x5050) //第一次配置
	{
		RCC->APB1ENR	|=	1 << 28;     //使能电源时钟
		RCC->APB1ENR	|=	1 << 27;     //使能备份时钟
		PWR->CR				|=	1 << 8;      //取消备份区写保护
		RCC->BDCR			|=	1 << 16;     //备份区域软复位
		RCC->BDCR			&=	~(1 << 16);  //备份区域软复位结束
		RCC->BDCR			|=	1 << 0;      //开启外部低速振荡器
		while ((!(RCC->BDCR & 0x02))) //等待外部时钟就绪	 &&temp<250
		{
			//temp++;
			//delay_ms(10);
		};
		//if(temp>=250)return 1;//初始化时钟失败,晶振有问题

		//RCC->BDCR|=1<<8; //LSI作为RTC时钟
		RCC->BDCR 	|= 	RCC_RTCCLKSource_LSE;
		RCC->BDCR		|=	1 << 15; //RTC时钟使能
		while (!(RTC->CRL & (1 << 5))) //等待RTC寄存器操作完成
		{};
		while (!(RTC->CRL & (1 << 3))) //等待RTC寄存器同步
		{};
		RTC->CRH		|=	0x01;         //允许秒中断
		while (!(RTC->CRL & (1 << 5))); //等待RTC寄存器操作完成
		RTC->CRL		|= 	1 << 4;              //允许配置

		RTC->PRLH		=		0x0000;
		RTC->PRLL		=		32767;          //时钟周期设置(有待观察,看是否跑慢了?)理论值：32767

		RTC_Set(2012, 9, 7, 13, 16, 55);  //设置时间

		RTC->CRL		&=	~(1 << 4);           //配置更新
		while (!(RTC->CRL & (1 << 5)));   //等待RTC寄存器操作完成
		BKP->DR1		=		0x5050;

	}
	else //系统继续计时
	{
		while (!(RTC->CRL & (1 << 3))); //等待RTC寄存器同步
		RTC->CRH	|=	0x01;         //允许秒中断
		while (!(RTC->CRL & (1 << 5))); //等待RTC寄存器操作完成
	}
	//RTC_Get();//更新时间
	//return 0; //ok
}



