
#include "./Esp8266_Wifi/esp8266.h"
#include <stdarg.h>
#include <string.h>
#include "./usart/usart.h"

/*
8
1 �����ƹ��ܣ�        AT+CLDSTART 
2 ֹͣ�ƹ���:         AT+CLDSTOP
3 ���ƴ�������:       AT+CLDSEND 
4 ���ñ��ؾ���������: AT+LANSTART 
5 ֹͣ���ؾ���������: AT+LANSTOP  
6 �˳���ͥ�飺        AT+CLDUNBIND
��ʹ��smartconfig ��ʱ�� ����ǰ
AT+CWMODE=1
AT+CWSTARTSMART
Smart get wifi info ��ȡ�ɹ�
AT+CWSTOPSMART Ȼ����ִ������ָ��

�ʼģ������Ϊģʽ3
�����ͣ�
AT+CWSMARTSTART=0 ʱ ģʽ���Զ��л�Ϊ1
���ģ�鸴λ�� ��ôģʽ���л�1 �����ڳ�ʼ����ʱ��Ӧ������Ϊ3

����������ģ��֮�� �ٰ�ģʽ���û���
����ٳ�ʼģʽ 3 �޸�Ϊ1 ֮�� ģ����� �����͸�λ֮�� ��ѯģʽΪ1

����Ѿ����ӳɹ� ��GOT IP ��ģʽΪ1
�ٷ��� AT+CWSMARTSTART=0
���ǿ������õ�

����Ҫ ���� AT+CWSMARTSTART=0 �������һ�� �ſ���
*/
unsigned char count_relink=0;
char AT[] = "AT\r\n";
char ATE0[] = "ATE0\r\n";
char AT_CWSTARTSMART[] = "AT+CWSMARTSTART";
char AT_QUIT_CWSTARTSMART[] = "AT+CWSTOPSMART\r\n";
char AT_RST[] = "AT+RST\r\n"; //��λģ��
char AT_CIPSTATUS[] = "AT+CIPSTATUS\r\n"; //��ѯģ������״̬
char AT_CWMODE_CHECK[] = "AT+CWMODE?\r\n";//��ѯģʽ
char AT_CWMODE[] = "AT+CWMODE";
char AT_CWLAP[] = "AT+CWLAP\r\n";//�г�����APվ��
char AT_CWJAP[] = "AT+CWJAP=";//AT+CWJAP="xueleitest","bbk123bbk123"
char AT_CWJAP_DEMO[]="AT+CWJAP=\"dolphinjun\",\"860118dolphin\"\r\n";
char AT_CWJAP_CHECK[] = "AT+CWJAP?\r\n";//�鿴�������Ǹ�����
char AT_CIFSR[] = "AT+CIFSR\r\n";
char AT_CIPMUX[] = "AT+CIPMUX";//����Ϊ��������
char AT_CIPMODE[] = "AT+CIPMODE";//����Ϊ͸��ģʽ
char HTTP_HOST[] = "www.mushi-tech.com";
//char HTTP_PORT[] = "8000";
//char HTTP_GET_MODE[] = "GET /api/urltestFree?para=w4/CsMK9w7zDhsK9 HTTP/1.1\r\n";
char TCP_MODE[] = "TCP";
char UDP_MODE[] = "UDP";
char AT_CIPSTART[] = "AT+CIPSTART";//AT+CIPSTART="TCP","test5.okii.com",8001,0
char AT_CIPSEND[] = "AT+CIPSEND\r\n";//׼����������
char HOST[] = "Host:test5.okii.com\r\n";
char AT_PING[] = "AT+PING="; // PING ���� ����ICMPЭ��
char AT_CWQAP[] = "AT+CWQAP\r\n";//�Ͽ���AP������
char AT_CSYSID[]="AT+CSYSID";
char AT_CLDSEND[]="AT+CLDSEND=10"; 
char AT_CLDSTART[]="AT+CLDSTART";
char AT_CUB[]="AT+CLDUNBIND"; 
char AT_SMARTLINK[]="AT+CWSMARTSTART=1";
char AT_LANSTART[]="AT+LANSTART";
char temp_bander[]="RPT:\"0xa1b23467\",\"0xaf321234\",\"192.168.0.123\",\"light\",\"123456a\"";
char AT_CIP3[]="AT+CIPSTART=3,\"UDP\",\"cloud.ai-thinker.com\",5001,2468,0" ;
char AT_CWAUTOCONN[]="AT+CWAUTOCONN";
int flag_error;

