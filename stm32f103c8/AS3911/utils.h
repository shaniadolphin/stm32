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
 *  \brief Common and helpful macros
 *
 */

#ifndef UTILS_H
#define UTILS_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "ams_types.h"

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
/*!
 * this macro evaluates an error variable \a ERR against an error code \a EC.
 * in case it is not equal it jumps to the given label \a LABEL.
 */
#define EVAL_ERR_NE_GOTO(EC, ERR, LABEL)                                   \
    if (EC != ERR) goto LABEL;

/*!
 * this macro evaluates an error variable \a ERR against an error code \a EC.
 * in case it is equal it jumps to the given label \a LABEL.
 */
#define EVAL_ERR_EQ_GOTO(EC, ERR, LABEL)                                   \
    if (EC == ERR) goto LABEL;


/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
extern void AMS_MEMMOVE(void *dest, const void *src, U32 length);

extern void AMS_MEMCPY(void *dest, const void *src, U32 length);

extern void* AMS_MEMSET(void* ptr, U8 val, U32 num);

extern void AMS_BYTESWAP(U8* buffer, U32 length);

#endif /* UTILS_H */

