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
 *  \brief Implementation of ISO-15693-3
 *
 */

#ifndef ISO_15693_3_H
#define ISO_15693_3_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "iso15693_2.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
#define ISO15693_UID_LENGTH 8
#define ISO15693_MAX_MEMORY_BLOCK_SIZE 32

#define ISO15693_CMD_INVENTORY 1
#define ISO15693_CMD_STAY_QUIET 2
#define ISO15693_CMD_READ_SINGLE_BLOCK 0x20
#define ISO15693_CMD_WRITE_SINGLE_BLOCK 0x21
#define ISO15693_CMD_READ_MULTIPLE_BLOCKS 0x23
#define ISO15693_CMD_SELECT 0x25
#define ISO15693_CMD_GET_SYSTEM_INFORMATION 0x2B

#define ISO15693_REQ_FLAG_INVENTORY       0x04
#define ISO15693_REQ_FLAG_PROT_EXTENSION  0x08
/* with INVENTORY flag set we have these bits : */
#define ISO15693_REQ_FLAG_SELECT          0x10
#define ISO15693_REQ_FLAG_ADDRESS         0x20
/* with INVENTORY flag not set we have these bits : */
#define ISO15693_REQ_FLAG_AFI             0x10
#define ISO15693_REQ_FLAG_1_SLOT          0x20
/*! OPTION flag is the same for both: */
#define ISO15693_REQ_FLAG_OPTION          0x40

#define ISO15693_RESP_FLAG_ERROR          0x01
#define ISO15693_RESP_FLAG_PROT_EXTENSION 0x08
/*
******************************************************************************
* GLOBAL DATATYPES
******************************************************************************
*/
/*!
 * structure representing an ISO15693 PICC
 */
typedef struct
{
    U8 flags; /*!< flag byte of response */
    U8 dsfid; /*!< Data Storage Format Identifier */
    U8 uid[ISO15693_UID_LENGTH]; /*!< UID of the PICC */
    U16 crc; /*!< CRC of response */
	U8 rssi; /*!< rssi level*/
}iso15693ProximityCard_t;

/*!
 * structure holding the information returned by #iso15693GetPiccSystemInformation
 */
typedef struct
{
    U8 flags; /*!< flag byte of response */
    U8 infoFlags; /*!< info flags */
    U8 uid[ISO15693_UID_LENGTH]; /*!< UID of the PICC */
    U8 dsfid;  /*!< Data Storage Format Identifier */
    U8 afi; /*!< Application Family Identifier */
    U8 memNumBlocks; /*!< number of blocks available */
    U8 memBlockSize; /*!< number of bytes per block */
    U8 icReference; /*!< IC reference field */
}iso15693PiccSystemInformation_t;

/*!
 * structure representing a memory block
 * of an ISO15693 PICC
 */
typedef struct
{
    U8 flags;
    U8 errorCode;
    U8 securityStatus; /*< security status byte */
    U8 blocknr;
    U8 data[ISO15693_MAX_MEMORY_BLOCK_SIZE]; /*!< the content */
    U8 actualSize; /*!< actual size of \a data */
}iso15693PiccMemoryBlock_t;

/*!
 * enum holding possible slot count values used by inventory command.
 */
