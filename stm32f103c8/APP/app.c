
/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#define GLOBALS

#include "stdarg.h"

#include "includes.h"
#include "globals.h"

#include "stm32f10x_dma.h"
#include "stm32f10x_adc.h"

#include "spi.h"

#include "fsmc_sram.h"

#include "stdio.h"

//#include "..\AS3911\as3911.h"
#include "..\AS3911\as3911_com.h"
#include "..\AS3911\as3911_interrupt.h"
#include "..\AS3911\as3911_com.h"
#include "Msg.h"
//#include "at_prc.h"
#include "nfc.h"

#include "sys_register.h"

enum{
	STATE_NORMAL = 0,
	STATE_RECEIVED_CODE = 1,
	STATE_REQUEST_REGISTER = 2,
	STATE_REGISTER_PASS = 3,
	STATE_UPDATE_WIFI1 = 6,
	STATE_UPDATE_WIFI2 = 7,
	STATE_UPDATE_WIFI3 = 8,
	STATE_UPDATE_WIFI4 = 9,
	STATE_UPDATE_WIFI5 = 10,
	STATE_UPDATE_DOMAIN = 16
};

#define ENDURE_FAILURE_TIMES		30

OS_EVENT *Com1_SEM;
OS_EVENT *Com1_MBOX;

OS_EVENT *SeMbox;   //定义发送消息邮箱数组指针
OS_EVENT *Key_BOX;   //定义发送消息邮箱数组指针

OS_EVENT *SemSpiflash;

/*
unsigned char recd_buf[64] = {
	0x55, 0x1e, 0x00, 0xa0, 0x00, 0xf3, 0xc0, 0x00,
	0x00, 0x00, 0x54, 0x01, 0x9f, 0xa6, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x00, 0xcc, 0x54, 0x39,
	0x3a, 0x66, 0xaa, 0x1c, 0xb6, 0xc5, 0x55
}; //接收缓冲区
*/
const unsigned char segment_data[16] =
{
	0x3f, //0 c0
	0x06, //1 f9
	0x5b, //2 a4
	0x4f, //3 b0
	0x66, //4 99
	0x6d, //5 92
	0x7d, //6 82
	0x07, //7 f8
	0x7f, //8 80
	0x6f, //9 90
	0x77, //a
	0x7c, //b
	0x39, //c
	0x5e, //d
	0x79, //e
	0x71 //f
};

//ESP8266模块连接AP指令
const char AT_CWJAP[] = "AT+CWJAP_CUR=";

const unsigned char LinkSeg[3] = {B_MIN_HUN, B_MIN_TEN, B_MIN_ONE};

//const char *tcp_ip = "221.238.40.119";
const char *tcp_ip = "www.ucanchess.com";
const char *tcp_port = "30012";

//本地解密KEY
unsigned char encrypt_key;
unsigned char DOMAIN[25] = {0};
unsigned short bat_vol = 4500;
unsigned char state_led_bak;
unsigned char bat_percent = 100;
unsigned char wifi_state;

unsigned short NetworkTimeOutCounter = NETWORK_TIME_OUT;

extern unsigned char display_dat_buf[13];
extern char msg[64];
extern unsigned char white_led;
extern unsigned char black_led;
extern unsigned char state_led;
extern unsigned char WifiStatusFlag;
extern unsigned int Time_year;
extern unsigned char Time_month;
extern unsigned char Time_date;
extern unsigned char Time_hour;
extern unsigned char Time_min;
extern unsigned char Time_sec;
extern unsigned int UnixTime;
extern unsigned char CpuID[];
extern unsigned char printenable;
extern SPI_CFG cfg;
extern TX_CFG tx_cfg;
extern unsigned char SendBuff[];

extern unsigned int led_flash_flag;

void LED_N_STATUS(unsigned char N, unsigned char STATUS);
void USART_OUT(USART_TypeDef *USARTx, char *Data, ...);
void printk(char *Data, ...);
char *itoa(int value, char *string, int radix);
extern void fun_para(void);

extern void NfcHandlerTask(void);    //<lfx
extern void Set_USBClock(unsigned char enbit);
extern void USB_Init(void);
extern unsigned char ESP8266_Int(void);
extern void USART_Config(USART_TypeDef *USARTx, u32 baud);
extern unsigned char SetTCPLINK(void);
extern unsigned char RTC_Get(unsigned int timecount);
extern unsigned char RTC_Set_Unix(unsigned int unixtime);
extern void SysinfoRead(void);
extern void SysinfoSave(void);
extern void testSpiflash(void);
extern void InitTags(void);


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK App_TaskStartStk[APP_TASK_START_STK_SIZE];
static  OS_STK Task_Led1Stk[Task_Led1_STK_SIZE];
static  OS_STK Task_Led2Stk[Task_Led2_STK_SIZE];

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void App_TaskCreate(void);

static  void App_TaskStart(void *p_arg);

static  void Task_Led1(void *p_arg);
static  void Task_Led2(void *p_arg);


/*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument : none.
*
* Return   : none.
*********************************************************************************************************/

int main(void)
{
	CPU_INT08U os_err;

	//禁止CPU中断
	CPU_IntDis();

	//UCOS 初始化
	OSInit();       /* Initialize "uC/OS-II, The Real-Time Kernel".         */

	//硬件平台初始化
	BSP_Init(); /* Initialize BSP functions.  */
	/* Configure FSMC Bank1 NOR/PSRAM */
	//Set_USBClock(1);
	//USB_Init();
	MsgInit();

	//建立主任务， 优先级最高  建立这个任务另外一个用途是为了以后使用统计任务
	os_err = OSTaskCreate((void (*)(void*))App_TaskStart,                    //指向任务代码的指针
						  (void*)0,                                     //任务开始执行时，传递给任务的参数的指针
						  (OS_STK*)&App_TaskStartStk[APP_TASK_START_STK_SIZE - 1], //分配给任务的堆栈的栈顶指针   从顶向下递减
						  (INT8U)APP_TASK_START_PRIO);                              //分配给任务的优先级
	os_err = os_err;
	//ucos的节拍计数器清0    节拍计数器是0-4294967295    对于节拍频率1000hz时， 每隔49.7天就重新计数
	OSTimeSet(0);
	OSStart();                                    /* Start multitasking (i.e. give control to uC/OS-II).  */
	/* Start multitasking (i.e. give control to uC/OS-II).  */

	return (0);
}

//const unsigned char test_cpuid[12] = {0X53,0xff,0x6e,0x6,0x48,0x85,0x57,0x57,0x56,0x29,0x11,0x87};
void read_cpu_id(void)
{
	int i;
	volatile unsigned char *pcpuid = (volatile unsigned char *)0x1ffff7e8;

	printk("CPUID is 0X");
	for (i = 0; i < 12; i++)
	{
		CpuID[i] = *pcpuid++;
		//CpuID[i] = test_cpuid[i];
		printk("%02x-", CpuID[i]);
	}
	printk("\r\n");
}

