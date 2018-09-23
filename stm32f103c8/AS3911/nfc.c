#include "nfc.h"
#include "as3911_interrupt.h"
#include "crc.h"
#include "stm32f10x.h"
#include "ucos_ii.h"
#include "stm32f10x_tim.h"
#include "ic.h"
#include "bsp.h"
#include "Msg.h"
#include "spiflash.h"
#include "crc.h"


#define NFC_TRIM_SUCESS				0x10
#define NFC_TRIM_FAIL				0x11


#define SOLDIER			0x08
#define CAR				0x06
#define HORSE			0x04
#define PHASE			0x02
#define QUEEN			0x01
#define KING			0x00
#define BLACK			0x10
#define BLACK_SOLDIER	(BLACK|SOLDIER)
#define BLACK_CAR		(BLACK|CAR)
#define BLACK_HORSE		(BLACK|HORSE)
#define BLACK_PHASE		(BLACK|PHASE)
#define BLACK_QUEEN		(BLACK|QUEEN)
#define BLACK_KING		(BLACK|KING)
#define CAR_MASK		0x0e
#define HORSE_MASK		0x0c
#define PHASE_MASK		0x0e
#define QUEEN_MASK		0x0f

#define CMD_CODE_RISETOCAR		0x01
#define CMD_CODE_RISETOHORSE	0x02
#define CMD_CODE_RISETOPHASE	0x03
#define CMD_CODE_RISETOQUEEN	0x04

#define CMD_CODE_PAUSE				0x05
#define CMD_CODE_ILLEGAL			0x06
#define CMD_CODE_CONTINUE			0x07
#define CMD_CODE_SCANNOTCOMPLETE	0x00

#define CMD_CODE_CHESSMISS		0x1e
#define CMD_CODE_BEINGEAT		0x1f

#define LED_SHINE_2S			2000


const U8 x_order[X_COILS] = { 7, 5, 6, 4, 3, 1, 2, 0 };
const U8 y_order[Y_COILS] = { 0, 2, 4, 6, 7, 5, 3, 1 };

const U8 segment_P = 0x73;

//每个天线的校准值(0~15),需要自己计算转换放到0x21寄存器的值
const unsigned char calibrate_data[16] = {0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8, 0xc0, 0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8};
U8 calibrate_x[X_COILS] = { 0x80, 0x88, 0x90, 0x98, 0xa0, 0x98, 0x90, 0x80 };
U8 calibrate_y[Y_COILS] = { 0x80, 0x90, 0x98, 0xa0, 0xa0, 0x98, 0x90, 0x80 };
const unsigned char driver_x[X_COILS] = {0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0};
const unsigned char driver_y[Y_COILS] = {0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0};

SPI_CFG cfg;
TX_CFG tx_cfg;

app_calibrate_info_t calibrate_info[X_COILS+Y_COILS];

tag_data_t scan_tag[TAG_NUMS+8];
U8 scan_cnt = 0;
U16 press_cnt = 0;
U8 play_part = 0;

tag_data_t* chess_board[Y_COILS][X_COILS];
tag_data_t* chess_board_old[Y_COILS][X_COILS];

U16 chess_steps;
U8 chess_mode;

U8 SelfTestMode;

U16 ReadTime = 0;
U8  AddTime = 0;
U16 WhiteTime;
U16 BlackTime;
U16 PassTime;
U8  WhiteIllegal = 0;		//白方违规
U8  BlackIllegal = 0;		//黑方违规
U16 PassMilliCounter = 0;	//用于耗时补偿
U16 TickCounter;
U8 StartErrorCnt;
U8 ErrorCode;
U16 WhiteShineCounter = 0;	//用于白LED警告清除
U16 BlackShineCounter = 0;	//用于黑LED警告清除
U16 StatusCounter = 0;		//用于状态状态显示清除
U16 NetSendCounter = 0;		//用于发送历史数据时，状态灯急闪

U8 TimeSetting = 0;
U8 SetReadTimeHun;
U8 SetReadTimeTen;
U8 SetReadTimeOne;
U8 SetAddTimeTen;
U8 SetAddTimeOne;

U8 CpuID[12];
extern U8 IMEI[6];

extern unsigned int UnixTime;
extern unsigned char white_led;
extern unsigned char black_led;
extern unsigned char state_led;
extern unsigned char state_led_bak;

extern unsigned short key_car_cnt;
extern unsigned short key_horse_cnt;
extern unsigned short key_phase_cnt;
extern unsigned short key_queen_cnt;
extern unsigned short key_black_cnt;
extern unsigned short key_white_cnt;

U8 rise_key = 0;

U16 recv_cnt = 0;

U8 SendBuff[BYTES_PER_STEP];
U32 unix_time_tmp;

void TimeSettingInit(void);
void power_off(void);
void LoadCaliBateProcess(void);
void WarnChessRising(void);

iso15693PhyConfig_t iso15693_config = {
	/*.coding =*/ ISO15693_VCD_CODING_1_4,
	/*.mi =*/ ISO15693_MODULATION_INDEX_OOK,
};

const CHESS_SCAN chess_scan[X_COILS+Y_COILS] = {
	{ 0, X_DIR },
	{ 2, X_DIR },
	{ 4, X_DIR },
	{ 6, X_DIR },
	{ 1, X_DIR },
	{ 3, X_DIR },
	{ 5, X_DIR },
	{ 7, X_DIR },
	{ 0, Y_DIR },
	{ 2, Y_DIR },
	{ 4, Y_DIR },
	{ 6, Y_DIR },
	{ 1, Y_DIR },
	{ 3, Y_DIR },
	{ 5, Y_DIR },
	{ 7, Y_DIR },
};

const CHESS_POS chess_pos[TAG_NUMS] = {
	{ 1, 0, SOLDIER },
	{ 1, 1, SOLDIER },
	{ 1, 2, SOLDIER },
	{ 1, 3, SOLDIER },
	{ 1, 4, SOLDIER },
	{ 1, 5, SOLDIER },
	{ 1, 6, SOLDIER },
	{ 1, 7, SOLDIER },
	{ 6, 0, BLACK_SOLDIER },
	{ 6, 1, BLACK_SOLDIER },
	{ 6, 2, BLACK_SOLDIER },
	{ 6, 3, BLACK_SOLDIER },
	{ 6, 4, BLACK_SOLDIER },
	{ 6, 5, BLACK_SOLDIER },
	{ 6, 6, BLACK_SOLDIER },
	{ 6, 7, BLACK_SOLDIER },
	{ 0, 0, CAR },
	{ 0, 7, CAR },
	{ 7, 0, BLACK_CAR },
	{ 7, 7, BLACK_CAR },
	{ 0, 1, HORSE },
	{ 0, 6, HORSE },
	{ 7, 1, BLACK_HORSE },
	{ 7, 6, BLACK_HORSE },
	{ 0, 2, PHASE },
	{ 0, 5, PHASE },
	{ 7, 2, BLACK_PHASE },
	{ 7, 5, BLACK_PHASE },
	{ 0, 3, QUEEN },
	{ 7, 3, BLACK_QUEEN },
	{ 0, 4, KING },
	{ 7, 4, BLACK_KING },
};


void printf_tag(void)
{
	int i;
	int j;

	for (i = 0; i < scan_cnt; i++)
	{
		printk("uid: ");
		for (j = ISO15693_UID_LENGTH - 1; j >= 0; j--)
		{
			printk("%02x", scan_tag[i].uid[j]);
		}
		printk("   data:");
		printk(" %02x", scan_tag[i].block_code);
		printk("   xRssi = %d, yRssi = %d", scan_tag[i].xRssi, scan_tag[i].yRssi);
		printk("   x = %d, y = %d\r\n", scan_tag[i].xPos, scan_tag[i].yPos);
	}
	if (scan_cnt)
	{
		printk(" sum = %d\r\n\r\n", scan_cnt);
	}
}

void printf_uid(U8 *uid)
{
	int j;

	for (j = ISO15693_UID_LENGTH - 1; j >= 0; j--)
	{
		printk("%02x", uid[j]);
	}
	printk("\r\n");
}

void printf_msg(U8 *buf)
{
	U8 cmd;
	U8 part;
	U8 special;
	U16 step;
	U32 time;
	U8 code;
	U8 pane;
	U8 cmd_code;

	cmd = (buf[0]&0xe0)>>5;
	part = (buf[0]&0x10)>>4;
	special = (buf[0]&0x8)>>3;
	step = (buf[0]&0x07);
	step = (step<<8)|buf[1];
	time = buf[8];
	time = (time<<8)|buf[9];
	time = (time<<8)|buf[10];
	time = (time<<8)|buf[11];
	code = buf[12]>>3;
	pane = (buf[12]&0x7);
	pane = (pane<<3)|((buf[13]&0xe0)>>5);
	cmd_code = buf[13]&0x1f;
	printk("cmd:%x, part:%d, special:%d, step:%d, time:0x%08x, code:0x%x, pane:%d, cmd_code:%x\r\n", cmd, part, special, step, time, code, pane, cmd_code);
}

void printf_buf_hex(U8 *buf, int len)
{
	int i;

	for (i=0; i<len; i++)
	{
		if (i && (i%16) == 0)
		{
			printk("\r\n");
		}
		printk("%02x ", buf[i]);
	}
	printk("\r\n");
}

void InitTags(void)
{
	U16 i;

	ClearMsg(MSG_START_PLAY);
	ClearMsg(MSG_ENABLE_TIME_COUNT);
	TickCounter = 0;
	chess_steps = 0;
	WhiteIllegal = 0;
	BlackIllegal = 0;
	scan_cnt = 0;
	memset(scan_tag, 0, sizeof(scan_tag));
	memset(chess_board, 0, sizeof(chess_board));

	for (i = 0; i < 13; i++)
	{
		display_dat_buf[i] = segment_data[0];
	}
}

//选择天线
void select_x_antenna(U8 antenna_num)
{
	GPIOB->BRR  = GPIO_Pin_7;
	GPIOB->BSRR = GPIO_Pin_8;
	switch (antenna_num)
	{
		case 0: //00000
			GPIOB->BRR  = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
			break;
		case 1: //00001
			GPIOB->BSRR = GPIO_Pin_4;
			GPIOB->BRR  = GPIO_Pin_5 | GPIO_Pin_6;
			break;
		case 2: //01010
			GPIOB->BSRR = GPIO_Pin_5;
			GPIOB->BRR  = GPIO_Pin_4 | GPIO_Pin_6;
			break;
		case 3: //01011
			GPIOB->BSRR = GPIO_Pin_4 | GPIO_Pin_5;
			GPIOB->BRR  = GPIO_Pin_6;
			break;
		case 4: //01100
			GPIOB->BSRR = GPIO_Pin_6;
			GPIOB->BRR  = GPIO_Pin_4 | GPIO_Pin_5;
			break;
		case 5: //01101
			GPIOB->BSRR = GPIO_Pin_4 | GPIO_Pin_6;
			GPIOB->BRR  = GPIO_Pin_5;
			break;
		case 6: //01110
			GPIOB->BSRR = GPIO_Pin_5 | GPIO_Pin_6;
			GPIOB->BRR  = GPIO_Pin_4;
			break;
		case 7: //01111
			GPIOB->BSRR = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
			break;
	}
}

void select_y_antenna(U8 antenna_num)
{
	GPIOB->BRR  = GPIO_Pin_8;
	GPIOB->BSRR = GPIO_Pin_7;
	switch (antenna_num)
	{
		case 0: //00000
			GPIOB->BRR  = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
			break;
		case 1: //00001
			GPIOB->BSRR = GPIO_Pin_4;
			GPIOB->BRR  = GPIO_Pin_5 | GPIO_Pin_6;
			break;
		case 2: //01010
			GPIOB->BSRR = GPIO_Pin_5;
			GPIOB->BRR  = GPIO_Pin_4 | GPIO_Pin_6;
			break;
		case 3: //01011
			GPIOB->BSRR = GPIO_Pin_4 | GPIO_Pin_5;
			GPIOB->BRR  = GPIO_Pin_6;
			break;
		case 4: //01100
			GPIOB->BSRR = GPIO_Pin_6;
			GPIOB->BRR  = GPIO_Pin_4 | GPIO_Pin_5;
			break;
		case 5: //01101
			GPIOB->BSRR = GPIO_Pin_4 | GPIO_Pin_6;
			GPIOB->BRR  = GPIO_Pin_5;
			break;
		case 6: //01110
			GPIOB->BSRR = GPIO_Pin_5 | GPIO_Pin_6;
			GPIOB->BRR  = GPIO_Pin_4;
			break;
		case 7: //01111
			GPIOB->BSRR = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
			break;
	}
}

