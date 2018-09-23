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
//#include "oled.h"
#include "./lcd/LCD.h"
#include "./adc/adc.h"
#include "./led/led.h"
#include "./usart/usart.h"
#include "msg.h"
#include "./ws2812B/ws2812B.h"


OS_EVENT *Com1_SEM;
OS_EVENT *Com1_MBOX; //���巢����Ϣ��������ָ�� ���ڴ洢���ڽ���ֵ 

unsigned short AD_Value[N][M]; //�������ADCת�������Ҳ��DMA��Ŀ���ַ
unsigned short After_filter[M]; //���������ƽ��ֵ֮��Ľ��

unsigned char angleTable[6];
unsigned char workmode = 0x00;
unsigned char settingflag = 0x00;

unsigned short valueCurr;
unsigned short valueCurrRd;


extern unsigned int Time_year;
extern unsigned char Time_month;
extern unsigned char Time_date;
extern unsigned char Time_hour;
extern unsigned char Time_min;
extern unsigned char Time_sec;
extern unsigned int UnixTime;
extern int Voltage;
void LED_N_STATUS(unsigned char num,unsigned char status);
extern void fun_para(void);
extern void displayADValue(void);
extern int Test(void);
extern unsigned short getCloseToResult(unsigned short *now,unsigned short final, unsigned short step);
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
static  OS_STK Task_Led3Stk[Task_Led3_STK_SIZE];
		

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void App_TaskCreate(void);

static  void App_TaskStart(void* p_arg);

