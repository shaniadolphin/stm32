#include  <includes.h>
#include "bsp_rtc.h"
#include "stm32f4xx_rtc.h"

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
                                                               
static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   AppTaskUpdateTCB;
static  CPU_STK  AppTaskUpdateStk[APP_CFG_TASK_UPDATE_STK_SIZE];

static  OS_TCB   AppTaskCOMTCB;
static  CPU_STK  AppTaskCOMStk[APP_CFG_TASK_COM_STK_SIZE];

static  OS_TCB   AppTaskUserIFTCB;
static  CPU_STK  AppTaskUserIFStk[APP_CFG_TASK_USER_IF_STK_SIZE];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  AppTaskCreate         (void);
static  void  AppTaskStart          (void     *p_arg);
static void   AppTaskUserIF         (void     *p_arg);
static void   AppTaskCOM			(void 	  *p_arg);

unsigned int Time_year   = 0;
unsigned char Time_month = 0;
unsigned char Time_date  = 0;
unsigned char Time_hour  = 0;
unsigned char Time_min   = 0;
unsigned char Time_sec   = 0;
unsigned char displayline = 0;
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
extern void printk(char *Data, ...);
extern void Lcd_Initialize(void);

extern unsigned char moving;
extern unsigned int leddisbuf[32*64];

void (*current_operation_index)();

typedef struct
{
	unsigned char current;
	unsigned char up;		//向上翻索引号
	unsigned char down;		//向下翻索引号
	unsigned char enter;	//确认索引号
	void (*current_operation)();
}key_table;

key_table code_table[2]=
{
	{0,3,1,4,(*bsp_Init)},	//第一层，显示【清华大学】、北京大学、重庆三峡学院、返回
	{1,0,2,8,(*CPU_Init)},	//第一层，显示清华大学、【北京大学】、重庆三峡学院、返回
};
/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
    OS_ERR  err;

