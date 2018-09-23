#include "includes.h"
#include "globals.h"
#include "sys_register.h"

#include "ucos_ii.h"
#include "string.h"
#include "Msg.h"


#if 1
#define SYS_SEND_MSG_START() USART_OUT(USART1, "AT+CIPSEND=16\r\n");
#else
#define SYS_SEND_MSG_START() USART_OUT(USART1, "AT+CIPSEND=12\r\n");
#endif


extern unsigned short NetSendCounter;
extern unsigned char CpuID[];
extern unsigned int UnixTime;
extern char msg[64];


void send_buf(unsigned char *buf, unsigned char key)
{
	unsigned char i;

	printk("Tx[]=");
	printf_buf_hex(buf, DATA_SEND_LEN);
	if (ENCRYPT_ENABLE)
	{
		EncryptData(&buf[2], ENCRYPT_LEN, key);
		printk("En[]=");
		printf_buf_hex(buf, DATA_SEND_LEN);
	}

	SYS_SEND_MSG_START();
	OSTimeDlyHMSM(0, 0, 0, 100);
	for (i = 0; i < DATA_SEND_LEN; i++)
	{
		USART_TX(USART1, buf[i]);
	}
}

/*
cmd     step                IMEI    UNIX TIME   user   state    check sum
3       13                  48      32          16     8        8
b'000   b'0 0000 0000 0001	IMEI    TIME        define
*/
void SendHeartbeat(unsigned char cmd, unsigned short step, unsigned int time_stamp, unsigned char percent, unsigned char state)
{
	unsigned char send_msg[DATA_SEND_LEN];

	//cmd
	send_msg[0] = (cmd << 5)| (step >> 8);
	//sn
	send_msg[1] = step&0xff;
	//IMEI
	printk("IMEI : %x-%x-%x-%x-%x-%x\r\n",IMEI[0],IMEI[1],IMEI[2],IMEI[3],IMEI[4],IMEI[5]);
	send_msg[2] = IMEI[0];
	send_msg[3] = IMEI[1];
	send_msg[4] = IMEI[2];
	send_msg[5] = IMEI[3];
	send_msg[6] = IMEI[4];
	send_msg[7] = IMEI[5];
	//UNIX TIME
	send_msg[8] = (time_stamp>>24)&0xff;
	send_msg[9] = (time_stamp>>16)&0xff;
	send_msg[10] = (time_stamp>>8)&0xff;
	send_msg[11] = time_stamp&0xff;
	//USER STATE
	send_msg[12] = percent;
	send_msg[13] = 0;

	send_msg[14] = state;
	send_msg[15] = calc_sum(send_msg, DATA_SEND_LEN-1);
	printk("Heartbeat check sum %x \r\n", send_msg[15]);

	send_buf(send_msg, encrypt_key);
}

/*----------------------------------------------------------------------------
 * @����:send_chess_data����һ������
 * @����:	 chess_buf����
			CpuID
 * @����:�ɹ���1  ʧ�ܣ�0
 * @��ʷ:
 �汾        ����            ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------
 1.0       2016.01.29      zxjun          ����ģ��
----------------------------------------------------------------------------*/
unsigned char send_chess_data(unsigned char *chess_buf, unsigned char historyflag)
{
	unsigned char send_msg[DATA_SEND_LEN];
	unsigned char timewaiting = 10;

	memcpy(send_msg, chess_buf, DATA_SEND_LEN);
	if (historyflag)
	{
		NetSendCounter = 1000;
		send_msg[0] = (chess_buf[0] | 0x40);
		send_msg[DATA_SEND_LEN-1] = calc_sum(send_msg, DATA_SEND_LEN-1);
	}

	ClearMsg(MSG_WIFI_CMD_OK);
	ClearMsg(MSG_WIFI_CMD_ERROR);

	send_buf(send_msg, encrypt_key);

	do	//���ͺ�ȴ�һ�����ͳɹ��������ʱ�����յ�����ֱ�ӷ���0
	{
		OSTimeDlyHMSM(0, 0, 0, 100);
		timewaiting--;
		if (1 == CheckMsg(MSG_WIFI_CMD_ERROR) || 0 == timewaiting)
		{
			return 0;
		}
	} while (0 == CheckMsg(MSG_WIFI_CMD_OK));

	return timewaiting;
}

/*
 * ϵͳע������ϳ�
 *
 * @cmd ��1�ֽ�
 * @step ��2�ֽ�
 * @cpuid ��3�ֽ�
 * @num4 ��4�ֽ�
 * @checksum У���
 *
 * @����ֵ ��
 *
 * ��ʽ˵����
 *  ������	���				  CPU ID			state	����
 *	3	   13	 				96				8		8
 *	b'010	b'0 0000 0000 0001	CPU ID			�汾x.x
 *	ע�᷵��
 *	������	 ���				  IMEI	  UNIX_TIME	 ��֤��	 state	����
 *	3		13					 48	 	 32		 	16		8		8
 *	b'010	b'0 0000 0000 0001	IMEI	TIME		code
 */
void send_cpuid_version(unsigned char cmd, unsigned short step, unsigned char *cpuid, unsigned char state, unsigned char key)
{
	unsigned char send_msg[DATA_SEND_LEN];

	//ע��λ��Ϣ����
	send_msg[0] = (cmd << 5) | (step >> 8);
	send_msg[1] = (step & 0xff);
	memcpy(&send_msg[2], cpuid, 12);
	send_msg[14] = state;
	send_msg[15] = calc_sum(send_msg, DATA_SEND_LEN-1);

	send_buf(send_msg, key);
}

/*
 * ������Ϣ�ϳ�
 * @cmd    ��ȡ��Ϣ������
 * @num    ��ȡ�ڼ�����ͬ��Ϣ
 * @part   ��ȡ��ͬ��Ϣ�����С��Ŀ
 *
 * @����ֵ  ��
 *
 * ˵����
 *  ����WIFI����
 * 	������			���			����		SSID				  ����
 * 	3				13			   8		  96					8
 * 	b'011	b'0 0000 0000 0001	length		"SSID part A "
 * 	b'011	b'0 0000 0000 0002	length		"SSID part B "
 * 	b'011	b'0 0000 0000 0003	length		"PASSWORD p A"
 * 	b'011	b'0 0000 0000 0004	length		"PASSWORD p B"
 *
 * 	����ʾ��:
 * 	b'0110 0000 (0x60)�����1��wifi ����Ϣ
 * 	b'0110 0001 (0x61)�����2��wifi ����Ϣ
 *
 * 	(char cmd, ������
 * 	char num,  �ڼ���WIFI
 * 	int step)  ����
 *
 * 	����WIFI��������ϳ�
 *
 * 	60 01 53 FF 6E 06 48 85 57 57 56 29 11 87 00 00
 * 	60 01 63 61 72 65 6B 69 64 73 63 61 72 65 00 00
 * 	ģ��carekid 16���Ʒ�����
 * 	60 01 63 61 72 65 6B 69 64 73 00 00 00 00 01 01
 *
 */
void send_cut_part(unsigned char cmd, unsigned char number, unsigned char part, unsigned char *buf, unsigned char len)
{
	unsigned char send_msg[DATA_SEND_LEN];

	send_msg[0] = (cmd << 5) | (number & 0x1f);
	send_msg[1] = part;
	strncpy(&send_msg[2], buf, 12);
	send_msg[DATA_SEND_LEN-2] = len;
	send_msg[DATA_SEND_LEN-1] = calc_sum(send_msg, DATA_SEND_LEN-1);

	send_buf(send_msg, encrypt_key);
}

