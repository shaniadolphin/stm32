#include "sys_register.h"

/*
������Լ����Ϣ
1��
���̳���ע�����������16bit��֤�봦��dataΪ16bit ���ص���֤�룩
INT32U tmp_data1 = data;
INT32U tmp_data2 = data;
tmp_data1 <<= 2;
tmp_data2 |= tmp_data1;//����18bit����
����ÿ3��bit�����������ʾ������ʾ��Χ��0-7��

2��
����WIFI���ݸ�ʽ��
��һ���ֽڶ���
��3λ��������
��5λ�������n��wifi ��Ϣ
ע������wifi��ϢԼ���ӵ�0����ʼ

����ʾ��:
b'011 00000 (0x60)�����0��wifi ����Ϣ
b'011 00001 (0x61)�����1��wifi ����Ϣ

3��
ע������Կ ��ʼ��Կ0xff
��ʼ��Կ���ڣ�����ע����ܣ�������ע��˽���

�������Ѿ�ע�ᣬ8λ�㷨��ԿԼ�����£�����Լ��˳����CPUID��ȡ8bit
�ֱ�ȡ CPUID�е�1 3 5 7 �ֽ��и���λ���һ���ֽڵ�key
ʾ����
cpuid[1] = b'10 xxxxx
cpuid[3] = b'11 xxxxx
cpuid[5] = b'10 xxxxx
cpuid[7] = b'11 xxxxx

==>
key = b'10 11 10 11(8bit)=0xBB

4��ע��ʱ�յ���֤�� �������ϴ���14���ֽ����λ��λ

ע�Ὺʼ��ʾ��
�м�6λ�������ʾF

ע���յ�����������6������
��������14���ֽ� ��7bit ��λ

��ʱ�ȴ��û��ļ� �û��ļ���
��������14���ֽ� ��6bit ��λ

�ȴ�������������������
��������14���Լ� ��5bit ��λ

�����ϴ����ֳɹ�
��������14���Լ� ��4bit ��λ

��һ�����ϴ�cpuid����10���ڣ�����3�Σ�Ȼ��ÿ10����һ�Ρ�һ���Ӻ�ѭ����
�ڶ������·�imei����֤�롣
���������ϴ�ȷ�ϣ�����һֱ��ʾ��֤�롣�������������ص���һ������
���Ĳ���ֻҪ�û��ĺڼ������ϴ�ע������10���ڣ�����3�Σ�û����Ӧ�͵���һ���û��ĺڼ��������������� �ص���һ����
���岽���������·��ɹ���
����������Ӧ�������յ��ɹ����

������ϲ��輴����������ע������


SMARTWIFIģʽ
�м�6λ�������ʾ1
���ӳɹ�����ʾ2
����ʧ�ܺ���ʾ3

�͵��������ʾ
�͵�澯�����������ʾFF

ע��Ŀǰ������ע���Ѿ�û������
*/


/*
 * �����������ص�16bit ���ݰ��ձ�ע˵������ת����18bit������ʾ��6���������
 *
 * @data 16λ����
 *
 * @����ֵ ��
 *
 * ˵����
 *  ���̳���ע�����������16bit��֤�봦��dataΪ16bit ���ص���֤�� ��
 *	INT32U tmp_data1 = data;
 *	INT32U tmp_data2 = data;
 *	tmp_data1 <<= 2;
 *	tmp_data2 |= tmp_data1;//����18bit����
 *	����ÿ3��bit�����������ʾ������ʾ��Χ��0-7��
 */
void Digital_Display(INT16U data)
{
	INT32U tmp_data1 = data;
	INT32U tmp_data2 = data;

	tmp_data1 <<= 2;
	tmp_data2 |= tmp_data1; //����18bit����

	display_dat_buf[W_FAULT] = 0;
	display_dat_buf[W_MIN_HUN] = 0;
	display_dat_buf[W_MIN_TEN] = 0;
	display_dat_buf[W_MIN_ONE] = segment_data[((0x07 << 15) & tmp_data2) >> 15];
	display_dat_buf[W_SEC_TEN] = segment_data[((0x07 << 12) & tmp_data2) >> 12];
	display_dat_buf[W_SEC_ONE] = segment_data[((0x07 << 9) & tmp_data2) >> 9];
	display_dat_buf[B_MIN_HUN] = segment_data[((0x07 << 6) & tmp_data2) >> 6];
	display_dat_buf[B_MIN_TEN] = segment_data[((0x07 << 3) & tmp_data2) >> 3];
	display_dat_buf[B_MIN_ONE] = segment_data[((0x07 << 0) & tmp_data2) >> 0];
	display_dat_buf[B_SEC_TEN] = 0;
	display_dat_buf[B_SEC_ONE] = 0;
	display_dat_buf[B_FAULT] = 0;

}