typedef enum
{
    ISO15693_NUM_SLOTS_1, /*!< 1 slot */
    ISO15693_NUM_SLOTS_16 /*!< 16 slots */
}iso15693NumSlots_t;

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
/*!
 *****************************************************************************
 *  \brief  Initialize ISO15693 mode.
 *  \note This function needs to be called every time after switching
 *  from a different mode.
 *
 *  \param[in] config : config to be used for iso15693_2
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693Initialize(const iso15693PhyConfig_t* config);

/*!
 *****************************************************************************
 *  \brief  Deinitialize ISO15693 mode.
 *  \note This function should be called every time iso 15693 is not needed
 *  any more.
 *  \param keep_on: if true the RF field will not be switched off
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693Deinitialize(U8 keep_on);

/*!
 *****************************************************************************
 *  \brief  Perform an ISO15693 inventory to return all PICCs in field.
 *
 *  This function performs an ISO15693 inventory which is used to return
 *  the UIDs of all PICCs in field.
 *  If \a maskLength is 0 all PICCs are addressed otherwise only
 *  PICCs with matched \a mask are returned. Using \a slotcnt the
 *  number of slots (1 or 16) can be chosen.
 *
 *  \param[in] slotcnt : Slotcount used (16 or 1)
 *  \param[in] maskLength : length of the mask if available (0 - 63)
 *  \param[in] mask : mask to use if \a maskLength is set, otherwise NULL
 *  \param[out] cards : buffer array where found card information is stored.
 *  \param[in] maxCards : maximum number of cards to return (= size of \a cards)
 *  \param[out] cardsFound : number of cards found and returned.
 *
 *  \return ERR_COLLISION : Collision which couldn't be resolved.
 *  \return ERR_NOTFOUND : No PICC could be selected.
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693Inventory(iso15693NumSlots_t slotcnt,
                    U8 maskLength,
                    U8* mask,
                    iso15693ProximityCard_t* cards,
                    U8 maxCards,
                    U8* cardsFound);

/*!
 *****************************************************************************
 *  \brief  Send command 'stay quiet' to given PICC.
 *
 *  Using this function the given \a card is send into quiet state so it does
 *  not respond to any inventory requests.
 *
 *  \param[in] card : PICC to be sent into quiet state.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 iso15693SendStayQuiet(const iso15693ProximityCard_t* card);

/*!
 *****************************************************************************
 *  \brief  Send command 'Select' to select a PICC for non-addressed mode.
 *
 *  Using this function the given \a card is selected for non-addressed mode.
 *
 *  \param[in] card : PICC to select.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NOTFOUND : Requested PICC could not be selected. (Not in field)
 *  \return ERR_CRC : CRC error.
 *  \return ERR_COLLISION : Collision happened.
 *  \return ERR_NONE : No error, PICC selected.
 *
 *****************************************************************************
 */
extern S8 iso15693SelectPicc(const iso15693ProximityCard_t* card);

/*!
 *****************************************************************************
 *  \brief  Send command 'get system information' to retrieve information
 *  from a given or selected PICC.
 *
 *  Using this function the system information value from a given or
 *  selected PICC is retrieved. If \a card is NULL then the PICC
 *  needs to be selected using #iso15693SelectPicc prior to calling
 *  this function. If \a card is not NULL then this value is treated
 *  as the PICCs UID.
 *
 *  \param[in] card : PICC to retrieve the information from.
 *                    If card is NULL then this parameter
 *                    is ignored and the information is fetched from the PICC
 *                    priorly selected with #iso15693SelectPicc
 *  \param[out] sysInfo : buffer of type #iso15693PiccSystemInformation_t
 *                      where the answer will be stored.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NOTFOUND : Requested PICC not available. (Not in field)
 *  \return ERR_NOTSUPP : Request not supported by PICC.
 *  \return ERR_CRC : CRC error.
 *  \return ERR_NOMEM : Not enough memory to perform this function.
 *  \return ERR_COLLISION : Collision happened.
 *  \return ERR_NONE : No error, respone written to \a sysInfo.
 *
 *****************************************************************************
 */
extern S8 iso15693GetPiccSystemInformation(const iso15693ProximityCard_t* card,
                                iso15693PiccSystemInformation_t* sysInfo);

/*!
 *****************************************************************************
 *  \brief  Read a single block from a given or selected PICC.
 *
 *  This function reads out a single block from a given or selected PICC.
 *  If \a card is NULL then the PICC
 *  needs to be selected using #iso15693SelectPicc prior to calling
 *  this function. If \a card is not NULL then this value is treated
 *  as the PICCs UID.
 *
 *  \param[in] card : PICC to read the block from.
 *                    If \a card is NULL then this parameter
 *                    is ignored and the information is fetched from the PICC
 *                    priorly selected with #iso15693SelectPicc
 *  \param[out] memBlock : buffer of type #iso15693PiccMemoryBlock_t
 *                      where the block and its size will be stored.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NOTFOUND : Requested PICC not available. (Not in field)
 *  \return ERR_NOTSUPP : Request not supported by PICC.
 *  \return ERR_CRC : CRC error.
 *  \return ERR_NOMEM : Not enough memory to perform this function.
 *  \return ERR_COLLISION : Collision happened.
 *  \return ERR_NONE : No error, block read out and written to \a memBlock.
 *
 *****************************************************************************
 */
