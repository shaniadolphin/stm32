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
 *      $Revision: 1.6 $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author Christian Eisendle
 *
 *  \brief AS3911 high level interface
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "nfc.h"
#include "platform.h"
#include "as3911.h"
#include "as3911_com.h"
#include "as3911_interrupt.h"
#include "delay.h"
#include "config.h"
#include "stdio.h"
#include <limits.h>
#include "ic.h"

#include "..\cm3\stm32f10x.h"

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/
static bool_t as3911ReceivePrepared; /*!< flag indicating whether receive
                                       has been prepared */
static U32 as3911NoResponseTime_64fcs;

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static S8 as3911DecodeErrorInterrupts(U32 mask)
{
    S8 err = ERR_REQUEST;
    mask &= AS3911_IRQ_MASK_ERR1 | AS3911_IRQ_MASK_PAR | AS3911_IRQ_MASK_CRC;

    if (mask == 0) return ERR_NONE;

    if (mask & AS3911_IRQ_MASK_CRC)
    {
        err = ERR_CRC;
    }
    if (mask & AS3911_IRQ_MASK_PAR)
    {
        err = ERR_PAR;
    }
    if (mask & AS3911_IRQ_MASK_ERR1)
    {
        err = ERR_FRAMING;
    }
    return err;
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
S8 as3911Initialize(void)
{
    S8 err;
    U8 result;
    U32 mask;
    //U16 vdd_mV;

    as3911ReceivePrepared = FALSE;

    /* first, reset the as3911 */
    err = as3911ExecuteCommand(AS3911_CMD_SET_DEFAULT);

    if (ERR_NONE == err)
    { /* enable pull downs on miso line */
        err = as3911ModifyRegister(AS3911_REG_IO_CONF2, 0,
                AS3911_REG_IO_CONF2_miso_pd1 |
                AS3911_REG_IO_CONF2_miso_pd2);
    }
    if (ERR_NONE == err)
    {
        /* after reset all interrupts are enabled. so disable them at first */
        err = as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);
        if (ERR_NONE == err)
        {
            /* and clear them, just to be sure... */
            err = as3911ClearInterrupts();
        }
    }

    /* Configure IO pins - interrupt pin of AS3911 */
	icEnableInterrupt();

    /* trim settings for VHBR board, will anyway changed later on */
    as3911WriteRegister(AS3911_REG_ANT_CAL_TARGET, 0x80);
    as3911WriteRegister(AS3911_REG_ANT_CAL_CONTROL, 0x30);

    /* enable oscillator frequency stable interrupt */
    err = as3911EnableInterrupts(AS3911_IRQ_MASK_OSC);

    /* enable oscillator and regulator output */
    err = as3911ModifyRegister(AS3911_REG_OP_CONTROL, 0, AS3911_REG_OP_CONTROL_en);

    if (ERR_NONE == err)
    {
        /* wait for the oscillator interrupt, max. 1000ms */
        mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_OSC, 500);
        if (0 == mask)
        {
            as3911DisableInterrupts(AS3911_IRQ_MASK_OSC);
            //FIXME chip bug, oscillator interrupt works only once... return ERR_TIMEOUT;
        }
        err = as3911DisableInterrupts(AS3911_IRQ_MASK_OSC);
    }

    /* Measure vdd and set sup3V bit accordingly */
    //vdd_mV = as3911MeasureVoltage(AS3911_REG_REGULATOR_CONTROL_mpsv_vdd);

    as3911ModifyRegister(AS3911_REG_IO_CONF2,
                         0,
                         AS3911_REG_IO_CONF2_sup3V);//(vdd_mV < 4000)?AS3911_REG_IO_CONF2_sup3V:0);

    /* No automatic channel selection, select AM(default), most often better */
    as3911ModifyRegister(AS3911_REG_OP_CONTROL,
            0,
            AS3911_REG_OP_CONTROL_rx_man);

#if ISO15693
    as3911ModifyRegister(AS3911_REG_RX_CONF3,
			 AS3911_REG_RX_CONF3_rg1_am0,
			AS3911_REG_RX_CONF3_rg1_am1 | AS3911_REG_RX_CONF3_rg1_am2 );
    as3911WriteRegister(AS3911_REG_RX_CONF4, 0xaa);

    as3911ReadRegister(AS3911_REG_RX_CONF2, &result);
    as3911WriteRegister(AS3911_REG_RX_CONF2, result & ~AS3911_REG_RX_CONF2_agc_en & ~AS3911_REG_RX_CONF2_sqm_dyn);
	as3911ExecuteCommand(AS3911_CMD_CLEAR_SQUELCH);
