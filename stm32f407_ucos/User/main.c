/*
*********************************************************************************************************
*	                                  
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : ����
*	�޸ļ�¼ :
*		�汾��   ����         ����          ˵��
*       v1.0    2014-02-27  Eric2013   ST�̼���V1.3.0�汾
*                                      uCOS-III�汾3.03.01
*
*   Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include  <includes.h>
#include "bsp_rtc.h"
#include "stm32f4xx_rtc.h"

#include "..\Libraries\CMSIS DSP_Library\fft.h"
#include "..\prink\printk.h"

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

extern void Lcd_Initialize(void);
extern void record(void);
extern u16 Get_Adc(u8 ch);
extern unsigned char moving;



void (*current_operation_index)();

typedef struct
{
	unsigned char current;
	unsigned char up;//���Ϸ�������
	unsigned char down;//���·�������
	unsigned char enter;//ȷ��������
	void (*current_operation)();
}key_table;

key_table code_table[2]=
{
	{0,3,1,4,(*bsp_Init)},//��һ�㣬��ʾ���廪��ѧ����������ѧ��������ϿѧԺ������
	{1,0,2,8,(*CPU_Init)},//��һ�㣬��ʾ�廪��ѧ����������ѧ����������ϿѧԺ������
};
/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: ��׼c������ڡ�
*	��    �Σ���
*	�� �� ֵ: ��
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
*	�� �� ��: AppTaskStart
*	����˵��: ����һ�����������ڶ�����ϵͳ�����󣬱����ʼ���δ������(��BSP_Init��ʵ��)
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ����2
*********************************************************************************************************
*/
static  void  AppTaskStart (void *p_arg)
{
	OS_ERR      err;
	float Temp = 1.1f;
	RCC_ClocksTypeDef  rcc_clocktemp;
	(void)p_arg;
 	bsp_Init();
	CPU_Init();
	BSP_Tick_Init();                      

#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
                                        
	AppTaskCreate();   
	//LCD_P8x16Str(0,0, "MODE 0:twinkling",WHITE, YELLOW, 0);
	RCC_GetClocksFreq(&rcc_clocktemp);
	printk("HCLK_Frequency = %d\r\n", (CPU_INT32U)rcc_clocktemp.HCLK_Frequency);	
	//printf("SYSCLK_Frequency = %d\r\n", (CPU_INT32U)rcc_clocktemp.SYSCLK_Frequency);	
	while (1)
	{                                         
		//Temp = Temp + 1.5f;
		//Temp = Temp * 1.0f;
		OSTimeDly(100, OS_OPT_TIME_DLY, &err);
		//bsp_LedOn(1);
		//OSTimeDly(50, OS_OPT_TIME_DLY, &err);
		//printk("HCLK_Frequency = %d\r\n", Get_Adc(1));	
		record();//ʹ��TIM1������¼һ��2S����Ƶ
#if 0		
		
		addr = ftr_start_addr+(comm-G_comm_fst)*size_per_comm+prc_count*size_per_ftr;
		if(save_mdl(VcBuf, addr)==save_ok)//�洢����ֵ
		{
			prc_count++;
			//GUI_ClrArea(&(Label[G_count]));
			//GUI_printf(&(Label[G_count]),"��ѵ��%d��",prc_count);
			if(prc_count == ftr_per_comm)
			{
				prc_count=0;
			}
			//GUI_ClrArea(&(Label[G_stus]));
			//GUI_DispStr(&(Label[G_stus]),"������Ч");
			/*
			sav_ftr=(v_ftr_tag *)addr;
			USART1_printf("mask=%d ",sav_ftr->save_sign);
			USART1_printf("frm_num=%d",sav_ftr->frm_num);
			for(i=0;i<((sav_ftr->frm_num)*mfcc_num);i++)
			{
				USART1_printf("%d,",sav_ftr->mfcc_dat[i]);
			}
			*/
		}
#endif		

	}
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskGUIUpdate
*	����˵��: LED2��˸	
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ����3
*********************************************************************************************************
*/
static void AppTaskGUIUpdate(void *p_arg)
{
	OS_ERR      err;

	(void)p_arg;
	//My_RTC_Init();	  
	while(1)
	{	
		//RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
		//RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
		OSTimeDly(50, OS_OPT_TIME_DLY, &err);
		bsp_LedOn(1);
		bsp_LedOn(2);
		bsp_LedOn(3);
		OSTimeDly(50, OS_OPT_TIME_DLY, &err);
		bsp_LedOff(1);
		bsp_LedOff(2);
		bsp_LedOff(3);		
		//TIM3_CNT
	}   
}
extern unsigned char updateflag;
/*
*********************************************************************************************************
*	�� �� ��: AppTaskCom
*	����˵��: LED3��˸	
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ����3
*********************************************************************************************************
*/
static void AppTaskCOM(void *p_arg)
{	
	OS_ERR      err;
	(void)p_arg;
	OSTimeDly(20, OS_OPT_TIME_DLY, &err);
	while(1)
	{
		OSTimeDly(50, OS_OPT_TIME_DLY, &err);	
	}	 						  	 	       											  
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskUserIF
*	����˵��: LED4��˸
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ����2
*********************************************************************************************************
*/
static void AppTaskUserIF(void *p_arg)
{
	OS_ERR      err;
  
	while (1) 
	{   		
        //bsp_LedToggle(4);
		OSTimeDly(100, OS_OPT_TIME_DLY, &err);     
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
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


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
