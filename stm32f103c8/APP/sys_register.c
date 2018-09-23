#include "sys_register.h"

/*
新增加约定信息
1、
棋盘初次注册服务器返回16bit验证码处理（data为16bit 返回的验证码）
INT32U tmp_data1 = data;
INT32U tmp_data2 = data;
tmp_data1 <<= 2;
tmp_data2 |= tmp_data1;//构成18bit数据
这样每3个bit用于数码管显示，即显示范围（0-7）

2、
请求WIFI数据格式：
第一个字节定义
高3位：命令码
低5位：请求第n个wifi 信息
注：请求wifi信息约定从第0个开始

请求示例:
b'011 00000 (0x60)请求第0个wifi 的信息
b'011 00001 (0x61)请求第1个wifi 的信息

3、
注册中密钥 初始密钥0xff
初始密钥用于，本地注册加密，服务器注册端解密

当机器已经注册，8位算法密钥约定如下，按照约定顺序在CPUID中取8bit
分别取 CPUID中第1 3 5 7 字节中高两位组成一个字节的key
示例：
cpuid[1] = b'10 xxxxx
cpuid[3] = b'11 xxxxx
cpuid[5] = b'10 xxxxx
cpuid[7] = b'11 xxxxx

==>
key = b'10 11 10 11(8bit)=0xBB

4、注册时收到验证码 心跳包上传第14个字节最高位置位

注册开始提示：
中间6位数码管显示F

注册收到服务器返回6个数字
心跳包第14个字节 第7bit 置位

此时等待用户拍键 用户拍键后
心跳包第14个字节 第6bit 置位

等待服务器返回握手命令
心跳包第14个自己 第5bit 置位

本地上传握手成功
心跳包第14个自己 第4bit 置位

第一步：上传cpuid。（10秒内，重试3次，然后每10秒试一次。一分钟后循环）
第二部：下发imei和验证码。
第三步：上传确认（棋盘一直显示验证码。除非棋盘重启回到第一步）。
第四步：只要用户拍黑键，就上传注册请求（10秒内，尝试3次，没有响应就等下一次用户拍黑键。除非棋盘重启 回到第一步）
第五步：服务器下发成功。
第六步：回应服务器收到成功命令。

完成以上步骤即可完整整个注册流程


SMARTWIFI模式
中间6位数码管显示1
连接成功后显示2
连接失败后显示3

低电数码管提示
低电告警数码管整体显示FF

注册目前不加密注册已经没问题了
*/


/*
 * 将服务器返回的16bit 数据按照备注说明规则转换成18bit数据显示的6个数码管上
 *
 * @data 16位数据
 *
 * @返回值 无
 *
 * 说明：
 *  棋盘初次注册服务器返回16bit验证码处理（data为16bit 返回的验证码 ）
 *	INT32U tmp_data1 = data;
 *	INT32U tmp_data2 = data;
 *	tmp_data1 <<= 2;
 *	tmp_data2 |= tmp_data1;//构成18bit数据
 *	这样每3个bit用于数码管显示，即显示范围（0-7）
 */
void Digital_Display(INT16U data)
{
	INT32U tmp_data1 = data;
	INT32U tmp_data2 = data;

	tmp_data1 <<= 2;
	tmp_data2 |= tmp_data1; //构成18bit数据

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
 * 注册信息解析
 *
 * @返回值 		  无
 * 说明：
 * 本地模拟解析指令
 * 非加密：40 01 53 FF 6E 06 48 85 57 57 56 29 11 87 00 00
 * 加密：  40 01 e1 00 a1 f9 6d 7a e9 a8 14 d6 67 78 f0 00
 * 判断加密是否正确可以使用以上指令模拟验证
 * 注册后IMEI : 53-ff-6e-6-48-85
 * 注册服务器时间
 * 2016年6月7日 7:18:1
 * 更新时间
 * 验证码=3587
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
				if ((data_buf[1] >> 5) == 2) //注册命令
				{
					printk("注册前IMEI : ");
					printf_buf_hex(IMEI, sizeof(IMEI));
					memcpy(IMEI, &data_buf[3], sizeof(IMEI));
					printk("注册后IMEI : ");
					printf_buf_hex(IMEI, sizeof(IMEI));

					update_machine_time(&data_buf[9]);

					//get codes and show LED
					verify_code = data_buf[13];
					verify_code <<= 8;
					verify_code |= data_buf[14];
					printk("验证码=0x%x \r\n", verify_code);
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
 * 获取两段信息从服务器 并在本地组合
 *
 * @cmd： 获取内容标志
 * @num: 同一内容中的第几个
 * @step_base: 例如获取wifi 其中一组的ssid,分成两段来获取 step码为1 2 那么对于wifi 信息来说还包含密码 用step 3 4 来代替 例如获取WIFI信息
 * @msg：获取内容保存
 *
 * @返回值
 *  1 成功
 *  0 失败
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
 * 只要服务器上WIFI信息状态发生变化 就更新本地所有保存信息 不做对比
 *
 * @参数表列 无
 *
 * @返回值
 *  非0 成功
 *  0  失败
 */
unsigned char update_wifi_info(unsigned char wifi_no)
{
	unsigned char ssid[25];
	unsigned char key[25];
	unsigned char len;

	printk("*************System enter get wifi info from server process*************** \r\n");
	//获取ssid
	len = receive_full_part(CMD_WIFI, wifi_no, 1, ssid);
	if (len)
	{
		//获取password
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
 * 获取域名信息
 *
 * @参数表列 无
 *
 * @返回值
 *  非0 成功
 *  0   失败
 *  备注：域名获取还未保存到本地
 */
unsigned char update_domain_info(void)
{
	unsigned char temp[25];
	unsigned char len;

	printk("*************System enter get domain info from server process*************** \r\n");

	//获取domain
	len = receive_full_part(CMD_DOMAIN, 0, 1, temp);
	if (len)
	{
		strcpy(DOMAIN, temp);
		save_domain_info(temp);
		printk("Update domain info success\r\n");
	}

	return len;
}
