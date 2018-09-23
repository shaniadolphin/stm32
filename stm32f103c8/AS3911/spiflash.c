#include "stm32f10x.h"
#include "spi.h"
#include "stm32f10x_spi.h"
#include "delay.h"
#include "spiflash.h"
#include "nfc.h"
#include "Msg.h"


void SPI_TYPE_FLASH(void)
{
	SPI_Cmd(SPI2, DISABLE);
	SPI2->CR1 |= SPI_CPOL_High;
	SPI_Cmd(SPI2, ENABLE);
}
void SPI_TYPE_NFC(void)
{
	SPI_Cmd(SPI2, DISABLE);
	SPI2->CR1 &= ~SPI_CPOL_High;
	SPI_Cmd(SPI2, ENABLE);
}

void SPI_FLASH_Enable()
{
	U8 os_err;
	OSSemPend(SemSpiflash, 0, &os_err);
	SPI_TYPE_FLASH();
	GPIO_ResetBits(GPIOB, GPIO_Pin_10);
}
void SPI_FLASH_Disable()
{
	GPIO_SetBits(GPIOB, GPIO_Pin_10);
	SPI_TYPE_NFC();
	OSSemPost(SemSpiflash);
}

//��ȡSPI_FLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
U8 SPI_Flash_ReadSR(void)
{
	U8 byte = 0;

	SPI_FLASH_Enable();					//ʹ������
	SPI_Send_Byte(W25X_ReadStatusReg);	//���Ͷ�ȡ״̬�Ĵ�������
	byte = SPI_Send_Byte(0Xff);			//��ȡһ���ֽ�
	SPI_FLASH_Disable();				//ȡ��Ƭѡ

	return byte;
}

//дSPI_FLASH״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void SPI_FLASH_Write_SR(U8 sr)
{
	SPI_FLASH_Enable();					//ʹ������
	SPI_Send_Byte(W25X_WriteStatusReg);	//����дȡ״̬�Ĵ�������
	SPI_Send_Byte(sr);					//д��һ���ֽ�
	SPI_FLASH_Disable();				//ȡ��Ƭѡ
}

//SPI_FLASHдʹ��
//��WEL��λ
void SPI_FLASH_Write_Enable(void)
{
	SPI_FLASH_Enable();					//ʹ������
	SPI_Send_Byte(W25X_WriteEnable);	//����дʹ��
	SPI_FLASH_Disable();				//ȡ��Ƭѡ
}

//SPI_FLASHд��ֹ
//��WEL����
void SPI_FLASH_Write_Disable(void)
{
	SPI_FLASH_Enable();					//ʹ������
	SPI_Send_Byte(W25X_WriteDisable);	//����д��ָֹ��
	SPI_FLASH_Disable();				//ȡ��Ƭѡ
}

//��ȡоƬID W25X16��ID:0XEF14
U16 SPI_Flash_ReadID(void)
{
	U16 Temp = 0;

	SPI_FLASH_Enable();
	SPI_Send_Byte(W25X_ManufactDeviceID); //���Ͷ�ȡID����
	SPI_Send_Byte(0x00);
	SPI_Send_Byte(0x00);
	SPI_Send_Byte(0x00);
	Temp |= SPI_Send_Byte(0xFF) << 8;
	Temp |= SPI_Send_Byte(0xFF);
	SPI_FLASH_Disable();

	return Temp;
}

//��ȡSPI FLASH
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void SPI_Flash_Read(U8 *pBuffer, U32 ReadAddr, U16 NumByteToRead)
{
	U16 i;

	SPI_FLASH_Enable();						//ʹ������
	SPI_Send_Byte(W25X_ReadData);			//���Ͷ�ȡ����
	SPI_Send_Byte((U8)((ReadAddr) >> 16));	//����24bit��ַ
	SPI_Send_Byte((U8)((ReadAddr) >> 8));
	SPI_Send_Byte((U8)ReadAddr);
	for (i = 0; i < NumByteToRead; i++)
	{
		pBuffer[i] = SPI_Send_Byte(0XFF);	//ѭ������
	}
	SPI_FLASH_Disable();					//ȡ��Ƭѡ
}

