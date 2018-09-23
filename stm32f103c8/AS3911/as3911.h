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
 *  \brief AS3911 declaration file
 *
 */
/*!
 *
 *
 * API:
 * - Initialize AS3911 driver: #as3911Initialize
 * - Deinitialize AS3911 driver: #as3911Deinitialize
 */

#ifndef AS3911_H
#define AS3911_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"

/*
******************************************************************************
* GLOBAL DATATYPES
******************************************************************************
*/
/*!
 * flags used for #as3911TxNBytes
 */
typedef enum
{
    AS3911_TX_FLAG_NONE = 0x0,  /*!< nomen est omen */
    AS3911_TX_FLAG_CRC = 0x1,  /*!< append crc sum to frame */
    AS3911_TX_FLAG_ANTCL = 0x2, /*!< send anticollision frame for 14443a*/
    AS3911_TX_FLAG_NFC_INITIATOR = 0x4, /*!< frame sent with initial collision avoidance */
    AS3911_TX_FLAG_NFC_TARGET = 0x8, /*!< frame sent with response collision avoidance */
}as3911TxFlag_t;

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
#define MS_TO_64FCS(A)  ((A) * 212)
#define MS_FROM_64FCS(A) ((A) / 212)

/* AS3911 direct commands */
#define AS3911_CMD_SET_DEFAULT              0xC1    /*!< Puts the chip in default state (same as after power-up) */
#define AS3911_CMD_CLEAR_FIFO               0xC2    /*!< Stops all activities and clears FIFO */
#define AS3911_CMD_TRANSMIT_WITH_CRC        0xC4    /*!< Transmit with CRC */
#define AS3911_CMD_TRANSMIT_WITHOUT_CRC     0xC5    /*!< Transmit without CRC */
#define AS3911_CMD_TRANSMIT_REQA            0xC6    /*!< Transmit REQA */
#define AS3911_CMD_TRANSMIT_WUPA            0xC7    /*!< Transmit WUPA */
#define AS3911_CMD_INITIAL_RF_COLLISION     0xC8    /*!< NFC transmit with Initial RF Collision Avoidance */
#define AS3911_CMD_RESPONSE_RF_COLLISION_N  0xC9    /*!< NFC transmit with Response RF Collision Avoidance */
#define AS3911_CMD_RESPONSE_RF_COLLISION_0  0xCA    /*!< NFC transmit with Response RF Collision Avoidance with n=0 */
#define AS3911_CMD_NORMAL_NFC_MODE          0xCB    /*!< NFC switch to normal NFC mode */
#define AS3911_CMD_ANALOG_PRESET            0xCC    /*!< Analog Preset */
#define AS3911_CMD_MASK_RECEIVE_DATA        0xD0    /*!< Mask recive data */
#define AS3911_CMD_UNMASK_RECEIVE_DATA      0xD1    /*!< Unmask recive data */
#define AS3911_CMD_MEASURE_AMPLITUDE        0xD3    /*!< Measure singal amplitude on RFI inputs */
#define AS3911_CMD_SQUELCH                  0xD4    /*!< Squelch */
#define AS3911_CMD_CLEAR_SQUELCH            0xD5    /*!< Clear Squelch */
#define AS3911_CMD_ADJUST_REGULATORS        0xD6    /*!< Adjust regulators */
#define AS3911_CMD_CALIBRATE_MODULATION     0xD7    /*!< Calibrate modulation depth */
#define AS3911_CMD_CALIBRATE_ANTENNA        0xD8    /*!< Calibrate antenna */
#define AS3911_CMD_MEASURE_PHASE            0xD9    /*!< Measure phase between RFO and RFI signal */
#define AS3911_CMD_MEASURE_RSSI             0xDA    /*!< clear RSSI bits and restart the measurement */
#define AS3911_CMD_TRANSPARENT_MODE         0xDC    /*!< Transparent mode */
#define AS3911_CMD_CALIBRATE_C_SENSOR       0xDD    /*!< Calibrate the capacitive sensor */
#define AS3911_CMD_MEASURE_CAPACITANCE      0xDE    /*!< Measure capacitance */
#define AS3911_CMD_MEASURE_VDD              0xDF    /*!< Measure power supply voltage */
#define AS3911_CMD_START_TIMER              0xE0    /*!< Start the general purpose timer */
#define AS3911_CMD_START_WUP_TIMER          0xE1    /*!< Start the wake-up timer */
#define AS3911_CMD_START_MASK_RECEIVE_TIMER 0xE2    /*!< Start the mask-receive timer */
#define AS3911_CMD_START_NO_RESPONSE_TIMER  0xE3    /*!< Start the no-repsonse timer */
#define AS3911_CMD_TEST_CLEARA              0xFA    /*!< Clear Test register */
#define AS3911_CMD_TEST_CLEARB              0xFB    /*!< Clear Test register */
#define AS3911_CMD_TEST_ACCESS              0xFC    /*!< Enable R/W access to the test registers */
#define AS3911_CMD_LOAD_PPROM               0xFD    /*!< Load data from the poly fuses to RAM */
#define AS3911_CMD_FUSE_PPROM               0xFE    /*!< Fuse poly fuses with data from the RAM */