#endif

    return err;
}

S8 as3911Deinitialize()
{
    S8 err;
    as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);
    /* Deinitialize IO pins - interrupt pin of AS3911 */
    //boardPeripheralPinDeinitialize(BOARD_PERIPHERAL_AS3911_INT);

    err = as3911WriteRegister(AS3911_REG_OP_CONTROL, 0x0);

    return err;
}

S8 as3911AdjustRegulators(U16* result_mV)
{
    U8 result;
    U8 io_conf2;
    S8 err;

    /* first check the status of the reg_s bit in AS3911_REG_VREG_DEF register.
       if this bit is set adjusting the regulators is not allowed */
    err = as3911ReadRegister(AS3911_REG_REGULATOR_CONTROL, &result);

    if (ERR_NONE == err)
    {
        if (result & AS3911_REG_REGULATOR_CONTROL_reg_s)
        {
            err = ERR_REQUEST;
            goto out;
        }
    }
    else
    {
        goto out;
    }

    err = as3911ExecuteCommandAndGetResult(AS3911_CMD_ADJUST_REGULATORS,
                                        AS3911_REG_REGULATOR_RESULT,
                                        5,
                                        &result);
    as3911ReadRegister(AS3911_REG_IO_CONF2, &io_conf2);

    result >>= AS3911_REG_REGULATOR_RESULT_shift_reg;
    result -= 5;
	if (result_mV)
	{
		if(io_conf2 & AS3911_REG_IO_CONF2_sup3V)
		{
			*result_mV = 2400;
			*result_mV += result * 100;
		}
		else
		{
			*result_mV = 3900;
			*result_mV += result * 120;
		}
	}

out:
    return err;
}

S8 as3911MeasureRF(U8* result)
{
    return as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_AMPLITUDE,
                                        AS3911_REG_AD_RESULT,
                                        10,
                                        result);
}

S8 as3911MeasureAntennaResonance(U8* result)
{
    return as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_PHASE,
                                        AS3911_REG_AD_RESULT,
                                        10,
                                        result);
}

S8 as3911CalibrateAntenna(U8* result)
{
    return as3911ExecuteCommandAndGetResult(AS3911_CMD_CALIBRATE_ANTENNA,
                                        AS3911_REG_ANT_CAL_RESULT,
                                        10,
                                        result);
}

S8 as3911CalibrateModulationDepth(U8* result)
{
    return as3911ExecuteCommandAndGetResult(AS3911_CMD_CALIBRATE_MODULATION,
                                        AS3911_REG_AM_MOD_DEPTH_RESULT,
                                        10,
                                        result);
}

S8 as3911PrepareReceive(bool_t reset)
{
    S8 err;

    if (reset)
    {
        /* Workaround for v2 silicon fifo problems:  always have crc in FIFO */
        as3911ModifyRegister(AS3911_REG_AUX, AS3911_REG_AUX_crc_2_fifo, AS3911_REG_AUX_crc_2_fifo);
        /* reset receive logic */
        err = as3911ExecuteCommand(AS3911_CMD_CLEAR_FIFO);
        EVAL_ERR_NE_GOTO(ERR_NONE, err, out);
        /* clear interrupts */
        as3911GetInterrupt(AS3911_IRQ_MASK_RXS | AS3911_IRQ_MASK_RXE  |
                           AS3911_IRQ_MASK_PAR  | AS3911_IRQ_MASK_CRC  |
                           AS3911_IRQ_MASK_ERR1 | AS3911_IRQ_MASK_FWL |
                           AS3911_IRQ_MASK_NRE);
    }
    err = as3911EnableInterrupts(AS3911_IRQ_MASK_RXS | AS3911_IRQ_MASK_RXE  |
                                 AS3911_IRQ_MASK_PAR  | AS3911_IRQ_MASK_CRC  |
                                 AS3911_IRQ_MASK_ERR1 | AS3911_IRQ_MASK_FWL |
                                 AS3911_IRQ_MASK_NRE);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    as3911ReceivePrepared = TRUE;

out:
    return err;
}

