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
 *      PROJECT:   AS3911
 *      $Revision: 1.2 $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author Christian Eisendle
 *
 *  \brief Platform specific header file
 *
 */

#ifndef PLATFORM_H
#define PLATFORM_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "ams_types.h"
#include "as3911errno.h"
#include "utils.h"

/*
******************************************************************************
* GLOBAL VARIABLES
******************************************************************************
*/
extern U16 IRQ_COUNT;

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

extern void NFC_IRQ_Disable(void);
extern void NFC_IRQ_Enable(void);

/*! macro which globally disables interrupts and increments interrupt count */
#define IRQ_INC_DISABLE() do {                                              \
    NFC_IRQ_Disable();                                                      \
    IRQ_COUNT++;                                                            \
} while(0)


/*! macro to globally enable interrupts again if interrupt count is 0 */
#define IRQ_DEC_ENABLE() do {                                               \
   if (IRQ_COUNT != 0) IRQ_COUNT--;                                         \
   if (IRQ_COUNT == 0)  NFC_IRQ_Enable();                                   \
} while(0)

#endif /* PLATFORM_H */

