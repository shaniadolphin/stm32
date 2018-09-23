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
 *  \brief AS3911 communication declaration file
 *
 */
/*!
 * This driver provides basic abstraction for communication with the AS3911.
 * It uses the SPI driver for interfacing with the AS3911.
 *
 * API:
 * - Read Register: #as3911ReadRegister
 * - Modify Register: #as3911ModifyRegister
 * - Write Register: #as3911WriteRegister
 * - Write Multiple Registers: #as3911WriteMultipleRegisters
 * - Load AS3911 FIFO with data: #as3911WriteFifo
 * - Read from AS3911 FIFO: #as3911ReadFifo
 * - Execute direct command: #as3911ExecuteCommand
 */

#ifndef AS3911_COM_H
#define AS3911_COM_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
#define AS3911_REG_IO_CONF1                     0x00        /*!< RW IO Configuration Register 1 */
#define AS3911_REG_IO_CONF2                     0x01        /*!< RW IO Configuration Register 2 */

#define AS3911_REG_OP_CONTROL                   0x02        /*!< RW Operation Control Register */
#define AS3911_REG_MODE                         0x03        /*!< RW Mode Definition Register */
#define AS3911_REG_BIT_RATE                     0x04        /*!< RW Bit Rate Definition Register */

#define AS3911_REG_ISO14443A_NFC                0x05        /*!< RW ISO14443A and NFC 106 kBit/s Settings Register */
#define AS3911_REG_ISO14443B_1                  0x06        /*!< RW ISO14443B Settings Register 1 */
#define AS3911_REG_ISO14443B_2                  0x07        /*!< RW ISO14443B Settings Register 2 */
#define AS3911_REG_STREAM_MODE                  0x08        /*!< RW Stream Mode Definition Register */
#define AS3911_REG_AUX                          0x09        /*!< RW Auxiliary Definition Register */
#define AS3911_REG_RX_CONF1                     0x0A        /*!< RW Receiver Configuration Register 1 */
#define AS3911_REG_RX_CONF2                     0x0B        /*!< RW Receiver Configuration Register 2 */
#define AS3911_REG_RX_CONF3                     0x0C        /*!< RW Receiver Configuration Register 3 */
#define AS3911_REG_RX_CONF4                     0x0D        /*!< RW Receiver Configuration Register 4 */

#define AS3911_REG_MASK_RX_TIMER                0x0E        /*!< RW Mask Receive Timer Register */
#define AS3911_REG_NO_RESPONSE_TIMER1           0x0F        /*!< RW No-response Timer Register 1 */
#define AS3911_REG_NO_RESPONSE_TIMER2           0x10        /*!< RW No-response Timer Register 2 */
#define AS3911_REG_GPT_CONTROL                  0x11        /*!< RW General Purpose Timer Control Register */
#define AS3911_REG_GPT1                         0x12        /*!< RW General Purpose Timer Register 1 */
#define AS3911_REG_GPT2                         0x13        /*!< RW General Purpose Timer Register 2 */

#define AS3911_REG_IRQ_MASK_MAIN                0x14        /*!< RW Mask Main Interrupt Register */
#define AS3911_REG_IRQ_MASK_TIMER_NFC           0x15        /*!< RW Mask Timer and NFC Interrupt Register */
#define AS3911_REG_IRQ_MASK_ERROR_WUP           0x16        /*!< RW Mask Error and Wake-up Interrupt Register */
#define AS3911_REG_IRQ_MAIN                     0x17        /*!< R  Main Interrupt Register */
#define AS3911_REG_IRQ_TIMER_NFC                0x18        /*!< R  Timer and NFC Interrupt Register */
#define AS3911_REG_IRQ_ERROR_WUP                0x19        /*!< R  Error and Wake-up Interrupt Register */
#define AS3911_REG_FIFO_RX_STATUS1              0x1A        /*!< R  FIFO RX Status Register 1 */
#define AS3911_REG_FIFO_RX_STATUS2              0x1B        /*!< R  FIFO RX Status Register 2 */
#define AS3911_REG_COLLISION_STATUS             0x1C        /*!< R  Collision Display Register */

#define AS3911_REG_NUM_TX_BYTES1                0x1D        /*!< RW Number of Transmitted Bytes Register 1 */
#define AS3911_REG_NUM_TX_BYTES2                0x1E        /*!< RW Number of Transmitted Bytes Register 2 */

#define AS3911_REG_NFCIP1_BIT_RATE              0x1F        /*!< R  NFCIP Bit Rate Detection Display Register */

#define AS3911_REG_AD_RESULT                    0x20        /*!< R  A/D Converter Output Register */

