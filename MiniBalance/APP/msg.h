
#ifndef _MSG_H_
#define _MSG_H_


typedef enum
{
	//system message
	MSG_KEYSCAN_PROCESSED = (unsigned int)0x00,
	UART_SEND_EN,
	UART_RECV_EN,
	ADC_FINISHED,
	
	KEY_SHORT_WHITE,
	KEY_LONG_WHITE,
	KEY_LONGLONG_WHITE,
	KEY_SHORT_BLACK,
	KEY_LONG_BLACK,
	KEY_LONGLONG_BLACK,

	MSG_WIFI_READY,
	MSG_WIFI_CMD_OK,
	MSG_WIFI_CMD_ERROR,
	MSG_WIFI_TCP_OK,
	MSG_WIFI_TCP_ERROR,
	MSG_WIFI_SSID_GOTIP,
	MSG_WIFI_SSID_DISCONNECT,
	MSG_WIFI_IPD_FLAG,	
	
	//End
	MSG_ID_MAX  //max message number

}MSG_ID;


extern void SetMsg(MSG_ID MsgId);
extern unsigned char GetMsg(MSG_ID MsgId);
extern void ClearMsg(MSG_ID MsgId);
extern unsigned char CheckMsg(MSG_ID MsgId);

#endif

