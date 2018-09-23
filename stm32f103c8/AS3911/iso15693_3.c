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
 *      $Revision: 1.10 $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author Christian Eisendle
 *
 *  \brief Implementation of ISO-15693-3
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "nfc.h"
#include "platform.h"
#include "utils.h"
#include "iso15693_2.h"
#include "iso15693_3.h"
#include "delay.h"
#include "as3911.h"
#include "as3911_interrupt.h"

#include "..\cm3\stm32f10x.h"

/*
******************************************************************************
* LOCAL MACROS
******************************************************************************
*/

//#define ISO_15693_DEBUG dbgLog
#define ISO_15693_DEBUG(x...)
//printk( x)

/*
******************************************************************************
* LOCAL DEFINES
******************************************************************************
*/
#define ISO15693_NUM_UID_BITS 64 /*!< number of UID bits */

#define ISO15693_BUFFER_SIZE 64 /*!< length of iso15693 general purpose buffer */
/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/
static U8 iso15693Buffer[ISO15693_BUFFER_SIZE+2]; /*!< general
                                        purpose buffer. 2 extra bytes are needed for crc*/
static U8 iso15693DirMarker[ISO15693_NUM_UID_BITS]; /*!< direction marker
                                used during inventory for binary tree search.
                                values: 0, 1, 2. 0 means no collision at current
                                bit position. 1 means that left path is tried and
                                2 means right path is tried */
static U8 iso15693DefaultSendFlags = 0; /*!< default flags used for iso15693SendRequest */

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static S8 iso15693SendRequest(U8 cmd,
                U8 flags,
                const iso15693ProximityCard_t* card,
                U8* rcvbuffer,
                U16 rcvbuffer_len,
                U16* actlength,
                U8* addSendData,
                U8 addSendDataLength);
/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
S8 iso15693Initialize(const iso15693PhyConfig_t* config)
{
    ISO_15693_DEBUG(__func__);
    return iso15693PhyInitialize(config);
}

S8 iso15693Deinitialize(U8 keep_on)
{
    ISO_15693_DEBUG(__func__);
    return iso15693PhyDeinitialize(keep_on);
}