//进入观测模式
S8 as3911_enter_observation_mode(S8 mode)
{
	return as3911EnterObsvMode(mode);
}

void turn_on_off_antenna(U8 status)
{
	if (status == ANTENNA_OFF)
	{
		as3911ModifyRegister(AS3911_REG_OP_CONTROL, AS3911_REG_OP_CONTROL_tx_en,
							 0);
	}
	else
	{
		as3911ModifyRegister(AS3911_REG_OP_CONTROL, 0,
							 AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_en);
	}
}


U8 cmp_uid(U8 *uid1, U8 *uid2)
{
	U8 i;

	for (i = 0; i < ISO15693_UID_LENGTH; i++)
	{
		if (uid1[i] != uid2[i])
		{
			return 0;
		}
	}

	return 1;
}

//下面为15693相关函数
void print_cards_uid_15693(U8 cardNum, iso15693ProximityCard_t *cards)
{
	int i;
	int j;

	for (i = 0; i < cardNum; i++)
	{
		//printk("UID:\r\n");
		for (j = (ISO15693_UID_LENGTH - 1); j >= 0; j--)
		{
			printk("%02x", (U8)cards[i].uid[j]);
		}
		printk("  rssi = %d\r\n", cards[i].rssi);
	}
}

// 测量天线amplitude/phase,并打印
S8 as3911MeasureAndPrintAntenna(void)
{
	U8 result;
	int  phase, amplitude;

	as3911MeasureRF(&result);
	amplitude = (result * 1302) / 100;
	printk("amplitude: %d mv ", (int)(amplitude));

	as3911MeasureAntennaResonance(&result);
	phase = ((result * 90) / 128);
	printk("phase: %d° ", (U8)phase);


	return 0;
}

//判断天线是否有问题,天线校准时判断
int judge_antenna_question(void)
{
	int i;
	unsigned int phase[16] = {0};
	unsigned char result=0;
	unsigned int max_phase = 0;
	unsigned int min_phase = 0xffff;

	for (i = 0; i < 16; i++)
	{
    	as3911WriteRegister(AS3911_REG_ANT_CAL_CONTROL, calibrate_data[i]);
		delayNMilliSeconds(2);

		//test
		if (CheckMsg(MSG_ERROR))
		{
			printk(" CAL=%x:", calibrate_data[i]);
			as3911MeasureAndPrintAntenna();
		}
		//test

		as3911MeasureAntennaResonance(&result);
		phase[i] = ((result*90)/128 );
		if (phase[i] > max_phase)
		{
			max_phase = phase[i];
		}
		if (phase[i] < min_phase)
		{
			min_phase = phase[i];
		}
	}
	printk("[%d][%d][%d] ", max_phase, min_phase, max_phase-min_phase);

	if ((max_phase - min_phase) > 80)
	{
		for (i = 5; i < 10; i++)
		{
			printk("%d ", phase[i]);
			if (phase[i] == phase[i + 1])
			{
				display_dat_buf[W_FAULT] = segment_data[i];
				display_dat_buf[B_FAULT] = segment_data[3];
				printk("judge_antenna_question 有问题1\r\n");
				return 1;
			}
		}
		printk("OK!\r\n");
		return 0;
	}
	else
	{
		display_dat_buf[B_FAULT] = segment_data[4];
		printk("judge_antenna_question error: max_phase-min_phase = %d\r\n", max_phase - min_phase);
		return 1;
	}
}

// 校准8*8矩阵天线
int All_Antenna_Calibration(NFC_ANTENNA_INFO* pdata)
{
	U8 row, col;
	U8 result = 0;
	int tmp, phase, amplitude;
	S8 err_flag = 0;

	turn_on_off_antenna(ANTENNA_ON);

	for (row = 0; row < X_COILS; row++)
	{
		ClearMsg(MSG_ERROR);
		printk("\r\nrow:%d  ", row);

		select_x_antenna(x_order[row]);    //选天线
		as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, driver_x[row]);     //输出阻抗设置
		as3911ModifyRegister(AS3911_REG_ANT_CAL_CONTROL, AS3911_REG_ANT_CAL_CONTROL_trim_s, 0x0);
		delayNMilliSeconds(2);
		as3911CalibrateAntenna(&result);
		if (result & (1 << 3))
		{
			display_dat_buf[B_FAULT] = segment_data[1];
			printk("[trim error]");
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_FAIL;
			err_flag = 0x22;
			SetMsg(MSG_ERROR);
		}
		else
		{
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_SUCESS;
		}
		result = result >> 4;
		printk("[TRIM:%2d]", result);
		calibrate_info[x_order[row]].trim_value = result;
		/*if ((result < ANTENNA_CALIBRATE_MIN_TRIM) || (result > ANTENNA_CALIBRATE_MAX_TRIM))
		{
			SetMsg(MSG_ERROR);
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_FAIL;
			err_flag = -1;
		}*/

		result = result << 3;
		result |= (1 << 7);
		printk("[%x]", result);
		calibrate_x[x_order[row]] = result;
		pdata->calibrate_info[x_order[row]].trim_value = result;

		delayNMilliSeconds(3);
		as3911MeasureRF(&result);
		tmp = (result * 1302)/100;
		delayNMilliSeconds(1);
		as3911MeasureRF(&result);
		amplitude = (result * 1302) / 100;
		amplitude = (tmp + amplitude) >> 1;
		printk(" [%4dmv]", (int)(amplitude));
		calibrate_info[x_order[row]].amplitude = amplitude;
		pdata->calibrate_info[x_order[row]].amplitude = amplitude;
		/*if ((amplitude <= ANTENNA_CALIBRATE_MIN_VOLTAGE) || (amplitude >= ANTENNA_CALIBRATE_MAX_VOLTAGE))
		{
			SetMsg(MSG_ERROR);
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_FAIL;
			err_flag = -1;
		}*/

		as3911MeasureAntennaResonance(&result);
		phase = ((result * 90) / 128);
		printk("[%3d°] ", (U8)phase);
		calibrate_info[x_order[row]].phase = phase;
		pdata->calibrate_info[x_order[row]].phase = phase;
		/*if ((phase <= ANTENNA_CALIBRATE_MIN_PHASE) || (phase >= ANTENNA_CALIBRATE_MAX_PHASE))
		{
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_FAIL;
			err_flag = -1;
			SetMsg(MSG_ERROR);
		}*/

		/*if (judge_antenna_question())
		{
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_FAIL;
			err_flag = 1;
		}*/
	}

	for (col = 0; col < Y_COILS; col++)
	{
		ClearMsg(MSG_ERROR);
		printk("\r\ncol:%d  ", col);

		select_y_antenna(y_order[col]);    //选天线
		as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, driver_y[col]);     //输出阻抗设置
		as3911ModifyRegister(AS3911_REG_ANT_CAL_CONTROL, AS3911_REG_ANT_CAL_CONTROL_trim_s, 0x0);
		delayNMilliSeconds(2);
		as3911CalibrateAntenna(&result);
		if (result & (1 << 3))
		{
			display_dat_buf[B_FAULT] = segment_data[2];
			printk("[trim error]");
			calibrate_info[y_order[col] + X_COILS].trim_flag = NFC_TRIM_FAIL;
			err_flag = 0x22;
			SetMsg(MSG_ERROR);
		}
		else
		{
			calibrate_info[y_order[col] + X_COILS].trim_flag = NFC_TRIM_SUCESS;
		}
		result = result >> 4;
		printk("[TRIM:%2d]", result);
		calibrate_info[y_order[col] + X_COILS].trim_value = result;
		/*if ((result < ANTENNA_CALIBRATE_MIN_TRIM) || (result > ANTENNA_CALIBRATE_MAX_TRIM))
		{
			calibrate_info[y_order[col]+X_COILS].trim_flag = NFC_TRIM_FAIL;
			err_flag = -1;
			SetMsg(MSG_ERROR);
		}*/
		result = result << 3;
		result |= (1 << 7);
		printk("[%x]", result);
		calibrate_y[y_order[col]] = result;
		pdata->calibrate_info[y_order[col] + X_COILS].trim_value = result;

		delayNMilliSeconds(3);
		as3911MeasureRF(&result);
		tmp = (result * 1302) / 100;
		delayNMilliSeconds(1);
		as3911MeasureRF(&result);
		amplitude = (result * 1302)/100;
		amplitude = (tmp + amplitude) >> 1;
		printk(" [%4dmv]", (int)(amplitude));
		calibrate_info[y_order[col] + X_COILS].amplitude = amplitude;
		pdata->calibrate_info[y_order[col] + X_COILS].amplitude = amplitude;
		/*if ((amplitude <= ANTENNA_CALIBRATE_MIN_VOLTAGE) || (amplitude >= ANTENNA_CALIBRATE_MAX_VOLTAGE))
		{
			calibrate_info[y_order[col]+X_COILS].trim_flag = NFC_TRIM_FAIL;
			err_flag = -1;
			SetMsg(MSG_ERROR);
		}*/

		as3911MeasureAntennaResonance(&result);
		phase = ((result * 90) / 128);
		printk("[%3d°] ", (U8)phase);
		calibrate_info[y_order[col] + X_COILS].phase = phase;
		pdata->calibrate_info[y_order[col] + X_COILS].phase = phase;
		/*if ((phase <= ANTENNA_CALIBRATE_MIN_PHASE) || (phase >= ANTENNA_CALIBRATE_MAX_PHASE))
		{
			calibrate_info[y_order[col] + X_COILS].trim_flag = NFC_TRIM_FAIL;
			err_flag = -1;
			SetMsg(MSG_ERROR);
		}*/

		/*if (judge_antenna_question())
		{
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_FAIL;
			err_flag = 1;
		}*/
	}
	turn_on_off_antenna(ANTENNA_OFF);

	if (err_flag == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}


int confirm_antenna_status(void)
{
	int i;
	unsigned char result;
	unsigned int phase[16] = {0};
	unsigned int max_phase = 0;
	unsigned int min_phase = 0xffff;

	for (i = 0; i < 16; i++)
	{
    	as3911WriteRegister(AS3911_REG_ANT_CAL_CONTROL, calibrate_data[i]);
		delayNMilliSeconds(2);
		as3911MeasureAntennaResonance(&result);
		phase[i] = ((result*90)/128);
		if (phase[i] > max_phase)
		{
			max_phase = phase[i];
		}
		if (phase[i] < min_phase)
		{
			min_phase = phase[i];
		}
	}
	if ((max_phase - min_phase) < 50)
	{
		printk("confirm_antenna_status 有问题 %d-%d=%d\n", max_phase, min_phase, max_phase - min_phase);
		return 1;
	}

	return 0;
}

//判断天线是否异常
int judge_antenna_status(void)
{
	int row, col;
	unsigned char result=0;

	printk("%s\n", __FUNCTION__);
	turn_on_off_antenna(ANTENNA_ON);

	for (row = 0; row < X_COILS; row++)
    {
		printk("row:%d  ", row);
        select_x_antenna(row);    //选天线
		as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, driver_x[row]);     //输出阻抗设置
		as3911ModifyRegister(AS3911_REG_ANT_CAL_CONTROL, AS3911_REG_ANT_CAL_CONTROL_trim_s, 0x0);
		delayNMilliSeconds(2);
		as3911CalibrateAntenna(&result);
		if (result & (1 << 3))
		{
			printk("[trim error]");
			if (confirm_antenna_status())
			{
				turn_on_off_antenna(ANTENNA_OFF);
				return 1;
			}
		}
    }

    for (col = 0; col < Y_COILS; col++)
    {
		printk("col:%d  ", col);
        select_y_antenna(col);    //选天线
		as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, driver_y[col]);     //输出阻抗设置
		as3911ModifyRegister(AS3911_REG_ANT_CAL_CONTROL, AS3911_REG_ANT_CAL_CONTROL_trim_s, 0x0);
		delayNMilliSeconds(2);

		as3911CalibrateAntenna(&result);
		if (result & (1 << 3))
		{
			printk("[trim error]");
			if (confirm_antenna_status())
			{
				turn_on_off_antenna(ANTENNA_OFF);
				return 1;
			}
		}
    }

	turn_on_off_antenna(ANTENNA_OFF);

	return 0;
}


