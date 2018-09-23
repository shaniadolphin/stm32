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
 *  \brief Implementation of ISO-15693-2
 *
 */
/*!
 *
 */

#ifndef ISO_15693_2_H
#define ISO_15693_2_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "as3911_com.h"

/*
******************************************************************************
* GLOBAL DATATYPES
******************************************************************************
*/
/*!
 * enum holding possible VCD codings
 */
typedef enum
{
    ISO15693_VCD_CODING_1_4,
    ISO15693_VCD_CODING_1_256
}iso15693VcdCoding_t;

/*!
 * enum holding possible VICC datarates
 */

/*!
 * enum holding VICC modulation indeces
 */
typedef enum
{
    ISO15693_MODULATION_INDEX_10 = AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_10percent,
    ISO15693_MODULATION_INDEX_14 = AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_14percent,
    ISO15693_MODULATION_INDEX_20 = AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_20percent,
    ISO15693_MODULATION_INDEX_30 = AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_30percent,
    ISO15693_MODULATION_INDEX_OOK = 0x80
}iso15693ModulationIndex_t;

/*!
 * enum holding demodulation types
 */
typedef enum
{
    ISO15693_DEMODULATION_TYPE_AM = 0,
    ISO15693_DEMODULATION_TYPE_PM
}iso15693DemodulationType_t;

/*!
 * Configuration parameter used by #iso15693PhyInitialize.
 */
typedef struct
{
    iso15693VcdCoding_t coding; /*!< desired VCD coding */
    iso15693ModulationIndex_t mi; /*!< modulation index */
}iso15693PhyConfig_t;

/*
******************************************************************************
* GLOBAL CONSTANTS
******************************************************************************
*/

/* FIXME check the values for nrt and mrg */
/* t1min = 318,6us = 4320/fc = 67 * 64/fc */
#define ISO15693_MASK_RECEIVE_TIME (67)

/* t1max = 323,3us = 4384/fc = 68.5 * 64/fc */
#define ISO15693_NO_RESPONSE_TIME (69 + 5)


#define ISO15693_REQ_FLAG_TWO_SUBCARRIERS 0x01
#define ISO15693_REQ_FLAG_HIGH_DATARATE   0x02
/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
/*!
 *****************************************************************************
 *  \brief  Initialize the ISO15693 phy.
 *  \note This function needs to be called every time after switching
 *  from a different mode and before #iso15693Initialize
 *
 *  \param[in] config : ISO15693 phy related configuration (See #iso15693PhyConfig_t)
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693PhyInitialize(const iso15693PhyConfig_t* config);

/*!
 *****************************************************************************
 *  \brief  Deinitialize ISO15693 physical mode.
 *  \note This function should be called every time iso 15693 is not needed
 *  any more.
 *  \param keep_on: if true the RF field will not be switched off
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693PhyDeinitialize(U8 keep_on);

/*!
 *****************************************************************************
 *  \brief  Return current phy configuration.
 *
 *  This function returns current Phy configuration previously
 *  set by #iso15693PhyInitialize.
 *
 *  \param[out] config : ISO15693 phy configuration.
 *
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693PhyGetConfiguration(iso15693PhyConfig_t* config);

/*!
 *****************************************************************************
 *  \brief  Change the default no response time
 *
 *  To restore the default no response time call with parameter
 *  ISO15693_NO_RESPONSE_TIME.
 *
 *  \param[in] nrt_64fcs : the no response time in fc/64 = 4.72us
 *
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693PhySetNoResponseTime_64fcs(U32 nrt_64fcs);

/*!
 *****************************************************************************
 *  \brief  Send an ISO15693 compatible frame
 *
 *  This function takes \a length bytes from \a buffer, perform proper
 *  encoding and sends out the frame to the AS3911.
 *
 *  \param[in] buffer : data to send, modified to adapt flags.
 *  \param[in] length : number of bytes to send.
 *  \param[in] sendCrc : If set to TRUE, CRC is appended to the frame
 *  \param[in] sendFlags: If set to TRUE, flag field is sent according to
 *                        ISO15693.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693PhySendFrame(U8* buffer, U16 length, bool_t sendCrc, bool_t sendFlags);

/*!
 *****************************************************************************
 *  \brief  Receive an ISO15693 compatible frame
 *
 *  This function receives an ISO15693 frame from the AS3911, decodes the frame
 *  and writes the raw data to \a buffer.
 *  \note Buffer needs to be big enough to hold CRC also (+2 bytes)
 *
 *  \param[out] buffer : buffer where received data shall be written to.
 *  \param[in] length : number of bytes to receive (=length of buffer).
 *  \param[out] actLength : actual number of bytes received.
 *  \param[out] bitsBeforeCol : in case of ERR_COLLISION this value holds the
 *   number of bits in the current byte where the collision happened.
 *
 *  \return ERR_COLLISION : collision occured, data uncorrect
 *  \return ERR_CRC : CRC error, data uncorrect
 *  \return ERR_TIMEOUT : timeout waiting for data.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693PhyReceiveFrame(U8* buffer,
                                U16 length,
                                U16* actLength,
                                U8* bitsBeforeCol,
                                U16 ignoreBits);

/*!
 *****************************************************************************
 *  \brief  Send single EOF without any leading data
 *
 *  This function is used to send an EOF without any leading data. This is needed
 *  for ISO15693-3 anti collision sequence.
 *
 *  \return ERR_IO : error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693PhySendEof(void);

#endif /* ISO_15693_2_H */

