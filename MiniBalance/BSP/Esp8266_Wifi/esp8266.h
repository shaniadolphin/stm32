#ifndef _ESP8266_WIFI_H_
#define _ESP8266_WIFI_H_

//#include "stm32f10x.h"
#include <stdint.h>
#include  "..\..\uCOS-II\Source\ucos_ii.h"



/*模块控制配置，移植到其它芯片时请修改这些宏*/

#define     ESP8266_WIFI_TX(cmd)                 esp8266_wifi_tx_printf("%s",cmd)   //发送字符串
#define     ESP8266_WIFI_IS_RX()                 (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) //获取串口数据寄存器状态

#define     ESP8266_WIFI_RX(len)                 ((char *)get_rebuff2(&(len)))  //获取串口接收到的数据长度

#define     ESP8266_WIFI_DELAY(time)             OSTimeDlyHMSM(0, 0, (time/1000), (time%1000));                //延时

#define     ESP8266_WIFI_SWAP16(data)            __REVSH(data)                  //交换16位高低字节

extern  uint8_t     esp8266_wifi_cmd         (char *cmd, char *reply,uint32_t waittime );
extern  uint8_t     esp8266_wifi_cmd_check   (char *reply);

#if 1
#define     esp8266_wifi_tx_printf(fmt, ...)         USART2_printf(USART2,fmt,##__VA_ARGS__)     //printf格式发送命令（纯发送，不进行任何接收数据处理）
#else
#define     esp8266_wifi_tx_printf(fmt, ...)         printf(fmt,##__VA_ARGS__)     //printf格式发送命令（纯发送，不进行任何接收数据处理）
#endif

#define     ESP8266_WIFI_CLEAN_RX()                  clean_rebuff2()
/***************ADD WIFI FUNC*****************/
extern char HTTP_HOST[];
extern char AT_CIPSEND[];
extern char TCP_MODE[];
//use for json phrase
extern char json_data[];
extern char json_data_test[];
//extern char p_str_turn[];

extern char oxygen_switch_flag;
extern char constant_temperature_switch_flag;
extern char change_water_flag;
extern char add_food_flag;
/*设置全局返回标志*/
enum{
    ESP8266_WIFI_TRUE,
    ESP8266_WIFI_FALSE,
		ESP8266_WIFI_TIMEOUT,
};

/*设置目标服务器的 通信类型 主机 端口号 和 侦测时间*/
typedef struct HTTP_REQUEST
{
	char *type;
	char *host;
	int port;
	int alive;
}HTTP_REQUEST;


/*板载传感器类型 如需添加请在此添加*/
enum SENSOR_TYPE
{
	DEV_LED,
	DEV_SMOKE,
	DEV_SWITCH,
};

/*传感器状态 请在此处添加传感器数据域特点*/
typedef struct SENSOR_STATUS
{
	int data; //传感器状态数据
	char *status;  //私有数据

}SENSOR_STATUS;

/*返回数据包解析*/
typedef struct IOT_DEV_DATA
{
	int machine_id;			//机器码
	int timestamp;			//时间戳
	int sensor_type;		//传感器类型
	char sensor_name[20];	//传感器名字
	SENSOR_STATUS sensor_status;//传感器返回的数据信息
	char *p;				//私有数据
}IOT_DEV_DATA;

/*上报数据包封装*/
typedef struct IOT_DEV_REPORT_PACK
{
	char *type;// GET or POST
	char *host;// HTTP_HOST
	char *http_report_router;//   /test/updateState
	char *http_receive_router;//   /test/getState
	char *http_heartbeat_router;//   
	char *http_uploadstate_router;//   
	char *http_version;
	IOT_DEV_DATA iot_dev_data;//sn=101&state=off
	
}IOT_DEV_REPORT_PACK;


/*要连接WIFI的ssid 和 密码*/
typedef struct AP_MSG
{
	char *ssid_name;
	char *code;
}AP_MSG;


/*模块连接状态*/
enum CIPSTATUS
{
	GET_IP=2,		//获得IP
	CONNECTED,		//已连接
	DISCONNECTED,	//断开连接
	NO_CONNECTED,	//没有连接WIFI
};

/*模块工作模式*/
enum ESP8266_MODE
{
	STATION=1,
	SOFTAP,
	STATION_AND_SOFTAP,
};