S8 as3911RxNBytes(U8* buf, U16 maxlength, U16* length, U16 timeout_ms)
{
    S8 err = ERR_NONE;
    U8 bytesToRead;
    U32 mask;
    U8 reg;

    *length = 0;

    /* check if as3911PrepareReceive was called before */
    if (!as3911ReceivePrepared)
    {
        return ERR_REQUEST;
    }

    if (0 == timeout_ms) timeout_ms = (as3911GetNoResponseTime_64fcs() * 5 + 999) / 1000;

#if AS3911_TXRX_ON_CSX
    as3911WriteTestRegister(0x1,0x04); /* digital demodulation on pins CSI and CSO(AM) */
#endif

    as3911ReadRegister(AS3911_REG_IO_CONF1,&reg);

    bytesToRead = (reg & AS3911_REG_IO_CONF1_fifo_lt)?80:64;

    /* wait for start of receive or no response interrupt */
    mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_RXS | AS3911_IRQ_MASK_RXE | AS3911_IRQ_MASK_NRE | AS3911_IRQ_MASK_GPE, timeout_ms);

    if (mask & (AS3911_IRQ_MASK_NRE | AS3911_IRQ_MASK_GPE))
    {
        err = ERR_TIMEOUT;
        goto out;
    }
    if (!(mask & AS3911_IRQ_MASK_RXS) && (mask & AS3911_IRQ_MASK_RXE))
    { /* rx end but not rx start interrupt */
        err = as3911DecodeErrorInterrupts(as3911GetInterrupt(AS3911_IRQ_MASK_ALL));
        goto out;
    }

    if (!mask)
    {
        //dbgLog("neither rxs nor nre nor gpe!\r\n");
        err = ERR_TIMEOUT;
        goto out;
    }

    do
    {
        /* wait either for FIFO waterlevel or end of receive interrupt */
        mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_RXE |
                                            AS3911_IRQ_MASK_FWL |
                                            AS3911_IRQ_MASK_CRC |
                                            AS3911_IRQ_MASK_PAR |
                                            AS3911_IRQ_MASK_ERR1|
                                            0
                                            , 10);

        err = as3911ReadRegister(AS3911_REG_FIFO_RX_STATUS1, &bytesToRead);
        /* Reading it two times to work around clock syncronisation bug in AS3911 */
        err = as3911ReadRegister(AS3911_REG_FIFO_RX_STATUS1, &bytesToRead);

        /* also read out number of received bits in last byte and handle it */
        EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

        if ((*length + bytesToRead) > maxlength)
        {
            as3911ReadFifo(buf, maxlength - *length);
            *length = maxlength;
            break;
        }

        *length += bytesToRead;
        err = as3911ReadFifo(buf, bytesToRead);
        EVAL_ERR_NE_GOTO(ERR_NONE, err, out);
        buf += bytesToRead;

        /* loop as S32 as rx is in progress */
    } while (mask & AS3911_IRQ_MASK_FWL);

    if(bytesToRead > AS3911_FIFO_DEPTH)
    {
        err = ERR_FIFO;
        goto out;
    }

    err = as3911DecodeErrorInterrupts(mask);

out:
    delayNMicroSeconds(500);
    as3911DisableInterrupts(AS3911_IRQ_MASK_RXS |
                        AS3911_IRQ_MASK_RXE |
                        AS3911_IRQ_MASK_FWL |
                        AS3911_IRQ_MASK_PAR |
                        AS3911_IRQ_MASK_CRC |
                        AS3911_IRQ_MASK_NRE |
                        AS3911_IRQ_MASK_ERR1);
    as3911ExecuteCommand(AS3911_CMD_MASK_RECEIVE_DATA);
#if AS3911_TXRX_ON_CSX
    as3911WriteTestRegister(0x1,0x00); /* default value */
#endif
    return err;
}

S8 as3911TxNBytes(const U8* frame, U16 numbytes, U8 numbits, as3911TxFlag_t flags)
{
    U16 txbytes;
    U32 mask;
    S8 err;
    U8 antcl = (flags & AS3911_TX_FLAG_ANTCL) ? AS3911_REG_ISO14443A_NFC_antcl : 0;
    U8 reg, fifo_refill;
    /* calculate total num of bits, use variable numbytes since it is 16-bit */
    numbytes = numbytes * 8 + numbits;

    /* Sometimes the FIFO is not in a proper state e.g. at HLTA */
    as3911ExecuteCommand(AS3911_CMD_CLEAR_FIFO);

    as3911ReadRegister(AS3911_REG_IO_CONF1,&reg);
    fifo_refill = (reg & AS3911_REG_IO_CONF1_fifo_lt)?80:64;

    /* enable FIFO water level and end of transmit interrupt */
    err = as3911EnableInterrupts(AS3911_IRQ_MASK_FWL | AS3911_IRQ_MASK_TXE);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    /* Set antcl bit for iso14443a anti-collison: proper timings and proper parity extraction */
    err = as3911ModifyRegister(AS3911_REG_ISO14443A_NFC, AS3911_REG_ISO14443A_NFC_antcl, antcl);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out_disable_interrupt);

    /* write number of bytes to be transmitted */
    err = as3911WriteRegister(AS3911_REG_NUM_TX_BYTES2, numbytes & 0xff);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out_disable_interrupt);

    err = as3911WriteRegister(AS3911_REG_NUM_TX_BYTES1, (U8)(numbytes >> 8) & 0xff);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out_disable_interrupt);

    /* Now numbytes is again bytes */
    numbytes = (numbytes + 7) / 8;

    /* fifo depth is AS3911_FIFO_DEPTH */
    if (numbytes > AS3911_FIFO_DEPTH)
    {
        txbytes = AS3911_FIFO_DEPTH;
        numbytes -= AS3911_FIFO_DEPTH;
    }
    else
    {
        txbytes = numbytes;
        numbytes = 0;
    }

    /* fill up the FIFO */
    err = as3911WriteFifo(frame, txbytes);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out_disable_interrupt);