//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!
void SPI_Flash_Write_Page(U8 *pBuffer, U32 WriteAddr, U16 NumByteToWrite)
{
	U16 i;

	SPI_FLASH_Write_Enable();				//SET WEL
	SPI_FLASH_Enable();						//ʹ������
	SPI_Send_Byte(W25X_PageProgram);		//����дҳ����
	SPI_Send_Byte((U8)((WriteAddr) >> 16));	//����24bit��ַ
	SPI_Send_Byte((U8)((WriteAddr) >> 8));
	SPI_Send_Byte((U8)WriteAddr);
	for (i = 0; i < NumByteToWrite; i++)
	{
		SPI_Send_Byte(pBuffer[i]);			//ѭ��д��
	}
	SPI_FLASH_Disable();					//ȡ��Ƭѡ
	SPI_Flash_Wait_Busy();					//�ȴ�д�����
}

//�޼���дSPI FLASH
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ����
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(U8 *pBuffer, U32 WriteAddr, U16 NumByteToWrite)
{
	U16 pageremain;

	pageremain = 256 - WriteAddr % 256;		//��ҳʣ����ֽ���
	if (NumByteToWrite <= pageremain)
	{
		pageremain = NumByteToWrite;		//������256���ֽ�
	}
	while (1)
	{
		SPI_Flash_Write_Page(pBuffer, WriteAddr, pageremain);
		if (NumByteToWrite == pageremain)
		{
			break; //д�������
		}
		else //NumByteToWrite>pageremain
		{
			pBuffer += pageremain;
			WriteAddr += pageremain;

			NumByteToWrite -= pageremain;				//��ȥ�Ѿ�д���˵��ֽ���
			if (NumByteToWrite > 256)
			{
				pageremain = 256;	//һ�ο���д��256���ֽ�
			}
			else
			{
				pageremain = NumByteToWrite;			//����256���ֽ���
			}
		}
	};
}

#if 0
//дSPI FLASH
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
U8 SPI_FLASH_BUF[4096];
void SPI_Flash_Write(U8 *pBuffer, U32 WriteAddr, U16 NumByteToWrite)
{
	U32 secpos;
	U16 secoff;
	U16 secremain;
	U16 i;

	secpos = WriteAddr / 4096;	//������ַ 0~511 for w25x16
	secoff = WriteAddr % 4096;	//�������ڵ�ƫ��
	secremain = 4096 - secoff;	//����ʣ��ռ��С

	if (NumByteToWrite <= secremain)
	{
		secremain = NumByteToWrite;		//������4096���ֽ�
	}
	while (1)
	{
		SPI_Flash_Read(SPI_FLASH_BUF, secpos * 4096, 4096);		//������������������
		for (i = 0; i < secremain; i++)		//У������
		{
			if (SPI_FLASH_BUF[secoff + i] != 0XFF)
			{
				break;		//��Ҫ����
			}
		}
		if (i < secremain)	//��Ҫ����
		{
			SPI_Flash_Erase_Sector(secpos);		//�����������
			for (i = 0; i < secremain; i++)		//����
			{
				SPI_FLASH_BUF[i + secoff] = pBuffer[i];
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF, secpos * 4096, 4096);	//д����������
		}
		else
		{
			SPI_Flash_Write_NoCheck(pBuffer, WriteAddr, secremain);		//д�Ѿ������˵�,ֱ��д������ʣ������.
		}
		if (NumByteToWrite == secremain)
		{
			break; //д�������
		}
		else //д��δ����
		{
			secpos++; //������ַ��1
			secoff = 0; //ƫ��λ��Ϊ0

			pBuffer += secremain;		//ָ��ƫ��
			WriteAddr += secremain;		//д��ַƫ��
			NumByteToWrite -= secremain;	//�ֽ����ݼ�
			if (NumByteToWrite > 4096)
			{
				secremain = 4096;		//��һ����������д����
			}
			else
			{
				secremain = NumByteToWrite;		//��һ����������д����
			}
		}
	}
}
#endif