//获取密key
unsigned char get_encrypt_key(void)
{
	char ret;

	ret  = (CpuID[1] & 0xc0)>>6;
	ret <<= 2;
	ret |= (CpuID[3] & 0xc0)>>6;
	ret <<= 2;
	ret |= (CpuID[5] & 0xc0)>>6;
	ret <<= 2;
	ret |= (CpuID[7] & 0xc0)>>6;

	printk("encrypt key = 0x%02x, 0x%02x 0x%02x 0x%02x 0x%02x\r\n", ret, CpuID[1], CpuID[3], CpuID[5], CpuID[7]);

	return ret;

}

void USART_TX(USART_TypeDef *USARTx, unsigned char ch)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
	USART_SendData(USARTx, ch);
}

unsigned char getTimeOutTxBuf(void)
{
	int i;
	unsigned int interval;
	unsigned int currTime = UnixTime;

	for (i = 0; i < MAX_TX_BUFF_LEN; i++)
	{
		if (tx_cfg.tx_buf[i].step < MAX_STEPS_PER_GAME)
		{
			interval = currTime - tx_cfg.tx_buf[i].time;
			if (interval > TX_TIME_OUT)
			{
				//printk("### currTime = %x, tx_cfg.tx_buf[i].time = %x ###\r\n", currTime, tx_cfg.tx_buf[i].time);
				return i;
			}
		}
	}
	return MAX_TX_BUFF_LEN;
}

unsigned char getEmptyTxBuf(void)
{
	int i;

	for (i = 0; i < MAX_TX_BUFF_LEN; i++)
	{
		if (tx_cfg.tx_buf[i].step >= MAX_STEPS_PER_GAME)
		{
			return i;
		}
	}
	return MAX_TX_BUFF_LEN;
}

unsigned char getMinTxBuf(void)
{
	int i;
	unsigned short min_step = MAX_STEPS_PER_GAME;
	unsigned char buf_no = MAX_TX_BUFF_LEN;

	for (i = 0; i < MAX_TX_BUFF_LEN; i++)
	{
		if (tx_cfg.tx_buf[i].step < min_step)
		{
			min_step = tx_cfg.tx_buf[i].step;
			buf_no = i;
		}
	}
	return buf_no;
}

void setTxBuf(unsigned char buf_no, unsigned short step)
{
	if (buf_no < MAX_TX_BUFF_LEN)
	{
		tx_cfg.tx_buf[buf_no].step = step;
		tx_cfg.tx_buf[buf_no].time = UnixTime;
	}
}

void clearTxBuf(unsigned short step)
{
	short i;

	for (i = 0; i < MAX_TX_BUFF_LEN; i++)
	{
		if (tx_cfg.tx_buf[i].step == step)
		{
			tx_cfg.tx_buf[i].step = MAX_STEPS_PER_GAME;
			tx_cfg.tx_buf[i].time = 0;
		}
	}
}

void clearAllTxBuf(void)
{
	short i;

	for (i = 0; i < MAX_TX_BUFF_LEN; i++)
	{
		tx_cfg.tx_buf[i].step = MAX_STEPS_PER_GAME;
		tx_cfg.tx_buf[i].time = 0;
	}
}

unsigned char getToTxGame(void)
{
	unsigned char i;

	for (i = 0; i < MAX_GAME_CNT; i++)
	{
		if (cfg.game_steps_cnt[i] != 0)
		{
			break;
		}
	}
	return  i;
}

void update_machine_time(unsigned char *time_buf)
{
	unsigned int net_unix_time = 0;

	net_unix_time |= time_buf[0];
	net_unix_time = net_unix_time << 8;
	net_unix_time |= time_buf[1];
	net_unix_time = net_unix_time << 8;
	net_unix_time |= time_buf[2];
	net_unix_time = net_unix_time << 8;
	net_unix_time |= time_buf[3];
	//printk("\r\nnet_unix_time=0x%x\r\n",net_unix_time);
	RTC_Get(net_unix_time);
	printk("\r\n服务器时间\r\n");
	printk("%d年%d月%d日 ", Time_year, Time_month, Time_date);
	printk("%d:%d:%d\r\n", (Time_hour + 8) % 24, Time_min, Time_sec);
	if ((abs(net_unix_time - UnixTime) > 30) && CheckMsg(FLAG_RTC_UPDATE) == 0)
	{
		RTC_Set_Unix(net_unix_time);
		SetMsg(FLAG_RTC_UPDATE);
		printk("\r\n更新时间aaa%08x\r\n", net_unix_time);
	}
}