#define AS3911_REG_ANT_CAL_CONTROL              0x21        /*!< RW Antenna Calibration Control Register */
#define AS3911_REG_ANT_CAL_TARGET               0x22        /*!< RW Antenna Calibration Target Register */
#define AS3911_REG_ANT_CAL_RESULT               0x23        /*!< R  Antenna Calibration Display Register */

#define AS3911_REG_AM_MOD_DEPTH_CONTROL         0x24        /*!< RW AM Modulation Depth Control Register */
#define AS3911_REG_AM_MOD_DEPTH_RESULT          0x25        /*!< R  AM Modulation Depth Display Register */
#define AS3911_REG_RFO_AM_ON_LEVEL              0x26        /*!< RW RFO AM Modulation (On) Level Definition Register */
#define AS3911_REG_RFO_AM_OFF_LEVEL             0x27        /*!< RW RFO Normal (AM Off) Level Definition Register */

#define AS3911_REG_FIELD_THRESHOLD              0x29        /*!< RW External Field Detector Threshold Register */

#define AS3911_REG_REGULATOR_CONTROL            0x2A        /*!< RW Regulated Voltage Control Register */
#define AS3911_REG_REGULATOR_RESULT             0x2B        /*!< R Regulator Display Register */

#define AS3911_REG_RSSI_RESULT                  0x2C        /*!< R RSSI Display Register*/
#define AS3911_REG_GAIN_RED_STATE               0x2D        /*!< R Gain Reduction State Register*/

#define AS3911_REG_CAP_SENSOR_CONTROL           0x2E        /*!< RW Capacitive Sensor Control Register */
#define AS3911_REG_CAP_SENSOR_RESULT            0x2F        /*!< R  Capacitive Sensor Display Register */

#define AS3911_REG_AUX_DISPLAY                  0x30        /*!< R Auxiliary Display Register */

#define AS3911_REG_WUP_TIMER_CONTROL            0x31        /*!< RW Wake-up Timer Control Register */
#define AS3911_REG_AMPLITUDE_MEASURE_CONF       0x32        /*!< RW Amplitude Measurement Configuration Register */
#define AS3911_REG_AMPLITUDE_MEASURE_REF        0x33        /*!< RW Amplitude Measurement Reference Register */
#define AS3911_REG_AMPLITUDE_MEASURE_AA_RESULT  0x34        /*!< R  Amplitude Measurement Auto Averaging Display Register */
#define AS3911_REG_AMPLITUDE_MEASURE_RESULT     0x35        /*!< R  Amplitude Measurement Display Register */
#define AS3911_REG_PHASE_MEASURE_CONF           0x36        /*!< RW Phase Measurement Configuration Register */
#define AS3911_REG_PHASE_MEASURE_REF            0x37        /*!< RW Phase Measurement Reference Register */
#define AS3911_REG_PHASE_MEASURE_AA_RESULT      0x38        /*!< R  Phase Measurement Auto Averaging Display Register */
#define AS3911_REG_PHASE_MEASURE_RESULT         0x39        /*!< R  Phase Measurement Display Register */
#define AS3911_REG_CAPACITANCE_MEASURE_CONF     0x3A        /*!< RW Capacitance Measurement Configuration Register */
#define AS3911_REG_CAPACITANCE_MEASURE_REF      0x3B        /*!< RW Capacitance Measurement Reference Register */
#define AS3911_REG_CAPACITANCE_MEASURE_AA_RESULT 0x3C       /*!< R  Capacitance Measurement Auto Averaging Display Register */
#define AS3911_REG_CAPACITANCE_MEASURE_RESULT   0x3D        /*!< R  Capacitance Measurement Display Register */

#define AS3911_REG_IC_IDENTITY                  0x3F        /*!< R  Chip Id: 0 for old silicon, v2 silicon: 0x09 */

/* Register bit definitions */

