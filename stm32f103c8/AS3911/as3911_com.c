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
 *  \brief Implementation of AS3911 communication.
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "as3911_com.h"
#include "as3911.h"
#include "delay.h"

/*
******************************************************************************
* LOCAL DEFINES
******************************************************************************
*/

extern void NFC_SPI_Select(void);
extern void NFC_SPI_Deselect(void);
extern S8 spiTxRx(const U8 * txData, unsigned int txLen, U8 * rxData, unsigned int rxLen);

#define AS3911_SPI_SELECT()		NFC_SPI_Select()
#define AS3911_SPI_DESELECT() 	NFC_SPI_Deselect()

#define AS3911_WRITE_MODE  (0)
#define AS3911_READ_MODE   (1 << 6)
#define AS3911_FIFO_LOAD   (2 << 6)
#define AS3911_FIFO_READ   (0xbf)
#define AS3911_CMD_MODE    (3 << 6)

extern volatile U32 as3911InterruptStatus;
extern volatile U32 as3911InterruptMask;
/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static S8 as3911Write(U8 cmd, const U8* values, U8 length);

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
S8 as3911ReadRegister(U8 reg, U8* val)
{
    U8 buf[2];
    S8 err;

    buf[0] = reg | AS3911_READ_MODE;
    buf[1] = 0;

    IRQ_INC_DISABLE();

    AS3911_SPI_SELECT();
    err = spiTxRx(buf, 1, buf, 1);
    AS3911_SPI_DESELECT();
    *val = buf[0];

    IRQ_DEC_ENABLE();

    return err;
}

S8 as3911ReadMultipleRegisters(U8 reg, U8* val, U8 length)
{
    S8 err;
    //U8 lastbyte;
    //U8 i;

    val[0] = reg | AS3911_READ_MODE;

    /* make this operation atomic */
    IRQ_INC_DISABLE();
    /* since the result is right shited by one byte we can't receive
       all bytes within one transfer. So we have to do another transfer
       for receiving the last byte */
    AS3911_SPI_SELECT();
    //err = spiTxRx(val, val, length);
    err = spiTxRx(val, 1, val, length);
#if 0
    if (ERR_NONE != err)
    {
        goto out;
    }
    err = spiTxRx(&lastbyte, &lastbyte, 1);

    /* left shift all bytes by one position */
    for (i = 0; i < (length - 1); i++)
    {
        val[i] = val[i+1];
    }
    val[i] = lastbyte;
out:
#endif
    AS3911_SPI_DESELECT();
    IRQ_DEC_ENABLE();
    return err;
}

S8 as3911WriteTestRegister(U8 reg, U8 val)
{
    S8 err;
    U8 buf[3];

    buf[0] = AS3911_CMD_TEST_ACCESS;
    buf[1] = reg | AS3911_WRITE_MODE;
    buf[2] = val;

    IRQ_INC_DISABLE();
    AS3911_SPI_SELECT();
    err = spiTxRx(buf, 3, NULL, 0);
    AS3911_SPI_DESELECT();
    IRQ_DEC_ENABLE();

    return err;
}

S8 as3911WriteRegister(U8 reg, U8 val)
{
    S8 err;
    U8 buf[2];

    buf[0] = reg | AS3911_WRITE_MODE;
    buf[1] = val;

    IRQ_INC_DISABLE();
    AS3911_SPI_SELECT();
    err = spiTxRx(buf, 2, NULL, 0);
    AS3911_SPI_DESELECT();
    IRQ_DEC_ENABLE();

    return err;
}

S8 as3911ModifyRegister(U8 reg, U8 clr_mask, U8 set_mask)
{
    S8 err;
    U8 tmp;

    /* make this operation atomic */
    IRQ_INC_DISABLE();

    err = as3911ReadRegister(reg, &tmp);

    if (ERR_NONE == err)
    {
        /* mask out the bits we don't want to change */
        tmp &= ~clr_mask;
        /* set the new value */
        tmp |= set_mask;
        err = as3911WriteRegister(reg, tmp);
    }
    IRQ_DEC_ENABLE();

    return err;
}

S8 as3911WriteMultipleRegisters(U8 reg, const U8* values, U8 length)
{
    reg |= AS3911_WRITE_MODE;

    return as3911Write(reg, values, length);
}


S8 as3911WriteFifo(const U8* values, U8 length)
{
    S8 err;
    U8 cmd = AS3911_FIFO_LOAD;

    /* make this operation atomic */
    IRQ_INC_DISABLE();
    {
        AS3911_SPI_SELECT();
        {
            err = spiTxRx(&cmd, 1, NULL, 0);

            if (ERR_NONE == err)
            {
                err = spiTxRx(values, length, NULL, 0);
            }
        }
        AS3911_SPI_DESELECT();
    }
    IRQ_DEC_ENABLE();

    return err;
}

S8 as3911ReadFifo(U8* buf, U8 length)
{
    S8 err = ERR_NONE;
    U8 cmd = AS3911_FIFO_READ;

    if (length > 0)
    {
        /* make this operation atomic */
        IRQ_INC_DISABLE();
        AS3911_SPI_SELECT();
        err = spiTxRx(&cmd, 1, NULL, 0);
        if (ERR_NONE == err)
        {
            err = spiTxRx(buf, 0, buf, length);
        }
        AS3911_SPI_DESELECT();
        IRQ_DEC_ENABLE();
    }

    return err;
}

S8 as3911ExecuteCommand(U8 cmd)
{
    S8 err;
    cmd |= AS3911_CMD_MODE;

    IRQ_INC_DISABLE();
    AS3911_SPI_SELECT();
    err = spiTxRx(&cmd, 1, NULL, 0);
    AS3911_SPI_DESELECT();
    IRQ_DEC_ENABLE();

    return err;
}

S8 as3911TestPrefix(void)
{
    S8 err;
    U8 cmd = AS3911_CMD_TEST_ACCESS;
    AS3911_SPI_SELECT();
    err = spiTxRx(&cmd, 1, NULL, 0);

    return err;
}

S8 as3911EnterObsvMode(S8 mode)
{
    S8 err;
	U8 cmd = 0xfc;
	U8 values[5] = {0};

	values[0] = 0x01;
	values[1] = mode;

    /* make this operation atomic */
    IRQ_INC_DISABLE();
    {
        AS3911_SPI_SELECT();
        {
            err = spiTxRx(&cmd, 1, NULL, 0);

            if (ERR_NONE == err)
            {
                err = spiTxRx(values, 2, NULL, 0);
            }
        }
        AS3911_SPI_DESELECT();
    }
    IRQ_DEC_ENABLE();

    return err;
}

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/
static S8 as3911Write(U8 cmd, const U8* values, U8 length)
{
    S8 err;

    /* make this operation atomic */
    IRQ_INC_DISABLE();
    AS3911_SPI_SELECT();
    err = spiTxRx(&cmd, 1, NULL, 0);

    if (ERR_NONE == err)
    {
        err = spiTxRx(values, length, NULL, 0);
    }
    AS3911_SPI_DESELECT();
    IRQ_DEC_ENABLE();

    return err;
}