unsigned char receive_factory_produce_result(unsigned char key)
{
	unsigned char cnt;

	for (cnt = 0; cnt < 30; cnt++)
	{
		if (data_buf[0] != 0)
		{
			data_buf[0] = 0;
			EncryptData(&data_buf[3], ENCRYPT_LEN, key);
			printk("Rx[]=");
			printf_buf_hex(&data_buf[1], DATA_SEND_LEN);
			if (calc_sum(&data_buf[1], DATA_SEND_LEN - 1) == data_buf[DATA_SEND_LEN])
			{
				NetworkTimeOutCounter = NETWORK_TIME_OUT;
				if ((data_buf[1]&0xe0) == 0xe0)
				{
					update_machine_time(&data_buf[9]);
					return 1;
				}
			}
			else
			{
				printk("checksum error = %d\r\n", __LINE__);
			}
		}
		OSTimeDlyHMSM(0, 0, 1, 0);
	}

	return 0;
}

/*
 * ע����Ϣ����
 *
 * @����ֵ 		  ��
 * ˵����
 * ����ģ�����ָ��
 * �Ǽ��ܣ�40 01 53 FF 6E 06 48 85 57 57 56 29 11 87 00 00
 * ���ܣ�  40 01 e1 00 a1 f9 6d 7a e9 a8 14 d6 67 78 f0 00
 * �жϼ����Ƿ���ȷ����ʹ������ָ��ģ����֤
 * ע���IMEI : 53-ff-6e-6-48-85
 * ע�������ʱ��
 * 2016��6��7�� 7:18:1
 * ����ʱ��
 * ��֤��=3587
 */
unsigned char receive_imei_info(void)
{
	unsigned char retry_times;
	unsigned short verify_code;

	for (retry_times = 10; retry_times > 0; retry_times--)
	{
		if (data_buf[0] != 0)
		{
			data_buf[0] = 0;
			printk("Rx[]=");
			printf_buf_hex(&data_buf[1], DATA_SEND_LEN);
			EncryptData(&data_buf[3], ENCRYPT_LEN, encrypt_key);
			printf_buf_hex(&data_buf[1], DATA_SEND_LEN);
			if (calc_sum(&data_buf[1], DATA_SEND_LEN - 1) == data_buf[DATA_SEND_LEN])
			{
				NetworkTimeOutCounter = NETWORK_TIME_OUT;
				if ((data_buf[1] >> 5) == 2) //ע������
				{
					printk("ע��ǰIMEI : ");
					printf_buf_hex(IMEI, sizeof(IMEI));
					memcpy(IMEI, &data_buf[3], sizeof(IMEI));
					printk("ע���IMEI : ");
					printf_buf_hex(IMEI, sizeof(IMEI));

					update_machine_time(&data_buf[9]);

					//get codes and show LED
					verify_code = data_buf[13];
					verify_code <<= 8;
					verify_code |= data_buf[14];
					printk("��֤��=0x%x \r\n", verify_code);
					Digital_Display(verify_code);
					return 1;
				}
			}
			else
			{
				printk("checksum error = %d\r\n", __LINE__);
			}
		}
		if (CheckMsg(FLAG_SERVER_CPUID))
		{
			break;
		}
		if (CheckMsg(MSG_ENTER_SMARTLINK))
		{
			break;
		}
		OSTimeDlyHMSM(0, 0, 1, 0);
	}

	return 0;
}

