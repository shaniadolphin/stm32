#include "./rtc/rtc.h"

const unsigned char table_week[12] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 }; //���������ݱ�
const unsigned char mon_table[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; //ƽ����·����ڱ�
unsigned int Time_year;
unsigned char Time_month;
unsigned char Time_date;
unsigned char Time_hour;
unsigned char Time_min;
unsigned char Time_sec;
unsigned int UnixTime;
/*----------------------------------------------------------------------------
 * @����: Is_Leap_Year
		  �ж��Ƿ�Ϊ����
 * @����: year
 * @����: 1�����ꣻ0����������
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------	
 1.0       2015.11.15     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
unsigned char Is_Leap_Year(unsigned short year)
{
	if (year % 4 == 0) //�����ܱ�4����
	{
		if (year % 100 == 0)
		{
			if (year % 400 == 0) return 1; //�����00��β,��Ҫ�ܱ�400����
			else return 0;
		}
		else return 1;
	}
	else return 0;
}
/*----------------------------------------------------------------------------
 * @����: RTC_Get
		  �ж��Ƿ�Ϊ����
 * @����: ��
 * @����: 0����ɸ��£��������������
 * @��ʷ: 
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------	
 1.0       2015.11.15     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
unsigned char RTC_Get(unsigned int timecount)
{
	static unsigned short daycnt = 0;
	//unsigned int timecount=0;
	unsigned int temp = 0;
	unsigned short temp1 = 0;

	//timecount   = RTC->CNTH;//�õ��������е�ֵ(������)
	//timecount <<= 16;
	//timecount  += RTC->CNTL;

	temp = timecount / 86400;   //�õ�����(��������Ӧ��)
	if (daycnt != temp) //����һ����
	{
		daycnt = temp;
		temp1 = 1970;   //��1970�꿪ʼ
		while (temp >= 365)
		{
			if (Is_Leap_Year(temp1)) //������
			{
				if (temp >= 366) temp -= 366; //�����������
				else
				{
					temp1++;
					break;
				}
			}
			else temp -= 365;      //ƽ��
			temp1++;
		}
		Time_year = temp1; //�õ����
		temp1 = 0;
		while (temp >= 28) //������һ����
		{
			if (Is_Leap_Year(Time_year) && temp1 == 1) //�����ǲ�������/2�·�
			{
				if (temp >= 29) temp -= 29; //�����������
				else break;
			}
			else
			{
				if (temp >= mon_table[temp1]) temp -= mon_table[temp1]; //ƽ��
				else break;
			}
			temp1++;
		}
		Time_month = temp1 + 1; //�õ��·�
		Time_date  = temp  + 1;     //�õ�����
	}
	temp      = timecount % 86400;          //�õ�������
	Time_hour = temp / 3600;        //Сʱ
	temp = temp % 3600;
	Time_min  = temp / 60;  //����
	Time_sec  = temp % 60;  //����

	return 0;
}
/*----------------------------------------------------------------------------
 * @����: RTC_Set
		  ����ʱ��
 * @����: syear��smon��sday��hour��min��sec
 * @����: 0����ɳɹ����������������
 * @��ʷ: 
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------	
 1.0       2015.11.15     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
unsigned char RTC_Set(unsigned short syear, unsigned char smon, unsigned char sday, unsigned char hour, unsigned char min, unsigned char sec)
{
	unsigned short t;
	unsigned int seccount = 0;
	if (syear < 1970 || syear > 2099) return 1;
	for (t = 1970; t < syear; t++) //��������ݵ��������
	{
		if (Is_Leap_Year(t)) seccount += 31622400; //�����������
		else seccount += 31536000;              //ƽ���������
	}
	smon -= 1;
	for (t = 0; t < smon; t++)    //��ǰ���·ݵ����������
	{
		seccount += (u32)mon_table[t] * 86400; //�·����������
		if (Is_Leap_Year(syear) && t == 1) seccount += 86400; //����2�·�����һ���������
	}
	seccount += (u32)(sday - 1) * 86400; //��ǰ�����ڵ����������
	seccount += (u32)hour * 3600;           //Сʱ������
	seccount += (u32)min * 60;                //����������
	seccount += sec;                              //�������Ӽ���ȥ

	//����ʱ��
	RCC->APB1ENR |= 1 << 28; //ʹ�ܵ�Դʱ��
	RCC->APB1ENR |= 1 << 27; //ʹ�ܱ���ʱ��
	PWR->CR |= 1 << 8;      //ȡ��������д����
							//���������Ǳ����!
	RTC->CRL |= 1 << 4;     //��������
	RTC->CNTL = seccount & 0xffff;
	RTC->CNTH = seccount >> 16;
	RTC->CRL &= ~(1 << 4); //���ø���
	while (!(RTC->CRL & (1 << 5))); //�ȴ�RTC�Ĵ����������
									//RTC_Get();//������֮�����һ������
	return 0;
}
unsigned char RTC_Set_Unix(unsigned int unixtime)
{
	//����ʱ��
	RCC->APB1ENR |= 1 << 28; //ʹ�ܵ�Դʱ��
	RCC->APB1ENR |= 1 << 27; //ʹ�ܱ���ʱ��
	PWR->CR |= 1 << 8;      //ȡ��������д����
							//���������Ǳ����!
	RTC->CRL |= 1 << 4;     //��������
	RTC->CNTL = unixtime & 0xffff;
	RTC->CNTH = unixtime >> 16;
	RTC->CRL &= ~(1 << 4); //���ø���
	while (!(RTC->CRL & (1 << 5))); //�ȴ�RTC�Ĵ����������
									//RTC_Get();//������֮�����һ������
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
  if(RTC->CRL & 0x0001)//�����ж�
	{							
	  UnixTime   = ((unsigned int)RTC->CNTH<<16)|RTC->CNTL;//�õ��������е�ֵ(������)
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
	if(RTC->CRL & 0x0002)//�����ж�
	{
		RTC->CRL &= ~(0x0002);		//�������ж�	  
		//USART_OUT(USART1,"\r\n����һ��ALARM�ж�\r\n");	   
	} 				  								 
	RTC->CRL &= 0x0FFA;         //�������������жϱ�־
	while(!(RTC->CRL & (1<<5)));//�ȴ�RTC�Ĵ����������	  
}

/*----------------------------------------------------------------------------
 * @����: RTC_Configuration
 * @����: ��
 * @����: ��
 * @��ʷ:
 �汾        ����          ����           �Ķ����ݺ�ԭ��
 ------    -----------	 ---------	 ----------------------	
 1.0       2015.11.15     zxjun          ����ģ��	
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
	//����ǲ��ǵ�һ������ʱ��
	//u8 temp=0;
	if (BKP->DR1 != 0x5050) //��һ������
	{
		RCC->APB1ENR	|=	1 << 28;     //ʹ�ܵ�Դʱ��
		RCC->APB1ENR	|=	1 << 27;     //ʹ�ܱ���ʱ��
		PWR->CR				|=	1 << 8;      //ȡ��������д����
		RCC->BDCR			|=	1 << 16;     //����������λ
		RCC->BDCR			&=	~(1 << 16);  //����������λ����
		RCC->BDCR			|=	1 << 0;      //�����ⲿ��������
		while ((!(RCC->BDCR & 0x02))) //�ȴ��ⲿʱ�Ӿ���	 &&temp<250
		{
			//temp++;
			//delay_ms(10);
		};
		//if(temp>=250)return 1;//��ʼ��ʱ��ʧ��,����������

		//RCC->BDCR|=1<<8; //LSI��ΪRTCʱ��
		RCC->BDCR 	|= 	RCC_RTCCLKSource_LSE;
		RCC->BDCR		|=	1 << 15; //RTCʱ��ʹ��
		while (!(RTC->CRL & (1 << 5))) //�ȴ�RTC�Ĵ����������
		{};
		while (!(RTC->CRL & (1 << 3))) //�ȴ�RTC�Ĵ���ͬ��
		{};
		RTC->CRH		|=	0x01;         //�������ж�
		while (!(RTC->CRL & (1 << 5))); //�ȴ�RTC�Ĵ����������
		RTC->CRL		|= 	1 << 4;              //��������

		RTC->PRLH		=		0x0000;
		RTC->PRLL		=		32767;          //ʱ����������(�д��۲�,���Ƿ�������?)����ֵ��32767

		RTC_Set(2012, 9, 7, 13, 16, 55);  //����ʱ��

		RTC->CRL		&=	~(1 << 4);           //���ø���
		while (!(RTC->CRL & (1 << 5)));   //�ȴ�RTC�Ĵ����������
		BKP->DR1		=		0x5050;

	}
	else //ϵͳ������ʱ
	{
		while (!(RTC->CRL & (1 << 3))); //�ȴ�RTC�Ĵ���ͬ��
		RTC->CRH	|=	0x01;         //�������ж�
		while (!(RTC->CRL & (1 << 5))); //�ȴ�RTC�Ĵ����������
	}
	//RTC_Get();//����ʱ��
	//return 0; //ok
}