#if AS3911_TXRX_ON_CSX
    as3911WriteTestRegister(0x1,0x0a); /* digital modulation on pin CSI */
#endif

    /* and transmit data */
    if (flags & AS3911_TX_FLAG_CRC)
    {
        err = as3911ExecuteCommand(AS3911_CMD_TRANSMIT_WITH_CRC);
    }
    else if (flags & AS3911_TX_FLAG_NFC_INITIATOR)
    {
        err = as3911ExecuteCommand(AS3911_CMD_INITIAL_RF_COLLISION);
    }
    else if (flags & AS3911_TX_FLAG_NFC_TARGET)
    {
        err = as3911ExecuteCommand(AS3911_CMD_RESPONSE_RF_COLLISION_N);
    }
    else
    {
        err = as3911ExecuteCommand(AS3911_CMD_TRANSMIT_WITHOUT_CRC);
    }

    EVAL_ERR_NE_GOTO(ERR_NONE, err, out_disable_interrupt);

    /* check if there is still some more data to be transmitted */
    while (numbytes > 0)
    {
        frame += txbytes;
        if (numbytes > fifo_refill)
        {
            txbytes = fifo_refill;
            numbytes -= fifo_refill;
        }
        else
        {
            txbytes = numbytes;
            numbytes = 0;
        }

        /* wait for FIFO waterlevel interrupt */
        mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_FWL, 100);
        if (0 == mask)
        {
            err = ERR_TIMEOUT;
        }

        /* fill up the FIFO */
        err = as3911WriteFifo(frame, txbytes);
        EVAL_ERR_NE_GOTO(ERR_NONE, err, out_disable_interrupt);
    }

    /* wait for transmit finshed interrupt. */
    mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_TXE, 100);
    if (0 == mask)
    {
        err = ERR_TIMEOUT;
    }

#if AS3911_TXRX_ON_CSX
    as3911WriteTestRegister(0x1,0x04); /* digital demodulation on pins CSI and CSO(AM) */
#endif

    as3911ReadRegister(AS3911_REG_FIFO_RX_STATUS2,&reg);
    if (reg & (AS3911_REG_FIFO_RX_STATUS2_fifo_ovr | AS3911_REG_FIFO_RX_STATUS2_fifo_unf))
    {
        err = ERR_FIFO;
    }


out_disable_interrupt:
    as3911DisableInterrupts(AS3911_IRQ_MASK_TXE);
out:
    return err;

}

S8 as3911SetBitrate(U8 txrate, U8 rxrate)
{
    S8 err = ERR_NONE;
    U8 reg;

    err = as3911ReadRegister(AS3911_REG_BIT_RATE, &reg);
    if (err) return err;
    if (rxrate != 0xff)
    {
        if(rxrate > 5)
        {
            err = ERR_PARAM;
        }
        else
        {
            reg &= ~AS3911_REG_BIT_RATE_mask_rxrate;
            reg |= rxrate << AS3911_REG_BIT_RATE_shift_rxrate;
        }
    }
    if (txrate != 0xff)
    {
        if(txrate > 6)
        {
            err = ERR_PARAM;
        }
        else
        {
            reg &= ~AS3911_REG_BIT_RATE_mask_txrate;
            reg |= txrate<<AS3911_REG_BIT_RATE_shift_txrate;
        }
    }
    err = as3911WriteRegister(AS3911_REG_BIT_RATE, reg);
    err |= as3911ExecuteCommand(AS3911_CMD_ANALOG_PRESET);
    return err;
}