/*模块工作模式*/
enum ESP8266_AUTO_CONNECT_MODE
{
	NO_AUTO_CONNECT=0,
	AUTO_CONNECT,
};

/*SMARTCONFIG模式*/
enum ESP8266_SMART_CONFIG_MODE
{
	AI_LINK=0,		//链接快 兼容性好
	ESP_TOUCH,		//链接慢，不会断开WIFI链接
	AIRKISS,		//智能连接方式 支持微信平台
};

/*设置单或者多连接*/
/*
1、默认为单链接
2、透传模式只能设置为单链接 即AT+CIPMODE=1 时候
3、在连接切换的时候要先关闭服务器 AT+CIPSERVER=0
*/
enum ESP8266_CIPMUX
{
	SINGLE_CONNECT=0,
	MULTI_CONNECT,
};

/*设置传输模式*/
/*
0、普通模式
1、透传模式
透传模式下 如果传输断开 模块会尝试重新连接服务器，此时输入+++退出传输模式 不再重连
*/
enum ESP8266_CIPMODE
{
	NONE_PENETRATE=0,
	PENETRATE,
};

enum ESP8266_REPORT_TYPE
{
	HEARTBEAT=1,
	SYSSTATUS,
};

enum ESP8266_IOT_STATUS
{
	SYS_NIT,//模块探测与初始化
	SYS_CONNECT_AP,//链接指定AP  开发模式
	SYS_AUTO_CONNECT,//使用模式 用于检测是否正常连接
	SYS_CHECK_AP_VALID,//判断AP是否可以正常上网
	SYS_CIPSEND,//发送设置
	SYS_REPORT_RECEIVE_PHRASE,//循环解析
	USER_SETTINE_MODE,
	SYS_EXCEPTION_HANDING,//Exception Handling
};

typedef struct TASK_PARA
{
	int para_num; //总参数个数
	int machine_mode;//WIFI模块工作模式
	int mux_mode;//单链接
	int auto_connect;//设置是否自动连接
	int pentrate_mode;//透传模式
	char* ping_ip; //ping 验证是否可以联网
	AP_MSG *para_apmsg; //设置的AP信息
	int smartconfig_mode;
	HTTP_REQUEST *http_request_para;//需要连接的服务器
	IOT_DEV_REPORT_PACK *iot_dev_report_pack;//上传本地和回传数据包
}TASK_PARA;


#define TEST_AT 				esp8266_wifi_at_test
#define CIPSTATUS_AT 		esp8266_wifi_cipstatus
#define CWAUTOCONN_AT   esp8266_wifi_cwautonn
#define MODE_AT 				esp8266_wifi_mode_set
#define REST_AT 				esp8266_wifi_reset
#define SMARTCONFIG_AT 	esp8266_wifi_smartconfig
#define QUIT_SMARTCONFIG_AT esp8266_wifi_quit_smartconfig

#define CLOSE_ECHO_AT 	esp8266_wifi_Close_Echo
#define CWJAP_CHECK_AT 	esp8266_wifi_cwjap_check
#define CWJAP_SET_AT 		esp8266_wifi_cwjap_set
#define PING_AT 				esp8266_wifi_cwjap_ping
#define CWLAP_AT 				esp8266_wifi_cwlap
#define HTTP_REQUEST_AT esp8266_wifi_http_request
#define CIPSEND_AT 			esp8266_wifi_cipsend
#define MUX_AT 					esp8266_wifi_nux_set
#define CIPMODE_AT 			esp8266_wifi_cipmode_set
#define CWQAP_AT 				esp8266_wifi_cwap
#define PHRASE_MSG			esp8266_wifi_http_reportdata_and_local_phrase

#define QUIT_PENTRATE   esp8266_wifi_quit_pentrate

#define ESP8266_WORK		esp8266_wifi_waitask

#define SOFTWARE_VER	0x10


#define POWERUP 		0
#define SETTCPLINK 		1
#define TCPLINKOK 		2
#define TCPLINKFAIL 	3
#define SMARTWIFI		4
#define SYS_REGISTER	5
#define PROGRAME_SET_AP	6//程序设定链接指定的AP

#define FLAG_WIFI_READY			0x01
#define FLAG_WIFI_SSID_GOTIP	0x02
#define FLAG_WIFI_CMD_OK		0x04
#define FLAG_WIFI_CMD_ERROR		0x08
#define FLAG_WIFI_IPD			0x80


void wifi_irq_process(void);

#endif