// 校准8*8矩阵天线
int AntennaCalibrationOld(void)
{
	int row, col;
	U8 result = 0;
	int phase, amplitude;
	S8 err_flag = 0;

	turn_on_off_antenna(ANTENNA_ON);
	as3911ModifyRegister(AS3911_REG_ANT_CAL_CONTROL, AS3911_REG_ANT_CAL_CONTROL_trim_s, 0x0);

	as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, RFO_AM_OFF_LEVEL_X);     //输出阻抗设置
	for (row = 0; row < X_COILS; row++)
	{
		printk("row:%d  ", row);

		select_x_antenna(x_order[row]);    //选天线
		delayNMilliSeconds(1);

		as3911CalibrateAntenna(&result);
		if (result & (1 << 3))
		{
			printk("[trim error]");
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_FAIL;
			err_flag = 0x22;
		}
		else
		{
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_SUCESS;
		}
		result = result >> 4;
		printk("[TRIM:%2d]", result);
		calibrate_info[x_order[row]].trim_value = result;
		result = result << 3;
		result |= (1 << 7);
		printk("[%x]", result);
		calibrate_x[x_order[row]] = result;

		as3911MeasureRF(&result);
		amplitude = (result * 1302) / 100;
		printk(" [%4dmv]", (int)(amplitude));
		calibrate_info[x_order[row]].amplitude = amplitude;
		if (amplitude <= 1000)
		{
			calibrate_info[x_order[row]].trim_flag = NFC_TRIM_FAIL;
		}

		as3911MeasureAntennaResonance(&result);
		phase = ((result * 90) / 128);
		printk("[%3d°]\r\n", (U8)phase);
		calibrate_info[x_order[row]].phase = phase;
	}

	//as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, RFO_AM_OFF_LEVEL_Y);     //输出阻抗设置
	for (col = 0; col < Y_COILS; col++)
	{
		printk("col:%d  ", col);

		select_y_antenna(y_order[col]);    //选天线
		delayNMilliSeconds(1);

		as3911CalibrateAntenna(&result);
		if (result & (1 << 3))
		{
			printk("[trim error]");
			calibrate_info[y_order[col] + X_COILS].trim_flag = NFC_TRIM_FAIL;
			err_flag = 0x22;
		}
		else
		{
			calibrate_info[y_order[col] + X_COILS].trim_flag = NFC_TRIM_SUCESS;
		}
		result = result >> 4;
		printk("[TRIM:%2d]", result);
		calibrate_info[y_order[col] + X_COILS].trim_value = result;
		result = result << 3;
		result |= (1 << 7);
		printk("[%x]", result);
		calibrate_y[y_order[col]] = result;

		as3911MeasureRF(&result);
		amplitude = (result * 1302) / 100;
		printk(" [%4dmv]", (int)(amplitude));
		calibrate_info[y_order[col] + X_COILS].amplitude = amplitude;
		if (amplitude <= 1000)
		{
			calibrate_info[y_order[col]].trim_flag = NFC_TRIM_FAIL;
		}

		as3911MeasureAntennaResonance(&result);
		phase = ((result * 90) / 128);
		printk("[%3d°]\r\n", (U8)phase);
		calibrate_info[y_order[col] + X_COILS].phase = phase;
	}
	turn_on_off_antenna(ANTENNA_OFF);

	if (err_flag == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void LoadCaliBateProcess(void)
{
	U8 buff[256];
	U8 i;
	NFC_ANTENNA_INFO *pdata = (NFC_ANTENNA_INFO*)buff;

#if 1
	SPI_Flash_Read(buff, ANTENNA_INFO_OFFSET, sizeof(NFC_ANTENNA_INFO));
	if (pdata->NFC_CHECK_RESULT == NFC_CALIBRATE_SUCESS_FLAG)
	{
		for (i = 0; i < X_COILS; i++)
		{
			calibrate_x[i] = pdata->calibrate_info[i].trim_value;
		}
		for (i = 0; i < Y_COILS; i++)
		{
			calibrate_y[i] = pdata->calibrate_info[i + X_COILS].trim_value;
		}
		if (judge_antenna_status())
		{
			display_dat_buf[W_MIN_HUN] = segment_data[3];
			printk("天线已经校准通过了 天线有问题!!!\n");
		}
		else
		{
			printk("天线已经校准通过了 天线OK!\n");
			chess_mode = CHESSMODE_IDLE;
		}
	}
	else
	{
		if (0 != All_Antenna_Calibration(pdata))
		{
			display_dat_buf[W_MIN_HUN] = segment_data[2];
			printk("!!! Antenna Calibration Error, Please Check The Antenna !!!\r\n");
			OSTimeDlyHMSM(0, 0, 0, 300);
		}
		else
		{
			pdata->NFC_CHECK_RESULT = NFC_CALIBRATE_SUCESS_FLAG;
			SPI_Flash_Erase_Sector(ANTENNA_INFO_OFFSET / SPI_BYTES_PER_SECTOR);
			SPI_Flash_Write_Page(buff, ANTENNA_INFO_OFFSET, sizeof(NFC_ANTENNA_INFO));
			chess_mode = CHESSMODE_IDLE;
		}
	}
	OSTimeDlyHMSM(0, 0, 0, 100);
#else
	printk("@@@@@@@@@@@@ AntennaCalibrationOld\r\n");
	AntennaCalibrationOld();
	chess_mode = CHESSMODE_IDLE;
#endif
}


extern volatile U32 as3911InterruptStatus;
extern volatile U32 as3911InterruptStatusToWaitFor;

U32 as3911WaitForInterruptsTimed(U32 mask, U16 tmo)
{
	U32 status;
	//S8 err = 0;

	icDisableInterrupt();
	as3911InterruptStatusToWaitFor = mask;
	status = as3911InterruptStatus & mask;
	if (status != 0)
	{
		// 中断已经发生过了，因此 OS_FLAG 不会再被置位，直接返回
		as3911InterruptStatus &= ~status;
		as3911InterruptStatusToWaitFor = 0;
		icEnableInterrupt();

		return status;
	}
	icEnableInterrupt();

	// 清除 NFC OS FLAG
	//OSFlagPost(g_pNFC_OS_Flag, NFC_OS_FLAG_TIMEOUT, OS_FLAG_CLR, &err);

	// 启动计时器
	delayNMilliSeconds(500);
	// 等待 NFC 中断发生或者计时器超时中断发生

	//printk("----111---------g_pNFC_OS_Flag\r\n");
	//OSFlagPend(g_pNFC_OS_Flag, NFC_OS_FLAG_INTERRUPT | NFC_OS_FLAG_TIMEOUT, OS_FLAG_WAIT_SET_ANY /*| OS_FLAG_CONSUME*/, 0, &err);
	//printk("-----222--------g_pNFC_OS_Flag\r\n");

	status = as3911InterruptStatus & mask;
	// 清除等待并已经发生的中断
	icDisableInterrupt();
	as3911InterruptStatus &= ~status;
	as3911InterruptStatusToWaitFor = 0;
	icEnableInterrupt();

	// 清除 NFC OS FLAG
	//OSFlagPost(g_pNFC_OS_Flag, NFC_OS_FLAG_INTERRUPT | NFC_OS_FLAG_TIMEOUT, OS_FLAG_CLR, &err);


	return status;
}

static const U8 EncryptKey[] =
{
	0xD6, 0xC7, 0xBB, 0xDB,
	0xBB, 0xFD, 0xC4, 0xBE,
	0xBF, 0xEC, 0xC0, 0xD6,
	0xCE, 0xDE, 0xC7, 0xEE
};

void DataEncodeDecode(U8 *pInput, U8 *pOutput, unsigned int Length)
{
	int i, j;

	if (!pInput || !pOutput || Length == 0)
		return;

	j = 0;
	for (i = 0; i < Length; i++)
	{
		pOutput[i] = pInput[i] ^ EncryptKey[j];
		j++;
		if (j >= sizeof(EncryptKey))
			j = 0;
	}
}

S8 iso15693ReadBlocks(iso15693ProximityCard_t *card, U8 *pData, U8 startBlock, U8 blockCount, U8 *pErrCode)
{
	S8 err;
	U8 i;
	U16 byteOffset = 0;
	iso15693PiccMemoryBlock_t memBlock;

	if (pErrCode)
	{
		*pErrCode = 0;
	}
	memBlock.blocknr = startBlock;
	for (i = 0; i < blockCount; i++)
	{
		err = iso15693ReadSingleBlock(card, &memBlock);
		if (err != ERR_NONE)
		{
			break;
		}
		if (memBlock.flags & ISO15693_RESP_FLAG_ERROR)
		{
			if (pErrCode)
			{
				*pErrCode = memBlock.errorCode;
			}
			err = ERR_IO;
			break;
		}

		memcpy(&pData[byteOffset], memBlock.data, memBlock.actualSize);
		byteOffset += memBlock.actualSize;
		memBlock.blocknr++;
	}

	return err;
}

S8 iso15693WriteBlocks(iso15693ProximityCard_t *card, U8 flags, U8 *pData, U8 startBlock, U8 blockCount, U8 blockLen)
{
	S8 err;
	U8 i;
	U16 byteOffset = 0;
	iso15693PiccMemoryBlock_t memBlock;

	memBlock.blocknr = startBlock;
	memBlock.actualSize = blockLen;
	for (i = 0; i < blockCount; i++)
	{
		memcpy(memBlock.data, &pData[byteOffset], memBlock.actualSize);
		err = iso15693WriteSingleBlock(card, flags, &memBlock);
		if (err != ERR_NONE)
		{
			printk("iso15693WriteBlocks err, i = %d\r\n", i);
			break;
		}
		else
		{
			//printk("iso15693WriteBlocks ok, i = %d\r\n", i);
		}

		byteOffset += memBlock.actualSize;
		memBlock.blocknr++;
	}

	return err;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

U8 find_tag_uid(iso15693ProximityCard_t *card_temp)
{
	int i;

	for (i = 0; i < scan_cnt; i++)
	{
		if (cmp_uid(card_temp->uid, scan_tag[i].uid))
		{
			return i;
		}
	}
	return 0xff; //改UID没有记录过数据
}

int save_tag_data_by_rssi(iso15693ProximityCard_t *card_temp, int tag_num, U8 pos, U8 dir)
{
	int i, j, err;
	U8 block_code;
	U8 errorCode;
	U8 cnt;
	U8 flag;

	for (i = 0; i < tag_num; i++)
	{
		cnt = find_tag_uid(&card_temp[i]);
		if (0xff == cnt)    //if have not been save, then save it
		{
			U8 block_data[32];

			err = iso15693ReadBlocks(&card_temp[i], block_data, 0, 1, &errorCode);
			if (ERR_NONE != err)
			{
				err = iso15693ReadBlocks(&card_temp[i], block_data, 0, 1, &errorCode);
			}
			if (ERR_NONE == err && 0 == errorCode)
			{
				//DataEncodeDecode(block_data, block_data, 8);
				//checksum = crcCalculateCcitt(0xffff, card_temp[i].uid, 8);
				//checksum = crcCalculateCcitt(checksum, block_data, 2);
				//checksum2 = ((U16)block_data[2] << 8) | block_data[3];
				//printk("checksum = %02x, checksum2 = %02x\r\n", checksum, checksum2);
				//printk("bld data:[%x][%x][%x][%x][%x][%x][%x][%x]\r\n", block_data[0], block_data[1], block_data[2], block_data[3],block_data[4], block_data[5], block_data[6], block_data[7]);
				if (block_data[0] == block_data[1] && block_data[0] < 0x20)
				{
					block_code = block_data[0];
					flag = 1;
				}
				else if (chess_mode == CHESSMODE_IDLE)    //把cpuid上传到服务器id库
				{
					U16 checksum = block_data[0];
					checksum = (checksum << 8) | block_data[1];
					if (checksum == crcCalculateCcitt(0xffff, card_temp[i].uid, 8))
					{
						SetMsg(FLAG_SERVER_CPUID);
					}
				}
			}
			else
			{
				printk("ReadBlocks err = %d, errorCode = %d\r\n", err, errorCode);
			}

			if (flag)
			{
				if (scan_cnt < TAG_NUMS + 8)
				{
					//printk("scan_cnt = %d\r\n", scan_cnt);
					memcpy(scan_tag[scan_cnt].uid, card_temp[i].uid, ISO15693_UID_LENGTH);
					scan_tag[scan_cnt].block_code = block_code;
					if (dir == X_DIR)
					{
						scan_tag[scan_cnt].xPos = pos;
						scan_tag[scan_cnt].xRssi = card_temp[i].rssi;
						scan_tag[scan_cnt].xCnt = press_cnt;
					}
					else
					{
						scan_tag[scan_cnt].yPos = pos;
						scan_tag[scan_cnt].yRssi = card_temp[i].rssi;
						scan_tag[scan_cnt].yCnt = press_cnt;
					}
					scan_tag[scan_cnt].cnt = press_cnt;
					scan_cnt++;
				}
				else
				{
					for (j = 0; j < scan_cnt; j++)
					{
						if (scan_tag[j].cnt != press_cnt)
						{
							memcpy(scan_tag[j].uid, card_temp[i].uid, ISO15693_UID_LENGTH);
							scan_tag[j].block_code = block_code;
							if (dir == X_DIR)
							{
								scan_tag[j].xPos = pos;
								scan_tag[j].xRssi = card_temp[i].rssi;
								scan_tag[j].xCnt = press_cnt;
							}
							else
							{
								scan_tag[j].yPos = pos;
								scan_tag[j].yRssi = card_temp[i].rssi;
								scan_tag[j].yCnt = press_cnt;
							}
							scan_tag[j].cnt = press_cnt;
							break;
						}
					}
				}
			}
			else
			{
				printk("bld data checksum error\r\n");
			}
		}
		else    //compare the rssi signal, save the more strong one
		{
			if (dir == X_DIR)
			{
				if (scan_tag[cnt].xCnt != press_cnt)
				{
					scan_tag[cnt].xPos = pos;
					scan_tag[cnt].xRssi = card_temp[i].rssi;
				}
				else if (card_temp[i].rssi > scan_tag[cnt].xRssi)	//card_temp[i].rssi > 4
				{
					scan_tag[cnt].xPos = pos;
					scan_tag[cnt].xRssi = card_temp[i].rssi;
				}
				scan_tag[cnt].xCnt = press_cnt;
			}
			else
			{
				if (scan_tag[cnt].yCnt != press_cnt)
				{
					scan_tag[cnt].yPos = pos;
					scan_tag[cnt].yRssi = card_temp[i].rssi;
				}
				else if (card_temp[i].rssi > scan_tag[cnt].yRssi)	//card_temp[i].rssi > 4
				{
					scan_tag[cnt].yPos = pos;
					scan_tag[cnt].yRssi = card_temp[i].rssi;
				}
				scan_tag[cnt].yCnt = press_cnt;
			}
			scan_tag[cnt].cnt = press_cnt;
		}
	}
	return 0;
}

U8 read_antenna(iso15693ProximityCard_t *card_temp, U8 line, U8 dir) //读取一个天线上的标签
{
	U8 actcnt = 0;  //接收到的标签个数
	S8 err;

	if ((dir == X_DIR && line >= X_COILS) || (dir == Y_DIR && line >= Y_COILS))
	{
		return actcnt;
	}

	if (dir == X_DIR)
	{
		as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, driver_x[line]);     //输出阻抗设置
		select_x_antenna(x_order[line]);    //<lfx
		as3911WriteRegister(AS3911_REG_ANT_CAL_CONTROL, calibrate_x[x_order[line]]); //写入天线校准值0x90 calibrate_y[1]
	}
	else
	{
		as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, driver_y[line]);     //输出阻抗设置
		select_y_antenna(y_order[line]);    //<lfx
		as3911WriteRegister(AS3911_REG_ANT_CAL_CONTROL, calibrate_y[y_order[line]]); //写入天线校准值0x90 calibrate_y[1]
	}
	//as3911MeasureAndPrintAntenna();

	turn_on_off_antenna(ANTENNA_ON); //打开天线
	delayNMicroSeconds(500);

	iso15693PhyInitialize(&iso15693_config);

	err = iso15693Inventory(ISO15693_NUM_SLOTS_1, 0, NULL, card_temp, ONE_ANTENNA_TAG_NUMS, &actcnt);

	save_tag_data_by_rssi(card_temp, actcnt, line, dir);

	iso15693PhyDeinitialize(0);
	//turn_on_off_antenna(ANTENNA_OFF); //关闭天线
	//delayNMicroSeconds(500);

	return actcnt;
}

