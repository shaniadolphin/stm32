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
 *  \author Christian Eisendle
 *
 *  \brief Interrupt controller driver
 *
 */
/*!
 *
 * Abstraction of the interrupt controller to enable/disable interrupt sources.
 */

#ifndef IC_H
#define IC_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
/*!
 *****************************************************************************
 *  \brief  Initialize interrupt controller.
 *
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 icInitialize(void);

/*!
 *****************************************************************************
 *  \brief  Deinitialize interrupt controller.
 *
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 icDeinitialize(void);

/*!
 *****************************************************************************
 *  \brief  Enable given interrupt source.
 *
 *  This function enables the interrupt request for source \a src
 *
 *  \param[in] src : Source to be enabled.
 *
 *  \return ERR_PARAM : Given source \a src not available.
 *  \return ERR_NONE : No error, interrupt request for \a src enabled.
 *
 *****************************************************************************
 */
extern void icEnableInterrupt(void);

/*!
 *****************************************************************************
 *  \brief  Disable given interrupt source.
 *
 *  This function disables the interrupt request for source \a src
 *
 *  \param[in] src : Source to be disabled.
 *
 *  \return ERR_PARAM : Given source \a src not available.
 *  \return ERR_NONE : No error, interrupt request for \a src disabled.
 *
 *****************************************************************************
 */
extern void icDisableInterrupt(void);

/*!
 *****************************************************************************
 *  \brief  Clear interrupt flag of given interrupt source
 *
 *  This function clears the interrupt flag for source \a src
 *
 *  \param[in] src : Source which flag should be cleared.
 *
 *  \return ERR_PARAM : Given source \a src not available.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 icClearInterrupt(void);

extern void NFC_INT_init(void);

#endif /* IC_H */