#define AS3911_REG_IO_CONF1_lf_clk_off                     (1<<0)
#define AS3911_REG_IO_CONF1_out_cl0                        (1<<1)
#define AS3911_REG_IO_CONF1_out_cl1                        (1<<2)
#define AS3911_REG_IO_CONF1_osc                            (1<<3)
#define AS3911_REG_IO_CONF1_fifo_lt                        (1<<4)
#define AS3911_REG_IO_CONF1_fifo_lr                        (1<<5)
#define AS3911_REG_IO_CONF1_rfo2                           (1<<6)
#define AS3911_REG_IO_CONF1_single                         (1<<7)
#define AS3911_REG_IO_CONF2_slow_up                        (1<<0)
#define AS3911_REG_IO_CONF2_io_18                          (1<<2)
#define AS3911_REG_IO_CONF2_miso_pd1                       (1<<3)
#define AS3911_REG_IO_CONF2_miso_pd2                       (1<<4)
#define AS3911_REG_IO_CONF2_vspd_off                       (1<<6)
#define AS3911_REG_IO_CONF2_sup3V                          (1<<7)
#define AS3911_REG_OP_CONTROL_wu                           (1<<2)
#define AS3911_REG_OP_CONTROL_tx_en                        (1<<3)
#define AS3911_REG_OP_CONTROL_rx_man                       (1<<4)
#define AS3911_REG_OP_CONTROL_rx_chn                       (1<<5)
#define AS3911_REG_OP_CONTROL_rx_en                        (1<<6)
#define AS3911_REG_OP_CONTROL_en                           (1<<7)
#define AS3911_REG_MODE_mask_om                            (0xf<<3)
#define AS3911_REG_MODE_om_nfc                             (0x0<<3)
#define AS3911_REG_MODE_om_iso14443a                       (0x1<<3)
#define AS3911_REG_MODE_om_iso14443b                       (0x2<<3)
#define AS3911_REG_MODE_om_felica                          (0x3<<3)
#define AS3911_REG_MODE_om_topaz                           (0x4<<3)
#define AS3911_REG_MODE_om_subcarrier_stream               (0xe<<3)
#define AS3911_REG_MODE_om_bpsk_stream                     (0xf<<3)
#define AS3911_REG_MODE_targ                               (1<<7)
#define AS3911_REG_BIT_RATE_mask_txrate                    (0xf<<4)
#define AS3911_REG_BIT_RATE_shift_txrate                   (4)
#define AS3911_REG_BIT_RATE_mask_rxrate                    (0xf<<0)
#define AS3911_REG_BIT_RATE_shift_rxrate                   (0)
#define AS3911_REG_ISO14443A_NFC_antcl                     (1<<0)
#define AS3911_REG_ISO14443A_NFC_mask_p_len                (0xf<<1)
#define AS3911_REG_ISO14443A_NFC_shift_p_len               (1)
#define AS3911_REG_ISO14443A_NFC_no_rx_par                 (1<<6)
#define AS3911_REG_ISO14443A_NFC_no_tx_par                 (1<<7)
#define AS3911_REG_ISO14443B_1_eof                         (1<<2)
#define AS3911_REG_ISO14443B_1_sof_1                       (1<<3)
#define AS3911_REG_ISO14443B_1_sof_0                       (1<<4)
#define AS3911_REG_ISO14443B_1_mask_egt                    (7<<5)
#define AS3911_REG_ISO14443B_1_shift_egt                   (5)
#define AS3911_REG_ISO14443B_2_eof_12                      (1<<3)
#define AS3911_REG_ISO14443B_2_no_eof                      (1<<4)
#define AS3911_REG_ISO14443B_2_no_sof                      (1<<5)
#define AS3911_REG_ISO14443B_2_tr1_0                       (1<<6)
#define AS3911_REG_ISO14443B_2_tr1_1                       (1<<7)
#define AS3911_REG_STREAM_MODE_mask_stx                    (7<<0)
#define AS3911_REG_STREAM_MODE_shift_stx                   (0)
#define AS3911_REG_STREAM_MODE_stx_106                     (0<<0)
#define AS3911_REG_STREAM_MODE_stx_212                     (1<<0)
#define AS3911_REG_STREAM_MODE_stx_424                     (2<<0)
#define AS3911_REG_STREAM_MODE_stx_848                     (3<<0)
#define AS3911_REG_STREAM_MODE_stx_1695                    (4<<0)
#define AS3911_REG_STREAM_MODE_stx_3390                    (5<<0)
#define AS3911_REG_STREAM_MODE_stx_6780                    (6<<0)
#define AS3911_REG_STREAM_MODE_mask_scp                    (3<<3)
#define AS3911_REG_STREAM_MODE_shift_scp                   (3)
#define AS3911_REG_STREAM_MODE_scp_1pulse                  (0<<3)
#define AS3911_REG_STREAM_MODE_scp_2pulses                 (1<<3)
#define AS3911_REG_STREAM_MODE_scp_4pulses                 (2<<3)
#define AS3911_REG_STREAM_MODE_scp_8pulses                 (3<<3)
#define AS3911_REG_STREAM_MODE_mask_scf                    (3<<5)
#define AS3911_REG_STREAM_MODE_shift_scf                   (5)
#define AS3911_REG_STREAM_MODE_scf_bpsk848                 (0<<5)
#define AS3911_REG_STREAM_MODE_scf_bpsk1695                (1<<5)
#define AS3911_REG_STREAM_MODE_scf_bpsk3390                (2<<5)
#define AS3911_REG_STREAM_MODE_scf_bpsk106                 (3<<5)
#define AS3911_REG_STREAM_MODE_scf_sc212                   (0<<5)
#define AS3911_REG_STREAM_MODE_scf_sc424                   (1<<5)
#define AS3911_REG_STREAM_MODE_scf_sc848                   (2<<5)
#define AS3911_REG_STREAM_MODE_scf_sc1695                  (3<<5)
#define AS3911_REG_AUX_nfc_n0                              (1<<0)
#define AS3911_REG_AUX_nfc_n1                              (1<<1)
#define AS3911_REG_AUX_rx_tol                              (1<<2)
#define AS3911_REG_AUX_ook_hr                              (1<<3)
#define AS3911_REG_AUX_en_fd                               (1<<4)
#define AS3911_REG_AUX_tr_am                               (1<<5)
#define AS3911_REG_AUX_crc_2_fifo                          (1<<6)
#define AS3911_REG_AUX_no_crc_rx                           (1<<7)
#define AS3911_REG_RX_CONF1_z12k                           (1<<0)
#define AS3911_REG_RX_CONF1_h80                            (1<<1)
#define AS3911_REG_RX_CONF1_h200                           (1<<2)
#define AS3911_REG_RX_CONF1_mask_lp                        (7<<3)
#define AS3911_REG_RX_CONF1_lp_1200khz                     (0<<3)
#define AS3911_REG_RX_CONF1_lp_600khz                      (1<<3)
#define AS3911_REG_RX_CONF1_lp_300khz                      (2<<3)
#define AS3911_REG_RX_CONF1_lp_2000khz                     (4<<3)
#define AS3911_REG_RX_CONF1_lp_7000khz                     (5<<3)
#define AS3911_REG_RX_CONF1_amd_sel                        (1<<6)
#define AS3911_REG_RX_CONF1_ch_sel                         (1<<7)
#define AS3911_REG_RX_CONF2_sqm_dyn                        (1<<1)
#define AS3911_REG_RX_CONF2_agc_alg                        (1<<2)
#define AS3911_REG_RX_CONF2_agc_m                          (1<<3)
#define AS3911_REG_RX_CONF2_agc_en                         (1<<4)
#define AS3911_REG_RX_CONF2_lf_en                          (1<<5)
#define AS3911_REG_RX_CONF2_lf_op                          (1<<6)
#define AS3911_REG_RX_CONF2_rx_lp                          (1<<7)
#define AS3911_REG_RX_CONF3_rg_nfc                         (1<<0)
#define AS3911_REG_RX_CONF3_lim                            (1<<1)
#define AS3911_REG_RX_CONF3_shift_rg1_pm                   (2)
#define AS3911_REG_RX_CONF3_mask_rg1_pm                    (0x7<<2)
#define AS3911_REG_RX_CONF3_rg1_pm0                        (1<<2)
#define AS3911_REG_RX_CONF3_rg1_pm1                        (1<<3)
#define AS3911_REG_RX_CONF3_rg1_pm2                        (1<<4)
#define AS3911_REG_RX_CONF3_shift_rg1_am                   (5)
#define AS3911_REG_RX_CONF3_mask_rg1_am                    (0x7<<5)
#define AS3911_REG_RX_CONF3_rg1_am0                        (1<<5)
#define AS3911_REG_RX_CONF3_rg1_am1                        (1<<6)
#define AS3911_REG_RX_CONF3_rg1_am2                        (1<<7)
#define AS3911_REG_RX_CONF4_shift_rg2_pm                   (0)
#define AS3911_REG_RX_CONF4_mask_rg2_pm                    (0xf<<0)
#define AS3911_REG_RX_CONF4_rg2_pm0                        (1<<0)
#define AS3911_REG_RX_CONF4_rg2_pm1                        (1<<1)
#define AS3911_REG_RX_CONF4_rg2_pm2                        (1<<2)
#define AS3911_REG_RX_CONF4_rg2_pm3                        (1<<3)
#define AS3911_REG_RX_CONF4_shift_rg2_am                   (4)
#define AS3911_REG_RX_CONF4_mask_rg2_am                    (0xf<<4)
#define AS3911_REG_RX_CONF4_rg2_am0                        (1<<4)
#define AS3911_REG_RX_CONF4_rg2_am1                        (1<<5)
#define AS3911_REG_RX_CONF4_rg2_am2                        (1<<6)
#define AS3911_REG_RX_CONF4_rg2_am3                        (1<<7)
#define AS3911_REG_GPT_CONTROL_nrt_step                    (1<<0)
#define AS3911_REG_GPT_CONTROL_nrt_emv                     (1<<1)
#define AS3911_REG_GPT_CONTROL_gptc0                       (1<<5)
#define AS3911_REG_GPT_CONTROL_gptc1                       (1<<6)
#define AS3911_REG_GPT_CONTROL_gptc2                       (1<<7)
#define AS3911_REG_GPT_CONTROL_gptc_mask                   (0x7<<5)
#define AS3911_REG_GPT_CONTROL_gptc_no_trigger             (0x0<<5)
#define AS3911_REG_GPT_CONTROL_gptc_erx                    (0x1<<5)
#define AS3911_REG_GPT_CONTROL_gptc_srx                    (0x2<<5)
#define AS3911_REG_GPT_CONTROL_gptc_etx_nfc                (0x3<<5)
#define AS3911_REG_FIFO_RX_STATUS2_np_lb                   (1<<0)
#define AS3911_REG_FIFO_RX_STATUS2_mask_fifo_lb            (7<<1)
#define AS3911_REG_FIFO_RX_STATUS2_shift_fifo_lb           (1)
#define AS3911_REG_FIFO_RX_STATUS2_fifo_lb0                (1<<1)
#define AS3911_REG_FIFO_RX_STATUS2_fifo_lb1                (1<<2)
#define AS3911_REG_FIFO_RX_STATUS2_fifo_lb2                (1<<3)
#define AS3911_REG_FIFO_RX_STATUS2_fifo_ncp                (1<<4)
#define AS3911_REG_FIFO_RX_STATUS2_fifo_ovr                (1<<5)
#define AS3911_REG_FIFO_RX_STATUS2_fifo_unf                (1<<6)
#define AS3911_REG_COLLISION_STATUS_c_pb                   (1<<0)
#define AS3911_REG_COLLISION_STATUS_mask_c_bit             (3<<1)
#define AS3911_REG_COLLISION_STATUS_shift_c_bit            (1)
#define AS3911_REG_COLLISION_STATUS_mask_c_byte            (0xf<<4)
#define AS3911_REG_COLLISION_STATUS_shift_c_byte           (4)
#define AS3911_AS3911_REG_NFCIP1_BIT_RATE_nfc_rate0        (1<<4)
#define AS3911_AS3911_REG_NFCIP1_BIT_RATE_nfc_rate1        (1<<5)
#define AS3911_AS3911_REG_NFCIP1_BIT_RATE_nfc_rate2        (1<<6)
#define AS3911_AS3911_REG_NFCIP1_BIT_RATE_nfc_rate3        (1<<7)
#define AS3911_REG_ANT_CAL_CONTROL_tre_0                   (1<<3)
#define AS3911_REG_ANT_CAL_CONTROL_tre_1                   (1<<4)
#define AS3911_REG_ANT_CAL_CONTROL_tre_2                   (1<<5)
#define AS3911_REG_ANT_CAL_CONTROL_tre_3                   (1<<6)
#define AS3911_REG_ANT_CAL_CONTROL_trim_s                  (1<<7)
#define AS3911_REG_ANT_CAL_RESULT_tri_err                  (1<<3)
#define AS3911_REG_ANT_CAL_RESULT_tri_0                    (1<<4)
#define AS3911_REG_ANT_CAL_RESULT_tri_1                    (1<<5)
#define AS3911_REG_ANT_CAL_RESULT_tri_2                    (1<<6)
#define AS3911_REG_ANT_CAL_RESULT_tri_3                    (1<<7)
#define AS3911_REG_AM_MOD_DEPTH_CONTROL_mask_mod           (0x3f<<1)
#define AS3911_REG_AM_MOD_DEPTH_CONTROL_shift_mod          (1)
#define AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_8percent       (0xb<<1)
#define AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_10percent      (0xe<<1)
#define AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_14percent      (0x14<<1)
#define AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_20percent      (0x20<<1)
#define AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_30percent      (0x37<<1)
#define AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_33percent      (0x3f<<1)
#define AS3911_REG_AM_MOD_DEPTH_CONTROL_am_s               (1<<7)
#define AS3911_REG_RFO_AM_MOD_LEVEL_dram0                  (1<<0)
#define AS3911_REG_RFO_AM_MOD_LEVEL_dram1                  (1<<1)
#define AS3911_REG_RFO_AM_MOD_LEVEL_dram2                  (1<<2)
#define AS3911_REG_RFO_AM_MOD_LEVEL_dram3                  (1<<3)
#define AS3911_REG_RFO_AM_MOD_LEVEL_dram4                  (1<<4)
#define AS3911_REG_RFO_AM_MOD_LEVEL_dram5                  (1<<5)
#define AS3911_REG_RFO_AM_MOD_LEVEL_dram6                  (1<<6)
#define AS3911_REG_RFO_AM_MOD_LEVEL_dram7                  (1<<7)
#define AS3911_REG_FIELD_THRESHOLD_rfe_t0                  (1<<0)
#define AS3911_REG_FIELD_THRESHOLD_rfe_t1                  (1<<1)
#define AS3911_REG_FIELD_THRESHOLD_rfe_t2                  (1<<2)
#define AS3911_REG_FIELD_THRESHOLD_rfe_t3                  (1<<3)
#define AS3911_REG_FIELD_THRESHOLD_trg_l0                  (1<<4)
#define AS3911_REG_FIELD_THRESHOLD_trg_l1                  (1<<5)
#define AS3911_REG_FIELD_THRESHOLD_trg_l2                  (1<<6)
#define AS3911_REG_REGULATOR_CONTROL_shift_mpsv            (1)
#define AS3911_REG_REGULATOR_CONTROL_mask_mpsv             (3<<1)
#define AS3911_REG_REGULATOR_CONTROL_mpsv_vdd              (0<<1)
#define AS3911_REG_REGULATOR_CONTROL_mpsv_vsp_a            (1<<1)
#define AS3911_REG_REGULATOR_CONTROL_mpsv_vsp_d            (2<<1)
#define AS3911_REG_REGULATOR_CONTROL_mpsv_vsp_rf           (3<<1)
#define AS3911_REG_REGULATOR_CONTROL_mask_rege             (0xf<<3)
#define AS3911_REG_REGULATOR_CONTROL_shift_rege            (3)
#define AS3911_REG_REGULATOR_CONTROL_reg_s                 (1<<7)
#define AS3911_REG_REGULATOR_RESULT_shift_reg              (4)
#define AS3911_REG_REGULATOR_RESULT_reg_0                  (1<<4)
#define AS3911_REG_REGULATOR_RESULT_reg_1                  (1<<5)
#define AS3911_REG_REGULATOR_RESULT_reg_2                  (1<<6)
#define AS3911_REG_REGULATOR_RESULT_reg_3                  (1<<7)
#define AS3911_REG_RSSI_RESULT_rssi_pm0                    (1<<0)
#define AS3911_REG_RSSI_RESULT_rssi_pm1                    (1<<1)
#define AS3911_REG_RSSI_RESULT_rssi_pm2                    (1<<2)
#define AS3911_REG_RSSI_RESULT_rssi_pm3                    (1<<3)
#define AS3911_REG_RSSI_RESULT_rssi_am_0                   (1<<4)
#define AS3911_REG_RSSI_RESULT_rssi_am_1                   (1<<5)
#define AS3911_REG_RSSI_RESULT_rssi_am_2                   (1<<6)
#define AS3911_REG_RSSI_RESULT_rssi_am_3                   (1<<7)
#define AS3911_REG_GAIN_RED_STATE_gs_pm_0                  (1<<0)
#define AS3911_REG_GAIN_RED_STATE_gs_pm_1                  (1<<1)
#define AS3911_REG_GAIN_RED_STATE_gs_pm_2                  (1<<2)
#define AS3911_REG_GAIN_RED_STATE_gs_pm_3                  (1<<3)
#define AS3911_REG_GAIN_RED_STATE_gs_am_0                  (1<<4)
#define AS3911_REG_GAIN_RED_STATE_gs_am_1                  (1<<5)
#define AS3911_REG_GAIN_RED_STATE_gs_am_2                  (1<<6)
#define AS3911_REG_GAIN_RED_STATE_gs_am_3                  (1<<7)
#define AS3911_REG_CAP_SENSOR_CONTROL_cs_g0                (1<<0)
#define AS3911_REG_CAP_SENSOR_CONTROL_cs_g1                (1<<1)
#define AS3911_REG_CAP_SENSOR_CONTROL_cs_g2                (1<<2)
#define AS3911_REG_CAP_SENSOR_CONTROL_cs_mcal0             (1<<3)
#define AS3911_REG_CAP_SENSOR_CONTROL_cs_mcal1             (1<<4)
#define AS3911_REG_CAP_SENSOR_CONTROL_cs_mcal2             (1<<5)
#define AS3911_REG_CAP_SENSOR_CONTROL_cs_mcal3             (1<<6)
#define AS3911_REG_CAP_SENSOR_CONTROL_cs_mcal4             (1<<7)
#define AS3911_REG_CAP_SENSOR_RESULT_cs_cal_err            (1<<1)
#define AS3911_REG_CAP_SENSOR_RESULT_cs_cal_end            (1<<2)
#define AS3911_REG_CAP_SENSOR_RESULT_cs_cal0               (1<<3)
#define AS3911_REG_CAP_SENSOR_RESULT_cs_cal1               (1<<4)
#define AS3911_REG_CAP_SENSOR_RESULT_cs_cal2               (1<<5)
#define AS3911_REG_CAP_SENSOR_RESULT_cs_cal3               (1<<6)
#define AS3911_REG_CAP_SENSOR_RESULT_cs_cal4               (1<<7)
#define AS3911_REG_AUX_DISPLAY_mrt_on                      (1<<0)
#define AS3911_REG_AUX_DISPLAY_nrt_on                      (1<<1)
#define AS3911_REG_AUX_DISPLAY_gpt_on                      (1<<2)
#define AS3911_REG_AUX_DISPLAY_rx_on                       (1<<3)
#define AS3911_REG_AUX_DISPLAY_osc_ok                      (1<<4)
#define AS3911_REG_AUX_DISPLAY_tx_on                       (1<<5)
#define AS3911_REG_AUX_DISPLAY_efd_o                       (1<<6)
#define AS3911_REG_AUX_DISPLAY_a_cha                       (1<<7)
#define AS3911_REG_WUP_TIMER_CONTROL_wcap                  (1<<0)
#define AS3911_REG_WUP_TIMER_CONTROL_wph                   (1<<1)
#define AS3911_REG_WUP_TIMER_CONTROL_wam                   (1<<2)
#define AS3911_REG_WUP_TIMER_CONTROL_wto                   (1<<3)
#define AS3911_REG_WUP_TIMER_CONTROL_wut0                  (1<<4)
#define AS3911_REG_WUP_TIMER_CONTROL_wut1                  (1<<5)
#define AS3911_REG_WUP_TIMER_CONTROL_wut2                  (1<<6)
#define AS3911_REG_WUP_TIMER_CONTROL_wur                   (1<<7)
#define AS3911_REG_AMPLITUDE_MEASURE_CONF_am_ae            (1<<0)
#define AS3911_REG_AMPLITUDE_MEASURE_CONF_am_aew0          (1<<1)
#define AS3911_REG_AMPLITUDE_MEASURE_CONF_am_aew1          (1<<2)
#define AS3911_REG_AMPLITUDE_MEASURE_CONF_am_aam           (1<<3)
#define AS3911_REG_AMPLITUDE_MEASURE_CONF_am_d0            (1<<4)
#define AS3911_REG_AMPLITUDE_MEASURE_CONF_am_d1            (1<<5)
#define AS3911_REG_AMPLITUDE_MEASURE_CONF_am_d2            (1<<6)
#define AS3911_REG_AMPLITUDE_MEASURE_CONF_am_d3            (1<<7)
#define AS3911_REG_PHASE_MEASURE_CONF_pm_ae                (1<<0)
#define AS3911_REG_PHASE_MEASURE_CONF_pm_aew0              (1<<1)
#define AS3911_REG_PHASE_MEASURE_CONF_pm_aew1              (1<<2)
#define AS3911_REG_PHASE_MEASURE_CONF_pm_aam               (1<<3)
#define AS3911_REG_PHASE_MEASURE_CONF_pm_d0                (1<<4)
#define AS3911_REG_PHASE_MEASURE_CONF_pm_d1                (1<<5)
#define AS3911_REG_PHASE_MEASURE_CONF_pm_d2                (1<<6)
#define AS3911_REG_PHASE_MEASURE_CONF_pm_d3                (1<<7)
#define AS3911_REG_CAPACITANCE_MEASURE_CONF_cm_ae          (1<<0)
#define AS3911_REG_CAPACITANCE_MEASURE_CONF_cm_aew0        (1<<1)
#define AS3911_REG_CAPACITANCE_MEASURE_CONF_cm_aew1        (1<<2)
#define AS3911_REG_CAPACITANCE_MEASURE_CONF_cm_aam         (1<<3)
#define AS3911_REG_CAPACITANCE_MEASURE_CONF_cm_d0          (1<<4)
#define AS3911_REG_CAPACITANCE_MEASURE_CONF_cm_d1          (1<<5)
#define AS3911_REG_CAPACITANCE_MEASURE_CONF_cm_d2          (1<<6)
#define AS3911_REG_CAPACITANCE_MEASURE_CONF_cm_d3          (1<<7)
#define AS3911_REG_IC_IDENTITY_v2                          (0x09)

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
/*!
 *****************************************************************************
 *  \brief  Returns the content of a register within the AS3911
 *
 *  This function is used to read out the content of AS3911 registers.
 *
 *  \param[in]  reg: Address of register to read.
 *  \param[out] val: Returned value.
 *
 *  \return ERR_IO : Error during communication, returned value may be unvalid.
 *  \return ERR_NONE : No error, register content written to \a val.
 *
 *****************************************************************************
 */