char msg[100];
char data_buf[64];
volatile unsigned int RxCounter1;
unsigned char printenable;
unsigned char WifiStatusFlag;


char wifi_powerup(void)
{
	unsigned char retry_cnt = 0;
	GPIO_InitTypeDef  GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure);		
	GPIO_SetBits(GPIOB, GPIO_Pin_5);  	//WIFI RST
	OSTimeDlyHMSM(0, 0, 0, 300);
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);  //WIFI RST
	OSTimeDlyHMSM(0, 0, 2, 0);
	uart2_init(115200);					//����1��ʼ��      
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	//USART_OUT(USART1, "AT+RST\r\n"); 	//���͸�λָ��
	OSTimeDlyHMSM(0, 0, 1, 100);

	for (retry_cnt = 10; retry_cnt > 0; retry_cnt--)
	{
		OSTimeDlyHMSM(0, 0, 1, 0);
		if (WifiStatusFlag & FLAG_WIFI_SSID_GOTIP)
		{
			break;
		}
	}
	if (retry_cnt)
	{
		printk("POWERUP Auto Connect can GOTIP \r\n");
		printwifi("ATE0\r\n"); //�ر�ָ�����					
		OSTimeDlyHMSM(0, 0, 0, 200);

		WifiStatusFlag = 0;
		printwifi("AT\r\n"); //�ر�ָ�����
		OSTimeDlyHMSM(0, 0, 0, 300);

		if (WifiStatusFlag & FLAG_WIFI_CMD_OK)
		{
			printk("POWERUP Auto Connect ATE0 return OK\r\n");
			WifiStatusFlag = 0;
			return 1;
		}
	}
	else
	{
		printk("PROGRAME_SET_AP ATE0\r\n");
		printwifi("ATE0\r\n"); //�ر�ָ�����					
		OSTimeDlyHMSM(0, 0, 0, 200);
		//wifi_state = PROGRAME_SET_AP;
	}
	return 0;
}

char wifi_tcp_setup(char *tcpaddr, char *portnum)
{
	unsigned char waittimesec = 0;
	do
	{
		printk("SETTCPLINK\r\n");
		//����TCP����
		printk("ready to register1\r\n");
		WifiStatusFlag = 0;
		//printwifi("AT+CIPSTART=");
		//printwifi("%d", 0x22);
		//printwifi("TCP");
		//printwifi("%d", 0x22);
		//printwifi(",");
		//printwifi("%d", 0x22);
		//printwifi("%s", tcpaddr);
		//printwifi("%d", 0x22);
		//printwifi(",");
		//printwifi("%s\r\n", portnum);
		printwifi("AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",tcpaddr,portnum);

		OSTimeDlyHMSM(0, 0, 0, 500);

		printk("ready to register2\r\n");
		if (WifiStatusFlag&FLAG_WIFI_CMD_OK) // TCP ���ӽ��� ���óɹ�
		{
			printk("tcplink OK\r\n");
			return 1;

		}
		else
		{
			waittimesec++;
			printk("tcplink try %d\r\n",waittimesec);
		}
		OSTimeDlyHMSM(0, 0, 2, 0);
	}
	while(waittimesec < 20);
	return 0;
}