//��������оƬ
//��Ƭ����ʱ��:
//W25X16:25s
//W25X32:40s
//W25X64:40s
//�ȴ�ʱ�䳬��...
void SPI_Flash_Erase_Chip(void)
{
	SPI_FLASH_Write_Enable();		//SET WEL
	SPI_Flash_Wait_Busy();
	SPI_FLASH_Enable();				//ʹ������
	SPI_Send_Byte(W25X_ChipErase);	//����Ƭ��������
	SPI_FLASH_Disable();			//ȡ��Ƭѡ
	SPI_Flash_Wait_Busy();			//�ȴ�оƬ��������
}

//����һ������
//Dst_Addr:������ַ 0~511 for w25x16
//����һ������������ʱ��:150ms
void SPI_Flash_Erase_Sector(U32 Dst_Addr)
{
	Dst_Addr *= 4096;
	SPI_FLASH_Write_Enable();				//SET WEL
	SPI_Flash_Wait_Busy();
	SPI_FLASH_Enable();						//ʹ������
	SPI_Send_Byte(W25X_SectorErase);		//������������ָ��
	SPI_Send_Byte((U8)((Dst_Addr) >> 16));	//����24bit��ַ
	SPI_Send_Byte((U8)((Dst_Addr) >> 8));
	SPI_Send_Byte((U8)Dst_Addr);
	SPI_FLASH_Disable();					//ȡ��Ƭѡ
	SPI_Flash_Wait_Busy();					//�ȴ��������
}

//�ȴ�����
void SPI_Flash_Wait_Busy(void)
{
	while ((SPI_Flash_ReadSR() & 0x01) == 0x01);	// �ȴ�BUSYλ���
}

//�������ģʽ
void SPI_Flash_PowerDown(void)
{
	SPI_FLASH_Enable();				//ʹ������
	SPI_Send_Byte(W25X_PowerDown);	//���͵�������
	SPI_FLASH_Disable();			//ȡ��Ƭѡ
	delayNMicroSeconds(3);			//�ȴ�TPD
}