extern S8 as3911ReadRegister(U8 reg, U8* val);

/*!
 *****************************************************************************
 *  \brief  Writes a given value to a register within the AS3911
 *
 *  This function is used to write \a val to address \a reg within the AS3911.
 *
 *  \param[in]  reg: Address of the register to write.
 *  \param[in]  val: Value to be written.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, \a val written to register \a reg.
 *
 *****************************************************************************
 */
extern S8 as3911WriteRegister(U8 reg, U8 val);

/*!
 *****************************************************************************
 *  \brief  Writes a given value to a test register within the AS3911
 *
 *  This function is used to write \a val to test address \a reg within the AS3911.
 *
 *  \param[in]  reg: Address of the register to write.
 *  \param[in]  val: Value to be written.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, \a val written to register \a reg.
 *
 *****************************************************************************
 */
extern S8 as3911WriteTestRegister(U8 reg, U8 val);

/*!
 *****************************************************************************
 *  \brief  Modifies a value within a AS3911 register
 *
 *  This function is used if only a particular bits should be changed within
 *  an AS3911 register.
 *
 *  \param[in]  reg: Address of the register to write.
 *  \param[in]  clr_mask: bitmask of bits to be cleared to 0.
 *  \param[in]  set_mask: bitmask of bits to be set to 1.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, \a val written to register \a reg.
 *
 *****************************************************************************
 */
