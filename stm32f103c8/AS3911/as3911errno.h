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
 *  \brief application specific error codes
 *
 */

#ifndef AS3911ERRNO_H
#define AS3911ERRNO_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "errno.h"

/*!
 * application specific errorcodes start at -50 */

#define ERR_CRC -50        /*!< crc error */ 
#define ERR_COLLISION -51  /*!< collision error */ 
#define ERR_NOTFOUND -52   /*!< transponder not found */ 
#define ERR_NOTUNIQUE -53  /*!< transponder not unique - more than one transponder in field */ 
#define ERR_NOTSUPP -54    /*!< requested operation not supported */ 
#define ERR_WRITE -55      /*!< write error */ 
#define ERR_FIFO  -56      /*!< fifo over or underflow error */ 
#define ERR_PAR   -57      /*!< parity error */ 
#define ERR_FRAMING -58    /*!< framing error */ 
#define ERR_DONE -59      /*!< transfer has already finished */
#define ERR_INTERNAL -59     /*!< internal error */
#define ERR_RF_COLLISION -60 /*!< \ingroup error codes RF collision during RF collision avoidance occured */

#define EXEC_AND_EVAL(EC, CMD)                              \
    do { EC = CMD; if (ERR_NONE != EC) return EC } while(0)

#endif /* AS3911ERRNO_H */