#define AS3911_FIFO_DEPTH       96 /*!< depth of FIFO */

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
/*!
 *****************************************************************************
 *  \brief  Initialize AS3911 driver
 *
 *  This function initializes the AS3911 driver.
 *
 *  \return ERR_NONE : No error, AS3911 driver initialized.
 *
 *****************************************************************************
 */
extern S8 as3911Initialize(void);

/*!
 *****************************************************************************
 *  \brief  Deinitialize AS3911 driver
 *
 *  Calling this function deinitializes the AS3911 driver.
 *
 *  \return ERR_IO : Communication error.
 *  \return ERR_NONE : No error, AS3911 driver deinitialized.
 *
 *****************************************************************************
 */
extern S8 as3911Deinitialize(void);

/*!
 *****************************************************************************
 *  \brief  Sets the bitrate
 *
 *  This function sets the bitrates for rx and tx
 *
 *  \param rxrate : speed is 2^rxrate * 106 kb/s
 *                  0xff : don't set rxrate
 *  \param txrate : speed is 2^txrate * 106 kb/s
 *                  0xff : don't set txrate
 *
 *  \return ERR_NONE : No error, both bit rates were set
 *  \return ERR_PARAM: At least one bit rate was invalid
 *
 *****************************************************************************
 */
extern S8 as3911SetBitrate(U8 rxrate, U8 txrate);

/*!
 *****************************************************************************
 *  \brief  Adjusts supply regulators according to the current supply voltage
 *
 *  This function the power level is measured in maximum load conditions and
 *  the regulated voltage reference is set to 250mV below this level.
 *  Execution of this function lasts arround 5ms. Result of calibration is
 *  written to \a result in milliVolts.
 *
 *  \return ERR_REQUEST : Adjustment not possible since reg_s bit is set.
 *  \return ERR_IO : Error during communication with AS3911.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 as3911AdjustRegulators(U16* result_mV);

/*!
 *****************************************************************************
 *  \brief  Measure RF
 *
 *  This function measured the amplitude on the RFI inputs and stores the
 *  result in parameter \a result.
 *
 *  \param[out] result: 8 bit S32 result of RF measurement.
 *
 *  \return ERR_IO : Error during communication with AS3911.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 as3911MeasureRF(U8* result);

/*!
 *****************************************************************************
 *  \brief  Measure Voltage
 *
 *  This function measures the voltage on one of VDD and VSP_*
 *  result in parameter \a result.
 *
 *  \param[in] mpsv : one of AS3911_REG_REGULATOR_CONTROL_mpsv_vdd
 *                           AS3911_REG_REGULATOR_CONTROL_mpsv_vsp_rf
 *                           AS3911_REG_REGULATOR_CONTROL_mpsv_vsp_a
 *                    or     AS3911_REG_REGULATOR_CONTROL_mpsv_vsp_d
 *
 *  \return the measured voltage in mV
 *
 *****************************************************************************
 */
extern U16 as3911MeasureVoltage(U8 mpsv);

/*!
 *****************************************************************************
 *  \brief  Calibrate antenna
 *
 *  This function is used to calibrate the antenna using a special sequence.
 *  The result is stored in the \a result parameter.
 *
 *  \param[out] result: 8 bit S32 result of antenna calibration algorithm.
 *
 *  \return ERR_IO : Error during communication with AS3911.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 as3911CalibrateAntenna(U8* result);

/*!
 *****************************************************************************
 *  \brief  Check antenna resonance
 *
 *  This function is used to measure the antenna LC tank resconance to determine
 *  whether a calibration is needed.
 *  The result is stored in the \a result parameter.
 *
 *  \param[out] result: 8 bit S32 result of the measurement.
 *
 *  \return ERR_IO : Error during communication with AS3911.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 as3911MeasureAntennaResonance(U8* result);

/*!
 *****************************************************************************
 *  \brief  Calibrate modulation depth
 *
 *  This function is used to calibrate the modulation depth using a special sequence.
 *  The result is stored in the \a result parameter.
 *
 *  \param[out] result: 8 bit S32 result of antenna calibration algorithm.
 *
 *  \return ERR_IO : Error during communication with AS3911.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 as3911CalibrateModulationDepth(U8* result);

/*!
 *****************************************************************************
 *  \brief  Prepare for upcomming receive
 *
 *  This function \b must be called before calling #as3911RxNBytes.
 *  It enables all required interrupt and prepares receive logic.
 *  It should be called before a request is sent to a PICC, i.e.
 *  first this function is to be called, then the request is sent to the PICC
 *  and the the result is fetched using #as3911RxNBytes.
 *  \note This function enables RXS and FWL interrupt of AS3911.
 *
 *  \param[in] reset : Reset receive logic
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, receive logic prepared.
 *
 *****************************************************************************
 */