extern S8 as3911ModifyRegister(U8 reg, U8 clr_mask, U8 set_mask);

/*!
 *****************************************************************************
 *  \brief  Writes multiple values to AS3911 registers
 *
 *  This function is used to write multiple values to the AS3911 using the
 *  auto-increment feature. That is, after each write the address pointer
 *  inside the AS3911 gets incremented automatically.
 *
 *  \param[in]  reg: Address of the frist register to write.
 *  \param[in]  values: pointer to a buffer containing the values to be written.
 *  \param[in]  length: Number of values to be written.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, \a length values written.
 *
 *****************************************************************************
 */
extern S8 as3911WriteMultipleRegisters(U8 reg, const U8* values, U8 length);

/*!
 *****************************************************************************
 *  \brief  Reads from multiple AS3911 registers
 *
 *  This function is used to read from multiple registers using the
 *  auto-increment feature. That is, after each read the address pointer
 *  inside the AS3911 gets incremented automatically.
 *
 *  \param[in]  reg: Address of the frist register to read from.
 *  \param[in]  values: pointer to a buffer where the result shall be written to.
 *  \param[in]  length: Number of registers to be read out.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, \a length registers read.
 *
 *****************************************************************************
 */
extern S8 as3911ReadMultipleRegisters(U8 reg, U8* values, U8 length);

