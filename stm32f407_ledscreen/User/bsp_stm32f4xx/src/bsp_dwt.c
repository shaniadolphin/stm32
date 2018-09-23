#include "bsp.h"

/*
*********************************************************************************************************
*                                             �Ĵ���
*********************************************************************************************************
*/

#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004

#define  DEM_CR_TRCENA                   (1 << 24)
#define  DWT_CR_CYCCNTENA                (1 <<  0)


/*
*********************************************************************************************************
*	�� �� ��: bsp_InitDWT
*	����˵��: ��ʼ��DWT. �ú����� bsp_Init() ���á�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitDWT(void)
{
	DEM_CR         |= (unsigned int)DEM_CR_TRCENA;   /* Enable Cortex-M4's DWT CYCCNT reg.  */
    DWT_CYCCNT      = (unsigned int)0u;
    DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;
}


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