extern S8 as3911PrepareReceive(bool_t reset);

/*!
 *****************************************************************************
 *  \brief  Read a given number of bytes out of the AS3911 FIFO
 *
 *  This function needs to be called in order to read from AS3911 FIFO.
 *  \note After leaving this function RXS and FWL interrupt of AS3911 are
 *  disabled regardless on their enable status before calling this function.
 *  It is important to call #as3911PrepareReceive prior to this function call.
 *
 *  \param[out]  buf: pointer to a buffer where the FIFO content shall be
 *                       written to.
 *  \param[in]  maxlength: Maximum number of values to be read. (= size of \a buf)
 *  \param[out] length: Actual read FIFO entries
 *  \param[in]  timeout_ms: timeout value for waiting for an interrupt, if 0
 *                          value aof as3911GetNoResponseTime_64fcs() is used.
 *
 *  \return ERR_NOMSG : Parity for frameing error
 *  \return ERR_REQUEST : Function can't be executed as it wasn't prepared
 *                     properly using #as3911PrepareReceive
 *  \return ERR_TIMEOUT : Receive timeout, No Resonpse IRQ or General Purpose IRQ
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, \a length values read.
 *
 *****************************************************************************
 */
extern S8 as3911RxNBytes(U8* buf, U16 maxlength, U16* length, U16 timeout_ms);

/*!
 *****************************************************************************
 *  \brief  Transmit a given number of bytes
 *
 *  This function takes the data given by \a frame and writes them into the FIFO of
 *  the AS3911. After that transmission is started. Depending on \a flags parameter
 *  ISO14443a/b or NFC frame is sent.
 *  For ISO14443a it is  possible to send a bit-oriented frame used for anticollision.
 *  \note This function only sends this frame. It doesn't wait for a reply.
 *
 *  \param[in] frame: data to be transmitted.
 *  \param[in] numbytes : Number of bytes to transmit.
 *  \param[in] numbits: Only used in case of ISO14443a anti-collision. If
 *                this parameter is > 0 then the last byte of \a frame contains
 *                these bits.
 *  \param[in] flags : Flags to control this function. Possible values:
 *        #AS3911_TX_FLAG_CRC : AS3911 appends CRC checksum to frame
 *        #AS3911_TX_FLAG_ANTCL: for ISO14443a anti-collision is sent. Parameter
 *                            \a numbits is used in this case)
 *        #AS3911_TX_FLAG_NFC_INITIATOR : Frame is sent with initial collision
 *                                        avoidance.
 *        #AS3911_TX_FLAG_NFC_TARGET : Frame is sent with response collision
 *                                        avoidance.
 *
 *  \return ERR_TIMEOUT : Timeout waiting for interrupts
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern S8 as3911TxNBytes(const U8* frame, U16 numbytes, U8 numbits, as3911TxFlag_t flags);


/*!
 *****************************************************************************
 *  \brief  execute a command an return the result
 *
 *  This function executes the given command, waits a little and returns the specified
 *  register afterwards.
 *  For certain commands the sleeptime is only used as a timeout for waiting
 *  for the proper interrupt.
 *
 *  \param cmd : one of AS3911_CMD_*
 *  \param resreg: the proper AS3911_REG_* registers
 *  \param sleeptime: either sleep time in ms or timeout when waiting for
 *                    completion interrupt
 *  \param[out] result: value of resreg
 */
extern S8 as3911ExecuteCommandAndGetResult(U8 cmd, U8 resreg, U8 sleeptime, U8* result);

/*!
 *****************************************************************************
 *  \brief  set no response time
 *
 *  This function executes sets the no response time to the defines value
 *
 *  \param nrt_64fcs : no response time in 64/fc = 4.72us
 *                    completion interrupt
 *
 *  \return ERR_PARAM : if time is too large
 */
extern S8 as3911SetNoResponseTime_64fcs(U32 nrt_64fcs);
extern U32 as3911GetNoResponseTime_64fcs(void);

/*!
 *****************************************************************************
 *  \brief  set general purpose timer timeout
 *
 *  This function sets the proper registers but does not start the timer actually
 *
 *  \param gpt_8fcs : general purpose timer timeout in 8/fc = 590ns
 *
 */
extern S8 as3911SetGPTime_8fcs(U16 gpt_8fcs);
/*!
 *****************************************************************************
 *  \brief  Starts GPT with given timeout
 *
 *  This function starts the general purpose timer with the given timeout
 *
 *  \param gpt_8fcs : general purpose timer timeout in 8/fc = 590ns
 *
 */
extern S8 as3911StartGPTimer_8fcs(U16 gpt_8fcs);
#endif /* AS3911_H */