/*!
 *****************************************************************************
 *  \brief  Writes values to AS3911 FIFO
 *
 *  This function needs to be called in order to write to the AS3911 FIFO.
 *
 *  \param[in]  values: pointer to a buffer containing the values to be written
 *                      to the FIFO.
 *  \param[in]  length: Number of values to be written.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, \a length values written.
 *
 *****************************************************************************
 */
extern S8 as3911WriteFifo(const U8* values, U8 length);

/*!
 *****************************************************************************
 *  \brief  Read values from AS3911 FIFO
 *
 *  This function needs to be called in order to read from AS3911 FIFO.
 *
 *  \param[out]  buf: pointer to a buffer where the FIFO content shall be
 *                       written to.
 *  \param[in]  length: Number of bytes to read. (= size of \a buf)
 *  \note: This function doesn't check whether \a length is really the
 *  number of available bytes in FIFO
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, \a length values read.
 *
 *****************************************************************************
 */
extern S8 as3911ReadFifo(U8* buf, U8 length);

/*!
 *****************************************************************************
 *  \brief  Execute a direct command
 *
 *  This function is used to start so-called direct command. These commands
 *  are implemented inside the chip and each command has unique code (see
 *  datasheet).
 *
 *  \param[in]  cmd : code of the direct command to be executed.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, execution of command started.
 *
 *****************************************************************************
 */
extern S8 as3911ExecuteCommand(U8 cmd);

/*!
 *****************************************************************************
 *  \brief  Prefix for other commands to do test mode operations
 *
 *  This function must be called before e.g. as3993ReadRegister() to access the
 *  testmode registers of AS3911.
 *  Be aware that the compound of (as3911TestPrefix(),as3911ReadRegister()) is
 *  not atomic, so no other actions should be ongoing in between.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error, execution of command started.
 *
 *****************************************************************************
 */
extern S8 as3911TestPrefix(void);

extern S8 as3911EnterObsvMode(S8 mode);

#endif /* AS3911_COM_H */

