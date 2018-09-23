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
 *      $Revision: 1.4 $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author Ulrich Herrmann
 *
 *  \brief AS3911 stream mode driver
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "ic.h"
#include "as3911.h"
#include "as3911_com.h"
#include "as3911_interrupt.h"
#include "delay.h"
#include "config.h"
#include "as3911_stream.h"
#include <string.h>

/*
******************************************************************************
* LOCAL MACROS
******************************************************************************
*/

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/
static U8 as3911StreamRxStarted;
static U8 as3911StreamRxDone;
static U8 as3911StreamFifoRefill;

static U8 as3911StreamSavedOpReg;

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
S8 as3911StreamInitialize(const struct as3911StreamConfig *config)
{
    U8 smd = 0;
    U8 buf[3];
    U8 reg;

    as3911ReadRegister(AS3911_REG_OP_CONTROL,&as3911StreamSavedOpReg);

    buf[0] = (as3911StreamSavedOpReg
           | AS3911_REG_OP_CONTROL_en
           | AS3911_REG_OP_CONTROL_tx_en
           | AS3911_REG_OP_CONTROL_rx_en
           )
           & ~AS3911_REG_OP_CONTROL_wu;
    buf[1] = AS3911_REG_MODE_om_subcarrier_stream;
    buf[2] = 0x00; /* AS3911_REG_BIT_RATE : 106 kBit/s both direction */

    as3911StreamRxStarted = 0;
    as3911StreamRxDone = 0;

    if (config->useBPSK)
    {
        buf[1] = AS3911_REG_MODE_om_bpsk_stream;
        if (config->din<2 || config->din>4) /* not in fc/4 .. fc/16 */
        {
            return ERR_PARAM;
        }
        smd |= (4 - config->din) << AS3911_REG_STREAM_MODE_shift_scf;

    }
    else
    {
        if (config->din<3 || config->din>6) /* not in fc/8 .. fc/64 */
        {
            return ERR_PARAM;
        }
        smd |= (6 - config->din) << AS3911_REG_STREAM_MODE_shift_scf;
        if (config->report_period_length == 0) return ERR_PARAM;
    }

    if (config->dout<1 || config->dout>7) /* not in fc/2 .. fc/128 */
    {
        return ERR_PARAM;
    }
    smd |= (7 - config->dout) << AS3911_REG_STREAM_MODE_shift_stx;

    if (config->report_period_length > 3) return ERR_PARAM;
    smd |= config->report_period_length << AS3911_REG_STREAM_MODE_shift_scp;

    as3911ModifyRegister(AS3911_REG_AUX, AS3911_REG_AUX_no_crc_rx, AS3911_REG_AUX_no_crc_rx);
    as3911WriteRegister(AS3911_REG_STREAM_MODE, smd);
    as3911WriteMultipleRegisters(AS3911_REG_OP_CONTROL, buf, 3);

    /* Setup fifo param */
    as3911ReadRegister(AS3911_REG_IO_CONF1,&reg);
    as3911StreamFifoRefill = (reg & AS3911_REG_IO_CONF1_fifo_lt)?80:64;

    return ERR_NONE;
}

S8 as3911StreamDeinitialize(U8 keep_on)
{
    as3911StreamSavedOpReg &= ~(AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_rx_en);
    if (keep_on) as3911StreamSavedOpReg |=
                  AS3911_REG_OP_CONTROL_en
                | AS3911_REG_OP_CONTROL_tx_en;

    as3911WriteRegister(AS3911_REG_OP_CONTROL, as3911StreamSavedOpReg);
    as3911ModifyRegister(AS3911_REG_AUX, AS3911_REG_AUX_no_crc_rx, 0);
    return ERR_NONE;
}

U8 as3911StreamBuffer[AS3911_FIFO_DEPTH];
U8 as3911StreamBufferIdx;
U16 as3911StreamBytesToTx;
U8 as3911StreamTxStarted;