//����
void SPI_Flash_WAKEUP(void)
{
	SPI_FLASH_Enable();						//ʹ������
	SPI_Send_Byte(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB
	SPI_FLASH_Disable();					//ȡ��Ƭѡ
	delayNMicroSeconds(3);					//�ȴ�TRES1
}


extern SPI_CFG cfg;
U8 IMEI[6];
static U8 InfoToWritePage;
void SysinfoRead(void)
{
	int i;
	U16 checksum;
	U32 dirty_flag;
	U32 addr;

	for (i=0; i<SPI_BYTES_PER_SECTOR/(SPI_BYTES_PER_PAGE); i++)
	{
		addr = SYS_INFO_OFFSET + i*SPI_BYTES_PER_PAGE;
		SPI_Flash_Read((U8*)&dirty_flag, addr, sizeof(dirty_flag));
		if (dirty_flag == 0xffffffff)
		{
			break;
		}
	}

	InfoToWritePage = (U8)i;
	if (i == 0)	//no record
	{
		addr = SYS_INFO_OFFSET;
	}
	else
	{
		addr = SYS_INFO_OFFSET + ((U32)InfoToWritePage-1)*SPI_BYTES_PER_PAGE;
	}

	if (sizeof(SPI_CFG) != 256)
	{
		for (i = 0; i < 10; i++)
		{
			printk("############# Warning !!!\r\n");
			printk("############# Error sizeof(SPI_CFG) = %d !!!\r\n", sizeof(SPI_CFG));
		}
	}
	memset((U8*)(&cfg), 0, sizeof(SPI_CFG));
	SPI_Flash_Read((U8*)(&cfg), addr, sizeof(SPI_CFG));
	checksum = crcCalculateCcitt(0xffff, (U8*)(&cfg), sizeof(SPI_CFG) - sizeof(cfg.check_sum));
	if (checksum != cfg.check_sum)
	{   //init
		printk("Read Crc Error!\r\n");
		cfg.curr_game = MAX_GAME_CNT;
		for (i = 0; i < MAX_GAME_CNT; i++)
		{
			cfg.game_steps_cnt[i] = 0;
		}
	}

	printk("Read Page = %d, checksum = 0x%x, Calc checksum = 0x%x\r\n", InfoToWritePage>0?InfoToWritePage-1:0, cfg.check_sum, checksum);
	printf_buf_hex((U8*)(&cfg), sizeof(SPI_CFG));
	printk("\r\n");
}

void SysinfoSave(void)
{
	U16 checksum;
	U32 addr;

	checksum = crcCalculateCcitt(0xffff, (U8*)(&cfg), sizeof(SPI_CFG) - sizeof(cfg.check_sum));
	cfg.check_sum = checksum;

	if (InfoToWritePage >= SPI_BYTES_PER_SECTOR/(SPI_BYTES_PER_PAGE))
	{
		SPI_Flash_Erase_Sector(SYS_INFO_OFFSET/SPI_BYTES_PER_SECTOR);
		InfoToWritePage = 0;
		printk("######## Erase sysinfo area!\r\n");
	}
	addr = SYS_INFO_OFFSET + (U32)InfoToWritePage*SPI_BYTES_PER_PAGE;
	SPI_Flash_Write_Page((U8*)(&cfg), addr, sizeof(SPI_CFG));

	printk("Save InfoToWritePage = %d, checksum = 0x%x, Calc checksum = 0x%x\r\n", InfoToWritePage, cfg.check_sum, checksum);
	printf_buf_hex((U8*)(&cfg), sizeof(SPI_CFG));
	printk("\r\n");

	InfoToWritePage += 1;
}

void testSpiflash(void)
{
	U8 buff[BYTES_PER_STEP] = { 0 };
	U8 msg_buff[BYTES_PER_STEP] = { 0 };
	U32 addr = SPI_TEST_OFFSET;

	if (SPI_Flash_ReadID() != 0xef14)
	{
		printk("###### Spiflash Id Error !!! ###### \r\n");
	}

	memset(msg_buff, 1, BYTES_PER_STEP);
	SPI_Flash_Erase_Sector(addr / SPI_BYTES_PER_SECTOR);
	SPI_Flash_Write_Page(msg_buff, addr, BYTES_PER_STEP);
	SPI_Flash_Read(buff, addr, BYTES_PER_STEP);
	if (memcmp(msg_buff, buff, BYTES_PER_STEP))
	{
		printk("###### Spiflash Read Write Error !!! ###### \r\n");
	}
}

void testflash(void)
{
	#if 0
	SPI_FLASH_Enable();
	SPI_Send_Byte(W25X_WriteDisable);
	SPI_FLASH_Disable();
	delayNMicroSeconds(1000);
	//SPI_Flash_Erase_Chip();

	if (1)
	{
		int i, j;
		U8 *pbuf;

		pbuf = (U8*)(&cfg);

		SPI_Flash_Erase_Sector(SYS_INFO_OFFSET/SPI_BYTES_PER_SECTOR);
		delayNMicroSeconds(1000);
		SPI_Flash_Read((U8*)(&cfg), SYS_INFO_OFFSET, sizeof(SPI_CFG));
		for (i=0; i<sizeof(SPI_CFG); i++)
		{
			if ((i%16) == 0)
			{
				printk("\r\n");
			}
			printk("%02x ", *(pbuf+i));
		}
		printk("\r\n");

		for (j=0; j<254; j++)
		{
			SPI_Flash_Erase_Sector(SYS_INFO_OFFSET / SPI_BYTES_PER_SECTOR);
			delayNMicroSeconds(1000);
			memset(pbuf, j, sizeof(SPI_CFG));
			SPI_Flash_Write_Page((U8*)(&cfg), SYS_INFO_OFFSET, sizeof(SPI_CFG));
			delayNMicroSeconds(1000);
			memset(pbuf, 0, sizeof(SPI_CFG));
			SPI_Flash_Read((U8*)(&cfg), SYS_INFO_OFFSET, sizeof(SPI_CFG));
			for (i=0; i<sizeof(SPI_CFG); i++)
			{
				if ((i%16) == 0)
				{
					printk("\r\n");
				}
				printk("%02x ", *(pbuf+i));
			}
			printk("\r\n");
		}
	}

	//SPI_FLASH_Write_SR(2);
	SPI_FLASH_Write_Enable();
	printk("SPI_Status Reg - 1 = 0x%x\r\n", SPI_Flash_ReadKK(0x05));
	printk("SPI_Status Reg - 2 = 0x%x\r\n", SPI_Flash_ReadKK(0x35));

	//printk("SPI_Flash_ReadSR = 0x%x\r\n", SPI_Flash_ReadSR(0x05));
	while (1)   //test
	{
		U16 spi_id;
		spi_id = SPI_Flash_ReadID();
		printk("SPI_Flash_ReadID = 0x%x\r\n", spi_id);
		OSTimeDly(300);
		SysinfoRead();
		SysinfoSave();
	}
	#endif
}

//��ȡwifi��Ϣ
U8 read_wifi(U32 num, U8* ssid, U8* key)
{
	if (num < 5)
	{
		WIFI_STRUCT wifi;
		U32 addr = WIFI1_OFFSET + num*WIFI_SPACE_LEN;
		U8 checksum;

		SPI_Flash_Read((U8*)&wifi, addr, sizeof(WIFI_STRUCT));
		checksum = calc_sum((U8*)&wifi, sizeof(WIFI_STRUCT)-1);
		if (checksum == wifi.checksum)
		{
			memcpy(ssid, wifi.SSID, WIFI_MAX_LEN);
			memcpy(key, wifi.KEY, WIFI_MAX_LEN);
			return 1;
		}
		else
		{
			if (num == 0)	//������wifi
			{
				strcpy(ssid, "qxtx");
				strcpy(key, "03150315");
				return 1;
			}
			if (num == 1)
			{
				strcpy(ssid, "qxtx");
				strcpy(key, "03150315");
				return 1;
			}
			if (num == 2)
			{
				strcpy(ssid, "chess4u");
				strcpy(key, "03150315");
				return 1;
			}
			memset(ssid, 0, WIFI_MAX_LEN);
			memset(key, 0, WIFI_MAX_LEN);
			return 0;
		}
	}
	return 0;
}

//����wifi��Ϣ
U8 save_wifi(U32 num, U8* ssid, U8* key)
{
	if (num < 5)
	{
		WIFI_STRUCT wifi;
		U32 addr = WIFI1_OFFSET + num*WIFI_SPACE_LEN;

		strncpy(wifi.SSID, ssid, WIFI_MAX_LEN-1);
		strncpy(wifi.KEY, key, WIFI_MAX_LEN-1);
		wifi.SSID[WIFI_MAX_LEN-1] = 0;
		wifi.KEY[WIFI_MAX_LEN-1] = 0;
		if (*ssid)
		{
			wifi.checksum = calc_sum((U8*)&wifi, sizeof(WIFI_STRUCT) - 1);
		}
		else
		{
			wifi.checksum = 0xff;
		}
		SPI_Flash_Erase_Sector(addr / SPI_BYTES_PER_SECTOR);
		SPI_Flash_Write_Page((U8*)&wifi, addr, sizeof(WIFI_STRUCT));
		return 1;
	}
	return 0;
}

//��ȡע����Ϣ����ȡע���־λMSG_SYS_REGISTER��ͬʱ��IMEI������
void read_register_info(void)
{
	REGISTER_STRUCT register_info;

	SPI_Flash_Read((U8*)&register_info, REGISTER_INFO_OFFSET, sizeof(REGISTER_STRUCT));
	if (register_info.register_flag == REGISTER_FLAG)
	{
		memcpy(IMEI, register_info.imei, sizeof(IMEI));
		SetMsg(FLAG_SYS_REGISTER);
	}
	else
	{
		memset(IMEI, 0, sizeof(IMEI));
		ClearMsg(FLAG_SYS_REGISTER);
	}

	#if 0		//test, must remove in product
	SetMsg(FLAG_SYS_REGISTER);
	IMEI[0] = 0x00;
	IMEI[1] = 0x01;
	IMEI[2] = 0x23;
	IMEI[3] = 0x45;
	IMEI[4] = 0x67;
	IMEI[5] = 0x89;
	#endif
}

//����ע����Ϣ������ע���־λMSG_SYS_REGISTER��ͬʱ����IMEI
void save_register_info(void)
{
	REGISTER_STRUCT register_info;

	if (CheckMsg(FLAG_SYS_REGISTER))
	{
		memcpy(register_info.imei, IMEI, sizeof(IMEI));
		register_info.register_flag = REGISTER_FLAG;
	}
	else
	{
		memset(register_info.imei, 0, sizeof(register_info.imei));
		memset(IMEI, 0, sizeof(IMEI));
		register_info.register_flag = 0;
	}
	SPI_Flash_Erase_Sector(REGISTER_INFO_OFFSET / SPI_BYTES_PER_SECTOR);
	SPI_Flash_Write_Page((U8*)&register_info, REGISTER_INFO_OFFSET, sizeof(REGISTER_STRUCT));
}

//��ȡ������Ϣ
U8 read_domain_info(unsigned char *domain)
{
	DOMAIN_STRUCT domain_info;

	SPI_Flash_Read((U8*)&domain_info, DOMAIN_INFO_OFFSET, sizeof(REGISTER_STRUCT));
	if (domain_info.domain_flag == DOMAIN_FLAG)
	{
		strcpy(domain, domain_info.domain);
	}
	else
	{
		strcpy(domain, tcp_ip);
	}
}

//����ע����Ϣ������ע���־λMSG_SYS_REGISTER��ͬʱ����IMEI
void save_domain_info(unsigned char *domain)
{
	DOMAIN_STRUCT domain_info;

	if (*domain)
	{
		strncpy(domain_info.domain, domain, sizeof(domain_info.domain));
		domain_info.domain[DOMAIN_MAX_LEN-1] = 0;
		domain_info.domain_flag = DOMAIN_FLAG;
	}
	else
	{
		memset(domain_info.domain, 0, sizeof(domain_info.domain));
		domain_info.domain_flag = 0;
	}
	SPI_Flash_Erase_Sector(DOMAIN_INFO_OFFSET / SPI_BYTES_PER_SECTOR);
	SPI_Flash_Write_Page((U8*)&domain_info, DOMAIN_INFO_OFFSET, sizeof(DOMAIN_STRUCT));
}

//�ָ���������
void factory_default(void)
{
	memset((U8*)&cfg, 0, sizeof(cfg));
	SysinfoSave();

	ClearMsg(FLAG_SYS_REGISTER);
	save_register_info();

	save_domain_info((unsigned char*)tcp_ip);

	save_wifi(0, "", "");
	save_wifi(1, "qxtx", "03150315");
	save_wifi(2, "chess4u", "03150315");
	save_wifi(3, "", "");
	save_wifi(4, "", "");

	printk("factory_default\r\n");
}


