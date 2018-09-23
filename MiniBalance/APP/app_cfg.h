/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210B-LK1 Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.10
* Programmer(s) : BH3NVN
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              TASKS NAMES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                               2
#define  APP_TASK_USER_IF_PRIO                             13
#define  APP_TASK_KBD_PRIO                                 12

#define  Task_Led1_PRIO                                    3
#define  Task_Led2_PRIO                                    4
#define  Task_Led3_PRIO                                    5

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_TASK_START_STK_SIZE                         1000
//#define  APP_TASK_KBD_STK_SIZE                           256
#define  Task_Com1_STK_SIZE                              300
//#define  APP_TASK_USER_IF_STK_SIZE                       256
#define  Task_Led1_STK_SIZE                         	 800
#define  Task_Led2_STK_SIZE                         	 800
#define  Task_Led3_STK_SIZE                         	 300
//#define	 Task_DAC_SPI_STK_SIZE							 100
//#define	 Task_Key_SCAN_STK_SIZE							 100

/*
*********************************************************************************************************
*                                                  LIB
*********************************************************************************************************
*/

#define  uC_CFG_OPTIMIZE_ASM_EN                 DEF_ENABLED
#define  LIB_STR_CFG_FP_EN                      DEF_DISABLED

#endif