S8 as3911StreamTransferPrepare(U16 total_length_bits_to_tx)
{
    S8 err = ERR_NONE;

    as3911ExecuteCommand(AS3911_CMD_CLEAR_FIFO);

    /* enable FIFO water level and end of transmit interrupt */
    as3911EnableInterrupts(AS3911_IRQ_MASK_FWL | AS3911_IRQ_MASK_TXE |
                           AS3911_IRQ_MASK_RXS | AS3911_IRQ_MASK_RXE |
                           AS3911_IRQ_MASK_NRE);
    /* Clear these interrupts, this is important if previous transfer was followed by a receive */
    as3911GetInterrupt(AS3911_IRQ_MASK_FWL | AS3911_IRQ_MASK_TXE |
                       AS3911_IRQ_MASK_RXS | AS3911_IRQ_MASK_RXE |
                       AS3911_IRQ_MASK_NRE);

    /* Errata 11: First TX byte is ignored in Subcarrier Stream mode */
    total_length_bits_to_tx += 8;

    as3911WriteRegister(AS3911_REG_NUM_TX_BYTES1, total_length_bits_to_tx >> 8);
    as3911WriteRegister(AS3911_REG_NUM_TX_BYTES2, total_length_bits_to_tx & 0xff);
    as3911StreamBytesToTx = (total_length_bits_to_tx + 7)/8;

    as3911StreamBufferIdx = 0; as3911StreamTxStarted = 0;

    /* Errata 11: First TX byte is ignored in Subcarrier Stream mode */
    as3911StreamBuffer[0] = 0; as3911StreamBufferIdx++;

#if AS3911_TXRX_ON_CSX
    as3911WriteTestRegister(0x1,0x0a); /* digital modulation on pin CSI */
#endif

    return err;
}

S8 as3911StreamTxNBytes(const U8* buffer, U16 numbytes)
{
    S8 err = ERR_NONE;
    U32 mask;

    while (numbytes || (as3911StreamBytesToTx && (as3911StreamBufferIdx >= as3911StreamBytesToTx)))
    {
        /* 1. Fill local buffer as much as possible */
        U8 free_in_buffer = AS3911_FIFO_DEPTH - as3911StreamBufferIdx;
        U8 tocopy = free_in_buffer;

        if (numbytes < tocopy)
            tocopy = numbytes;

        memcpy(as3911StreamBuffer+as3911StreamBufferIdx, buffer, tocopy);
        numbytes -= tocopy;
        buffer += tocopy;
        as3911StreamBufferIdx += tocopy;

        /* 2. Transmit if necessary  */
        if (numbytes || (as3911StreamBytesToTx && (as3911StreamBufferIdx >= as3911StreamBytesToTx)))
        {
            U8 bytes_left_in_buf = as3911StreamBufferIdx - as3911StreamFifoRefill;
            U8 bytes_to_tx = as3911StreamFifoRefill;

            if (as3911StreamBufferIdx < as3911StreamFifoRefill)
            {
                bytes_to_tx = as3911StreamBufferIdx;
                bytes_left_in_buf = 0;
            }

            if(as3911StreamTxStarted)
            {
                /* wait for FIFO waterlevel interrupt */
                mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_FWL, 100);
                if (0 == mask)
                {
                    err = ERR_TIMEOUT;
                    break;
                }
            }

            /* fill up the FIFO */
            as3911WriteFifo(as3911StreamBuffer, bytes_to_tx);
            memmove((void*)as3911StreamBuffer,
                    (void*)(as3911StreamBuffer + bytes_to_tx),
                    (size_t)bytes_left_in_buf);
            as3911StreamBytesToTx -= bytes_to_tx;
            as3911StreamBufferIdx -= bytes_to_tx;

            if(!as3911StreamTxStarted)
            {
                as3911ExecuteCommand(AS3911_CMD_TRANSMIT_WITHOUT_CRC);
                as3911StreamTxStarted = 1;
            }
        }

    }
    return err;
}

