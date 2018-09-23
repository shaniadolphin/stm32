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
 *      $Revision: 1.2 $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author Ulrich Herrmann
 *
 *  \brief AS3911 Interrupt header file
 *
 */

#ifndef AS3911_INTERRUPT_H
#define AS3911_INTERRUPT_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/* Main interrupt register. */
#define AS3911_IRQ_MASK_ALL             U32_C(0xFFFFFF) /*!< All AS3911 interrupt sources. */
#define AS3911_IRQ_MASK_NONE            U32_C(0)        /*!< No AS3911 interrupt source. */
#define AS3911_IRQ_MASK_OSC             U32_C(0x80)     /*!< AS3911 oscillator stable interrupt. */
#define AS3911_IRQ_MASK_FWL             U32_C(0x40)     /*!< AS3911 FIFO water level inerrupt. */
#define AS3911_IRQ_MASK_RXS             U32_C(0x20)     /*!< AS3911 start of receive interrupt. */
#define AS3911_IRQ_MASK_RXE             U32_C(0x10)     /*!< AS3911 end of receive interrupt. */
#define AS3911_IRQ_MASK_TXE             U32_C(0x08)     /*!< AS3911 end of transmission interrupt. */
#define AS3911_IRQ_MASK_COL             U32_C(0x04)     /*!< AS3911 bit collision interrupt. */

/* Timer and NFC interrupt register. */
#define AS3911_IRQ_MASK_DCT             U32_C(0x8000)   /*!< AS3911 termination of direct command interrupt. */
#define AS3911_IRQ_MASK_NRE             U32_C(0x4000)   /*!< AS3911 no-response timer expired interrupt. */
#define AS3911_IRQ_MASK_GPE             U32_C(0x2000)   /*!< AS3911 general purpose timer expired interrupt. */
#define AS3911_IRQ_MASK_EON             U32_C(0x1000)   /*!< AS3911 external field on interrupt. */
#define AS3911_IRQ_MASK_EOF             U32_C(0x0800)   /*!< AS3911 external field off interrupt. */
#define AS3911_IRQ_MASK_CAC             U32_C(0x0400)   /*!< AS3911 collision during RF collision avoidance interrupt. */
#define AS3911_IRQ_MASK_CAT             U32_C(0x0200)   /*!< AS3911 minimum guard time expired interrupt. */
#define AS3911_IRQ_MASK_NFCT            U32_C(0x0100)   /*!< AS3911 initiator bit rate recognized interrupt. */

/* Error and wake-up interrupt register. */
#define AS3911_IRQ_MASK_CRC             U32_C(0x800000) /*!< AS3911 CRC error interrupt. */
#define AS3911_IRQ_MASK_PAR             U32_C(0x400000) /*!< AS3911 parity error interrupt. */
#define AS3911_IRQ_MASK_ERR2            U32_C(0x200000) /*!< AS3911 soft framing error interrupt. */
#define AS3911_IRQ_MASK_ERR1            U32_C(0x100000) /*!< AS3911 hard framing error interrupt. */
#define AS3911_IRQ_MASK_WT              U32_C(0x080000) /*!< AS3911 wake-up interrupt. */
#define AS3911_IRQ_MASK_WAM             U32_C(0x040000) /*!< AS3911 wake-up due to amplitude interrupt. */
#define AS3911_IRQ_MASK_WPH             U32_C(0x020000) /*!< AS3911 wake-up due to phase interrupt. */
#define AS3911_IRQ_MASK_WCAP            U32_C(0x010000) /*!< AS3911 wake-up due to capacitance measurement. */

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
/*!
 *****************************************************************************
 *  \brief  Enable a given AS3911 Interrupt source
 *
 *  This function enables all interrupts given by \a mask,
 *  AS3911_IRQ_MASK_ALL enables all interrupts.
 *
 *  \param[in] mask: mask indicating the interrupts to be enabled
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, given interrupts enabled.
 *
 *****************************************************************************
 */
extern S8 as3911EnableInterrupts(U32 mask);

/*!
 *****************************************************************************
 *  \brief  Disable one or more a given AS3911 Interrupt sources
 *
 *  This function disables all interrupts given by \a mask. 0xff disables all.
 *
 *  \param[in] mask: mask indicating the interrupts to be disabled.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, given interrupts disabled.
 *
 *****************************************************************************
 */
extern S8 as3911DisableInterrupts(U32 mask);

/*!
 *****************************************************************************
 *  \brief  Clear all as3911 irq flags
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, given interrupts disabled.
 *
 *****************************************************************************
 */
extern S8 as3911ClearInterrupts(void);

/*!
 *****************************************************************************
 *  \brief  Wait until an AS3911 interrupt occurs
 *
 *  This function is used to access the AS3911 interrupt flags. Use this
 *  to wait for max. \a tmo milliseconds for the \b first interrupt indicated
 *  with mask \a mask to occur.
 *
 *  \param[in] mask : mask indicating the interrupts to wait for.
 *  \param[in] tmo : time in milliseconds until timeout occurs. If set to 0
 *                   the functions waits forever.
 *
 *  \return : 0 if timeout occured otherwise a mask indicating the cleared
 *              interrupts.
 *
 *****************************************************************************
 */
extern U32 as3911WaitForInterruptsTimed(U32 mask, U16 tmo);

/*!
 *****************************************************************************
 *  \brief  Get status for the given interrupt
 *
 *  This function is used to check whether the interrupt given by \a mask
 *  has occured. If yes the interrupt gets cleared. This function returns
 *  only status bits which are inside \a mask.
 *
 *  \param[in] mask : mask indicating the interrupt to check for.
 *
 *  \return the mask of the interrupts occurred
 *
 *****************************************************************************
 */
extern U32 as3911GetInterrupt(U32 mask);

#endif /* AS3911_ISR_H */