unsigned char receive_cut_part(unsigned char cmd, unsigned char number, unsigned char part, char *buff)
{
	unsigned char retry_times;
	unsigned char rx_len = 0;

	for (retry_times = 10; retry_times > 0; retry_times--)
	{
		if (data_buf[0] != 0)
		{
			data_buf[0] = 0;
			EncryptData(&data_buf[3], ENCRYPT_LEN, encrypt_key);
			printk("recv_msg[]=");
			printf_buf_hex(&data_buf[1], DATA_SEND_LEN);
			if (calc_sum(&data_buf[1], DATA_SEND_LEN - 1) == data_buf[DATA_SEND_LEN])
			{
				NetworkTimeOutCounter = NETWORK_TIME_OUT;
				if ((data_buf[1] >> 5) == cmd && (data_buf[1] & 0x1f) == number && data_buf[2] == part)
				{
					rx_len = data_buf[DATA_SEND_LEN-1];
					printk("stringlenth=%d\r\n", rx_len);

					strncpy(buff, &data_buf[3], 12);
					printk("system wifi state phrase success\r\n");

					return rx_len;
				}
			}
			else
			{
				printk("checksum error = %d\r\n", __LINE__);
			}
		}

		if (CheckMsg(MSG_ENTER_SMARTLINK))
		{
			break;
		}
		OSTimeDlyHMSM(0, 0, 1, 0);
	}

	return rx_len;
}

/*
 * ��ȡ������Ϣ�ӷ����� ���ڱ������
 *
 * @cmd�� ��ȡ���ݱ�־
 * @num: ͬһ�����еĵڼ���
 * @step_base: �����ȡwifi ����һ���ssid,�ֳ���������ȡ step��Ϊ1 2 ��ô����wifi ��Ϣ��˵���������� ��step 3 4 ������ �����ȡWIFI��Ϣ
 * @msg����ȡ���ݱ���
 *
 * @����ֵ
 *  1 �ɹ�
 *  0 ʧ��
 */
unsigned char receive_full_part(unsigned char cmd, unsigned char number, unsigned char part, unsigned char *buff)
{
	unsigned char retry_times;
	unsigned char len;
	unsigned char buff1[DATA_SEND_LEN];
	unsigned char buff2[DATA_SEND_LEN];

	data_buf[0] = 0;
	memset(buff1, 0, DATA_SEND_LEN);
	for (retry_times = 0; retry_times < 10; retry_times++)
	{
		send_cut_part(cmd, number, part, buff, strlen(buff));
		OSTimeDlyHMSM(0, 0, 0, 800);
		printk("receive_full_part = %d\r\n", retry_times);
		len = receive_cut_part(cmd, number, part, buff1);
		if (len)
		{
			break;
		}
		if (CheckMsg(MSG_ENTER_SMARTLINK))
		{
			break;
		}
	}
	if (len)
	{
		data_buf[0] = 0;
		memset(buff2, 0, DATA_SEND_LEN);
		for (retry_times = 0; retry_times < 10; retry_times++)
		{
			send_cut_part(cmd, number, part+1, buff, strlen(buff));
			OSTimeDlyHMSM(0, 0, 0, 800);
			printk("receive_full_part = %d\r\n", retry_times);
			len = receive_cut_part(cmd, number, part+1, buff2);
			if (len)
			{
				break;
			}
			if (CheckMsg(MSG_ENTER_SMARTLINK))
			{
				break;
			}
		}
	}
	if (len)
	{
		strcpy(buff, buff1);
		strcat(buff, buff2);
	}

	return len;
}

/*
 * ֻҪ��������WIFI��Ϣ״̬�����仯 �͸��±������б�����Ϣ �����Ա�
 *
 * @�������� ��
 *
 * @����ֵ
 *  ��0 �ɹ�
 *  0  ʧ��
 */
unsigned char update_wifi_info(unsigned char wifi_no)
{
	unsigned char ssid[25];
	unsigned char key[25];
	unsigned char len;

	printk("*************System enter get wifi info from server process*************** \r\n");
	//��ȡssid
	len = receive_full_part(CMD_WIFI, wifi_no, 1, ssid);
	if (len)
	{
		//��ȡpassword
		len = receive_full_part(CMD_WIFI, wifi_no, 3, key);
	}

	if (len)
	{
		save_wifi(wifi_no, ssid, key);
		printk("Update wifi info success\r\n");
	}

	return len;
}

/*
 * ��ȡ������Ϣ
 *
 * @�������� ��
 *
 * @����ֵ
 *  ��0 �ɹ�
 *  0   ʧ��
 *  ��ע��������ȡ��δ���浽����
 */
unsigned char update_domain_info(void)
{
	unsigned char temp[25];
	unsigned char len;

	printk("*************System enter get domain info from server process*************** \r\n");

	//��ȡdomain
	len = receive_full_part(CMD_DOMAIN, 0, 1, temp);
	if (len)
	{
		strcpy(DOMAIN, temp);
		save_domain_info(temp);
		printk("Update domain info success\r\n");
	}

	return len;
}