U8 HandleScan(U8 checkkey)
{
	int i;
	U8 card_cnt;
	U8 retval = 1;
	iso15693ProximityCard_t card_temp[ONE_ANTENNA_TAG_NUMS];

	press_cnt++;
	for (i = 0; i < X_COILS + Y_COILS; i++)
	{
		if (checkkey && CheckMsg(checkkey))
		{
			retval = 0;
			break;
		}

		card_cnt = read_antenna(card_temp, chess_scan[i].line, chess_scan[i].dir);
		//print_cards_uid_15693(card_cnt, card_temp);
		//printk("cardsNum:[%d]\r\n\r\n", card_cnt);
	}
	printf_tag();

	return retval;
}

U8 write_tag_block_code(iso15693ProximityCard_t *card_temp, int tag_num, U8 block_code)
{
	int i, err;
	U8 errorCode;
	U8 block_data[32];
	U8 flag;
	U16 checksum;
	U8 write_sum;

	write_sum = 0;
	for (i = 0; i < tag_num; i++)
	{
		flag = 0;
		/*err = iso15693ReadBlocks(&card_temp[i], block_data, 0, 1, &errorCode);
		if (ERR_NONE != err)
		{
			err = iso15693ReadBlocks(&card_temp[i], block_data, 0, 1, &errorCode);
		}
		if (ERR_NONE == err && 0 == errorCode)
		{*/
			if (block_code == 0xff)
			{
				checksum == crcCalculateCcitt(0xffff, card_temp[i].uid, 8);
				block_data[0] = checksum >> 8;
				block_data[1] = checksum;
				flag = 1;
			}
			else if (block_data[0] != block_data[1] || block_data[0] != block_code)
			{
				block_data[0] = block_code;
				block_data[1] = block_code;
				flag = 1;
			}
			if (flag)
			{
				err = iso15693WriteBlocks(&card_temp[i], ISO15693_REQ_FLAG_ADDRESS, block_data, 0, 1, 4);
				if (ERR_NONE != err)
				{
					err = iso15693WriteBlocks(&card_temp[i], ISO15693_REQ_FLAG_ADDRESS, block_data, 0, 1, 4);
				}
				if (ERR_NONE != err)
				{
					printk("write err=%d\n", err);
				}
				else
				{
					write_sum++;
					printk("write ok\n");
				}
			}
		/*}
		else
		{
			printk("ReadBlocks err = %d, errorCode = %d\r\n", err, errorCode);
		}*/
	}

	return write_sum;
}

U8 write_antenna_block_code(iso15693ProximityCard_t *card_temp, U8 line, U8 dir, U8 block_code) //写一个天线上的标签
{
	U8 actcnt = 0;  //接收到的标签个数
	S8 err;
	U8 write_sum;

	if ((dir == X_DIR && line >= X_COILS) || (dir == Y_DIR && line >= Y_COILS))
	{
		return actcnt;
	}

	if (dir == X_DIR)
	{
		as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, driver_x[line]);     //输出阻抗设置
		select_x_antenna(x_order[line]);    //<lfx
		as3911WriteRegister(AS3911_REG_ANT_CAL_CONTROL, calibrate_x[x_order[line]]); //写入天线校准值0x90 calibrate_y[1]
	}
	else
	{
		as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, driver_y[line]);     //输出阻抗设置
		select_y_antenna(y_order[line]);    //<lfx
		as3911WriteRegister(AS3911_REG_ANT_CAL_CONTROL, calibrate_y[y_order[line]]); //写入天线校准值0x90 calibrate_y[1]
	}
	//as3911MeasureAndPrintAntenna();

	turn_on_off_antenna(ANTENNA_ON); //打开天线
	delayNMicroSeconds(500);

	iso15693PhyInitialize(&iso15693_config);

	err = iso15693Inventory(ISO15693_NUM_SLOTS_1, 0, NULL, card_temp, ONE_ANTENNA_TAG_NUMS, &actcnt);

	write_sum = write_tag_block_code(card_temp, actcnt, block_code);

	iso15693PhyDeinitialize(0);
	//turn_on_off_antenna(ANTENNA_OFF); //关闭天线
	//delayNMicroSeconds(500);

	return write_sum;
}

U8 WriteChess(U8 block_code)
{
	int i;
	U8 card_cnt = 0;
	U8 retval = 1;
	iso15693ProximityCard_t card_temp[ONE_ANTENNA_TAG_NUMS];

	for (i = 0; i < X_COILS; i++)
	{
		card_cnt += write_antenna_block_code(card_temp, chess_scan[i].line, chess_scan[i].dir, block_code);
		//print_cards_uid_15693(card_cnt, card_temp);
		//printk("cardsNum:[%d]\r\n\r\n", card_cnt);
	}
	//printf_tag();

	return card_cnt;
}

void HandleSaveProcess(U8 *msg_buff)
{
	U32 addr;
	U16 steps;

	steps = msg_buff[0] & 0x07;
	steps <<= 8;
	steps |= msg_buff[1];

	if (cfg.curr_game < MAX_GAME_CNT && steps < MAX_STEPS_PER_GAME)
	{
		addr = (U32)cfg.curr_game * BYTES_PER_GAME + steps * BYTES_PER_STEP;
		if ((addr % SPI_BYTES_PER_SECTOR) == 0)
		{
			SPI_Flash_Erase_Sector(addr / SPI_BYTES_PER_SECTOR);
		}
		if (steps == 0)
		{
			cfg.game_steps_cnt[cfg.curr_game] = 0;
		}
		SPI_Flash_Write_Page(msg_buff, addr, BYTES_PER_STEP);
		//SPI_Flash_Read(buff, addr, BYTES_PER_STEP);
		//printk("spi save step addr = 0x%x\r\n", addr);
		/*for (i = 0; i < BYTES_PER_STEP; i++)
		{
			printk("%02x ", buff[i]);
		}
		printk("\r\n");*/
		printk("  SAVE: ");
		printf_buf_hex(msg_buff, BYTES_PER_STEP);
		cfg.game_steps_cnt[cfg.curr_game] += 1;
		chess_steps++;
	}
	else
	{
		//show error
		printk("save error curr_game = %d, steps = %d\r\n", cfg.curr_game, steps);
	}
}

void HandleFactoryProcess(void)
{
	if (CheckMsg(KEY_LONGLONG_CAR) && CheckMsg(KEY_LONGLONG_HORSE) && CheckMsg(KEY_LONGLONG_PHASE) && CheckMsg(KEY_LONGLONG_QUEEN))	//factory default
	{
		ClearMsg(KEY_LONGLONG_CAR);
		ClearMsg(KEY_LONGLONG_HORSE);
		ClearMsg(KEY_LONGLONG_PHASE);
		ClearMsg(KEY_LONGLONG_QUEEN);
		factory_default();
		power_off();
	}
	if (key_car_cnt == 0 && key_queen_cnt == 0 && CheckMsg(KEY_LONGLONG_HORSE) && CheckMsg(KEY_LONGLONG_PHASE))	//smartlink
	{
		ClearMsg(KEY_LONGLONG_HORSE);
		ClearMsg(KEY_LONGLONG_PHASE);
		SetMsg(MSG_ENTER_SMARTLINK);
	}
}

U8 read_game_step(U32 game_no, U32 step_no, U8* step_buff)
{
	if (game_no < MAX_GAME_CNT && step_no < cfg.game_steps_cnt[game_no])
	{
		U32 addr = game_no*BYTES_PER_GAME + step_no*BYTES_PER_STEP;
		SPI_Flash_Read(step_buff, addr, BYTES_PER_STEP);
		printk(" FETCH: ");
		printf_buf_hex(step_buff, BYTES_PER_STEP);
		return 1;
	}
	return 0;
}