S8 iso15693Inventory(iso15693NumSlots_t slotcnt,
                    U8 maskLength,
                    U8* mask,
                    iso15693ProximityCard_t* cards,
                    U8 maxCards,
                    U8* cardsFound)
{
    S8 err; /* error variable */
    U16 i; /* count variable */
    U8 maskLengthBytes; /* length of the mask in full bytes */
    U8 bitmask;  /* temporary variable for holding bitmasks */
    U16 actlength; /* actual number of bytes received */
    U8 currColPos; /* bitposition of the currently processed collision within 64bit UID */
    U8 bitsBeforeCol; /* number of bits received within last byte before collision */
    S16 slot; /* number of the slot currently being processed */
    U8 slotNumPos; /* gives the position of the slot number within the mask */
    S16 currColSlot; /* index of the slot whose collision is currently processed */
    U16 colSlots; /* 16 bit long marker holding all slots with collisions (indicated by set bit */
    U64 collisions; /* 64 bit long marker holding all unresolved collisions within 64bit UID */
    iso15693ProximityCard_t* crdptr = cards; /* pointer to the card currently used */
    U8 crdidx = 0; /* index of the card currently used */
    iso15693PhyConfig_t cfg; /* phy configuration */
    U8 timeout;
	U8 result;

    /* retrieve phy configuration */
    iso15693PhyGetConfiguration(&cfg);
    /* depending on set modulation index and datarate, there has to be a
       timeout before sending EOF for slot count = 16! */

    if (cfg.mi != ISO15693_MODULATION_INDEX_OOK)
    {
        timeout = 5;
    }
    else
    {
        timeout = 1;
    }

    if (maxCards == 0)
    {
        err = ERR_NOMEM;
        goto out;
    }

    AMS_MEMSET(iso15693DirMarker, 0, ISO15693_NUM_UID_BITS);

    /* set inventory flag */
    iso15693Buffer[0] = ISO15693_REQ_FLAG_INVENTORY;

    /* if number of slot is 1 then set the appropriate flag */
    if (ISO15693_NUM_SLOTS_1 == slotcnt)
    {
        iso15693Buffer[0] |= ISO15693_REQ_FLAG_1_SLOT;
    }
    iso15693Buffer[1] = ISO15693_CMD_INVENTORY;
    iso15693Buffer[2] = maskLength;

    /* convert maskLength from number of bits to bytes */
    maskLengthBytes = (maskLength + 7) >> 3; //(maskLength >> 3) + (((maskLength & 7) > 0) ? 1 : 0);
    if ((maskLengthBytes + 3) > ISO15693_BUFFER_SIZE)
    {
        err = ERR_NOMEM;
        goto out;
    }

    ISO_15693_DEBUG("maskLengthBytes: 0x%x\r\n", maskLengthBytes);

    /* mask out unneeded bits */
    if (maskLength & 7)
    {
        bitmask = (1 << (maskLength & 7)) - 1;
        mask[maskLengthBytes-1] &= bitmask;
    }
    AMS_MEMCPY(&iso15693Buffer[3], mask, maskLengthBytes);

    slotNumPos = maskLength & 7;
    currColPos = 0;
    collisions = 0;
    colSlots = 0;
    currColSlot = -1;
    slot = (slotcnt == ISO15693_NUM_SLOTS_1) ? -1 : 15;
    do
    {
        /* this outer loop iterates as long as there are unresolved
           collisions (including slots with unresolved collisions */
        do
        {
            /* inner loop iterates once over all slots (in case of slot count 16)
               or only one iteration in case of slot count 1.
               After first 16 slot iterations (slot count 16) slot count is
               changed to 1 */

            if ((slotcnt == ISO15693_NUM_SLOTS_1) || (slot == 15))
            {
                ISO_15693_DEBUG("Raw data: ");
                for (i = 0; i < (3 + maskLengthBytes); i++)
                {
                    ISO_15693_DEBUG("0x%x ", iso15693Buffer[i]);
                }
                ISO_15693_DEBUG("\r\n");

                /* send the request. Note: CRC is appended by physical layer.
                   Add 3 to mask for flag field, command field and mask length */
                iso15693PhySendFrame(iso15693Buffer, 3 + maskLengthBytes, TRUE, TRUE);
            }
            else
            {
                /* in case if slot count 16 slot is incremented by just sending EOF */
                //delayNMilliSeconds(timeout);
                delayNMicroSeconds(50);
                iso15693PhySendEof();
                ISO_15693_DEBUG("slot: 0x%x\r\n", 15 - slot);
            }

            /* receive data ignoring collisions in first 2 bytes */
            err = iso15693PhyReceiveFrame((U8*)crdptr,
                        sizeof(iso15693ProximityCard_t),
                        &actlength,
                        &bitsBeforeCol,
                        16);
			as3911ReadRegister(AS3911_REG_RSSI_RESULT, &result);
			crdptr->rssi = result>>4;
			if (ERR_NOTFOUND == err && 3 <= crdptr->rssi)
			{
				err = ERR_COLLISION;
				actlength = maskLengthBytes+2;
			}
			#ifdef ERR_SIGNAL
			if ( ERR_SIGNAL == err )
			{
				err = ERR_COLLISION;
				actlength = maskLengthBytes+2;
			}
			#endif
            if (ERR_COLLISION == err)
            {
                if (actlength < 2)
                {
                    /* collision before actually receiving UID!!! This should
                    not happen since we ignored collisions in these bytes. */
                    ISO_15693_DEBUG("out slot=%d\r\n ", slot);
					#ifdef ERR_SIGNAL
					if ( ISO15693_NUM_SLOTS_1 == slotcnt )
					{
						//goto out;
					}
					#else
                    err = ERR_COLLISION;
                    goto out;
					#endif
                }
                if ((actlength - 2 + (bitsBeforeCol ? 1 : 0)) < maskLengthBytes)
                {
                    /* we provided a mask but received fewer bytes which is actually
                       impossible. exit at this point with collision error */
                    ISO_15693_DEBUG("acl: 0x%x ", actlength);
                    ISO_15693_DEBUG("mlb: 0x%x ", maskLengthBytes);
                    ISO_15693_DEBUG("bbc: 0x%x\r\n", bitsBeforeCol);
					#ifdef ERR_SIGNAL
					if ( ISO15693_NUM_SLOTS_1 == slotcnt )
					{
						//goto out;
					}
					#else
                    err = ERR_COLLISION;
                    goto out;
					#endif
                }

                if (ISO15693_NUM_SLOTS_1 == slotcnt)
                {
                    /* in case slot count is 1 collision needs to be resolved */
                    ISO_15693_DEBUG("bbc: 0x%x\r\n", bitsBeforeCol);
                    ISO_15693_DEBUG("al: 0x%x\r\n", actlength);

                    /* find position of collision within received UID and
                       update mask and mask length appropriately */
                    iso15693Buffer[2] = ((actlength - 2) << 3) + bitsBeforeCol + 1;
                    if (iso15693Buffer[2] > ISO15693_NUM_UID_BITS)
                    {
                        iso15693Buffer[2] = ISO15693_NUM_UID_BITS;
                        actlength = 9;
                    }
                    currColPos = iso15693Buffer[2] - 1;
                    collisions |= ((U64)1 << (U64)currColPos);
                    maskLengthBytes = actlength - 1;

                    /* copy received UID to mask */
                    AMS_MEMCPY(&iso15693Buffer[3], crdptr->uid, maskLengthBytes);
                    bitmask = (1 << bitsBeforeCol) - 1;

                    /* clear bit where collision happened which means try
                       left branch of the tree first */
                    iso15693Buffer[2+maskLengthBytes] &= bitmask;

                    if (1 == iso15693DirMarker[currColPos])
                    {
                        /* if left branch has been tried out before (dirMarker set to 1)
                           the set the bit where collision happened to 1, i.e.
                           try right branch */
                        iso15693Buffer[2+maskLengthBytes] |= (1 << (currColPos & 7));
                    }
                    /* in any case increment dirMarker to indicate the way we chose */
                    iso15693DirMarker[currColPos]++;
                }
                else
                {
                    /* in case of slot count 16 just mark that there is a collision
                    within this slot. Resolve it later when switching back to slot count 1 */
                    colSlots |= 1 << (15 - slot);
                }
            }
            else
            {
                if (ERR_NONE == err)
                {
                    /* received all bytes without collision - store UID */
                    ISO_15693_DEBUG("store UID\r\n");
					if ( 2 < crdptr->rssi )
					{
						crdidx++;
						if (crdidx >= maxCards)
						{
							ISO_15693_DEBUG("max cards reached\r\n");
							goto out_max_cards;
						}
						crdptr++;
					}
                }
                if (ISO15693_NUM_SLOTS_1 == slotcnt)
                {

                    i = ISO15693_NUM_UID_BITS;
                    /* a collisions has been resolved. Go back in the tree to find
                     next collision */
                    while (i--)
                    {
                        if (collisions & ((U64)1 << (U64)i))
                        {
                            if (iso15693DirMarker[i] > 1)
                            {
                                /* dirMarker 2 means that both paths (left 'n right)
                                   have been tried (=resolved). Remove this collision */
                                ISO_15693_DEBUG("remove col: 0x%x\r\n", i);
                                collisions &= ~((U64)((U64)1 << (U64)i));
                                iso15693DirMarker[i] = 0;
                                if (currColSlot >= 0)
                                {
                                    /* if this collision was within a slot unmark
                                       also this slot */
                                    colSlots &= ~(1 << currColSlot);
                                    currColSlot = -1;
                                }
                            }
                            else
                            {
                                ISO_15693_DEBUG("next col: 0x%x\r\n", i);
                                /* update collision position. dirMarker 1
                                also means that left branch was tried before.
                                Switch to right branch now */
                                currColPos = i;
                                iso15693Buffer[2] = currColPos + 1;
                                maskLengthBytes = (currColPos >> 3) + 1;
                                iso15693Buffer[2+maskLengthBytes] |= (1 << (currColPos & 7));
                                iso15693DirMarker[currColPos]++;
                                break;
                            }
                        }
                    }

                    if ((currColSlot >= 0) && (iso15693DirMarker[currColPos] == 0))
                    {
                        /* a slot where a collision was found before has been processed
                           with no collision. So unmark this slot */
                        colSlots &= ~(1 << currColSlot);
                        currColSlot = -1;
                    }
                }
            }

            if (slot >= 0)
            {
                slot--;
            }
        } while (slot >= 0);

        /* after 16 iterations switch back to slot count 0 which means
         a normal binary tree search */
        if (ISO15693_NUM_SLOTS_16 == slotcnt)
        {
            slotcnt = ISO15693_NUM_SLOTS_1;
            iso15693Buffer[0] |= ISO15693_REQ_FLAG_1_SLOT;
        }
        if (!collisions && (ISO15693_NUM_SLOTS_1 == slotcnt))
        {
            /* if all collisions are resolved check the slots for open collisions */
            for (i = 0; i < 16; i++)
            {
                if ((1 << i) & colSlots)
                {
                    /* found a slot with unresolved collision.
                       Reset mask length to original value and append slot number to mask */
                    maskLengthBytes = (maskLength >> 3) + (((maskLength & 7) > 0) ? 1 : 0);
                    if (slotNumPos == 0)
                    {
                        /* add an additional byte in case slot number starts at LSB */
                        maskLengthBytes++;
                    }
                    if (slotNumPos > 4)
                    {
                        /* also if slot number would overlap add an additional byte */
                        maskLengthBytes++;
                        /* add slot number to mask */
                        iso15693Buffer[2+maskLengthBytes] &= ~((1 << (8 - slotNumPos)) - 1);
                        iso15693Buffer[2+maskLengthBytes] |= i >> (8 - slotNumPos);
                        iso15693Buffer[1+maskLengthBytes] &= (1 << slotNumPos) - 1;
                        iso15693Buffer[1+maskLengthBytes] |= (i << slotNumPos);
                    }
                    else
                    {
                        /* add slot number to mask */
                        iso15693Buffer[2+maskLengthBytes] &= (1 << slotNumPos) - 1;
                        iso15693Buffer[2+maskLengthBytes] |= (i << slotNumPos);
                    }
                    /* in any case number of mask bits needs to be incremented by 4 */
                    iso15693Buffer[2] = maskLength + 4;
                    ISO_15693_DEBUG("checking col slot 0x%x\r\n", i);
                    currColSlot = i;
                    break;
                }
            }

        }

        /* do not stop before all collisions in all slots are resolved */
    } while (collisions || colSlots);

out_max_cards:
    err = ERR_NONE;
out:
    *cardsFound = crdidx;

    if (*cardsFound == 0)
    {
        err = ERR_NOTFOUND;
    }

    return err;
}

