#ifndef _NFC_H_
#define _NFC_H_

#include "delay.h"
#include "as3911.h"
#include "iso15693_3.h"
#include "includes.h"


#define ISO15693	 1	//打开/关闭15693支持

#define	RFO_AM_OFF_LEVEL_X	0xe0
#define	RFO_AM_OFF_LEVEL_Y	0xe0

#define ONE_ANTENNA_TAG_NUMS  21    //一个天线读取最多标签数量
#define X_COILS			8  //X方向的天线数8
#define Y_COILS			8  //Y方向的天线数8
#define TAG_NUMS		32              //支持的标签总数

#define ANTENNA_OFF     0
#define ANTENNA_ON      1

#define X_DIR			0
#define Y_DIR			1

#define ANTENNA_CALIBRATE_MIN_TRIM		1			//天线校准最小档位
#define ANTENNA_CALIBRATE_MAX_TRIM		14			//天线校准最大档位
#define ANTENNA_CALIBRATE_MIN_VOLTAGE	1100     	//天线校准最小电压
#define ANTENNA_CALIBRATE_MAX_VOLTAGE	2000     	//天线校准最高电压
#define ANTENNA_CALIBRATE_MIN_PHASE		60			//天线校准最小相位
#define ANTENNA_CALIBRATE_MAX_PHASE		140			//天线校准最大相位

typedef struct
{
	U8 uid[ISO15693_UID_LENGTH];
	U16 block_code:5;
	U16 chess_code:5;
	U16 rise_code:5;
	U16 miss:1;
	U16 xPos:3;
	U16 yPos:3;
	U16 xOld:3;
	U16 yOld:3;
	U16 reserve:4;
	U16 xRssi:8;
	U16 yRssi:8;
	U16 xCnt;
	U16 yCnt;
	U16 cnt;
}tag_data_t;

typedef struct{
	U8 line;
	U8 dir;
}CHESS_SCAN;

typedef struct app_calibrate_info
{
	unsigned char trim_flag;
	unsigned char trim_value;
	unsigned short amplitude;
	unsigned char phase;
}app_calibrate_info_t;

typedef struct _CHESS_POS_{
	U8 y;
	U8 x;
	U8 code;
}CHESS_POS;


#define		MAX_GAME_CNT				120
typedef struct _SPI_CFG_{
	U8  curr_game;	//当前棋局编号
	U8  reserve[11];
	U16 game_steps_cnt[MAX_GAME_CNT];//每一局的步数

	U32 check_sum;
}SPI_CFG;

typedef struct {
	unsigned short step;
	unsigned int time;
}TX_BUF;
#define MAX_TX_BUFF_LEN		5
#define TX_TIME_OUT			6
typedef struct _TX_CFG_{
	U8 curr_tx_game;	//当前组发送编号
	U16 curr_tx_step;	//当前组发送步数
	U16 curr_rx_step;//当前组上一次接收步数
	U8 his_tx_game;//历史组发送编号
	U16 his_tx_step;//历史组发送步数
	U16 his_rx_step;//历史组上一次接收步数
	TX_BUF tx_buf[MAX_TX_BUFF_LEN];
	U16 heartbeat_tx_step;
	U16 heartbeat_rx_step;
	U32 heartbeat_tx_time;
}TX_CFG;

typedef struct{
	U16 cmd_code:5;
	U16 pane_code:6;
	U16 chess_code:5;
	U32 time;
	U16 step:11;
	U16 special:1;
	U16 press_part:1;
	U16 cmd:3;
	U8  no;
}DELAY_MSG;

typedef struct
{
	unsigned char trim_value;   //电容档位
	unsigned char phase;		//相位
	unsigned short amplitude;	//电压幅值
	unsigned char driver_level;	//输出阻抗
	unsigned char gain;
}save_calibrate_info_t;

#define NFC_CALIBRATE_SUCESS_FLAG	0x5555aaa2
typedef struct{
	save_calibrate_info_t calibrate_info[16];	//天线校准不同档位的信息
	unsigned int NFC_CHECK_RESULT;		//天线校准的结果
}NFC_ANTENNA_INFO;

/*
OBSERVATION mode : 通过csi和cso两个引脚观测内部信号
	CSI pin functionality
	CSO pin functionality
OBSERVATION_MODE_1  	01
	Analog output of AM channel (before digitizer)
	Digital output of AM channel (after digitizer)
OBSERVATION_MODE_2		02
	Analog output of PM channel (before digitizer)
	Digital output of PM channel (after digitizer)
OBSERVATION_MODE_3		03
	Analog output of AM channel (before digitizer)
	Analog output of PM channel (before digitizer)
OBSERVATION_MODE_4		04
	Digital output of AM channel (after digitizer)
	Digital output of PM channel (after digitizer)
OBSERVATION_MODE_5		0A
	Digital TX modulation signal
	Not used
*/
#define OBSERVATION_MODE_DEBUG  0
#define OBSERVATION_MODE_1	0x01
#define OBSERVATION_MODE_2	0x02
#define OBSERVATION_MODE_3	0x03
#define OBSERVATION_MODE_4	0x04
#define OBSERVATION_MODE_5	0x0A


extern void select_x_antenna(U8 antenna_num);
extern void select_y_antenna(U8 antenna_num);
extern void turn_on_off_antenna(U8 status);
extern void print_tags_point(void);

//-------------------------------------------------
//15693
#define	ANTANNA_DRIVER_SINGLE	1   	//单端驱动
#define	D15693_AUTO_CALIBRATION	0        //实时自动校准

extern U8 chess_mode;
typedef enum {
	CHESSMODE_CALIBRATE,
	CHESSMODE_IDLE,
	CHESSMODE_TIME_SETTING,
	CHESSMODE_PREPARE_TO_PLAY,
	CHESSMODE_PLAYING,
	CHESSMODE_SELF_TEST,
}CHESSMODE;

extern S8 as3911MeasureAndPrintAntenna(void);
extern int All_Antenna_Calibration(NFC_ANTENNA_INFO *pdata);
extern S8 as3911_enter_observation_mode(S8 mode);
extern void SysinfoRead(void);
extern void SysinfoSave(void);
extern unsigned int calc_sum(unsigned char* buf, unsigned int length);

#define	ERR_COOR	0xff	// 错误坐标

extern void print_cards_uid_15693(U8 cardNum, iso15693ProximityCard_t *cards);

extern void printk(char *Data,...);
extern void printf_buf_hex(U8 *buf, int len);
extern void USART_TX(USART_TypeDef *USARTx, unsigned char ch);

extern OS_EVENT *SemSpiflash;
extern unsigned int led_flash_flag;
extern const unsigned char segment_data[16];
extern unsigned char display_dat_buf[13];
extern unsigned short bat_vol;
extern unsigned char bat_percent;
extern unsigned short NetworkTimeOutCounter;


#define NETWORK_TIME_OUT		15000


#define ENCRYPT_ENABLE		1 //上传和收到数据加解密开关

#define CMD_REGISTER		2
#define CMD_WIFI			6
#define CMD_DOMAIN			4
#define CMD_DATA			1
#define CMD_HISTORY			3
#define CMD_PRODUCE			7
#define CMD_RESERVE			5
#define CMD_HEARTBEAT		0


#endif //_NFC_H_