S8 SendMessageStep0(U8 cmd, U32 time_stamp, U8 ret, U16 read_time, U8 add_time, U8 extend)
{
	U8 send_msg[BYTES_PER_STEP];
	int i = 0;
	int j;
	U16 step = chess_steps;

	send_msg[i++] = ((cmd << 5) & 0xe0) | ((step >> 8) & 0x07);
	send_msg[i++] = (step & 0xff);
	for (j=0; j<sizeof(IMEI); j++)
	{
		send_msg[i++] = IMEI[j];
	}
	send_msg[i++] = (time_stamp >> 24) & 0xff;
	send_msg[i++] = (time_stamp >> 16) & 0xff;
	send_msg[i++] = (time_stamp >> 8) & 0xff;
	send_msg[i++] = (time_stamp & 0xff);
	send_msg[i++] = (ret << 7) | ((read_time >> 3) & 0x7f);
	send_msg[i++] = ((read_time << 5) & 0xe0) | (add_time & 0x1f);
	send_msg[i++] = extend;
	send_msg[i++] = calc_sum(send_msg, BYTES_PER_STEP-1);

	HandleSaveProcess(send_msg);

	return 0;
}

S8 SendMessage(U8 cmd, U8 press_part, U8 special, U32 time_stamp, U8 chess_code, U8 pane_code, U8 cmd_code, U8 extend)
{
	U8 send_msg[BYTES_PER_STEP];
	int i = 0;
	int j;
	U16 step = chess_steps;

	SetMsg(MSG_SEND_MOVE);
	send_msg[i++] = ((cmd << 5) & 0xe0) | (press_part<<4) | (special<<3) | ((step >> 8) & 0x07);
	send_msg[i++] = (step & 0xff);
	for (j=0; j<sizeof(IMEI); j++)
	{
		send_msg[i++] = IMEI[j];
	}
	send_msg[i++] = (time_stamp >> 24) & 0xff;
	send_msg[i++] = (time_stamp >> 16) & 0xff;
	send_msg[i++] = (time_stamp >> 8) & 0xff;
	send_msg[i++] = (time_stamp & 0xff);
	send_msg[i++] = (chess_code << 3) | ((pane_code >> 3) & 0x7);
	send_msg[i++] = (pane_code << 5) | (cmd_code & 0x1f);
	send_msg[i++] = extend;
	send_msg[i++] = calc_sum(send_msg, BYTES_PER_STEP-1);

	HandleSaveProcess(send_msg);

	return 0;
}

DELAY_MSG delay_msg;
void set_delay_msg(U8 cmd, U8 press_part, U8 special, U32 time_stamp, U8 chess_code, U8 pane_code, U8 cmd_code, U8 extend, U8 no)
{
	delay_msg.cmd = cmd;
	delay_msg.press_part = press_part;
	delay_msg.special = special;
	delay_msg.time = time_stamp;
	delay_msg.chess_code = chess_code;
	delay_msg.pane_code = pane_code;
	delay_msg.cmd_code = cmd_code;
	delay_msg.no = no;
}

void SendDelayMessage(U8 extent)
{
	SendMessage(delay_msg.cmd, delay_msg.press_part, delay_msg.special, delay_msg.time, delay_msg.chess_code, delay_msg.pane_code, extent, 0);
	scan_tag[delay_msg.no].rise_code = extent;
}

void BlackMove(void)
{
	U8 i;

	for (i=0; i<scan_cnt; i++)
	{
		if (scan_tag[i].block_code >= 0x10)
		{
			if (scan_tag[i].cnt == press_cnt)
			{
				if (chess_board[scan_tag[i].yPos][scan_tag[i].xPos] != chess_board_old[scan_tag[i].yPos][scan_tag[i].xPos])
				{
					if (scan_tag[i].miss == 1)
					{
						SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, 0, 0);
						scan_tag[i].miss = 0;
					}
					else
					{
						SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, 0, 0);
					}
					if (scan_tag[i].block_code == BLACK_SOLDIER && scan_tag[i].rise_code == 0)
					{
						if (scan_tag[i].yOld == 1 && scan_tag[i].yPos == 0)
						{
							printk("BLACK_SOLDIER rising...\r\n");
							if (GetMsg(KEY_RISING))
							{
								if (rise_key == KEY_LONG_CAR)
								{
									SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_RISETOCAR, 0);
									scan_tag[i].rise_code = CMD_CODE_RISETOCAR;
									printk("rise to car\r\n");
								}
								if (rise_key == KEY_LONG_HORSE)
								{
									SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_RISETOHORSE, 0);
									scan_tag[i].rise_code = CMD_CODE_RISETOHORSE;
									printk("rise to horse\r\n");
								}
								if (rise_key == KEY_LONG_PHASE)
								{
									SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_RISETOPHASE, 0);
									scan_tag[i].rise_code = CMD_CODE_RISETOPHASE;
									printk("rise to phase\r\n");
								}
								if (rise_key == KEY_LONG_QUEEN)
								{
									SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_RISETOQUEEN, 0);
									scan_tag[i].rise_code = CMD_CODE_RISETOQUEEN;
									printk("rise to queen\r\n");
								}
							}
							else
							{
								set_delay_msg(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, 0, 0, i);
								WarnChessRising();
								SetMsg(MSG_DELAY_SEND);
							}
						}
					}
					scan_tag[i].xOld = scan_tag[i].xPos;
					scan_tag[i].yOld = scan_tag[i].yPos;
				}
			}
			else
			{
				if (scan_tag[i].miss == 0)
				{
					SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_BEINGEAT, 0);
					scan_tag[i].miss = 1;
				}
			}
		}
	}
}

void WhiteMove(void)
{
	U8 i;

	for (i=0; i<scan_cnt; i++)
	{
		if (scan_tag[i].block_code < 0x10)
		{
			if (scan_tag[i].cnt == press_cnt)
			{
				if (chess_board[scan_tag[i].yPos][scan_tag[i].xPos] != chess_board_old[scan_tag[i].yPos][scan_tag[i].xPos])
				{
					if (scan_tag[i].miss == 1)
					{
						SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, 0, 0);
						scan_tag[i].miss = 0;
					}
					else
					{
						SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, 0, 0);
					}
					if (scan_tag[i].block_code == SOLDIER && scan_tag[i].rise_code == 0)
					{
						if (scan_tag[i].yOld == 6 && scan_tag[i].yPos == 7)
						{
							printk("WHITE_SOLDIER rising...\r\n");
							if (GetMsg(KEY_RISING))
							{
								if (rise_key == KEY_LONG_CAR)
								{
									SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_RISETOCAR, 0);
									scan_tag[i].rise_code = CMD_CODE_RISETOCAR;
									printk("rise to car\r\n");
								}
								if (rise_key == KEY_LONG_HORSE)
								{
									SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_RISETOHORSE, 0);
									scan_tag[i].rise_code = CMD_CODE_RISETOHORSE;
									printk("rise to horse\r\n");
								}
								if (rise_key == KEY_LONG_PHASE)
								{
									SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_RISETOPHASE, 0);
									scan_tag[i].rise_code = CMD_CODE_RISETOPHASE;
									printk("rise to phase\r\n");
								}
								if (rise_key == KEY_LONG_QUEEN)
								{
									SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_RISETOQUEEN, 0);
									scan_tag[i].rise_code = CMD_CODE_RISETOQUEEN;
									printk("rise to queen\r\n");
								}
							}
							else
							{
								set_delay_msg(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, 0, 0, i);
								WarnChessRising();
								SetMsg(MSG_DELAY_SEND);
							}
						}
					}
					scan_tag[i].xOld = scan_tag[i].xPos;
					scan_tag[i].yOld = scan_tag[i].yPos;
				}
			}
			else
			{
				if (scan_tag[i].miss == 0)
				{
					SendMessage(CMD_DATA, play_part, 0, unix_time_tmp, scan_tag[i].chess_code, 8*scan_tag[i].yPos+scan_tag[i].xPos, CMD_CODE_BEINGEAT, 0);
					scan_tag[i].miss = 1;
				}
			}
		}
	}
}

void HandleChessMove(void)
{
	U8 i;

	unix_time_tmp = UnixTime;
	memcpy(&chess_board_old, chess_board, sizeof(chess_board));
	memset(chess_board, 0, sizeof(chess_board));
	for (i=0; i < scan_cnt; i++)
	{
		if (scan_tag[i].cnt == press_cnt)
		{
			if (0 == chess_board[scan_tag[i].yPos][scan_tag[i].xPos])
			{
				chess_board[scan_tag[i].yPos][scan_tag[i].xPos] = &scan_tag[i];
			}
			else
			{
				//here handle error
				printk("error y = %d, x = %d\r\n", scan_tag[i].yPos, scan_tag[i].xPos);
				printf_uid(chess_board[scan_tag[i].yPos][scan_tag[i].xPos]->uid);
				printf_uid(scan_tag[i].uid);
			}
		}
	}

	if (CheckMsg(MSG_BLACK_PLAYING))
	{
		BlackMove();
		WhiteMove();
	}
	else
	{
		WhiteMove();
		BlackMove();
	}
}

S8 CheckChessPosition(U8 *error_x, U8 *error_y)
{
	U8 i;

	memset(chess_board, 0, sizeof(chess_board));
	for (i = 0; i < scan_cnt; i++)
	{
		if (scan_tag[i].xRssi && scan_tag[i].yRssi && scan_tag[i].cnt == press_cnt)
		{
			if (0 == chess_board[scan_tag[i].yPos][scan_tag[i].xPos])
			{
				chess_board[scan_tag[i].yPos][scan_tag[i].xPos] = &scan_tag[i];
			}
			else    //here handle error
			{
				printk("xPos = %d, yPos = %d ", scan_tag[i].xPos, scan_tag[i].yPos);
				printf_uid(chess_board[scan_tag[i].yPos][scan_tag[i].xPos]->uid);
				printf_uid(scan_tag[i].uid);
				*error_y = scan_tag[i].yPos;
				*error_x = scan_tag[i].xPos;
				return -1;
			}
		}
	}

	//check chess position
	for (i = 0; i < TAG_NUMS; i++)
	{
		if (chess_board[chess_pos[i].y][chess_pos[i].x] == 0 || chess_board[chess_pos[i].y][chess_pos[i].x]->block_code != chess_pos[i].code)
		{
			*error_y = chess_pos[i].y;
			*error_x = chess_pos[i].x;
			return -3;
		}
	}

	return 0;
}

void ClearChessKeyMsg(void)
{
	ClearMsg(KEY_SHORT_CAR);
	ClearMsg(KEY_SHORT_HORSE);
	ClearMsg(KEY_SHORT_PHASE);
	ClearMsg(KEY_SHORT_QUEEN);
}

void ClearLongKeyMsg(void)
{
	ClearMsg(KEY_LONG_CAR);
	ClearMsg(KEY_LONG_HORSE);
	ClearMsg(KEY_LONG_PHASE);
	ClearMsg(KEY_LONG_QUEEN);
}

void AssignChessCode(void)
{
	int i;
	int j;
	U8 soldier = 0;
	U8 car = 0;
	U8 horse = 0;
	U8 phase = 0;
	U8 b_soldier = 0;
	U8 b_car = 0;
	U8 b_horse = 0;
	U8 b_phase = 0;
	U8 block_code;

	for (j = 0; j < Y_COILS; j++)
	{
		for (i = 0; i < X_COILS; i++)
		{
			if (chess_board[j][i])
			{
				block_code = chess_board[j][i]->block_code;
				if (block_code == SOLDIER)
				{
					chess_board[j][i]->chess_code = block_code+soldier;
					soldier++;
				}
				else if (block_code == BLACK_SOLDIER)
				{
					chess_board[j][i]->chess_code = block_code+b_soldier;
					b_soldier++;
				}
				else if (block_code == CAR)
				{
					chess_board[j][i]->chess_code = block_code+car;
					car++;
				}
				else if (block_code == BLACK_CAR)
				{
					chess_board[j][i]->chess_code = block_code+b_car;
					b_car++;
				}
				else if (block_code == HORSE)
				{
					chess_board[j][i]->chess_code = block_code+horse;
					horse++;
				}
				else if (block_code == BLACK_HORSE)
				{
					chess_board[j][i]->chess_code = block_code+b_horse;
					b_horse++;
				}
				else if (block_code == PHASE)
				{
					chess_board[j][i]->chess_code = block_code+phase;
					phase++;
				}
				else if (block_code == BLACK_PHASE)
				{
					chess_board[j][i]->chess_code = block_code+b_phase;
					b_phase++;
				}
				else	//king, queen
				{
					chess_board[j][i]->chess_code = block_code;
				}
			}
		}
	}
}