S8 iso15693SendStayQuiet(const iso15693ProximityCard_t* card)
{
	S8 err;
    U32 mask;
    U16 actlength;

    /* just send the command - no reply sent by the PICC */
    err = iso15693SendRequest(ISO15693_CMD_STAY_QUIET,
            iso15693DefaultSendFlags,
            card,
            NULL,
            0,
            &actlength,
            NULL,
            0);

#if 1
	mask = as3911WaitForInterruptsTimed(AS3911_IRQ_MASK_TXE, 100);// 100
	if (0 == mask)
	{
		err = ERR_TIMEOUT;
	}
#endif
	return err;
}

S8 iso15693SelectPicc(const iso15693ProximityCard_t* card)
{
    U16 actlength;
    S8 err = 0;

    //err = iso15693SendRequest(ISO15693_CMD_SELECT,
    iso15693SendRequest(ISO15693_CMD_SELECT,
            iso15693DefaultSendFlags,
            card,
            iso15693Buffer,
            4,
            &actlength,
            NULL,
            0);
    //EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    if (iso15693Buffer[0] != 0)
    {
        err = ERR_NOMSG;
    }

//out:
    return err;
}

S8 iso15693GetPiccSystemInformation(const iso15693ProximityCard_t* card,
                                iso15693PiccSystemInformation_t* sysInfo)
{
    S8 err;
    U16 actlength;
    U8 offset = 0;

    err = iso15693SendRequest(ISO15693_CMD_GET_SYSTEM_INFORMATION,
            iso15693DefaultSendFlags,
            card,
            iso15693Buffer,
            17,
            &actlength,
            NULL,
            0);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    sysInfo->dsfid = 0;
    sysInfo->afi = 0;
    sysInfo->memNumBlocks = 0;
    sysInfo->memBlockSize = 0;
    sysInfo->icReference = 0;
    /* process data */
    if (actlength > 4)
    {
        /* copy first 10 bytes which are fixed */
        AMS_MEMCPY((U8*)sysInfo, iso15693Buffer, 10);
        /* evaluate infoFlags field */
        if (sysInfo->infoFlags & 0x1)
        {
            /* dsfid field present */
            sysInfo->dsfid = iso15693Buffer[10];
            offset++;
        }
        if (sysInfo->infoFlags & 0x2)
        {
            /* afi field present */
            sysInfo->afi = iso15693Buffer[10+offset];
            offset++;
        }
        if (sysInfo->infoFlags & 0x4)
        {
            /* memory size field present */
            sysInfo->memNumBlocks = iso15693Buffer[10+offset];
            sysInfo->memBlockSize = iso15693Buffer[11+offset];
            offset += 2;
        }
        if (sysInfo->infoFlags & 0x8)
        {
            /* ic reference field present */
            sysInfo->icReference = iso15693Buffer[10+offset];
        }
    }
    else
    {
        /* error field set */
        err = ERR_NOTSUPP;
    }

out:
    return err;
}

