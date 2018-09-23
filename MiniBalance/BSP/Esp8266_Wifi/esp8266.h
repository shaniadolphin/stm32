#ifndef _ESP8266_WIFI_H_
#define _ESP8266_WIFI_H_

//#include "stm32f10x.h"
#include <stdint.h>
#include  "..\..\uCOS-II\Source\ucos_ii.h"



/*ģ��������ã���ֲ������оƬʱ���޸���Щ��*/

#define     ESP8266_WIFI_TX(cmd)                 esp8266_wifi_tx_printf("%s",cmd)   //�����ַ���
#define     ESP8266_WIFI_IS_RX()                 (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) //��ȡ�������ݼĴ���״̬

#define     ESP8266_WIFI_RX(len)                 ((char *)get_rebuff2(&(len)))  //��ȡ���ڽ��յ������ݳ���

#define     ESP8266_WIFI_DELAY(time)             OSTimeDlyHMSM(0, 0, (time/1000), (time%1000));                //��ʱ

#define     ESP8266_WIFI_SWAP16(data)            __REVSH(data)                  //����16λ�ߵ��ֽ�

extern  uint8_t     esp8266_wifi_cmd         (char *cmd, char *reply,uint32_t waittime );
extern  uint8_t     esp8266_wifi_cmd_check   (char *reply);

#if 1
#define     esp8266_wifi_tx_printf(fmt, ...)         USART2_printf(USART2,fmt,##__VA_ARGS__)     //printf��ʽ������������ͣ��������κν������ݴ���
#else
#define     esp8266_wifi_tx_printf(fmt, ...)         printf(fmt,##__VA_ARGS__)     //printf��ʽ������������ͣ��������κν������ݴ���
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
/*����ȫ�ַ��ر�־*/
enum{
    ESP8266_WIFI_TRUE,
    ESP8266_WIFI_FALSE,
		ESP8266_WIFI_TIMEOUT,
};

/*����Ŀ��������� ͨ������ ���� �˿ں� �� ���ʱ��*/
typedef struct HTTP_REQUEST
{
	char *type;
	char *host;
	int port;
	int alive;
}HTTP_REQUEST;


/*���ش��������� ����������ڴ����*/
enum SENSOR_TYPE
{
	DEV_LED,
	DEV_SMOKE,
	DEV_SWITCH,
};

/*������״̬ ���ڴ˴���Ӵ������������ص�*/
typedef struct SENSOR_STATUS
{
	int data; //������״̬����
	char *status;  //˽������

}SENSOR_STATUS;

/*�������ݰ�����*/
typedef struct IOT_DEV_DATA
{
	int machine_id;			//������
	int timestamp;			//ʱ���
	int sensor_type;		//����������
	char sensor_name[20];	//����������
	SENSOR_STATUS sensor_status;//���������ص�������Ϣ
	char *p;				//˽������
}IOT_DEV_DATA;

/*�ϱ����ݰ���װ*/
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


/*Ҫ����WIFI��ssid �� ����*/
typedef struct AP_MSG
{
	char *ssid_name;
	char *code;
}AP_MSG;


/*ģ������״̬*/
enum CIPSTATUS
{
	GET_IP=2,		//���IP
	CONNECTED,		//������
	DISCONNECTED,	//�Ͽ�����
	NO_CONNECTED,	//û������WIFI
};

/*ģ�鹤��ģʽ*/
enum ESP8266_MODE
{
	STATION=1,
	SOFTAP,
	STATION_AND_SOFTAP,
};

/*ģ�鹤��ģʽ*/
enum ESP8266_AUTO_CONNECT_MODE
{
	NO_AUTO_CONNECT=0,
	AUTO_CONNECT,
};

/*SMARTCONFIGģʽ*/
enum ESP8266_SMART_CONFIG_MODE
{
	AI_LINK=0,		//���ӿ� �����Ժ�
	ESP_TOUCH,		//������������Ͽ�WIFI����
	AIRKISS,		//�������ӷ�ʽ ֧��΢��ƽ̨
};

/*���õ����߶�����*/
/*
1��Ĭ��Ϊ������
2��͸��ģʽֻ������Ϊ������ ��AT+CIPMODE=1 ʱ��
3���������л���ʱ��Ҫ�ȹرշ����� AT+CIPSERVER=0
*/
enum ESP8266_CIPMUX
{
	SINGLE_CONNECT=0,
	MULTI_CONNECT,
};

/*���ô���ģʽ*/
/*
0����ͨģʽ
1��͸��ģʽ
͸��ģʽ�� �������Ͽ� ģ��᳢���������ӷ���������ʱ����+++�˳�����ģʽ ��������
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
	SYS_NIT,//ģ��̽�����ʼ��
	SYS_CONNECT_AP,//����ָ��AP  ����ģʽ
	SYS_AUTO_CONNECT,//ʹ��ģʽ ���ڼ���Ƿ���������
	SYS_CHECK_AP_VALID,//�ж�AP�Ƿ������������
	SYS_CIPSEND,//��������
	SYS_REPORT_RECEIVE_PHRASE,//ѭ������
	USER_SETTINE_MODE,
	SYS_EXCEPTION_HANDING,//Exception Handling
};

typedef struct TASK_PARA
{
	int para_num; //�ܲ�������
	int machine_mode;//WIFIģ�鹤��ģʽ
	int mux_mode;//������
	int auto_connect;//�����Ƿ��Զ�����
	int pentrate_mode;//͸��ģʽ
	char* ping_ip; //ping ��֤�Ƿ��������
	AP_MSG *para_apmsg; //���õ�AP��Ϣ
	int smartconfig_mode;
	HTTP_REQUEST *http_request_para;//��Ҫ���ӵķ�����
	IOT_DEV_REPORT_PACK *iot_dev_report_pack;//�ϴ����غͻش����ݰ�
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
#define PROGRAME_SET_AP	6//�����趨����ָ����AP

#define FLAG_WIFI_READY			0x01
#define FLAG_WIFI_SSID_GOTIP	0x02
#define FLAG_WIFI_CMD_OK		0x04
#define FLAG_WIFI_CMD_ERROR		0x08
#define FLAG_WIFI_IPD			0x80


void wifi_irq_process(void);

#endif