void HandleTime(void)
{
	if (CheckMsg(MSG_START_PLAY))
	{
		if (CheckMsg(MSG_TIME_RUNNING))
		{
			TickCounter++;
			if (TickCounter >= 1000)
			{
				U8  b_illegal;
				U8  w_illegal;
				U16 b_time;
				U16 w_time;
				U16 minite;
				U16 second;

				TickCounter = 0;

				if (CheckMsg(MSG_ENABLE_TIME_COUNT))
				{
					if (CheckMsg(MSG_BLACK_DECREASE))
					{
						if (BlackTime)
						{
							BlackTime--;
						}
					}
					else
					{
						if (WhiteTime)
						{
							WhiteTime--;
						}
					}
					b_time = BlackTime;
					w_time = WhiteTime;
					b_illegal = BlackIllegal;
					w_illegal = WhiteIllegal;
				}
				else
				{
					if (CheckMsg(MSG_BLACK_DECREASE))
					{
						BlackTime++;
					}
					else
					{
						WhiteTime++;
					}
					b_time = BlackTime;
					w_time = WhiteTime;
					b_illegal = BlackIllegal;
					w_illegal = WhiteIllegal;
				}

				display_dat_buf[B_FAULT] = segment_data[b_illegal%10];
				display_dat_buf[W_FAULT] = segment_data[w_illegal%10];

				minite = w_time/60;
				second = w_time%60;
				display_dat_buf[W_MIN_HUN] = segment_data[minite/100];
				display_dat_buf[W_MIN_TEN] = segment_data[minite%100/10];
				display_dat_buf[W_MIN_ONE] = segment_data[minite%10] | 0x80;
				display_dat_buf[W_SEC_TEN] = segment_data[second/10];
				display_dat_buf[W_SEC_ONE] = segment_data[second%10];

				minite = b_time/60;
				second = b_time%60;
				display_dat_buf[B_MIN_HUN] = segment_data[minite/100];
				display_dat_buf[B_MIN_TEN] = segment_data[minite%100/10];
				display_dat_buf[B_MIN_ONE] = segment_data[minite%10] | 0x80;
				display_dat_buf[B_SEC_TEN] = segment_data[second/10];
				display_dat_buf[B_SEC_ONE] = segment_data[second%10];
			}
		}
	}
	if (WhiteShineCounter)
	{
		WhiteShineCounter--;
		if (0 == WhiteShineCounter)
		{
			white_led = LED_OFF;
		}
	}
	if (BlackShineCounter)
	{
		BlackShineCounter--;
		if (0 == BlackShineCounter)
		{
			black_led = LED_OFF;
		}
	}
	if (StatusCounter)
	{
		StatusCounter--;
		if (0 == StatusCounter)
		{
			if (display_dat_buf[M_STATE] == segment_data[5] || display_dat_buf[M_STATE] == segment_data[8])
			{
				display_dat_buf[M_STATE] = 0;
				led_flash_flag &= ~(1 << M_STATE);
			}
		}
	}
	if (CheckMsg(MSG_PASSTIME_COUNT))
	{
		PassMilliCounter++;
		if (PassMilliCounter >= 1000)
		{
			PassMilliCounter = 0;
			PassTime++;
		}
	}

	if (NetSendCounter)
	{
		NetSendCounter--;
		if (0 == NetSendCounter)
		{
			state_led = state_led_bak;
		}
		else
		{
			state_led = LED_FAST_SHINE;
		}
	}

	if (NetworkTimeOutCounter)
	{
		NetworkTimeOutCounter--;
		if (0 == NetworkTimeOutCounter)
		{
			if (state_led == LED_ON)
			{
				state_led = LED_SLOW_SHINE;
			}
		}
	}
}

void IdleModeProcess(void)
{
	static U8 self_test_key = 0;

	OSTimeDlyHMSM(0, 0, 0, 10);
	if (key_car_cnt == KEY_LONGLONG_CNT && key_white_cnt == 0 && key_black_cnt == 0)
	{
		if (display_dat_buf[M_STATE] != segment_data[0xa]
			&& display_dat_buf[M_STATE] != segment_data[0xb]
			&& display_dat_buf[M_STATE] != segment_data[1])
		{
			display_dat_buf[M_STATE] = segment_data[0xf];
		}
		if (key_horse_cnt > KEY_SHORT_CNT && key_phase_cnt == 0 && key_queen_cnt == 0)
		{
			self_test_key = 1;
			printk("self_test_key = %d\r\n", self_test_key);
			display_dat_buf[M_STATE] = segment_data[0xa];
		}
		else if (self_test_key == 1 && key_horse_cnt == 0 && key_phase_cnt > KEY_SHORT_CNT && key_queen_cnt == 0)
		{
			self_test_key = 2;
			printk("self_test_key = %d\r\n", self_test_key);
			display_dat_buf[M_STATE] = segment_data[0xb];
		}
		else if (self_test_key == 2 && key_horse_cnt == 0 && key_phase_cnt == 0 && key_queen_cnt > KEY_SHORT_CNT)
		{
			int i;
			for (i = 0; i < 13; i++)
			{
				display_dat_buf[i] = 0;
			}
			led_flash_flag = 0;
			white_led = LED_OFF;
			black_led = LED_OFF;
			state_led = LED_OFF;
			self_test_key = 3;
			chess_mode = CHESSMODE_SELF_TEST;
			SelfTestMode = 0;
			display_dat_buf[M_STATE] = segment_data[SelfTestMode+1];
			SetMsg(FLAG_SELF_TEST);
			ClearMsg(KEY_LONG_CAR);
			printk("Enter ### SelfTestProcess ###\r\n");
		}
	}
	else
	{
		self_test_key = 0;
	}
	if (0 == CheckMsg(FLAG_SYS_REGISTER))
	{
		HandleScan(0);
		OSTimeDlyHMSM(0, 0, 0, 100);
		//printk("Please register !\r\n");
		return;
	}

	ClearMsg(MSG_PASSTIME_COUNT);

	if (GetMsg(KEY_SHORT_BLACK))
	{
		printk("key black short\r\n");
		StartErrorCnt = 0;
		chess_mode = CHESSMODE_PREPARE_TO_PLAY;
		white_led = LED_OFF;
		black_led = LED_OFF;
	}
	if (GetMsg(KEY_LONG_BLACK))
	{
		chess_mode = CHESSMODE_TIME_SETTING;
		ClearMsg(MSG_START_PLAY);
		TimeSettingInit();

		//SetMsg(MSG_DISPLAY_UPDATE);
		printk("CHESSMODE_TIME_SETTING\r\n");
	}

	if (GetMsg(KEY_SHORT_WHITE))
	{
		printk("key white short\r\n");
		StartErrorCnt = 0;
	}
}

void TimeSettingInit(void)
{
	U16 minite;

	white_led = LED_OFF;
	black_led = LED_OFF;

	ReadTime = 100 * 60;
	AddTime = 10;
	SetMsg(MSG_ENABLE_TIME_COUNT);

	SetReadTimeHun = ReadTime/60/100;
	SetReadTimeTen = ReadTime/60%100/10;
	SetReadTimeOne = ReadTime/60%10;
	SetAddTimeTen = AddTime/10;
	SetAddTimeOne = AddTime%10;
	TimeSetting = 0;
	led_flash_flag = (1<<B_SEC_TEN) | (1<<B_SEC_ONE);
	display_dat_buf[W_FAULT] = 0;
	display_dat_buf[B_FAULT] = 0;

	minite = ReadTime/60;
	display_dat_buf[W_MIN_HUN] = segment_data[minite/100];
	display_dat_buf[W_MIN_TEN] = segment_data[minite%100/10];
	display_dat_buf[W_MIN_ONE] = segment_data[minite%10] | 0x80;
	display_dat_buf[W_SEC_TEN] = segment_data[0];
	display_dat_buf[W_SEC_ONE] = segment_data[0];

	display_dat_buf[B_MIN_HUN] = 0;
	display_dat_buf[B_MIN_TEN] = 0;
	display_dat_buf[B_MIN_ONE] = 0;
	display_dat_buf[B_SEC_TEN] = segment_data[AddTime / 10];
	display_dat_buf[B_SEC_ONE] = segment_data[AddTime % 10];
}

void TimeSettingProcess(void)
{
	if (GetMsg(KEY_SHORT_QUEEN))
	{
		if (TimeSetting)
		{
			TimeSetting = 0;
			led_flash_flag = (1<<B_SEC_TEN) | (1<<B_SEC_ONE);
		}
		else
		{
			TimeSetting = 1;
			led_flash_flag = (1<<W_MIN_HUN) | (1<<W_MIN_TEN) | (1<<W_MIN_ONE);
		}
	}

	if (GetMsg(KEY_SHORT_PHASE))
	{
		if (TimeSetting)
		{
			SetReadTimeHun = (SetReadTimeHun+1)%10;
			SetMsg(MSG_DISPLAY_UPDATE);
		}
	}
	if (GetMsg(KEY_SHORT_HORSE))
	{
		if (TimeSetting)
		{
			SetReadTimeTen = (SetReadTimeTen+1)%10;
		}
		else
		{
			if (SetAddTimeTen >= 3)
			{
				SetAddTimeTen = 0;
			}
			else
			{
				SetAddTimeTen = (SetAddTimeTen+1)%10;
			}
			if (SetAddTimeTen >= 3)
			{
				SetAddTimeOne = 0;
			}
		}
		SetMsg(MSG_DISPLAY_UPDATE);
	}
	if (GetMsg(KEY_SHORT_CAR))
	{
		if (TimeSetting)
		{
			SetReadTimeOne = (SetReadTimeOne+1)%10;
		}
		else
		{
			if (SetAddTimeTen < 3)
			{
				SetAddTimeOne = (SetAddTimeOne + 1) % 10;
			}
		}
		SetMsg(MSG_DISPLAY_UPDATE);
	}

	if (GetMsg(MSG_DISPLAY_UPDATE))
	{
		display_dat_buf[W_FAULT] = 0;
		display_dat_buf[W_MIN_HUN] = segment_data[SetReadTimeHun];
		display_dat_buf[W_MIN_TEN] = segment_data[SetReadTimeTen];
		display_dat_buf[W_MIN_ONE] = segment_data[SetReadTimeOne] | 0x80;
		display_dat_buf[W_SEC_TEN] = segment_data[0];
		display_dat_buf[W_SEC_ONE] = segment_data[0];

		display_dat_buf[B_FAULT] = 0;
		display_dat_buf[B_MIN_HUN] = 0;
		display_dat_buf[B_MIN_TEN] = 0;
		display_dat_buf[B_MIN_ONE] = 0;
		display_dat_buf[B_SEC_TEN] = segment_data[SetAddTimeTen];
		display_dat_buf[B_SEC_ONE] = segment_data[SetAddTimeOne];
	}

	if (GetMsg(KEY_SHORT_BLACK))
	{
		ReadTime = ((U16)SetReadTimeHun*100 + (U16)SetReadTimeTen*10 + (U16)SetReadTimeOne)*60;
		AddTime = SetAddTimeTen*10 + SetAddTimeOne;
		WhiteTime = ReadTime;
		BlackTime = ReadTime;
		led_flash_flag = 0;
		SetMsg(MSG_DISPLAY_UPDATE);
		chess_mode = CHESSMODE_IDLE;
		printk("CHESSMODE_IDLE\r\n");
		if (ReadTime == 0)
		{
			ClearMsg(MSG_ENABLE_TIME_COUNT);
		}
	}
	if (GetMsg(KEY_LONG_BLACK))
	{
		ReadTime = ((U16)SetReadTimeHun*100 + (U16)SetReadTimeTen*10 + (U16)SetReadTimeOne)*60;
		AddTime = SetAddTimeTen*10 + SetAddTimeOne;
		WhiteTime = ReadTime;
		BlackTime = ReadTime;
		led_flash_flag = 0;
		SetMsg(MSG_DISPLAY_UPDATE);
		chess_mode = CHESSMODE_IDLE;
		printk("CHESSMODE_IDLE\r\n");
		if (ReadTime == 0)
		{
			AddTime = 0;
			ClearMsg(MSG_ENABLE_TIME_COUNT);
		}
	}
	if (GetMsg(KEY_SHORT_WHITE))
	{
	}

	if (GetMsg(MSG_DISPLAY_UPDATE))
	{
		U16 minite;
		U16 second;

		display_dat_buf[W_FAULT] = 0;
		display_dat_buf[B_FAULT] = 0;

		minite = WhiteTime/60;
		second = WhiteTime%60;
		display_dat_buf[W_MIN_HUN] = segment_data[minite/100];
		display_dat_buf[W_MIN_TEN] = segment_data[minite%100/10];
		display_dat_buf[W_MIN_ONE] = segment_data[minite%10] | 0x80;
		display_dat_buf[W_SEC_TEN] = segment_data[second/10];
		display_dat_buf[W_SEC_ONE] = segment_data[second%10];

		minite = BlackTime/60;
		second = BlackTime%60;
		display_dat_buf[B_MIN_HUN] = segment_data[minite/100];
		display_dat_buf[B_MIN_TEN] = segment_data[minite%100/10];
		display_dat_buf[B_MIN_ONE] = segment_data[minite%10] | 0x80;
		display_dat_buf[B_SEC_TEN] = segment_data[second/10];
		display_dat_buf[B_SEC_ONE] = segment_data[second%10];
	}

	OSTimeDlyHMSM(0, 0, 0, 50);
}