U16 as3911MeasureVoltage(U8 mpsv)
{
    U8 result;
    U16 mV;

    mpsv &= AS3911_REG_REGULATOR_CONTROL_mask_mpsv;

    as3911ModifyRegister(AS3911_REG_REGULATOR_CONTROL,
                         AS3911_REG_REGULATOR_CONTROL_mask_mpsv,
                         mpsv);

    as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_VDD,
            AS3911_REG_AD_RESULT,
            100,
            &result);
    mV = ((U16)result) * 23;
    mV += ((((U16)result) * 438) + 500) / 1000;

    return mV;
}

U32 as3911GetNoResponseTime_64fcs(void)
{
    return as3911NoResponseTime_64fcs;
}

S8 as3911StartGPTimer_8fcs(U16 gpt_8fcs)
{
    S8 err = as3911SetGPTime_8fcs(gpt_8fcs);

    as3911ModifyRegister(AS3911_REG_GPT_CONTROL,
            AS3911_REG_GPT_CONTROL_gptc_mask,
            AS3911_REG_GPT_CONTROL_gptc_no_trigger);

    as3911ExecuteCommand(AS3911_CMD_START_TIMER);

    return err;
}

S8 as3911SetGPTime_8fcs(U16 gpt_8fcs)
{
    S8 err = ERR_NONE;

    as3911WriteRegister(AS3911_REG_GPT1, gpt_8fcs >> 8);
    as3911WriteRegister(AS3911_REG_GPT2, gpt_8fcs & 0xff);

    return err;
}

S8 as3911SetNoResponseTime_64fcs(U32 nrt_64fcs)
{
    S8 err = ERR_NONE;
    U8 nrt_step = 0;

    as3911NoResponseTime_64fcs = nrt_64fcs;
    if (nrt_64fcs > USHRT_MAX)
    {
        nrt_step = AS3911_REG_GPT_CONTROL_nrt_step;
        nrt_64fcs = (nrt_64fcs + 63) / 64;
        if (nrt_64fcs > USHRT_MAX)
        {
            nrt_64fcs = USHRT_MAX;
            err = ERR_PARAM;
        }
        as3911NoResponseTime_64fcs = 64 * nrt_64fcs;
    }

    as3911ModifyRegister(AS3911_REG_GPT_CONTROL, AS3911_REG_GPT_CONTROL_nrt_step, nrt_step);
    as3911WriteRegister(AS3911_REG_NO_RESPONSE_TIMER1, nrt_64fcs >> 8);
    as3911WriteRegister(AS3911_REG_NO_RESPONSE_TIMER2, nrt_64fcs & 0xff);

    return err;
}

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/
/*!
 *****************************************************************************
 *  \brief  Executes a direct command and returns the result
 *
 *  This function executes the direct command given by \a cmd waits for
 *  \a sleeptime and returns the result read from register \a resreg.
 *
 *  \param[in] cmd: direct command to execute.
 *  \param[in] resreg: Address of the register containing the result.
 *  \param[in] sleeptime: time in milliseconds to wait before reading the result.
 *  \param[out] result: 8 bit S32 result
 *
 *  \return ERR_IO : Error during communication with AS3911.
 *  \return ERR_NONE : No error, result of direct command written to \a result.
 *
 *****************************************************************************
 */
S8 as3911ExecuteCommandAndGetResult(U8 cmd, U8 resreg, U8 sleeptime, U8* result)
{
    S8 err;

    if (cmd == AS3911_CMD_ADJUST_REGULATORS)
    {
        err = as3911ExecuteCommand(cmd);
        delayNMilliSeconds(3);
    }
    else if (   (cmd >= AS3911_CMD_INITIAL_RF_COLLISION && cmd <= AS3911_CMD_RESPONSE_RF_COLLISION_0)
            || (cmd == AS3911_CMD_MEASURE_AMPLITUDE)
            || (cmd >= AS3911_CMD_ADJUST_REGULATORS && cmd <= AS3911_CMD_MEASURE_PHASE)
            || (cmd >= AS3911_CMD_CALIBRATE_C_SENSOR && cmd <= AS3911_CMD_MEASURE_VDD)
            || (cmd >= 0xFD && cmd <= 0xFE )
       )
    {
        as3911EnableInterrupts(AS3911_IRQ_MASK_DCT);
        as3911GetInterrupt(AS3911_IRQ_MASK_DCT);
        err = as3911ExecuteCommand(cmd);
        as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_DCT, sleeptime);
        as3911DisableInterrupts(AS3911_IRQ_MASK_DCT);
    }
    else
    {
        err = as3911ExecuteCommand(cmd);
        delayNMilliSeconds(sleeptime);
    }

    /* read out the result if the pointer is not NULL */
    if (result)
        err |= as3911ReadRegister(resreg, result);

    return err;
}

