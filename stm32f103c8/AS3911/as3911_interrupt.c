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
 *  \brief AS3911 Interrupt handling and ISR
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "as3911_interrupt.h"
#include "as3911_com.h"
#include "ic.h"
#include "delay.h"
#include "as3911.h"

#define AS3911_IRQ_MASK_TIM (0x02) /*< additional interrupts in AS3911_REG_IRQ_TIMER_NFC */
#define AS3911_IRQ_MASK_ERR (0x01) /*< additional interrupts in AS3911_REG_IRQ_ERROR_WUP */

/*
******************************************************************************
* GLOBAL VARIABLES
******************************************************************************
*/
volatile U32 as3911InterruptStatus = 0;
volatile U32 as3911InterruptMask = 0; /* negative mask = AS3911 mask regs */
volatile U32 as3911InterruptStatusToWaitFor = 0;

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
S8 as3911ModifyInterrupts(U32 clr_mask, U32 set_mask)
{
    S8 err = 0;
    int i;
    U32 new_mask;

    icDisableInterrupt();
    new_mask = (~as3911InterruptMask & set_mask) |
                (as3911InterruptMask & clr_mask);
    as3911InterruptMask &= ~clr_mask;
    as3911InterruptMask |= set_mask;
    for (i=0; i<3 ; i++)
    {
        if (! ((new_mask >> (8*i)) & 0xff)) continue;
        err = as3911WriteRegister(AS3911_REG_IRQ_MASK_MAIN + i,
                (as3911InterruptMask>>(8*i))&0xff);
        //dbgLog("INT %x -> %x\r\n", AS3911_REG_IRQ_MASK_MAIN + i, (as3911InterruptMask>>(8*i))&0xff);
        /* Writes also AS3911_REG_IRQ_MASK_TIMER_NFC and AS3911_REG_IRQ_MASK_ERROR_WUP */
    }
    icEnableInterrupt();
    return err;
}

S8 as3911EnableInterrupts(U32 mask)
{
    return as3911ModifyInterrupts(mask,0);
}

S8 as3911DisableInterrupts(U32 mask)
{
    return as3911ModifyInterrupts(0,mask);
}

S8 as3911ClearInterrupts(void)
{
    S8 err;
    U8 iregs[3];

    err = as3911ReadMultipleRegisters(AS3911_REG_IRQ_MAIN, iregs, 3);

    if (ERR_NONE == err)
    {
        icDisableInterrupt();
        as3911InterruptStatus = 0;
        icEnableInterrupt();
    }

    return err;
}

U32 as3911GetInterrupt(U32 mask)
{
    mask &= as3911InterruptStatus;

    if (mask)
    {
        /* clear interrupt */
        icDisableInterrupt();
        as3911InterruptStatus &= ~mask;
        icEnableInterrupt();
    }

    return mask;
}

