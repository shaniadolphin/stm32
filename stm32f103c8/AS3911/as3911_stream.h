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
 *  \brief AS3911 stream mode driver declaration file
 *
 */
/*!
 *
 * In addition to the so-called transparent mode (which existed also in AS3910)
 * the AS3911 now supports a stream mode which relieves the MCU from generating
 * real-time signals. It allows implementation of ISO15693 and others which are
 * not directly supported by AS3911.
 *
 * For TX the bit stream which is put into FIFO is directly driving the
 * modulator. Modulation frequencies can be between fc/128 and fc/2.
 *
 * For RX there exist two options: Either subcarrier decoding or BPSK.
 * - Subcarrier coding is possible for frequencies between fc/64 and fc/8.
 * - BPSK is possible for frequencies between fc/4 and fc/16.
 * For RX also the report_period_length can be specified. I.e. how many subcarrier
 * pulses form one rx bit going into FIFO.
 *
 * Flow:
  \dot
  digraph stream_flow{
  as3911StreamInitialize->as3911StreamTransferPrepare;
  as3911StreamTransferPrepare->as3911StreamTxNBytes;
  as3911StreamTxNBytes->as3911StreamTxNBytes;
  as3911StreamTxNBytes->as3911StreamRxNBytes;
  as3911StreamRxNBytes->as3911StreamRxNBytes;
  as3911StreamRxNBytes->as3911StreamRxStop;
  as3911StreamRxStop->as3911StreamTransferPrepare;
  as3911StreamRxStop->as3911StreamDeinitialize;
  }
  \enddot
 */

#ifndef AS3911_STREAM_H
#define AS3911_STREAM_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"

/*! Parameters how the stream mode should work */
struct as3911StreamConfig {
    U8 useBPSK; /*!< 0: subcarrier, 1:BPSK */
    U8 din; /*!< the divider for the in subcarrier frequency: fc/2^din  */
    U8 dout; /*!< the divider for the in subcarrier frequency fc/2^dout */
    U8 report_period_length; /*!< the length of the reporting period 2^report_period_length*/
};

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*!
 *****************************************************************************
 *  \brief  Initialize stream mode driver
 *
 *  This function initializes the stream with the given parameters
 *
 *  \param[in] config : all settings for bitrates, type, etc.

 *  \return ERR_NONE : No error, stream mode driver initialized.
 *
 *****************************************************************************
 */
extern S8 as3911StreamInitialize(const struct as3911StreamConfig *config);

/*!
 *****************************************************************************
 *  \brief  Deinitialize stream mode driver
 *
 *  Calling this function deinitializes stream mode driver.
 *
 *  \param keep_on: if true the RF field will not be switched off
 *
 *  \return ERR_NONE : No error, stream mode driver deinitialized.
 *
 *****************************************************************************
 */
extern S8 as3911StreamDeinitialize(U8 keep_on);

/*!
 *****************************************************************************
 *  \brief  Prepare for upcoming transfer
 *
 *  This function is used to bring the AS3911 into a state where
 *  as3911StreamTxnBytes() can be called. as3911StreamInitialize() has to be
 *  called before.
 *
 *  \param[in] total_length_bits_to_tx : number of bits to transfer.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 as3911StreamTransferPrepare(U16 total_length_bits_to_tx);
/*!
 *****************************************************************************
 *  \brief  Transmit a given number of bytes
 *
 *  This function is used transmit a given number of bytes using stream
 *  mode. The AS3911 needs to be in stream mode (as3911StreamInitialize())
 *  and transfer needs to be prepared (as3911StreamTransferPrepare())
 *  This functions can be called multiple times to provide on seamless
 *  modulated signal.
 *
 *  \param[in] buffer : buffer to transmit.
 *  \param[in] length : number of bytes to transmit.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 as3911StreamTxNBytes(const U8* buffer, U16 length);

/*!
 *****************************************************************************
 *  \brief  Receive a given number of bytes
 *
 *  This function is used receive a given number of bytes using stream
 *  mode. The AS3911 needs to be in stream mode (#as3911StreamInitialize).
 *  Usually as3911StreamTxNBytesCont() was called just before. The first
 *  call will then wait for end of transmit and start RX.
 *
 *
 *  \param[in] buffer : buffer used to store the data.
 *  \param[in] size   : number of bytes to receive.
 *  \param[out] actSize : number of bits actually received
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *  \return ERR_DONE : If receive is already done, no more bytes available
 *
 *****************************************************************************
 */
extern S8 as3911StreamRxNBytesCont(U8* buffer, U16 size, U16 *actSize);

/*!
 *****************************************************************************
 *  \brief  Stop the current transfer (also prematurely).
 *
 *  This function stops any ongoing transfer and cleans everything up to
 *  allow new transfers
 *
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern U32 as3911StreamRxStop(void);
#endif /* AS3911_STREAM_H */