void synchDateTimeFromNTPServer()
{
	char *cmd;
	char *respMessage;
	char packetBuffer[48];
	printwifi("AT+CIPMUX=0\r\n");
	//respMessage = execATCommand(cmd, 500, false);

	//Connect to the NTP server
	printwifi("AT+CIPSTART=\"UDP\",\"1.cn.pool.ntp.org\",123\r\n");
	//respMessage = execATCommand(cmd, 500, false);
	
	printwifi("AT+CIPSEND=48\r\n");
	//respMessage = execATCommand(cmd, 50, false);

	memset(packetBuffer, 0, 48); 
	packetBuffer[0] = 0xe3; // LI, Version, Mode
	packetBuffer[1] = 0; // Stratum, or type of clock
	packetBuffer[2] = 6; // Polling Interval
	packetBuffer[3] = 0xEC; // Peer Clock Precision
	packetBuffer[12] = 49; 
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	//Serial.write(packetBuffer, 48);
	//String data = "";
	//unsigned long t1 = millis();
	//char ntpPackage[48];
	//parserNTPMessage(ntpPackage);
	printwifi("AT+CIPCLOSE\r\n");
}

/**
* ��ȡESP8266 IP��ַ
*/
char getIPAddr(char *ipaddr)
{
	printf("AT+CIFSR\r\n");
	//char ip = msg.substring(23,38);
	//if(ip.indexOf("\"") > 0)
	{
		//ipaddr = ip.substring(0, ip.indexOf("\""));
		return 1;
	}
	return 0;
}

void wifi_irq_process(void)
{
	unsigned char i;
	unsigned char j;
	char *datastring;
	
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)      //�ж϶��Ĵ����Ƿ�ǿ�
	{
		msg[RxCounter1] =  USART_ReceiveData(USART2);   //�����Ĵ��������ݻ��浽���ջ�������
		if (printenable)
			USART_SendData(USART1, msg[RxCounter1]);
		if (RxCounter1 == 0)
		{
			if (msg[0] == 0x0D || msg[0] == 0x0A)
			{
				RxCounter1 = 0;
			}
			else
			{
				RxCounter1++;
			}
		}
		else if (RxCounter1 > 63)
		{
			RxCounter1 = 0;
		}
		else
		{
			if (msg[RxCounter1 - 1] == 0x0D && msg[RxCounter1] == 0x0A)
			{
				msg[RxCounter1] = '\0';

				if ((datastring = strstr(msg, "+IPD")) != NULL)
				{
					SetMsg(MSG_WIFI_IPD_FLAG);
					WifiStatusFlag |= FLAG_WIFI_IPD;
					data_buf[0] = 0;
					for (i = 0; i < RxCounter1; i++)
					{
						if (datastring[i] == ',')
							break;
					}
					i++;
					for (; i < RxCounter1; i++)
					{
						if (datastring[i] == ':')
							break;
						data_buf[0] = data_buf[0] * 10 + (datastring[i] - '0');

					}
					i++;
					//printk("recv num=%d\r\n",data_buf[0]);
					for (j = 0; j < data_buf[0]; j++)
					{
						data_buf[1 + j] = datastring[i + j];
					}
				}
				else if (strstr(msg, "ready") != NULL)
				{
					WifiStatusFlag |= FLAG_WIFI_READY;
					SetMsg(MSG_WIFI_READY);
				}
				else if (strstr(msg, "GOT IP") != NULL)
				{
					WifiStatusFlag |= FLAG_WIFI_SSID_GOTIP;
					SetMsg(MSG_WIFI_SSID_GOTIP);
				}
				else if (strstr(msg, "OK") != NULL)
				{
					WifiStatusFlag |= FLAG_WIFI_CMD_OK;
					SetMsg(MSG_WIFI_CMD_OK);
				}
				else if (strstr(msg, "ERROR") != NULL)
				{
					WifiStatusFlag |= FLAG_WIFI_CMD_ERROR;
					SetMsg(MSG_WIFI_CMD_ERROR);
				}

				RxCounter1 = 0;
			}
			RxCounter1++;
		}
		//
		//ÿ���յ���ָ���64�ֽڣ��Ͱ�������գ�����ָ��ָ��ͷ
	}
	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)                     //
	{
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	}
}