S8 iso15693ReadSingleBlock(const iso15693ProximityCard_t* card,
                    iso15693PiccMemoryBlock_t* memBlock)
{
    S8 err;
    U16 actlength;

    err = iso15693SendRequest(ISO15693_CMD_READ_SINGLE_BLOCK,
            iso15693DefaultSendFlags,
            card,
            iso15693Buffer,
            ISO15693_BUFFER_SIZE,
            &actlength,
            &memBlock->blocknr,
            1);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    if (actlength >= 2)
    {
        memBlock->flags = iso15693Buffer[0];
        if (memBlock->flags & ISO15693_RESP_FLAG_ERROR)
        {
            memBlock->errorCode = iso15693Buffer[1];
        }
        else
        {
            memBlock->actualSize = actlength - 1;
            memBlock->errorCode = 0;
            AMS_MEMCPY(memBlock->data, &iso15693Buffer[1], memBlock->actualSize);
        }
    }

out:
    return err;
}

S8 iso15693ReadMultipleBlocks(const iso15693ProximityCard_t* card,
                                U8 startblock,
                                U8 count,
                                iso15693PiccMemoryBlock_t memBlock[])
{
    S8 err;
    U16 actlength;
    U16 i;
    U8* ptr;
    U8 addData[2];
    iso15693PiccMemoryBlock_t* memBlockPtr = memBlock;

    addData[0] = startblock;
    addData[1] = count - 1;

    err = iso15693SendRequest(ISO15693_CMD_READ_MULTIPLE_BLOCKS,
            iso15693DefaultSendFlags,
            card,
            iso15693Buffer,
            ISO15693_BUFFER_SIZE,
            &actlength,
            addData,
            2);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    /* increment pointer by one since first byte is flags byte */
    ptr = iso15693Buffer + 1;
    if (actlength > 4)
    {
        for (i = 0; i < count; i++)
        {
            memBlockPtr->actualSize = (actlength - 1) / count;
            AMS_MEMCPY(memBlockPtr->data,
                    ptr,
                    memBlockPtr->actualSize);
            /* increment by data block length */
            ptr += memBlockPtr->actualSize;
            memBlockPtr++;
        }
    }
    else
    {
        err = ERR_NOTSUPP;
    }

out:
    return err;

}