extern S8 iso15693ReadSingleBlock(const iso15693ProximityCard_t* card,
                                iso15693PiccMemoryBlock_t* memBlock);

/*!
 *****************************************************************************
 *  \brief  Read a multiple blocks from a given or selected PICC.
 *
 *  This function reads out \a count blocks from a given or selected PICC
 *  starting at block \a startblock. If \a card is NULL then the PICC
 *  needs to be selected using #iso15693SelectPicc prior to calling
 *  this function. If \a card is not NULL then this value is treated
 *  as the PICCs UID.
 *
 *  \param[in] card : PICC to read the block from.
 *                    If \a card is NULL then this parameter
 *                    is ignored and the information is fetched from the PICC
 *                    priorly selected with #iso15693SelectPicc
 *  \param[in] startblock : number of the first block to read out
 *  \param[in] count : number of blocks to read out.
 *  \param[out] memBlock : array #iso15693PiccMemoryBlock_t with size \a count
 *                      where the blocks and their size will be stored.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NOTFOUND : Requested PICC not available. (Not in field)
 *  \return ERR_NOTSUPP : Request not supported by PICC.
 *  \return ERR_CRC : CRC error.
 *  \return ERR_NOMEM : Not enough memory to perform this function.
 *  \return ERR_COLLISION : Collision happened.
 *  \return ERR_NONE : No error, block read out and written to \a memBlock.
 *
 *****************************************************************************
 */
extern S8 iso15693ReadMultipleBlocks(const iso15693ProximityCard_t* card,
                                U8 startblock,
                                U8 count,
                                iso15693PiccMemoryBlock_t memBlock[]);

/*!
 *****************************************************************************
 *  \brief  Write a single block of a given or selected PICC.
 *
 *  This function writes a single block from a given or selected PICC.
 *  If \a card is NULL then the PICC
 *  needs to be selected using #iso15693SelectPicc prior to calling
 *  this function. If \a card is not NULL then this value is treated
 *  as the PICCs UID.
 *
 *  \param[in] card : PICC whose block should be written.
 *                    If \a card is NULL then this parameter
 *                    is ignored and the information is fetched from the PICC
 *                    priorly selected with #iso15693SelectPicc
 *  \param[in] flags : flags to be sent to the card. The bit
 *                     #ISO15693_REQ_FLAG_OPTION specifies if the response
 *                     must be polled separately.
 *                     This is required by some cards (e.g. from TI) to work
 *                     other cards refuse write command if option flag is set.
 *  \param[out] memBlock : buffer of type #iso15693PiccMemoryBlock_t
 *                      containing the block data to write.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NOTFOUND : Requested PICC not available. (Not in field)
 *  \return ERR_NOTSUPP : Request not supported by PICC.
 *  \return ERR_CRC : CRC error.
 *  \return ERR_NOMEM : Not enough memory to perform this function.
 *  \return ERR_COLLISION : Collision happened.
 *  \return ERR_NONE : No error, block read out and written to \a memBlock.
 *
 *****************************************************************************
 */
extern S8 iso15693WriteSingleBlock(const iso15693ProximityCard_t* card,
                                U8 flags,
                                iso15693PiccMemoryBlock_t* memBlock);

/*!
 *****************************************************************************
 *  \brief  Generic command to write and read arbitrary byte arrays
 *
 *  This function writes a the given buffer and receives an answer.
 *
 *  \param[in] txBuf
 *  \param[in] txLen
 *  \param[out] rxBuf
 *  \param[in] rxLen
 *  \param[out] actlength : return the actual received length inside rxBuf
 *  \param[in] response_wait_ms : number of ms to wait before sending another
 *                                EOF and receive the answer afterwards
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NOTFOUND : Requested PICC not available. (Not in field)
 *  \return ERR_NOTSUPP : Request not supported by PICC.
 *  \return ERR_CRC : CRC error.
 *  \return ERR_NOMEM : Not enough memory to perform this function.
 *  \return ERR_COLLISION : Collision happened.
 *  \return ERR_NONE : No error, block read out and written to \a memBlock.
 *
 *****************************************************************************
 */
extern S8 iso15693TxRxNBytes(
                U8* txBuf,
                U8 txLen,
                U8* rxBuf,
                U16 rxLen,
                U16* actlength,
                U16 response_wait_ms);
#endif /* ISO_15693_3_H */