//    BSP_IntDisAll();                                          /* Disable all interrupts.                              */
    
    OSInit(&err);                                               /* Init uC/OS-III.                                      */


	OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"App Task Start",
                 (OS_TASK_PTR   )AppTaskStart, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
    
    (void)&err;
    
    return (0);
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskStart
*	功能说明: 这是一个启动任务，在多任务系统启动后，必须初始化滴答计数器(在BSP_Init中实现)
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：2
*********************************************************************************************************
*/
static  void  AppTaskStart (void *p_arg)
{
	OS_ERR      err;
	float Temp = 1.1f;
	RCC_ClocksTypeDef  rcc_clocktemp;
	(void)p_arg;
	unsigned char func_index=0;
 	bsp_Init();
	CPU_Init();
	BSP_Tick_Init();                      
	bsp_LedOn(1);
	bsp_LedOn(2);
	bsp_LedOn(3);
#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
                                        
	AppTaskCreate();   
	//LCD_P8x16Str(0,0, "MODE 0:twinkling",WHITE, YELLOW, 0);
	RCC_GetClocksFreq(&rcc_clocktemp);
	//printf("HCLK_Frequency = %d\r\n", (CPU_INT32U)rcc_clocktemp.HCLK_Frequency);	
	//printf("SYSCLK_Frequency = %d\r\n", (CPU_INT32U)rcc_clocktemp.SYSCLK_Frequency);	
	while (1)
	{                                         
		//Temp = Temp + 1.5f;
		//Temp = Temp * 1.0f;
		//Lcd_Initialize();	
		//if ((GPIO_PORT_KEYUSER2->IDR & GPIO_PIN_KEYUSER2) == 0)printk("AppTaskStart Temp = %f\r\n", Temp);
		//dispaly_process();
		//GPIOA->BSRRL = GPIO_Pin_6;
		//bsp_LedToggle(1);
		//D7258_PORT_BSRR = GPIO_Pin_6 | GPIO_Pin_7;
		//OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
		//OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
		//GPIOA->BSRRH = GPIO_Pin_6;
		//D7258_PORT_BSRR = (GPIO_Pin_6 | GPIO_Pin_7) << 16;
		display_full();
		//OSTimeDly(100, OS_OPT_TIME_DLY, &err);
		//LCD_Printint(80,120 + 0, (int)Temp, WHITE, YELLOW, 0);
		//current_operation_index = code_table[func_index].current_operation;
		//(*current_operation_index)();//执行当前操作函数
		//bsp_LedToggle(1);
		//OSTimeDly(50, OS_OPT_TIME_DLY, &err);//200*(1000/50)
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGUIUpdate
*	功能说明: LED2闪烁	
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：3
*********************************************************************************************************
*/
static void AppTaskGUIUpdate(void *p_arg)
{
	OS_ERR      err;
	float Temp = 1.1f;
	unsigned int bkcolortmp = 0x000000;
	unsigned int crcolortmp = 0x7f7f7f;
	unsigned char secold = 0;	
	(void)p_arg;
	My_RTC_Init();	  
	while(1)
	{	

		RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
		RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
#if 1		
		LED_P8x16Char(0,0,RTC_TimeStruct.RTC_Hours/10 + '0',crcolortmp,bkcolortmp);
		LED_P8x16Char(0 +1*8,0,RTC_TimeStruct.RTC_Hours%10 + '0',crcolortmp,bkcolortmp);
		LED_P8x16Str(0 +2*8,0,":",crcolortmp,bkcolortmp);
		LED_P8x16Char(0 +3*8,0,RTC_TimeStruct.RTC_Minutes/10 + '0',crcolortmp,bkcolortmp);
		LED_P8x16Char(0 +4*8,0,RTC_TimeStruct.RTC_Minutes%10 + '0',crcolortmp,bkcolortmp);
		LED_P8x16Str(0 +5*8,0,":",crcolortmp,bkcolortmp);
		LED_P8x16Char(0 +6*8,0,RTC_TimeStruct.RTC_Seconds/10 + '0',crcolortmp,bkcolortmp);
		LED_P8x16Char(0 +7*8,0,RTC_TimeStruct.RTC_Seconds%10 + '0',crcolortmp,bkcolortmp);
		//LED_P8x16Char(0,16,Time_hour/10 + '0',crcolortmp,bkcolortmp);
		//LED_P8x16Char(0 +1*8,16,Time_hour%10 + '0',crcolortmp,bkcolortmp);
		//LED_P8x16Str(0 +2*8,16,"-",crcolortmp,bkcolortmp);
		LED_P8x16Char(0 +3*8,16,RTC_DateStruct.RTC_Month/10 + '0',crcolortmp,bkcolortmp);
		LED_P8x16Char(0 +4*8,16,RTC_DateStruct.RTC_Month%10 + '0',crcolortmp,bkcolortmp);
		LED_P8x16Str(0 +5*8,16,"-",crcolortmp,bkcolortmp);
		LED_P8x16Char(0 +6*8,16,RTC_DateStruct.RTC_Date/10 + '0',crcolortmp,bkcolortmp);
		LED_P8x16Char(0 +7*8,16,RTC_DateStruct.RTC_Date%10 + '0',crcolortmp,bkcolortmp);
		//if(((Time_sec % 10) == 1) && ((secold % 10) == 0))
		//{
		//	crcolortmp ++;
		//	if(crcolortmp == 8)crcolortmp = 1;
		//}
		secold = RTC_TimeStruct.RTC_Seconds;	
#else
		//LED_P8x16Char(	0, 			0, RTC_TimeStruct.RTC_Hours/10 + '0', crcolortmp, bkcolortmp);
		LED_P8x16Str(	0, 			0,  "abcdefgh", crcolortmp, bkcolortmp);
		//LED_P8x16Char(	1, 			16, RTC_TimeStruct.RTC_Hours/10 + '0', crcolortmp, bkcolortmp);
		//LED_P8x16Str(	0, 			16, "abcdefgh", crcolortmp, bkcolortmp);	
		LED_GB1616(	0 , 			16, "支", crcolortmp, bkcolortmp);		
		LED_GB1616(	0 +1*16, 		16, "持", crcolortmp, bkcolortmp);
		LED_GB1616(	0 +2*16 , 		16, "竖", crcolortmp, bkcolortmp);		
		LED_GB1616(	0 +3*16 , 		16, "屏", crcolortmp, bkcolortmp);	
#endif
		OSTimeDly(50, OS_OPT_TIME_DLY, &err);
		bsp_LedToggle(3);
		//OSTimeDly(50, OS_OPT_TIME_DLY, &err);//200*(1000/50)
		//LED_P8x16Str(0 +2*8,0,"efgd",0x0000ff,0xff0000);	
		//OSTimeDly(50, OS_OPT_TIME_DLY, &err);
		//OSTimeDlyHMSM(0, 0, 1, 000);		
	}   
}
extern unsigned char updateflag;
/*
*********************************************************************************************************
*	函 数 名: AppTaskCom
*	功能说明: LED3闪烁	
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：3
*********************************************************************************************************
*/
static void AppTaskCOM(void *p_arg)
{	
	OS_ERR      err;
	unsigned char i;
	(void)p_arg;
	//InitCLUT(color_map);//初始化颜色表 
	OSTimeDly(20, OS_OPT_TIME_DLY, &err);
	//test_fft();
	while(1)
	{
		//bsp_LedToggle(3);
		displayline++;
		updateflag = 0;
		if(displayline == 16)displayline = 0;
		//bsp_LedToggle(3);
		//OSTimeDly(50, OS_OPT_TIME_DLY, &err);//200*(1000/50)
		//TestfftFun();
		OSTimeDly(50, OS_OPT_TIME_DLY, &err);	
	}
	 						  	 	       											   
   
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskUserIF
*	功能说明: LED4闪烁
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：2
*********************************************************************************************************
*/
static void AppTaskUserIF(void *p_arg)
{
	OS_ERR      err;
	(void)p_arg;
	OSTimeDly(20, OS_OPT_TIME_DLY, &err);  
	while (1) 
	{   		
        bsp_LedToggle(2);
		OSTimeDly(100, OS_OPT_TIME_DLY, &err);     
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppTaskCreate (void)
{
	OS_ERR      err;
	
	/***********************************/
	OSTaskCreate((OS_TCB       *)&AppTaskUpdateTCB,             
                 (CPU_CHAR     *)"App Task Update",
                 (OS_TASK_PTR   )AppTaskGUIUpdate, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_UPDATE_PRIO,
                 (CPU_STK      *)&AppTaskUpdateStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_UPDATE_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_UPDATE_STK_SIZE,
                 (OS_MSG_QTY    )1,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/***********************************/
	OSTaskCreate((OS_TCB       *)&AppTaskCOMTCB,            
                 (CPU_CHAR     *)"App Task COM",
                 (OS_TASK_PTR   )AppTaskCOM, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_COM_PRIO,
                 (CPU_STK      *)&AppTaskCOMStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE,
                 (OS_MSG_QTY    )2,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/***********************************/
	OSTaskCreate((OS_TCB       *)&AppTaskUserIFTCB,             
                 (CPU_CHAR     *)"App Task UserIF",
                 (OS_TASK_PTR   )AppTaskUserIF, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_USER_IF_PRIO,
                 (CPU_STK      *)&AppTaskUserIFStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
				 
}


/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