S8 iso15693WriteSingleBlock(const iso15693ProximityCard_t* card,
                                U8 flags,
                                iso15693PiccMemoryBlock_t* memBlock)
{
    S8 err;
    U16 actlength;
    U8 notused;

    if (!(flags & ISO15693_REQ_FLAG_OPTION))
    {
        iso15693PhySetNoResponseTime_64fcs(4238); /* ~ 20ms */
    }
    /* just send the request and wait separately for the answer */
    err = iso15693SendRequest(ISO15693_CMD_WRITE_SINGLE_BLOCK,
            flags,
            card,
            NULL,
            0,
            &actlength,
            (U8*)&memBlock->blocknr,
            memBlock->actualSize+1);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    if (flags & ISO15693_REQ_FLAG_OPTION)
    {
        /* according to the standard wait 20ms before sending EOF */
        delayNMilliSeconds(5);
        /* send EOF */
        err = iso15693PhySendEof();
        EVAL_ERR_NE_GOTO(ERR_NONE, err, out);
    }

    err = iso15693PhyReceiveFrame(iso15693Buffer,
                4,
                &actlength,
                &notused,
                0);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    if (actlength > 3)
    {
        err = ERR_NOTSUPP;
    }

out:
    if (!(flags & ISO15693_REQ_FLAG_OPTION))
        iso15693PhySetNoResponseTime_64fcs(ISO15693_NO_RESPONSE_TIME);
    return err;
}