/*
*********************************************************************************************************
*                                          App_TaskStart()
*
* Description : The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
*
* Argument : p_arg       Argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
*
* Return   : none.
*
* Caller   : This is a task.
*
* Note     : none.
*********************************************************************************************************
*/
static  void App_TaskStart(void *p_arg)
{
	unsigned char retry_cnt = 0;
	unsigned char waittimesec = 0;

	//初始化ucos时钟节拍
	OS_CPU_SysTickInit();                                      /* Initialize the SysTick.       */
	//使能ucos 的统计任务
	#if (OS_TASK_STAT_EN > 0)
	//----统计任务初始化函数
	OSStatInit();                                               /* Determine CPU capacity.                              */
	#endif

	SemSpiflash = OSSemCreate(1);

	sys_power_cmd(1);
	SysinfoRead();
	read_register_info();
	read_domain_info(DOMAIN);
	read_cpu_id();
	encrypt_key = get_encrypt_key();

	InitTags();

	tx_cfg.curr_tx_step = 0;
	tx_cfg.his_tx_step = 0;
	tx_cfg.curr_rx_step = 0;
	tx_cfg.his_rx_step = 0;
	tx_cfg.curr_tx_game = MAX_GAME_CNT;
	tx_cfg.his_tx_game = MAX_GAME_CNT;
	tx_cfg.heartbeat_tx_step = 0;
	tx_cfg.heartbeat_rx_step = 0;
	tx_cfg.heartbeat_tx_time = UnixTime;
	wifi_state = POWERUP;
	App_TaskCreate();

	while (1)
	{
		if (GetMsg(MSG_ENTER_SMARTLINK))
		{
			wifi_state = SMARTWIFI;
		}
		switch (wifi_state)
		{
			case POWERUP:
			{
				state_led = LED_SLOW_SHINE;
				state_led_bak = LED_SLOW_SHINE;
				if (0 == CheckMsg(FLAG_SYS_REGISTER))
				{
					white_led = LED_FAST_SHINE;
					black_led = LED_FAST_SHINE;
				}

				WifiStatusFlag = 0;
				GPIO_ResetBits(GPIOB, GPIO_Pin_11);  //WIFI RST
				OSTimeDlyHMSM(0, 0, 0, 30);
				GPIO_SetBits(GPIOB, GPIO_Pin_11);    //WIFI RST
				OSTimeDlyHMSM(0, 0, 2, 0);
				USART_Config(USART1, 115200);          //串口1初始化
				USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
				USART_OUT(USART1, "AT+RST\r\n"); //发送复位指令
				OSTimeDlyHMSM(0, 0, 1, 100);

				for (retry_cnt = 10; retry_cnt > 0; retry_cnt--)
				{
					OSTimeDlyHMSM(0, 0, 2, 0);
					//printk("rstmsg=%s\r\n", msg);
					if (WifiStatusFlag&FLAG_WIFI_SSID_GOTIP)
					{
						break;
					}
				}
				if (retry_cnt)
				{
					printk("POWERUP Auto Connect can GOTIP \r\n");
					USART_OUT(USART1, "ATE0\r\n"); //关闭指令回显
					OSTimeDlyHMSM(0, 0, 0, 200);

					WifiStatusFlag = 0;
					USART_OUT(USART1, "AT\r\n"); //关闭指令回显
					OSTimeDlyHMSM(0, 0, 0, 300);

					if (WifiStatusFlag&FLAG_WIFI_CMD_OK)
					{
						printk("POWERUP Auto Connect ATE0 return ok \r\n");
						wifi_state = SETTCPLINK;
						WifiStatusFlag = 0;
						break;
					}
				}
				else
				{
					wifi_state = PROGRAME_SET_AP;
				}
				break;
			}

			case SMARTWIFI: //进入SMARTWIFI
			{
				printk("ENTER SMARTWIFI...\r\n");
				state_led = LED_OFF;
				printenable = 1;
				WifiStatusFlag = 0;
				USART_OUT(USART1, "AT+CWQAP\r\n"); //退出和当前AP的链接
				OSTimeDlyHMSM(0, 0, 0, 500);
				if (WifiStatusFlag&FLAG_WIFI_CMD_OK)
				{
					printk("quit ap connect success...\r\n");
				}

				//初始化开启智能WIFI次数
				USART_OUT(USART1, "AT+CWMODE=1\r\n"); //开启STATION模式
				OSTimeDlyHMSM(0, 0, 0, 200);
				WifiStatusFlag = 0;
				USART_OUT(USART1, "AT+CWSMARTSTART\r\n"); //开启智能WIFI
				for (retry_cnt = 100; retry_cnt > 0; retry_cnt--)
				{
					printk("SMARTWIFI OPEN SMARTSTART recvmsg:%s %02x\r\n", msg, WifiStatusFlag);
					OSTimeDlyHMSM(0, 0, 0, 500);
					if (WifiStatusFlag&FLAG_WIFI_CMD_OK)
					{
						break;
					}
				}
				//user has 2mins to setup and init wifi ssid and code
				if (retry_cnt)
				{
					for (waittimesec = 60; waittimesec > 0; waittimesec--)
					{
						OSTimeDlyHMSM(0, 0, 1, 0);
						printk("SMARTWIFI USER retrytimes:%d waittimesec,%02x\r\n", waittimesec, WifiStatusFlag);
						if (WifiStatusFlag&FLAG_WIFI_SSID_GOTIP)
						{
							break;
						}
					}
				}
				else
				{
					waittimesec = 0;
				}
				USART_OUT(USART1, "AT+CWSTOPSMART\r\n"); //退出智能WIFI
				if (waittimesec)
				{
					printk("SMARTWIFI user setting success \r\n");
					wifi_state = SETTCPLINK;
				}
				else
				{
					printk("SMARTWIFI user setting failed \r\n");
					wifi_state = POWERUP;
				}

				printenable = 0;
				state_led = LED_SLOW_SHINE;
				break;
			}

			case PROGRAME_SET_AP:
			{
				char tmp_at[50];
				unsigned char ssid[WIFI_MAX_LEN];
				unsigned char key[WIFI_MAX_LEN];
				int i;

				printk("PROGRAME_SET_AP\r\n");
				//循环测试链接指定AP
				for (i = 0; i < MAX_WIFI_NUM; i++)
				{
					if (read_wifi(i, ssid, key))
					{
						WifiStatusFlag = 0;
						printk("Connecting wifi: no=%d, SSID=%s, KEY=%s\r\n", i, ssid, key);
						sprintf(tmp_at, "%s\"%s\",\"%s\"\r\n%c", AT_CWJAP, ssid, key, '\0');
						USART_OUT(USART1, tmp_at);
						//printk("AT instruction is %s \r\n", tmp_at);

						for (retry_cnt = 10; retry_cnt > 0; retry_cnt--)
						{
							OSTimeDlyHMSM(0, 0, 1, 0);
							if (CheckMsg(MSG_ENTER_SMARTLINK))
							{
								goto connect_break;
							}
							if (WifiStatusFlag&FLAG_WIFI_SSID_GOTIP)
							{
								wifi_state = SETTCPLINK;
								printk("Local ap program connect success\r\n");
								goto connect_break;
							}
							printk("connecting retry_cnt=%d \r\n", retry_cnt);
						}
					}
				}
				//自动设置失败 进入SMARTWIFI SETTING
				wifi_state = SMARTWIFI;
				printk("Local ap program connect failed\r\n");
			connect_break:
				break;
			}

			case SETTCPLINK:
			{
				printk("SETTCPLINK\r\n");

				state_led = LED_SLOW_SHINE;
				state_led_bak = LED_SLOW_SHINE;
				for (retry_cnt = 20; retry_cnt > 0; retry_cnt--)
				{
					//设置TCP连接
					printk("Connecting %s:%s --> %d\r\n", DOMAIN, tcp_port, retry_cnt);
					WifiStatusFlag = 0;
					USART_OUT(USART1, "AT+CIPSTART=");
					USART_TX(USART1, 0x22);
					USART_OUT(USART1, "TCP");
					USART_TX(USART1, 0x22);
					USART_OUT(USART1, ",");
					USART_TX(USART1, 0x22);
					USART_OUT(USART1, "%s", DOMAIN);
					USART_TX(USART1, 0x22);
					USART_OUT(USART1, ",");
					USART_OUT(USART1, "%s\r\n", tcp_port);

					OSTimeDlyHMSM(0, 0, 0, 500);

					if (WifiStatusFlag&FLAG_WIFI_CMD_OK) // TCP 连接建立 设置成功
					{
						wifi_state = SYS_REGISTER;
						printk("Connect OK !\r\n");
						break;
					}
					if (CheckMsg(MSG_ENTER_SMARTLINK))
					{
						break;
					}
					OSTimeDlyHMSM(0, 0, 2, 0);
				}
				if (retry_cnt == 0)
				{
					printk("Connect fail !!!\r\n");
					wifi_state = POWERUP;
				}
				break;
			}

			case SYS_REGISTER:
			{
				unsigned char i;
				unsigned char shine_pos;
				unsigned char ret;
				unsigned short step_received;

				if (CheckMsg(FLAG_SYS_REGISTER)) //已注册
				{
					printk("SYS_REGISTER: System has already register\r\n");

					data_buf[0] = 0;
					for (retry_cnt = 20; retry_cnt > 0; retry_cnt--)
					{
						SendHeartbeat(CMD_HEARTBEAT, 0, UnixTime, bat_percent, STATE_NORMAL);
						OSTimeDlyHMSM(0, 0, 0, 500);
						printk("rtctime retrycnt %d*500ms\r\n", 20 - retry_cnt);
						if (data_buf[0] != 0)
						{
							data_buf[0] = 0;
							EncryptData(&data_buf[3], ENCRYPT_LEN, encrypt_key);
							printk("data_buf[]=");
							printf_buf_hex(&data_buf[1], DATA_SEND_LEN);
							if (calc_sum(&data_buf[1], DATA_SEND_LEN - 1) == data_buf[DATA_SEND_LEN])
							{
								NetworkTimeOutCounter = NETWORK_TIME_OUT;
								update_machine_time(&data_buf[9]);
								break;
							}
							else
							{
								printk("checksum error = %d\r\n", __LINE__);
							}
						}
						if (CheckMsg(MSG_ENTER_SMARTLINK))
						{
							goto register_break;
						}
					}
					if (retry_cnt == 0)
					{
						//printk("\r\n服务器连接失败\r\n");
						wifi_state = POWERUP;	//服务器连接失败，wifi模块重启
					}
					else
					{
						wifi_state = TCPLINKOK;
					}
				}
				else if (GetMsg(FLAG_SERVER_CPUID))
				{
				factory_report:
					ret = 0;
					data_buf[0] = 0;
					for (retry_cnt = 10; retry_cnt > 0; retry_cnt--)
					{
						send_cpuid_version(CMD_PRODUCE, 1, CpuID, SOFTWARE_VER, 0xff);
						OSTimeDlyHMSM(0, 0, 0, 500);
						printk("Send cpuid to server %d ...\r\n", retry_cnt);
						ret = receive_factory_produce_result(0xff);
						if (ret)
						{
							break;
						}
					}
					if (ret == 0)
					{
						wifi_state = POWERUP;
					}
				}
				else
				{
					printk("SYS_REGISTER: System enter register program \r\n");
					//注册流程
					ret = 0;
					data_buf[0] = 0;
					for (retry_cnt = 10; retry_cnt > 0; retry_cnt--)
					{
						send_cpuid_version(CMD_REGISTER, 1, CpuID, SOFTWARE_VER, 0xff);
						OSTimeDlyHMSM(0, 0, 0, 500);
						printk("note:please waiting,system registering...%d times Countdown\r\n", retry_cnt);
						ret = receive_imei_info();
						if (ret)
						{
							//SendHeartbeat(CMD_HEARTBEAT, tx_cfg.heartbeat_tx_step, UnixTime, bat_percent, STATE_RECEIVED_CODE);
							break;
						}
						if (CheckMsg(MSG_ENTER_SMARTLINK))
						{
							goto register_break;
						}
						if (GetMsg(FLAG_SERVER_CPUID))
						{
							goto factory_report;
						}
					}
					if (ret)
					{
						ClearMsg(KEY_SHORT_BLACK);
						while (1)
						{
							OSTimeDlyHMSM(0, 0, 0, 200);
							if (GetMsg(KEY_SHORT_BLACK))
							{
								break;
							}
							if (CheckMsg(MSG_ENTER_SMARTLINK))
							{
								goto register_break;
							}
							if (GetMsg(FLAG_SERVER_CPUID))
							{
								goto factory_report;
							}
						}
						display_dat_buf[W_FAULT] = display_dat_buf[W_MIN_ONE];
						display_dat_buf[W_MIN_HUN] = display_dat_buf[W_SEC_TEN];
						display_dat_buf[W_MIN_TEN] = display_dat_buf[W_SEC_ONE];
						display_dat_buf[W_MIN_ONE] = display_dat_buf[B_MIN_HUN];
						display_dat_buf[W_SEC_TEN] = display_dat_buf[B_MIN_TEN];
						display_dat_buf[W_SEC_ONE] = display_dat_buf[B_MIN_ONE];
						display_dat_buf[B_MIN_HUN] = 0;
						display_dat_buf[B_MIN_TEN] = 0;
						display_dat_buf[B_MIN_ONE] = 0;
						display_dat_buf[B_SEC_TEN] = segment_data[6];
						display_dat_buf[B_SEC_ONE] = segment_data[0];
						display_dat_buf[B_FAULT] = 0;
						for (waittimesec = 0; waittimesec < 60; waittimesec++)
						{
							SendHeartbeat(CMD_HEARTBEAT, tx_cfg.heartbeat_tx_step, UnixTime, bat_percent, STATE_REQUEST_REGISTER);
							tx_cfg.heartbeat_tx_step = (tx_cfg.heartbeat_tx_step + 1) & 0x1fff;
							OSTimeDlyHMSM(0, 0, 1, 0);
							shine_pos = waittimesec%(sizeof(LinkSeg)+1);
							if (shine_pos == 0)
							{
								display_dat_buf[LinkSeg[0]] = 0x40;
								display_dat_buf[LinkSeg[1]] = 0;
								display_dat_buf[LinkSeg[2]] = 0;
							}
							else if (shine_pos == 1)
							{
								display_dat_buf[LinkSeg[0]] = 0x40;
								display_dat_buf[LinkSeg[1]] = 0x40;
								display_dat_buf[LinkSeg[2]] = 0;
							}
							else if (shine_pos == 2)
							{
								display_dat_buf[LinkSeg[0]] = 0x40;
								display_dat_buf[LinkSeg[1]] = 0x40;
								display_dat_buf[LinkSeg[2]] = 0x40;
							}
							else
							{
								display_dat_buf[LinkSeg[0]] = 0;
								display_dat_buf[LinkSeg[1]] = 0;
								display_dat_buf[LinkSeg[2]] = 0;
							}
							display_dat_buf[B_SEC_TEN] = segment_data[(59 - waittimesec)/10];
							display_dat_buf[B_SEC_ONE] = segment_data[(59 - waittimesec)%10];
							if (data_buf[0] != 0)
							{
								data_buf[0] = 0;
								EncryptData(&data_buf[3], ENCRYPT_LEN, encrypt_key);
								printk("Rx: ");
								printf_buf_hex(&data_buf[1], DATA_SEND_LEN);
								if (calc_sum(&data_buf[1], DATA_SEND_LEN - 1) == data_buf[DATA_SEND_LEN])
								{
									NetworkTimeOutCounter = NETWORK_TIME_OUT;
									if ((data_buf[1] & 0xe0) == 0x00)
									{
										step_received = data_buf[1] & 0x07; //从接收到返回数据计算当前接收到的步数
										step_received = step_received << 8;
										step_received = step_received + data_buf[2];
										tx_cfg.heartbeat_rx_step = step_received;
										if (data_buf[15] == STATE_REGISTER_PASS) //收到服务器返回握手信息
										{
											SetMsg(FLAG_REGISTER_END);
											printk("#########\r\n");
											break;
										}
									}
								}
								else
								{
									printk("checksum error = %d\r\n", __LINE__);
								}
							}
							if (CheckMsg(MSG_ENTER_SMARTLINK))
							{
								goto register_break;
							}
							if (GetMsg(FLAG_SERVER_CPUID))
							{
								goto factory_report;
							}
						}
					}
					led_flash_flag = 0;
					if (GetMsg(FLAG_REGISTER_END))
					{
						for (i = 0; i < 13; i++)
						{
							display_dat_buf[i] = segment_data[0];
						}
						wifi_state = TCPLINKOK;
						white_led = LED_OFF;
						black_led = LED_OFF;
						SetMsg(FLAG_SYS_REGISTER);
						save_register_info();
						printk("system register success\r\n");
					}
					else
					{
						for (i = 0; i < 13; i++)
						{
							display_dat_buf[i] = 0;
						}
						display_dat_buf[M_STATE] = segment_data[0xf];
						wifi_state = POWERUP;
						ClearMsg(FLAG_SYS_REGISTER);
						printk("system register failed... try to next register\r\n");
					}
				}
			register_break:
				break;
			}

			case TCPLINKOK:
			{
				unsigned short step_received;

				state_led = LED_ON;
				state_led_bak = LED_ON;

				printk("TCPLINKOK\r\n");
				OSTimeDlyHMSM(0, 0, 0, 500);
				data_buf[0] = 0;
				retry_cnt = ENDURE_FAILURE_TIMES;
				tx_cfg.heartbeat_tx_time = UnixTime;
				if (CHESSMODE_PLAYING == chess_mode)  //有当前棋局从断网时接收到的最后一步棋开始续传
				{
					if (tx_cfg.curr_tx_game != cfg.curr_game)
					{
						tx_cfg.curr_tx_game = cfg.curr_game;
						tx_cfg.curr_tx_step = 0;
						tx_cfg.curr_rx_step = 0;
						clearAllTxBuf();
					}
					SetMsg(MSG_PLAYING_FLAG);
					printk("Start to send curr: game=%d, step=%d, total=%d\r\n", tx_cfg.curr_tx_game, tx_cfg.curr_tx_step, cfg.game_steps_cnt[tx_cfg.curr_tx_game]);
				}
				else                                //无当前棋局从断网时接收到的最后一步棋开始续传
				{
					if (tx_cfg.his_tx_game >= MAX_GAME_CNT)
					{
						tx_cfg.his_tx_game = getToTxGame();
						tx_cfg.his_tx_step = 0;
						tx_cfg.his_rx_step = 0;
						clearAllTxBuf();
					}
					else
					{
						tx_cfg.his_tx_step = tx_cfg.his_rx_step;
					}
					ClearMsg(MSG_PLAYING_FLAG);
					if (tx_cfg.his_tx_game < MAX_GAME_CNT)
					{
						printk("Start to send history: game=%d, step=%d, total=%d\r\n", tx_cfg.his_tx_game, tx_cfg.his_rx_step, cfg.game_steps_cnt[tx_cfg.his_tx_game]);
					}
					else
					{
						printk("No history to send.\r\n");
					}
				}
				do
				{
					unsigned char buf_no;
					unsigned char timeout_no;

					if (1 == CheckMsg(MSG_PLAYING_FLAG))
					{
						if (cfg.curr_game == tx_cfg.curr_tx_game)   //当前棋局
						{
							timeout_no = getTimeOutTxBuf();
							if (timeout_no >= MAX_TX_BUFF_LEN)  //没有超时的情况
							{
								buf_no = getEmptyTxBuf();
								if (buf_no < MAX_TX_BUFF_LEN)   //发送缓冲没满
								{
									if (read_game_step(tx_cfg.curr_tx_game, tx_cfg.curr_tx_step, SendBuff)) //读取当前步数
									{
										printk("Tx curr: game=%d, step=%d, total=%d : ", tx_cfg.curr_tx_game, tx_cfg.curr_tx_step, cfg.game_steps_cnt[tx_cfg.curr_tx_game]);
										if (send_chess_data(SendBuff, 0)) //发送数据，并判断数据是否正确，步数步进是否为1，步数内容是否非空
										{
											setTxBuf(buf_no, tx_cfg.curr_tx_step);
											tx_cfg.curr_tx_step++;
											retry_cnt = ENDURE_FAILURE_TIMES;
											//printk("Send End\r\n");
										}
										else
										{
											retry_cnt--;
											printk("Tx error !!!\r\n");
										}
									}
									else    //如果读取不到
									{
										if (CHESSMODE_PLAYING != chess_mode)    //当前棋局已经下完
										{
											if (tx_cfg.curr_tx_step == cfg.game_steps_cnt[tx_cfg.curr_tx_game]) //棋局最后一步包含在发送缓冲中
											{
												buf_no = getMinTxBuf();
												if (buf_no >= MAX_TX_BUFF_LEN)  //发送缓冲区是否为空，这盘棋发送完了，到发送历史数据了
												{
													printk("Tx curr game=%d complete !\r\n", tx_cfg.curr_tx_game);
													cfg.game_steps_cnt[tx_cfg.curr_tx_game] = 0;
													ClearMsg(MSG_PLAYING_FLAG);
													tx_cfg.his_tx_step = tx_cfg.his_rx_step;
												}
											}
										}
									}
								}
							}
							else    //有超时的情况，读取超时步数重发
							{
								if (read_game_step(tx_cfg.curr_tx_game, tx_cfg.tx_buf[timeout_no].step, SendBuff))
								{
									printk("Tx curr time out: game=%d, step=%d, total=%d : ", tx_cfg.curr_tx_game, tx_cfg.tx_buf[timeout_no].step, cfg.game_steps_cnt[tx_cfg.curr_tx_game]);
									if (send_chess_data(SendBuff, 0)) //发送数据，并判断数据是否正确，步数步进是否为1，步数内容是否非空
									{
										setTxBuf(timeout_no, tx_cfg.tx_buf[timeout_no].step);
										retry_cnt = ENDURE_FAILURE_TIMES;
										//printk("Send End\r\n");
									}
									else
									{
										retry_cnt--;
										printk("Tx error !!!\r\n");
									}
								}
							}
						}
						else    //如果新棋局开始，从新棋局开始发送
						{
							clearAllTxBuf();
							tx_cfg.curr_tx_step = 0;
							tx_cfg.curr_rx_step = 0;
							tx_cfg.curr_tx_game = cfg.curr_game;
							printk("Start to send curr to new curr: game=%d, step=%d, total=%d\r\n", tx_cfg.curr_tx_game, tx_cfg.curr_tx_step, cfg.game_steps_cnt[tx_cfg.curr_tx_game]);
						}
					}
					else //历史棋局
					{
						if (CHESSMODE_PLAYING != chess_mode)
						{
							timeout_no = getTimeOutTxBuf();
							if (timeout_no >= MAX_TX_BUFF_LEN)  //没有超时的情况
							{
								buf_no = getEmptyTxBuf();
								if (buf_no < MAX_TX_BUFF_LEN)   //发送缓冲没满
								{
									if (read_game_step(tx_cfg.his_tx_game, tx_cfg.his_tx_step, SendBuff))   //读取步数
									{
										printk("Tx history: game=%d, step=%d, total=%d : ", tx_cfg.his_tx_game, tx_cfg.his_tx_step, cfg.game_steps_cnt[tx_cfg.his_tx_game]);
										if (send_chess_data(SendBuff, 1)) //发送数据，并判断数据是否正确，步数步进是否为1，步数内容是否非空
										{
											setTxBuf(buf_no, tx_cfg.his_tx_step);
											tx_cfg.his_tx_step++;
											retry_cnt = ENDURE_FAILURE_TIMES;
											//printk("Tx End\r\n");
										}
										else
										{
											retry_cnt--;
											printk("Tx error !!!\r\n");
										}
									}
									else    //如果读取不到
									{
										if (tx_cfg.his_tx_game < MAX_GAME_CNT)
										{
											if (tx_cfg.his_tx_step == cfg.game_steps_cnt[tx_cfg.his_tx_game])   //棋局最后一步包含在发送缓冲中
											{
												buf_no = getMinTxBuf();
												if (buf_no >= MAX_TX_BUFF_LEN)  //发送缓冲区是否为空，这盘棋发送完了，到发送下一盘了
												{
													printk("Tx history game=%d complete !\r\n", tx_cfg.his_tx_game);
													cfg.game_steps_cnt[tx_cfg.his_tx_game] = 0;
													tx_cfg.his_tx_game = getToTxGame();
													tx_cfg.his_tx_step = 0;
												}
											}
											else
											{
												printk("### Unknow error !!!\r\n");
											}
										}
										else    //没有历史棋局
										{
											tx_cfg.his_tx_game = getToTxGame();
											tx_cfg.his_tx_step = 0;
											tx_cfg.his_rx_step = 0;
											clearAllTxBuf();
										}
									}
								}
							}
							else    //有超时的情况，读取超时步数重发
							{
								if (read_game_step(tx_cfg.his_tx_game, tx_cfg.tx_buf[timeout_no].step, SendBuff))
								{
									printk("Tx history time out: game=%d, step=%d, total=%d : ", tx_cfg.his_tx_game, tx_cfg.tx_buf[timeout_no].step, cfg.game_steps_cnt[tx_cfg.his_tx_game]);
									if (send_chess_data(SendBuff, 1)) //发送数据，并判断数据是否正确，步数步进是否为1，步数内容是否非空
									{
										setTxBuf(timeout_no, tx_cfg.tx_buf[timeout_no].step);
										retry_cnt = ENDURE_FAILURE_TIMES;
										//printk("Tx End\r\n");
									}
									else
									{
										retry_cnt--;
										printk("Tx error !!!\r\n");
									}
								}
							}
						}
						else    //开始下棋，转到发送当前棋局状态
						{
							tx_cfg.curr_tx_game = cfg.curr_game;
							tx_cfg.curr_tx_step = 0;
							tx_cfg.curr_rx_step = 0;
							clearAllTxBuf();
							SetMsg(MSG_PLAYING_FLAG);
							printk("Start to send history to new curr: game=%d, step=%d, total=%d\r\n", tx_cfg.curr_tx_game, tx_cfg.curr_tx_step, cfg.game_steps_cnt[tx_cfg.curr_tx_game]);
						}
					}

					if (UnixTime - tx_cfg.heartbeat_tx_time > 10)
					{
						tx_cfg.heartbeat_tx_time = UnixTime;
						printk("tx_cfg.heartbeat_tx_step:%d\r\n", tx_cfg.heartbeat_tx_step);
						SendHeartbeat(CMD_HEARTBEAT, tx_cfg.heartbeat_tx_step, UnixTime, bat_percent, STATE_NORMAL);
						tx_cfg.heartbeat_tx_step = (tx_cfg.heartbeat_tx_step + 1) & 0x1fff;
						OSTimeDlyHMSM(0, 0, 0, 100);
						if (abs(tx_cfg.heartbeat_tx_step - tx_cfg.heartbeat_rx_step) > 6)
						{
							tx_cfg.heartbeat_tx_step = tx_cfg.heartbeat_rx_step;
							retry_cnt = 0;
						}
					}

					if (data_buf[0] != 0)
					{
						data_buf[0] = 0;
						EncryptData(&data_buf[3], ENCRYPT_LEN, encrypt_key);
						if (calc_sum(&data_buf[1], DATA_SEND_LEN - 1) == data_buf[DATA_SEND_LEN])
						{
							NetworkTimeOutCounter = NETWORK_TIME_OUT;
							step_received = data_buf[1] & 0x07; //从接收到返回数据计算当前接收到的步数
							step_received = step_received << 8;
							step_received = step_received + data_buf[2];
							if ((data_buf[1] & 0xe0) == 0x20)
							{
								buf_no = getMinTxBuf();
								if (buf_no < MAX_TX_BUFF_LEN)   //保存已经顺序发送完成的最大步数
								{
									tx_cfg.curr_rx_step = tx_cfg.tx_buf[buf_no].step;
								}
								printk("Rx curr step=%d :", step_received);
								printf_buf_hex(&data_buf[1], BYTES_PER_STEP);
								clearTxBuf(step_received);
								tx_cfg.heartbeat_tx_time = UnixTime;
							}
							else if ((data_buf[1] & 0xe0) == 0x60)
							{
								buf_no = getMinTxBuf();
								if (buf_no < MAX_TX_BUFF_LEN)   //保存已经顺序发送完成的最大步数
								{
									tx_cfg.his_rx_step = tx_cfg.tx_buf[buf_no].step;
								}
								printk("Rx history step=%d :", step_received);
								printf_buf_hex(&data_buf[1], BYTES_PER_STEP);
								clearTxBuf(step_received);
								tx_cfg.heartbeat_tx_time = UnixTime;
							}
							else if ((data_buf[1] & 0xe0) == 0x00)
							{
								printk("Reveive tx_cfg.heartbeat_tx_step:%d, expand byte data_buf[14]=%x\r\n", step_received, data_buf[15]);
								if (data_buf[15] >= STATE_UPDATE_WIFI1 && data_buf[15] <= STATE_UPDATE_WIFI5)	//wifi更新
								{
									update_wifi_info(data_buf[15]-STATE_UPDATE_WIFI1);
								}
								else if (data_buf[15] == STATE_UPDATE_DOMAIN)	//域名更新
								{
									update_domain_info();
								}
								tx_cfg.heartbeat_rx_step = step_received;
								retry_cnt = 10; //重试次数重新置位
							}
						}
						else
						{
							printk("checksum error = %d\r\n", __LINE__);
						}
					}
					if (CheckMsg(MSG_ENTER_SMARTLINK))
					{
						goto breaktomainlook;
					}
					OSTimeDlyHMSM(0, 0, 0, 500);
				} while (retry_cnt);
				wifi_state = POWERUP;
			breaktomainlook:
				break;
			}
			case TCPLINKFAIL:
				printk("TCPLINKFAIL\r\n");
				//OSTimeDlyHMSM(0, 0, 30, 0);每30秒重试一次
				break;
			default:

				break;
		}
	}
}