S8 as3911StreamRxNBytesCont(U8* buffer, U16 size, U16 *actSize)
{
    U32 mask;
    S8 err = ERR_NONE;
    U8 bytesToRead;

    bytesToRead = as3911StreamFifoRefill;
    *actSize = 0;

    if(!as3911StreamRxStarted)
    {
        /* wait for transmit finshed interrupt. */
        mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_TXE, 100);
        if (0 == mask)
        {
			#ifdef ERR_SIGNAL
			err = ERR_SIGNAL;
			#else
            err = ERR_TIMEOUT;
			#endif
        }

#if AS3911_TXRX_ON_CSX
        as3911WriteTestRegister(0x1,0x04); /* digital demodulation on pins CSI and CSO(AM) */
#endif

        mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_RXS |
                                            AS3911_IRQ_MASK_NRE
                                            , (as3911GetNoResponseTime_64fcs() * 5 + 999) / 1000);
        if (AS3911_IRQ_MASK_NRE & mask)
        {
            err = ERR_TIMEOUT;
        }

        if (!mask)
        {
            //dbgLog("neither rxs nor nre!\r\n");
			#ifdef ERR_SIGNAL
			err = ERR_SIGNAL;
			#else
			err = ERR_TIMEOUT;
			#endif
        }
    }
    if (0 != err) return err;
    if(as3911StreamRxDone)
        return ERR_DONE;

    while (size && bytesToRead)
    {
        mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_FWL | AS3911_IRQ_MASK_RXE, 100);
        if (mask & AS3911_IRQ_MASK_RXE)
        {
            U8 stat2;
            as3911StreamRxDone = 1;
            as3911ReadRegister(AS3911_REG_FIFO_RX_STATUS1, &bytesToRead);
            /* Reading it two times to work around clock syncronisation bug in AS3911 */
            as3911ReadRegister(AS3911_REG_FIFO_RX_STATUS1, &bytesToRead);
            as3911ReadRegister(AS3911_REG_FIFO_RX_STATUS2, &stat2);

            if(bytesToRead > AS3911_FIFO_DEPTH)
            {
				#ifdef ERR_SIGNAL
				err = ERR_SIGNAL;
				#else
				err = ERR_FIFO;
				#endif
                break;
            }

            if(bytesToRead > size)
            {
                bytesToRead = size;
            }

            as3911ReadFifo(buffer,bytesToRead);
            size -= bytesToRead;
            buffer += bytesToRead;
            *actSize += bytesToRead * 8;
            if (stat2 & AS3911_REG_FIFO_RX_STATUS2_fifo_ncp)
            {
                *actSize -= (8 - ((stat2 & AS3911_REG_FIFO_RX_STATUS2_mask_fifo_lb)
                                   >> AS3911_REG_FIFO_RX_STATUS2_shift_fifo_lb));

            }
            break;
        }
        else if (mask & AS3911_IRQ_MASK_FWL)
        {
            if(bytesToRead > size)
            {
                bytesToRead = size;
            }
            as3911ReadFifo(buffer,bytesToRead);
            size -= bytesToRead;
            buffer += bytesToRead;
            *actSize += bytesToRead * 8;
        }
        else
        { /* Should never hit */
			#ifdef ERR_SIGNAL
			err = ERR_SIGNAL;
			break;
			#else
			err = ERR_FIFO;
			#endif
        }

    }
    return err;
}

U32 as3911StreamRxStop(void)
{
    as3911StreamRxStarted = 0;
    as3911StreamRxDone = 0;
    as3911ExecuteCommand(AS3911_CMD_MASK_RECEIVE_DATA);
    as3911ExecuteCommand(AS3911_CMD_CLEAR_FIFO);
#if AS3911_TXRX_ON_CSX
    as3911WriteTestRegister(0x1,0x00); /* restore default */
#endif
    as3911DisableInterrupts(AS3911_IRQ_MASK_RXS |
                        AS3911_IRQ_MASK_TXE |
                        AS3911_IRQ_MASK_RXE |
                        AS3911_IRQ_MASK_NRE |
                        AS3911_IRQ_MASK_FWL );
    return AS3911_IRQ_MASK_NONE;
}