S8 iso15693TxRxNBytes(
                U8* txBuf,
                U8 txLen,
                U8* rxBuf,
                U16 rxLen,
                U16* actlength,
                U16 response_wait_ms)
{
    U8 tmp;
    S8 err;
    U8 flags = *txBuf;

    if (!(flags & ISO15693_REQ_FLAG_OPTION))
    {
        //iso15693PhySetNoResponseTime_64fcs(4238); /* ~ 20ms */
        iso15693PhySetNoResponseTime_64fcs(6357); /* ~ 30ms needed for SL13A */
    }
    /* send the data */
    err = iso15693PhySendFrame(txBuf, txLen, TRUE, TRUE);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    if (rxLen > 0)
    {
        if (response_wait_ms)
        {
            if (flags & ISO15693_REQ_FLAG_OPTION)
            {
                /* according to the standard wait 20ms before sending EOF */
                delayNMilliSeconds(response_wait_ms);
                /* send EOF */
                err = iso15693PhySendEof();
                EVAL_ERR_NE_GOTO(ERR_NONE, err, out);
            }
        }
        err = iso15693PhyReceiveFrame(rxBuf,
                rxLen,
                actlength,
                &tmp,
                0);
    }

out:
    if (!(flags & ISO15693_REQ_FLAG_OPTION))
        iso15693PhySetNoResponseTime_64fcs(ISO15693_NO_RESPONSE_TIME);
    return err;
}

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/
static S8 iso15693SendRequest(U8 cmd,
                U8 flags,
                const iso15693ProximityCard_t* card,
                U8* rcvbuffer,
                U16 rcvbuffer_len,
                U16* actlength,
                U8* addSendData,
                U8 addSendDataLength)
{
    U8 tmp;
    S8 err;
    U8 length;

    iso15693Buffer[0] = flags;

    /* sanity checks first */
    if ((ISO15693_UID_LENGTH + addSendDataLength + 2) > ISO15693_BUFFER_SIZE)
    {
        return ERR_NOMEM;
    }

    if (card == NULL)
    {
        /* uid is NULL which means that selected PICC (using #iso15693SelectPicc)
           is used */
        /* set select flag */
        iso15693Buffer[0] |= ISO15693_REQ_FLAG_SELECT;
        length = 2;
    }
    else
    {
        /* set address flag */
        iso15693Buffer[0] |= ISO15693_REQ_FLAG_ADDRESS;
        /* copy UID */
        AMS_MEMCPY((void*)&iso15693Buffer[2], (void*)card->uid, ISO15693_UID_LENGTH);
        length = 2 + ISO15693_UID_LENGTH;
    }

    iso15693Buffer[1] = cmd;

    /* append additional data to be sent */
    AMS_MEMCPY(&iso15693Buffer[length], addSendData, addSendDataLength);
    length += addSendDataLength;

    /* send the data */
    err = iso15693PhySendFrame(iso15693Buffer, length, TRUE, TRUE);
    EVAL_ERR_NE_GOTO(ERR_NONE, err, out);

    if (rcvbuffer_len > 0)
    {
        err = iso15693PhyReceiveFrame(rcvbuffer,
                rcvbuffer_len,
                actlength,
                &tmp,
                0);
    }

out:
    return err;
}

S8 iso15693_count_a_tag_with_uid(const iso15693ProximityCard_t* card)
{
	S8 err = ERR_NONE;
	if (card->uid[6] == 0x04)
	{
		err = iso15693SelectPicc(card);
		if ( ERR_NONE != err )
		{
			err = iso15693SelectPicc(card);
		}
	}
	else
	{
		U8 addsenddata = 0;
		U16 actlength;
		iso15693SendRequest(ISO15693_CMD_READ_SINGLE_BLOCK, 0, card,
							iso15693Buffer, 4, &actlength, &addsenddata, 1);
		if ( iso15693Buffer[0] != 0 )
		{
			iso15693SendRequest(ISO15693_CMD_READ_SINGLE_BLOCK, 0, card,
								iso15693Buffer, 4, &actlength, &addsenddata, 1);
			if ( iso15693Buffer[0] != 0 )
			{
				err = ERR_NOMSG;
			}
		}
	}
	if ( ERR_NONE != err )
	{
		return 0;// ²»´æÔÚ
	}
	else
	{
		return 1;// ´æÔÚ
	}
}


