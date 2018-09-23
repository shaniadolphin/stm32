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
 *  \brief delay module declaration file
 *
 */
/*!
 *
 * The delay module is used to delay the current program for given number
 * of milli or micro seconds. It makes use of the system timers to allow
 * quite exact delays.
 *
 * API:
 * - Initialize delay module: #delayInitialize
 * - Deinitialize delay module: #delayDeinitialize
 * - Delay for N milli seconds: #delayNMilliSeconds
 * - Delay for N micro seconds: #delayNMicroSeconds
 * - Prepare for later micro seconds delay: #delayNMicroSecondsPrepare
 * - Run previously prepared delay: #delayNMicroSecondsRun
 */

#ifndef DELAY_H
#define DELAY_H

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
 *  \brief  Delay the program for a given number of milli seconds
 *
 *  This function doesn't return before \a ms milli seconds passed by.
 *
 *  \param[in] ms: number of milli seconds to delay
 *
 *  \return ERR_NONE : No error, program delayed for \a ms milliseconds.
 *
 *****************************************************************************
 */
extern S8 delayNMilliSeconds(U16 ms);

/*!
 *****************************************************************************
 *  \brief  Delay the program for a given number of micro seconds
 *
 *  This function doesn't return before \a us micro seconds passed by.
 *  \note In case of very small delays (< 5us) to overhead for setting up
 *  the timer might lead to an inaccuracy. In this case the setup should be
 *  done outside of the time critical part by calling #delayNMicroSecondsPrepare.
 *  Then the actual delay loop is started by calling #delayNMicroSecondsRun
 *
 *  \param[in] us: number of micro seconds to delay
 *
 *  \return ERR_NONE : No error, program delayed for \a us micro seconds.
 *
 *****************************************************************************
 */
extern S8 delayNMicroSeconds(U16 us);

#endif /* DELAY_H */

