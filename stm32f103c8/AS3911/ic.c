/*
 *****************************************************************************
 * Copyright by ams AG                                                       *
 * All rights are reserved.                                                  *
 *                                                                           *
 * IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
 * THE SOFTWARE.                                                             *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
 *****************************************************************************
 */
/*
 *      PROJECT:   AS3911 firmware
 *      $Revision: 1.3 $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author Christian Eisendle
 *
 *  \brief Interrupt controller driver for PIC24f series
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "ic.h"
#include "nfc.h"
#include "..\FWlib\inc\stm32f10x_exti.h"
#include "ucos_ii.h"


U16 IRQ_COUNT = 0;
U8 WaitIntFlag = 0;


/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/


//as3911中断脚初始化
void NFC_INT_init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
	/* Configure Key Button EXTI Line to generate an interrupt on falling edge */
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	//EXTI_GenerateSWInterrupt(EXTI_Line8); //产生一个软件中断  EXTI_Line6中断允许
	//request_irq(EXP_GPIO_NFC_INT, NFCIntHandler, 0);
}

void NFC_IRQ_Enable(void)
{
	//enable_irq(EXP_GPIO_NFC_INT);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void NFC_IRQ_Disable(void)
{
	//disable_irq(EXP_GPIO_NFC_INT);
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}

S8 icInitialize()
{
    return ERR_NONE;
}

S8 icDeinitialize()
{
    return ERR_NONE;
}

void icEnableInterrupt(void)
{
	NFC_IRQ_Enable();
}

void icDisableInterrupt(void)
{
	NFC_IRQ_Disable();
}

S8 icClearInterrupt(void)
{
    return ERR_NONE;
}


void NFC_SPI_Select(void)
{
	U8 os_err;
	OSSemPend(SemSpiflash, 0, &os_err);

	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	//OUTREG32(A_GPIO_PXPES(1), 0x3c000000);
	//REG_SSI_CR0 |= SSI_CR0_SSIE;
}

void NFC_SPI_Deselect(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	//OUTREG32(A_GPIO_PXPEC(1), 0x3c000000);
	//REG_SSI_CR0 &= ~SSI_CR0_SSIE;
	OSSemPost(SemSpiflash);
}

S8 spiTxRx(const U8 *txData, unsigned int txLen, U8 *rxData, unsigned int rxLen)
{
	unsigned int i = 0;
	U8 data;

	for (i=0; i < txLen; i++)
	{
		if (txData != NULL)
		{
			data = SPI_Send_Byte(txData[i]);
			//printk("1 Write %d Rx = %d\r\n", txData[i]&0x3f, data);
		}
		else
		{
			data = SPI_Send_Byte(0);
			//printk("1 Write %d Rx = %d\r\n", 0, data);
		}
	}

	for (i=0; i < rxLen; i++)
	{
		if ((rxData != NULL))
		{
			 data = SPI_Send_Byte(0);
			 rxData[i] = data;
		}
		else
		{
			data = SPI_Send_Byte(0);
		}
		//printk("2 Rx = %d\r\n", data);
	}

	return ERR_NONE;
}


S8 delayNMicroSeconds(U16 us)
{
	TIM3->PSC = 71;
	TIM3->ARR = us;
	TIM3->CR1|=(1<<3);
	TIM3->CNT = 0;
	TIM3->CR1|= 0x01;
	while((TIM3->SR & 0x01)==0)
	{
		/*if (WaitIntFlag)
		{
			break;
		}*/
	}
	TIM3->SR = 0;

	return ERR_NONE;
}

S8 delayNMilliSeconds(U16 ms)
{
#if 0
	WaitIntFlag = 0;
	TIM4->PSC = 35999;
	TIM4->ARR = ms * 2;
	TIM4->CR1|=(1<<3);
	TIM4->CNT = 0;
	TIM4->CR1|= 0x01;
	//printk("== delay ms = %d, SR = %d ==", ms, TIM3->SR & 0x01);
	while((TIM4->SR & 0x01)==0)
	{
		if (WaitIntFlag)
		{
			break;
		}
	}
	//printk("== WaitIntFlag = %d, SR = %d ==", WaitIntFlag, TIM3->SR & 0x01);
	TIM4->SR = 0;
#else
	U16 i;

	WaitIntFlag = 0;
	for (i=0; i<ms; i++)
	{
		delayNMicroSeconds(1000);
		if (WaitIntFlag)
		{
			break;
		}
	}
#endif
	return ERR_NONE;
}



extern U8 WaitIntFlag;



#define AS3911_READ_MODE   (1 << 6)

static S8 as3911ReadRegister_for_int(U8 reg, U8 *val)
{
	U8 buf[2];
	S8 err;

	buf[0] = reg | AS3911_READ_MODE;  //寄存器地址|读模式
	buf[1] = 0;

	//SSI_IE enable
	//REG_SSI_CR0 |= SSI_CR0_SSIE;

	//OUTREG32(A_GPIO_PXPES(1), 0x3c000000);
	NFC_SPI_Select();
	err = spiTxRx(buf, 1, buf, 1); //发送寄存器地址，并读取值
	//OUTREG32(A_GPIO_PXPEC(1), 0x3c000000);
	NFC_SPI_Deselect();

	//SSI_IE disable
	//REG_SSI_CR0 &= ~SSI_CR0_SSIE;

	*val = buf[0]; //将读取到的值存到*val中

	return err;
}

#define AS3911_IRQ_MASK_TIM (0x02) /*< additional interrupts in AS3911_REG_IRQ_TIMER_NFC */
#define AS3911_IRQ_MASK_ERR (0x01) /*< additional interrupts in AS3911_REG_IRQ_ERROR_WUP */

extern volatile U32 as3911InterruptStatus;
extern volatile U32 as3911InterruptStatusToWaitFor;
//extern OS_FLAG_GRP * g_pNFC_OS_Flag;

void NFCIntHandler(unsigned int arg)
{
	U8 iregs[3] = { 0, 0, 0 };
	U32 NFCReg;
	//S8 err = 0;

	//printk("Enter NFCIntHandler...\r\n");
	//disable_irq(EXP_GPIO_NFC_INT);
	//OUTREG32(A_GPIO_PXFLGC(NFC_INT_PIN / 32), 1 << (NFC_INT_PIN % 32));

	//do
	{
		/* just read out the status register to keep the isr S16 and simple */
		as3911ReadRegister_for_int(AS3911_REG_IRQ_MAIN, iregs);  //读取as3911主中断寄存器的值,放到iregs[0]
		if (iregs[0] & AS3911_IRQ_MASK_TIM) //是否有timer or nfc event
		{
			as3911ReadRegister_for_int(AS3911_REG_IRQ_TIMER_NFC, iregs + 1); //如果有，则读取timer/nfc中断寄存器，放到iregs[1]
		}
		if (iregs[0] & AS3911_IRQ_MASK_ERR) //是否有error or wake_up
		{
			as3911ReadRegister_for_int(AS3911_REG_IRQ_ERROR_WUP, iregs + 2); //如果有则读取error/wake-up中断寄存器，放到iregs[2]
		}

		NFCReg  = (U32)iregs[0];
		NFCReg |= (U32)iregs[1] << 8;
		NFCReg |= (U32)iregs[2] << 16;
		//printk("iregs[0] = %d\r\n", iregs[0]);
		//printk("iregs[1] = %d\r\n", iregs[1]);
		//printk("iregs[2] = %d\r\n", iregs[2]);
		//printk("as3911InterruptStatusToWaitFor = %d\r\n", as3911InterruptStatusToWaitFor);

		as3911InterruptStatus |= NFCReg;
	} //while (__gpio_get_pin(NFC_INT_PIN));

	//enable_irq(EXP_GPIO_NFC_INT);

	if ((as3911InterruptStatus & as3911InterruptStatusToWaitFor) != 0)
	{
		WaitIntFlag = 1;
		//printk("OSFlagPost...\r\n");
		//OSFlagPost(g_pNFC_OS_Flag, NFC_OS_FLAG_INTERRUPT, OS_FLAG_SET, &err);
	}
	//printk("Exit NFCIntHandler...\r\n");
}