void PrepareToPlay()
{
	S8 error;
	U8 error_x = 0, error_y = 0;

	HandleScan(0);
	printf_tag();
	error = CheckChessPosition(&error_x, &error_y);
	//printk("CheckChessPosition error = %d\r\n", error);
	if (scan_cnt == TAG_NUMS && 0 == error)
	{
		//display...
		WhiteTime = ReadTime;
		BlackTime = ReadTime;

		display_dat_buf[M_STATE] = 0;
		led_flash_flag = 0;
		TickCounter = 0;
		chess_steps = 0;
		WhiteIllegal = 0;
		BlackIllegal = 0;
		ClearMsg(MSG_PAUSE);
		ClearMsg(MSG_DELAY_SEND);
		ClearMsg(MSG_BLACK_PLAYING);
		ClearMsg(MSG_BLACK_DECREASE);

		ClearChessKeyMsg();
		ClearLongKeyMsg();
		CheckMsg(KEY_RISING);
		if (StartErrorCnt < 2)
		{
			printk("start playing...\r\n");
			AssignChessCode();
			if (cfg.curr_game < MAX_GAME_CNT-1)
			{
				cfg.curr_game += 1;
			}
			else
			{
				cfg.curr_game = 0;
			}
			chess_mode = CHESSMODE_PLAYING;
			SetMsg(MSG_START_PLAY);
			SetMsg(MSG_PLAYING_FLAG);
			SetMsg(MSG_TIME_RUNNING);
			SendMessageStep0(1, UnixTime, 1, ReadTime / 60, AddTime, 0);
			PassMilliCounter = 0;
			PassTime = 0;
			SetMsg(MSG_PASSTIME_COUNT);
		}
		else
		{
			display_dat_buf[W_FAULT] = segment_data[0];
			display_dat_buf[B_FAULT] = segment_data[0];
			white_led = LED_OFF;
			black_led = LED_OFF;
			chess_mode = CHESSMODE_IDLE;
			printk("go back to idle\r\n");
		}
	}
	else
	{
		//Error Warning
		led_flash_flag = (1 << W_FAULT) + (1 << M_STATE) + (1 << B_FAULT);
		display_dat_buf[M_STATE] = segment_data[0x0e];
		display_dat_buf[W_FAULT] = segment_data[error_x];
		display_dat_buf[B_FAULT] = segment_data[error_y];
		StartErrorCnt++;
		white_led = LED_FAST_SHINE;
		black_led = LED_FAST_SHINE;
	}

	if (GetMsg(KEY_SHORT_WHITE))
	{
	}
	if (GetMsg(KEY_SHORT_BLACK))
	{
	}
	if (GetMsg(KEY_LONG_BLACK))
	{
		chess_mode = CHESSMODE_TIME_SETTING;
		TimeSettingInit();
		printk("key back to idle\r\n");
	}
}

void EnterPauseStatus(void)
{
	ClearMsg(MSG_PASSTIME_COUNT);
	SetMsg(MSG_PAUSE);
	ClearMsg(MSG_TIME_RUNNING);
	display_dat_buf[M_STATE] = segment_P;
	led_flash_flag = (1 << M_STATE);
	if (CheckMsg(MSG_BLACK_PLAYING))
	{
		led_flash_flag |= (1 << B_MIN_HUN) | (1 << B_MIN_TEN) | (1 << B_MIN_ONE) | (1 << B_SEC_TEN) | (1 << B_SEC_ONE);
	}
	else
	{
		led_flash_flag |= (1 << W_MIN_HUN) | (1 << W_MIN_TEN) | (1 << W_MIN_ONE) | (1 << W_SEC_TEN) | (1 << W_SEC_ONE);
	}
}

void ExitPauseStatus(void)
{
	SetMsg(MSG_PASSTIME_COUNT);
	ClearMsg(MSG_PAUSE);
	SetMsg(MSG_TIME_RUNNING);
	display_dat_buf[M_STATE] = 0;
	led_flash_flag &= ~((1 << M_STATE));
	led_flash_flag &= ~((1 << B_MIN_HUN) | (1 << B_MIN_TEN) | (1 << B_MIN_ONE) | (1 << B_SEC_TEN) | (1 << B_SEC_ONE));
	led_flash_flag &= ~((1 << W_MIN_HUN) | (1 << W_MIN_TEN) | (1 << W_MIN_ONE) | (1 << W_SEC_TEN) | (1 << W_SEC_ONE));
}

void KeyChessRising(U8 key)
{
	if (chess_mode == CHESSMODE_PLAYING)
	{
		display_dat_buf[M_STATE] = segment_data[5];
		led_flash_flag |= (1 << M_STATE);
		StatusCounter = LED_SHINE_2S;
		rise_key = key;
		SetMsg(KEY_RISING);
	}
}

void WarnChessRising(void)
{
	display_dat_buf[M_STATE] = segment_data[8];
	led_flash_flag |= (1 << M_STATE);
	StatusCounter = LED_SHINE_2S;

	WhiteShineCounter = LED_SHINE_2S;
	white_led = LED_FAST_SHINE;
	BlackShineCounter = LED_SHINE_2S;
	black_led = LED_FAST_SHINE;
}

void PlayModeProcess(void)
{
	U8 retval;

	if (CheckMsg(MSG_DELAY_SEND))
	{
		if (GetMsg(KEY_RISING))
		{
			if (rise_key == KEY_LONG_CAR)
			{
				SendDelayMessage(CMD_CODE_RISETOCAR);
				ClearMsg(MSG_DELAY_SEND);
				printk("rise to car\r\n");
			}
			if (rise_key == KEY_LONG_HORSE)
			{
				SendDelayMessage(CMD_CODE_RISETOHORSE);
				ClearMsg(MSG_DELAY_SEND);
				printk("rise to horse\r\n");
			}
			if (rise_key == KEY_LONG_PHASE)
			{
				SendDelayMessage(CMD_CODE_RISETOPHASE);
				ClearMsg(MSG_DELAY_SEND);
				printk("rise to phase\r\n");
			}
			if (rise_key == KEY_LONG_QUEEN)
			{
				SendDelayMessage(CMD_CODE_RISETOQUEEN);
				ClearMsg(MSG_DELAY_SEND);
				printk("rise to queen\r\n");
			}
		}
		if (CheckMsg(MSG_DELAY_SEND) && (GetMsg(KEY_SHORT_HORSE) || GetMsg(KEY_SHORT_PHASE)
			|| GetMsg(KEY_SHORT_BLACK) || GetMsg(KEY_SHORT_WHITE)))
		{
			WarnChessRising();
		}
	}
	else
	{
		if (GetMsg(KEY_SHORT_WHITE))
		{
			if (0 == CheckMsg(MSG_PAUSE))
			{
				if (CheckMsg(MSG_BLACK_DECREASE) != 0)
				{
					WhiteShineCounter = LED_SHINE_2S;
					white_led = LED_FAST_SHINE;
					BlackShineCounter = LED_SHINE_2S;
					black_led = LED_FAST_SHINE;
				}
				else
				{
					ClearMsg(MSG_SEND_MOVE);
					ClearMsg(MSG_BLACK_PLAYING);
					play_part = 0;
					PassMilliCounter = 0;
					PassTime = 0;
					SetMsg(MSG_PASSTIME_COUNT);
					white_led = LED_ON;
					retval = HandleScan(KEY_SHORT_BLACK);	//只有黑方能中断白方的扫描
					white_led = LED_OFF;
					if (retval)
					{
						HandleChessMove();
						if (0 == GetMsg(MSG_SEND_MOVE))
						{
							SendMessage(CMD_DATA, play_part, 1, UnixTime, 0, 0, 0, 0);
						}
					}
					else
					{
						SendMessage(CMD_DATA, play_part, 1, UnixTime, 0, 0, CMD_CODE_SCANNOTCOMPLETE, 0);
					}

					if (0 == CheckMsg(MSG_DELAY_SEND))	//兵到底没升变
					{
						ClearMsg(MSG_TIME_RUNNING);
						TickCounter = 0;
						WhiteTime += AddTime;
						SetMsg(MSG_BLACK_DECREASE);
						SetMsg(MSG_TIME_RUNNING);

						SetMsg(MSG_BLACK_PLAYING);
					}
				}
				ClearLongKeyMsg();
				CheckMsg(KEY_RISING);
			}
			else
			{
				if (WhiteIllegal < 9)
				{
					WhiteIllegal += 1;
				}
				if (CheckMsg(MSG_BLACK_DECREASE) == 1)
				{
					if (CheckMsg(MSG_ENABLE_TIME_COUNT))
					{
						if (WhiteTime >= PassTime)
						{
							WhiteTime -= PassTime;
						}
						else
						{
							WhiteTime = 0;
						}
						BlackTime += PassTime;
					}
					else
					{
						WhiteTime += PassTime;
						if (BlackTime >= PassTime)
						{
							BlackTime -= PassTime;
						}
						else
						{
							BlackTime = 0;
						}
					}
					ClearMsg(MSG_BLACK_DECREASE);
					ClearMsg(MSG_BLACK_PLAYING);
				}
				ExitPauseStatus();
				SendMessage(CMD_DATA, 0, 1, UnixTime, 0, 0, CMD_CODE_ILLEGAL, 0);
			}
			ClearMsg(KEY_SHORT_WHITE);
		}

		if (GetMsg(KEY_SHORT_BLACK))
		{
			if (0 == CheckMsg(MSG_PAUSE))
			{
				if (CheckMsg(MSG_BLACK_DECREASE) != 1)
				{
					WhiteShineCounter = LED_SHINE_2S;
					white_led = LED_FAST_SHINE;
					BlackShineCounter = LED_SHINE_2S;
					black_led = LED_FAST_SHINE;
				}
				else
				{
					ClearMsg(MSG_SEND_MOVE);
					SetMsg(MSG_BLACK_PLAYING);
					play_part = 1;
					PassMilliCounter = 0;
					PassTime = 0;
					SetMsg(MSG_PASSTIME_COUNT);
					black_led = LED_ON;
					retval = HandleScan(KEY_SHORT_WHITE);	//只有白方能中断黑方的扫描
					black_led = LED_OFF;
					if (retval)
					{
						HandleChessMove();
						if (0 == GetMsg(MSG_SEND_MOVE))
						{
							SendMessage(CMD_DATA, play_part, 1, UnixTime, 0, 0, 0, 0);
						}
					}
					else
					{
						SendMessage(CMD_DATA, play_part, 1, UnixTime, 0, 0, CMD_CODE_SCANNOTCOMPLETE, 0);
					}

					if (0 == CheckMsg(MSG_DELAY_SEND))	//兵到底没升变
					{
						ClearMsg(MSG_TIME_RUNNING);
						TickCounter = 0;
						BlackTime += AddTime;
						ClearMsg(MSG_BLACK_DECREASE);
						SetMsg(MSG_TIME_RUNNING);

						ClearMsg(MSG_BLACK_PLAYING);
					}
				}
				ClearLongKeyMsg();
				CheckMsg(KEY_RISING);
			}
			else
			{
				if (BlackIllegal < 9)
				{
					BlackIllegal += 1;
				}
				if (CheckMsg(MSG_BLACK_DECREASE) == 0)
				{
					if (CheckMsg(MSG_ENABLE_TIME_COUNT))
					{
						if (BlackTime >= PassTime)
						{
							BlackTime -= PassTime;
						}
						else
						{
							BlackTime = 0;
						}
						WhiteTime += PassTime;
					}
					else
					{
						BlackTime += PassTime;
						if (WhiteTime >= PassTime)
						{
							WhiteTime -= PassTime;
						}
						else
						{
							WhiteTime = 0;
						}
					}
					SetMsg(MSG_BLACK_DECREASE);
					SetMsg(MSG_BLACK_PLAYING);
				}
				ExitPauseStatus();
				SendMessage(CMD_DATA, 1, 1, UnixTime, 0, 0, CMD_CODE_ILLEGAL, 0);
			}
		}
		ClearMsg(KEY_SHORT_BLACK);
	}

	if (CheckMsg(MSG_PAUSE))
	{
		if (GetMsg(KEY_SHORT_QUEEN))
		{
			ExitPauseStatus();
			SendMessage(CMD_DATA, 0, 1, UnixTime, 0, 0, CMD_CODE_CONTINUE, 0);
		}
		if (GetMsg(KEY_SHORT_CAR))
		{
			ExitPauseStatus();
			SendMessage(CMD_DATA, 1, 1, UnixTime, 0, 0, CMD_CODE_CONTINUE, 0);
		}
	}
	else
	{
		if (GetMsg(KEY_SHORT_QUEEN))
		{
			EnterPauseStatus();
			SendMessage(CMD_DATA, 0, 1, UnixTime, 0, 0, CMD_CODE_PAUSE, 0);
		}
		if (GetMsg(KEY_SHORT_CAR))
		{
			EnterPauseStatus();
			SendMessage(CMD_DATA, 1, 1, UnixTime, 0, 0, CMD_CODE_PAUSE, 0);
		}
	}

	if (GetMsg(KEY_LONG_BLACK) || GetMsg(KEY_LONG_WHITE) || CheckMsg(KEY_LONGLONG_WHITE))
	{
		SendMessage(CMD_DATA, 1, 0, UnixTime, 0, 0, 0x1c, 0);

		WhiteTime = ReadTime;
		BlackTime = ReadTime;

		display_dat_buf[M_STATE] = segment_data[0];
		display_dat_buf[W_FAULT] = segment_data[0];
		display_dat_buf[B_FAULT] = segment_data[0];

		display_dat_buf[W_MIN_HUN] = segment_data[0];
		display_dat_buf[W_MIN_TEN] = segment_data[0];
		display_dat_buf[W_MIN_ONE] = segment_data[0];
		display_dat_buf[W_SEC_TEN] = segment_data[0];
		display_dat_buf[W_SEC_ONE] = segment_data[0];

		display_dat_buf[B_MIN_HUN] = segment_data[0];
		display_dat_buf[B_MIN_TEN] = segment_data[0];
		display_dat_buf[B_MIN_ONE] = segment_data[0];
		display_dat_buf[B_SEC_TEN] = segment_data[0];
		display_dat_buf[B_SEC_ONE] = segment_data[0];

		led_flash_flag = 0;
		chess_mode = CHESSMODE_IDLE;
		ClearMsg(MSG_START_PLAY);
		ClearMsg(MSG_TIME_RUNNING);
		ClearMsg(MSG_DELAY_SEND);
		ClearMsg(MSG_PAUSE);
		SysinfoSave();
	}

	OSTimeDlyHMSM(0, 0, 0, 10);
}