//LED1闪烁任务----------------------------------------
static void Task_Led1(void *p_arg) //负责AD9850设置
{
	OSTimeDlyHMSM(0, 0, 0, 800);
	NfcHandlerTask();
}

#define SYS_BATTERY_LOW				3700
#define SYS_ZERO_VOLTAGE			3550
#define SYS_FULL_VOLTAGE			3990
#define SYS_CHARGE_ZERO_VOLTAGE		3620
#define SYS_CHARGE_FULL_VOLTAGE		4050
//LED2闪烁任务----------------------------------------
unsigned char CalBatteryPercent(unsigned char charging, unsigned int voltage)
{
	unsigned char percent;
	unsigned int zero_voltage;
	unsigned int full_voltage;

	//1、判断充电状态
	if (charging == 1)
	{
		zero_voltage = SYS_CHARGE_ZERO_VOLTAGE;
		full_voltage = SYS_CHARGE_FULL_VOLTAGE;
	}
	else
	{
		zero_voltage = SYS_ZERO_VOLTAGE;
		full_voltage = SYS_FULL_VOLTAGE;
	}

	if (charging)
	{
		if (voltage <= zero_voltage)
		{
			percent = 0;
		}
		else if (voltage >= full_voltage)
		{
			percent = 100;
		}
		else
		{
			percent = (voltage - zero_voltage)*100/(full_voltage-zero_voltage);
		}
	}
	else
	{
		if (voltage <= zero_voltage)
		{
			percent = 0;
		}
		else if (voltage >= full_voltage)
		{
			percent = 100;
		}
		else
		{
			percent = (voltage - zero_voltage)*100/(full_voltage-zero_voltage);
		}
	}

	return percent;
}