static  void Task_Led1(void* p_arg);
static  void Task_Led2(void* p_arg);
static  void Task_Led3(void* p_arg);
/*----------------------------------------------------------------------------
 * @����: 	main ����������		
 * @����: 				
 * @����: 	
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
int main(void)
{
	CPU_INT08U os_err;	
	//��ֹCPU�ж�
	CPU_IntDis();	
	//UCOS ��ʼ��
	OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel".         */	
	//Ӳ��ƽ̨��ʼ��
	BSP_Init();                                                 /* Initialize BSP functions.  */
   	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);  
	//���������� ���ȼ����  ���������������һ����;��Ϊ���Ժ�ʹ��ͳ������
	os_err = OSTaskCreate(
				(void (*) (void *)) App_TaskStart,	  		  				//ָ����������ָ��
                (void *) 0,								  					//����ʼִ��ʱ�����ݸ�����Ĳ�����ָ��
				(OS_STK *) &App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],	//���������Ķ�ջ��ջ��ָ��   �Ӷ����µݼ�
				(INT8U) APP_TASK_START_PRIO);								//�������������ȼ�
	os_err = os_err;
	//ucos�Ľ��ļ�������0    ���ļ�������0-4294967295    ���ڽ���Ƶ��100hzʱ�� ÿ��497������¼��� 
	OSTimeSet(0);
	OSStart(); 	/* Start multitasking (i.e. give control to uC/OS-II).  */
	return (0);
}
/*----------------------------------------------------------------------------
 * @����: 	App_TaskCreate ����������		
 * @����: 				
 * @����: 	
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
static  void App_TaskCreate(void)
{
   //CPU_INT08U os_err;
 
   //Com1_SEM=OSSemCreate(1);		     //��������1�жϵ��ź���
   	//Com1_MBOX=OSMboxCreate((void *) 0);		     //��������1�жϵ���Ϣ����
   	//SeMbox = OSMboxCreate((void *)0);//����һ������ 
	//Key_BOX= OSMboxCreate((void *)0);//����һ������
	//Key_MBOX=OSMboxCreate((void *)0);
   
   //����1���ռ���������---------------------------------------------------------   
/****	
   OSTaskCreateExt(Task_Com1,									  //ָ����������ָ��
   					(void *)0,									  //����ʼִ��ʱ�����ݸ�����Ĳ�����ָ��
					(OS_STK *)&Task_Com1Stk[Task_Com1_STK_SIZE-1],//���������Ķ�ջ��ջ��ָ��   �Ӷ����µݼ�
					Task_Com1_PRIO,								  //�������������ȼ�
					Task_Com1_PRIO,								  //Ԥ�����Ժ�汾�������ʶ���������а汾ͬ�������ȼ�
					(OS_STK *)&Task_Com1Stk[0],					  //ָ�������ջջ�׵�ָ�룬���ڶ�ջ�ļ���
                    Task_Com1_STK_SIZE,							  //ָ����ջ�����������ڶ�ջ�ļ���
                    (void *)0,									  //ָ���û����ӵ��������ָ�룬������չ�����������ƿ�
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);	  //ѡ�ָ���Ƿ������ջ���飬�Ƿ񽫶�ջ��0,�����Ƿ�Ҫ���и�������ȵȡ�
*****/ 
	//LED1 ��˸����------------------------------------------------------
   OSTaskCreateExt(Task_Led1,
					(void *)0,
					(OS_STK *)&Task_Led1Stk[Task_Led1_STK_SIZE-1],
					Task_Led1_PRIO,
					Task_Led1_PRIO,
					(OS_STK *)&Task_Led1Stk[0],
                    Task_Led1_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
   //LED2 ��˸����------------------------------------------------------
   OSTaskCreateExt(Task_Led2,
					(void *)0,
					(OS_STK *)&Task_Led2Stk[Task_Led2_STK_SIZE-1],
					Task_Led2_PRIO,Task_Led2_PRIO,
					(OS_STK *)&Task_Led2Stk[0],
                    Task_Led2_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR); 
}
/*----------------------------------------------------------------------------
 * @����: 	App_TaskStart ��ʼ���������������ô�����������������������		
 * @����: 				
 * @����: 	
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
static  void App_TaskStart(void* p_arg)
{
	(void) p_arg;
	//��ʼ��ucosʱ�ӽ���
	OS_CPU_SysTickInit();	/*Initialize the SysTick*/	
	//ʹ��ucos ��ͳ������
	#if (OS_TASK_STAT_EN > 0)
	//----ͳ�������ʼ������  
	OSStatInit();			/*Determine CPU capacity*/
	#endif
	//��������������
	printk("App_TaskStart!!\r\n");
	App_TaskCreate();	

	OSTimeDlyHMSM(0, 0, 1, 500);

	while (1)
	{	
		//LED_N_STATUS(1,1);
		ws2812Test();
		OSTimeDlyHMSM(0, 0, 1, 0);	
		//LED_N_STATUS(1,0);
		OSTimeDlyHMSM(0, 0, 1, 0);		
	}
}
/*----------------------------------------------------------------------------
 * @����: 	Task_Led1 ��ʾ��������������������ʾ����		
 * @����: 				
 * @����: 	
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
extern int Encoder_Left;
extern int Encoder_Right;             //���ұ��������������
extern int Moto1;
extern int Moto2;                     //���PWM���� Ӧ��Motor�� ��Moto�¾�	
extern int Temperature;               //��ʾ�¶�
static  void Task_Led1(void* p_arg)
{
	//INT8U err;
	unsigned char timecnt;
	//double valueview[5];
	//char strview[20];
	(void) p_arg;	
	LCD_GPIO_Config();		
	//Lcd_Initialize();	
	//Lcd_ColorBox(0, 0, 480, 320, BLACK);
	OSTimeDlyHMSM(0, 0, 2, 000);		
	while (1)
	{
		for(timecnt = 0;timecnt<8;timecnt++)
		{
			//LCD_P8x16Str(80-16,120 + 0, "1:", WHITE, YELLOW, 0);
			//LCD_Printint(80,120 + 0, UnixTime&0xffff, WHITE, YELLOW, 0);
			//LCD_P8x16Str(80-16,120 +16, "2:", WHITE, YELLOW, 0);
			//LCD_Printint(80,120 +16, After_filter[0], WHITE, YELLOW, 0);
			//LCD_P8x16Str(80-16,120 +32, "3:", WHITE, YELLOW, 0);
			//LCD_Printint(80,120 +32, Temperature, WHITE, YELLOW, 0);
			//LCD_P8x16Str(320-16,120 + 0, "4:", WHITE, YELLOW, 0);
			//LCD_Printint(320,120 + 0, Encoder_Left, WHITE, YELLOW, 0);
			//LCD_P8x16Str(320-16,120 +16, "5:", WHITE, YELLOW, 0);
			//LCD_Printint(320,120 +16, Encoder_Right, WHITE, YELLOW, 0);
			//LCD_P8x16Str(320-16,120 +32, "6:", WHITE, YELLOW, 0);
			//LCD_Printint(320,120 +32, Moto1, WHITE, YELLOW, 0);
			//LCD_P8x16Str(80-16,120 +48, "7:", WHITE, YELLOW, 0);
			//LCD_Printint(80,120 +48, Moto2, WHITE, YELLOW, 0);
			
			//LCD_P8x16Str(80-16,120 +64, "8:", WHITE, YELLOW, 0);
			//LCD_Printint(80,120 +64, After_filter[0], WHITE, YELLOW, 0);	
			//LCD_P8x16Str(80-16,120 +90, "9:", WHITE, YELLOW, 0);
			//LCD_Printint(80,120 +90, After_filter[1], WHITE, YELLOW, 0);	
			//LCD_P8x16Str(80-16,120 +106, "a:", WHITE, YELLOW, 0);
			//LCD_Printint(80,120 +106, After_filter[2], WHITE, YELLOW, 0);
			//LCD_P8x16Str(80-16,120 +122, "b:", WHITE, YELLOW, 0);
			//LCD_Printint(80,120 +122, After_filter[3], WHITE, YELLOW, 0);			
			OSTimeDlyHMSM(0, 0, 0, 250);
			if(timecnt < 4)
			{
				LED_N_STATUS(1,1);
				printk("Task_Led1\r\n");
			}
			else
				LED_N_STATUS(1,0);	
		}
	}
}
/*----------------------------------------------------------------------------
 * @����: 	Task_Led2 ���������ͨ�������������		
 * @����: 				
 * @����: 	
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.07.08     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
static  void Task_Led2(void* p_arg)
{
    //ESP8266ģ������APָ��
    const char AT_CWJAP[] = "AT+CWJAP_CUR=";
    const char AT_CWJAP_CUR[] = "AT+CWJAP_CUR=";
    const char AT_CIPSTART[] = "AT+CIPSTART=";

    const char *tcp_ip = "192.168.99.1";
    const char *tcp_port = "30012";
    unsigned char wifi_state;
	(void) p_arg;	
	
	while (1)
	{
#if 0
		switch (wifi_state)
		{
			case POWERUP:
			{
				WifiStatusFlag = 0;
				GPIO_SetBits(GPIOB, GPIO_Pin_5);  	//WIFI RST
				OSTimeDlyHMSM(0, 0, 1, 000);
				GPIO_ResetBits(GPIOB, GPIO_Pin_5);  //WIFI RST
				OSTimeDlyHMSM(0, 0, 0, 500);
				uart2_init(115200);       //����1��ʼ��
				USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);            
				for (retry_cnt = 0; retry_cnt < CONNECT_AP_TIMES; retry_cnt++)
				{
					printk("Connecting to default wifi %ds\r\n", retry_cnt);
					OSTimeDlyHMSM(0, 0, 1, 0);
					if (WifiStatusFlag & FLAG_WIFI_SSID_GOTIP)//���IP
					{
						break;
					}
					
				}    
#endif
        OSTimeDlyHMSM(0, 0, 1, 0);
	}
}


static  void Task_Led3(void* p_arg)
{

}


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

void App_TaskCreateHook(OS_TCB* ptcb)
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

void App_TaskDelHook(OS_TCB* ptcb)
{
   (void) ptcb;
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
void App_TCBInitHook(OS_TCB* ptcb)
{
   (void) ptcb;
}
#endif

#endif