void SelfTestProcess(void)
{
	int i;

	display_dat_buf[M_STATE] = segment_data[SelfTestMode+1];

	if (SelfTestMode == 0)	//按键
	{
		if (key_white_cnt > KEY_SHORT_CNT)
		{
			display_dat_buf[W_FAULT] = segment_data[0];
		}
		else
		{
			display_dat_buf[W_FAULT] = 0;
		}
		if (key_queen_cnt > KEY_SHORT_CNT)
		{
			display_dat_buf[W_MIN_HUN] = segment_data[0];
		}
		else
		{
			display_dat_buf[W_MIN_HUN] = 0;
		}
		if (key_phase_cnt > KEY_SHORT_CNT)
		{
			display_dat_buf[W_SEC_ONE] = segment_data[0];
		}
		else
		{
			display_dat_buf[W_SEC_ONE] = 0;
		}
		if (key_horse_cnt > KEY_SHORT_CNT)
		{
			display_dat_buf[B_MIN_HUN] = segment_data[0];
		}
		else
		{
			display_dat_buf[B_MIN_HUN] = 0;
		}
		if (key_car_cnt > KEY_SHORT_CNT)
		{
			display_dat_buf[B_SEC_ONE] = segment_data[0];
		}
		else
		{
			display_dat_buf[B_SEC_ONE] = 0;
		}
		if (key_black_cnt > KEY_SHORT_CNT)
		{
			display_dat_buf[B_FAULT] = segment_data[0];
		}
		else
		{
			display_dat_buf[B_FAULT] = 0;
		}
	}
	else if (SelfTestMode == 1)	//显示
	{
		for (i = 0; i < 13; i++)
		{
			display_dat_buf[i] = segment_data[8];
		}
		white_led = LED_SLOW_SHINE;
		black_led = LED_SLOW_SHINE;
		state_led = LED_SLOW_SHINE;
		led_flash_flag = 0x1fff;
	}
	else if (SelfTestMode == 2)	//电量
	{
		unsigned char percent = bat_percent;
		unsigned short voltage = bat_vol;

		display_dat_buf[W_MIN_HUN] = segment_data[voltage/1000];
		display_dat_buf[W_MIN_TEN] = segment_data[voltage%1000/100];
		display_dat_buf[W_MIN_ONE] = segment_data[voltage%100/10];
		display_dat_buf[W_SEC_TEN] = segment_data[voltage%10];
		display_dat_buf[W_SEC_ONE] = 0;
		if (percent == 100)
		{
			display_dat_buf[B_MIN_HUN] = segment_data[1];
			display_dat_buf[B_MIN_TEN] = segment_data[0];
			display_dat_buf[B_MIN_ONE] = segment_data[0];
		}
		else
		{
			display_dat_buf[B_MIN_HUN] = 0;
			display_dat_buf[B_MIN_TEN] = segment_data[percent/10];
			display_dat_buf[B_MIN_ONE] = segment_data[percent%10];
		}
	}
	else if (SelfTestMode == 3)	//wifi
	{
		if (CheckMsg(MSG_WIFI_SSID_GOTIP))
		{
			display_dat_buf[B_MIN_HUN] = 0x6f;
			display_dat_buf[B_MIN_TEN] = 0x5c;
			display_dat_buf[B_MIN_ONE] = 0x31;
			display_dat_buf[B_SEC_TEN] = segment_data[1];
			display_dat_buf[B_SEC_ONE] = segment_P;
		}
	}
	else if (SelfTestMode == 4)	//天线
	{
		char flag = 0;
		HandleScan(0);
		for (i = 0; i < scan_cnt; i++)
		{
			if (scan_tag[i].xRssi && scan_tag[i].yRssi && scan_tag[i].cnt == press_cnt)
			{
				flag = 1;
				display_dat_buf[W_FAULT] = segment_data[scan_tag[i].xPos];
				display_dat_buf[B_FAULT] = segment_data[scan_tag[i].yPos];
				display_dat_buf[W_SEC_TEN] = segment_data[scan_tag[i].xRssi/10];
				display_dat_buf[W_SEC_ONE] = segment_data[scan_tag[i].xRssi%10];
				display_dat_buf[B_MIN_HUN] = segment_data[scan_tag[i].yRssi/10];
				display_dat_buf[B_MIN_TEN] = segment_data[scan_tag[i].yRssi%10];
			}
		}
		if (flag == 0)
		{
			display_dat_buf[W_FAULT] = 0;
			display_dat_buf[B_FAULT] = 0;
			display_dat_buf[W_SEC_TEN] = 0;
			display_dat_buf[W_SEC_ONE] = 0;
			display_dat_buf[B_MIN_HUN] = 0;
			display_dat_buf[B_MIN_TEN] = 0;
		}
	}
	if (GetMsg(KEY_LONG_BLACK))
	{
		for (i = 0; i < 13; i++)
		{
			display_dat_buf[i] = segment_data[0];
		}
		chess_mode = CHESSMODE_IDLE;
		return;
	}
	if (GetMsg(KEY_LONG_CAR))
	{
		SelfTestMode += 1;
		if (SelfTestMode > 4)
		{
			SelfTestMode = 0;
		}
		for (i = 0; i < 13; i++)
		{
			display_dat_buf[i] = 0;
		}
		led_flash_flag = 0;
		white_led = LED_OFF;
		black_led = LED_OFF;
		state_led = LED_OFF;
		display_dat_buf[M_STATE] = segment_data[SelfTestMode+1];
	}
	OSTimeDlyHMSM(0, 0, 0, 100);
}

void power_off(void)
{
	printk("power_off\r\n");
	white_led = LED_FAST_SHINE;
	black_led = LED_FAST_SHINE;
	WhiteShineCounter = LED_SHINE_2S;
	BlackShineCounter = LED_SHINE_2S;
	//add power off code
	display_dat_buf[M_STATE] = 0;
	display_dat_buf[W_FAULT] = 0;
	display_dat_buf[B_FAULT] = 0;

	display_dat_buf[W_MIN_HUN] = 0;
	display_dat_buf[W_MIN_TEN] = 0;
	display_dat_buf[W_MIN_ONE] = 0;
	display_dat_buf[W_SEC_TEN] = 0;
	display_dat_buf[W_SEC_ONE] = 0;

	display_dat_buf[B_MIN_HUN] = 0;
	display_dat_buf[B_MIN_TEN] = 0;
	display_dat_buf[B_MIN_ONE] = 0;
	display_dat_buf[B_SEC_TEN] = 0;
	display_dat_buf[B_SEC_ONE] = 0;

	sys_power_cmd(0);
	OSTimeDlyHMSM(0, 0, 0, 500);
	OSTimeDlyHMSM(0, 0, 0, 500);
	OSTimeDlyHMSM(0, 0, 0, 500);
	OSTimeDlyHMSM(0, 0, 0, 500);
	led_flash_flag = 0;
	white_led = LED_OFF;
	black_led = LED_OFF;
	state_led = LED_OFF;
}

void NfcHandlerTask(void)
{
	printk("\r\n\r\n<<<<<<<<<<<<<< 进入NFC任务 >>>>>>>>>>>>>>>>>>>>>\r\n");
	printk("编译日期：");
	printk(__DATE__);
	printk("\r\n编译时间：");
	printk(__TIME__);
	printk("\r\n");

	//两侧指示灯长亮，表示棋局未开始
	white_led = LED_ON;
	black_led = LED_ON;

	//InitTags();
	NFC_INT_init(); //-----
	as3911ClearInterrupts();
	as3911Initialize();
	#if OBSERVATION_MODE_DEBUG
	as3911_enter_observation_mode(OBSERVATION_MODE_1);
	#endif
	#if	ANTANNA_DRIVER_SINGLE
	as3911ModifyRegister(AS3911_REG_IO_CONF1, 0x0, AS3911_REG_IO_CONF1_single);
	#endif

	/*{
		U8 sum;

		sum = WriteChess(QUEEN);
		printk("Write sum = %d\r\n", sum);
	}
	while (1)
	{
		HandleScan(0);
		OSTimeDlyHMSM(0, 0, 2, 0);
	}*/
	chess_mode = CHESSMODE_CALIBRATE;
	while (1)
	{
		switch (chess_mode)
		{
			case CHESSMODE_CALIBRATE:
				LoadCaliBateProcess();
				break;
			case CHESSMODE_IDLE:
				IdleModeProcess();
				break;
			case CHESSMODE_TIME_SETTING:
				TimeSettingProcess();
				break;
			case CHESSMODE_PREPARE_TO_PLAY:
				PrepareToPlay();
				break;
			case CHESSMODE_PLAYING:
				PlayModeProcess();
				break;
			case CHESSMODE_SELF_TEST:
				SelfTestProcess();
				break;
			default:
				display_dat_buf[W_MIN_HUN] = segment_data[1];
				OSTimeDlyHMSM(0, 0, 0, 10);
				break;
		}

		HandleFactoryProcess();

		if (GetMsg(KEY_LONGLONG_WHITE))
		{
			SysinfoSave();
			power_off();
		}
	}
}