static void Task_Led2(void *p_arg) //负责显示
{
	unsigned short ADC_ConvertedValue = 0;
	while (1)
	{
		OSTimeDlyHMSM(0, 0, 5, 0);
		while (0 == ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
		ADC_ConvertedValue = ADC_GetConversionValue(ADC1); //获得ADC1的转换数据
		bat_vol = 2 * 3350 * ADC_ConvertedValue / 4096;
		bat_percent = CalBatteryPercent(0, bat_vol);
		printk("Voltage = %d mV, percent = %d\r\n", bat_vol, bat_percent);
		if (bat_percent < 10)
		{
			display_dat_buf[M_STATE] = 0x38;
		}
		if (1 && bat_percent == 100 && chess_mode != CHESSMODE_PLAYING)
		{
			display_dat_buf[M_STATE] = segment_data[0xf];
		}
	}
}

/*
*********************************************************************************************************
*                                            App_TaskCreate()
*
* Description : Create the application tasks.
*
* Argument : none.
*
* Return   : none.
*
* Caller   : App_TaskStart().
*
* Note     : none.
*********************************************************************************************************
*/

static void App_TaskCreate(void)
{
	//CPU_INT08U os_err;

	//Com1_SEM=OSSemCreate(1);		     //建立串口1中断的信号量
	//Com1_MBOX = OSMboxCreate((void *)0);          //建立串口1中断的消息邮箱
	//SeMbox = OSMboxCreate((void *)0); //创建一个邮箱
	//Key_BOX = OSMboxCreate((void *)0); //创建一个邮箱
	//Key_MBOX=OSMboxCreate((void *)0);


	//串口1接收及发送任务---------------------------------------------------------
	/***
	OSTaskCreateExt(Task_Com1,                                     //指向任务代码的指针
					(void *)0,                                    //任务开始执行时，传递给任务的参数的指针
					(OS_STK *)&Task_Com1Stk[Task_Com1_STK_SIZE - 1], //分配给任务的堆栈的栈顶指针   从顶向下递减
					Task_Com1_PRIO,                               //分配给任务的优先级
					Task_Com1_PRIO,                               //预备给以后版本的特殊标识符，在现行版本同任务优先级
					(OS_STK *)&Task_Com1Stk[0],                   //指向任务堆栈栈底的指针，用于堆栈的检验
					Task_Com1_STK_SIZE,                           //指定堆栈的容量，用于堆栈的检验
					(void *)0,                                    //指向用户附加的数据域的指针，用来扩展任务的任务控制块
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);     //选项，指定是否允许堆栈检验，是否将堆栈清0,任务是否要进行浮点运算等等。
	**/
	//LED1 闪烁任务------------------------------------------------------
	OSTaskCreateExt(Task_Led1, (void*)0, (OS_STK*)&Task_Led1Stk[Task_Led1_STK_SIZE - 1], Task_Led1_PRIO, Task_Led1_PRIO, (OS_STK*)&Task_Led1Stk[0],
					Task_Led1_STK_SIZE,
					(void*)0,
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	//LED2 闪烁任务------------------------------------------------------
	OSTaskCreateExt(Task_Led2, (void*)0, (OS_STK*)&Task_Led2Stk[Task_Led2_STK_SIZE - 1], Task_Led2_PRIO, Task_Led2_PRIO, (OS_STK*)&Task_Led2Stk[0],
					Task_Led2_STK_SIZE,
					(void*)0,
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	//LED3 闪烁任务------------------------------------------------------
	/***
	OSTaskCreateExt(Task_Led3,
					(void *)0,
					(OS_STK *)&Task_Led3Stk[Task_Led3_STK_SIZE - 1],
					Task_Led3_PRIO,
					Task_Led3_PRIO,
					(OS_STK *)&Task_Led3Stk[0],
					Task_Led3_STK_SIZE,
					(void *)0,
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	***/
}

void USART_PRINT(USART_TypeDef *USARTx, char *Data, va_list ap)
{
	const char *s;
	int d;
	char buf[100];
	unsigned char prefix = 0x20;

	while (*Data != 0)
	{                                         //判断是否到达字符串结束符
		if (*Data == 0x5c)                       //'\'
		{
			switch (*++Data)
			{
				case 'r':                   //回车符
					USART_TX(USARTx, 0x0d);
					Data++;
					break;
				case 'n':                                     //换行符
					USART_TX(USARTx, 0x0a);
					Data++;
					break;
				default:
					Data++;
					break;
			}
		}
		else if (*Data == '%')
		{
			switch (*++Data)
			{
				case 's':       //字符串
					s = va_arg(ap, const char*);
					for (; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
					break;
				case 'd':   //十进制
					d = va_arg(ap, int);
					itoa(d, buf, 10);
					for (s = buf; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
					break;
				case 'x':       //十六进制
				case 'X':       //十六进制
					d = va_arg(ap, int);
					itoa(d, buf, 16);
					for (s = buf; *s; s++)
					{
						USART_TX(USARTx, *s);
					}
					Data++;
					break;
				case '0':
					prefix = '0';
					Data++;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
					int outlen = *Data - '0';
					int len;
					int radix;
					int i;

					Data++;
					if (*Data == 'd' || *Data == 'x' || *Data == 'X')
					{
						if (*Data == 'd')
						{
							radix = 10;
						}
						else
						{
							radix = 16;
						}
						d = va_arg(ap, int);
						itoa(d, buf, radix);
						len = strlen(buf);
						s = buf;
						if (outlen >= len)
						{
							for (i = 0; i < outlen - len; i++)
							{
								USART_TX(USARTx, prefix);
							}
						}
						else
						{
							for (i = 0; i < len - outlen; i++)
							{
								s++;
							}
						}
						for (; *s; s++)
						{
							USART_TX(USARTx, *s);
						}
						prefix = 0x20;
					}
					Data++;
				}
					break;
				default:
					Data++;
					break;
			}
		}
		else
		{
			USART_TX(USARTx, *Data++);
		}
	}
}

void printk(char *Data, ...)
{
	va_list ap;

	va_start(ap, Data);
	USART_PRINT(USART2, Data, ap);
	va_end(ap);
}

/******************************************************
		格式化串口输出函数
		"\r"	回车符	   USART_OUT(USART1, "abcdefg\r")
		"\n"	换行符	   USART_OUT(USART1, "abcdefg\r\n")
		"%s"	字符串	   USART_OUT(USART1, "字符串是：%s","abcdefg")
		"%d"	十进制	   USART_OUT(USART1, "a=%d",10)
**********************************************************/
void USART_OUT(USART_TypeDef *USARTx, char *Data, ...)
{
	va_list ap;

	va_start(ap, Data);
	USART_PRINT(USART1, Data, ap);
	va_end(ap);
}

/******************************************************
		整形数据转字符串函数
		char *itoa(int value, char *string, int radix)
		radix=10 标示是10进制	非十进制，转换结果为0;

		例：d=-379;
		执行	itoa(d, buf, 10); 后

		buf="-379"
**********************************************************/
char *itoa(int value, char *string, int radix)
{
	int i = 0, j = 0;
	int data = value;
	char temp[16];
	char MinusFlag;
	int pos;

	if (radix != 16 && radix != 10)
	{
		string[j] = 0;
		return 0;
	}

	MinusFlag = (radix == 10 && value < 0);
	if (MinusFlag)
	{
		data = -data;
	}
	else
	{
		data = value;
	}

	do
	{
		pos = data % radix;
		data /= radix;
		if (pos < 10)
		{
			temp[i] = pos + '0';
		}
		else
		{
			temp[i] = pos + 'a' - 10;
		}
		i++;
	}while (data > 0);

	if (MinusFlag)
	{
		temp[i++] = '-';
	}
	temp[i] = 0;
	i--;

	while (i >= 0)
	{
		string[j] = temp[i];
		j++;
		i--;
	}
	string[j] = 0;

	return string;

} /* NCL_Itoa */


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          uC/OS-II APP HOOKS
*********************************************************************************************************
*********************************************************************************************************
*/

#if (OS_APP_HOOKS_EN > 0)
/*
*********************************************************************************************************
*                                      TASK CREATION HOOK (APPLICATION)
*
* Description : This function is called when a task is created.
*
* Argument : ptcb   is a pointer to the task control block of the task being created.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void App_TaskCreateHook(OS_TCB *ptcb)
{
}

/*
*********************************************************************************************************
*                                    TASK DELETION HOOK (APPLICATION)
*
* Description : This function is called when a task is deleted.
*
* Argument : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void App_TaskDelHook(OS_TCB *ptcb)
{
	(void)ptcb;
}

/*
*********************************************************************************************************
*                                      IDLE TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskIdleHook(), which is called by the idle task.  This hook
*               has been added to allow you to do such things as STOP the CPU to conserve power.
*
* Argument : none.
*
* Note     : (1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

	#if OS_VERSION >= 251
void App_TaskIdleHook(void)
{
}
	#endif

/*
*********************************************************************************************************
*                                        STATISTIC TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskStatHook(), which is called every second by uC/OS-II's
*               statistics task.  This allows your application to add functionality to the statistics task.
*
* Argument : none.
*********************************************************************************************************
*/

void App_TaskStatHook(void)
{
}

/*
*********************************************************************************************************
*                                        TASK SWITCH HOOK (APPLICATION)
*
* Description : This function is called when a task switch is performed.  This allows you to perform other
*               operations during a context switch.
*
* Argument : none.
*
* Note     : 1 Interrupts are disabled during this call.
*
*            2  It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                  task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

	#if OS_TASK_SW_HOOK_EN > 0
void App_TaskSwHook(void)
{
}
	#endif

/*
*********************************************************************************************************
*                                     OS_TCBInit() HOOK (APPLICATION)
*
* Description : This function is called by OSTCBInitHook(), which is called by OS_TCBInit() after setting
*               up most of the TCB.
*
* Argument : ptcb    is a pointer to the TCB of the task being created.
*
* Note     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

	#if OS_VERSION >= 204
void App_TCBInitHook(OS_TCB *ptcb)
{
	(void)ptcb;
}
	#endif

#endif
