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

//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
U8 SPI_Flash_ReadSR(void)
{
	U8 byte = 0;

	SPI_FLASH_Enable();					//使能器件
	SPI_Send_Byte(W25X_ReadStatusReg);	//发送读取状态寄存器命令
	byte = SPI_Send_Byte(0Xff);			//读取一个字节
	SPI_FLASH_Disable();				//取消片选

	return byte;
}

//写SPI_FLASH状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void SPI_FLASH_Write_SR(U8 sr)
{
	SPI_FLASH_Enable();					//使能器件
	SPI_Send_Byte(W25X_WriteStatusReg);	//发送写取状态寄存器命令
	SPI_Send_Byte(sr);					//写入一个字节
	SPI_FLASH_Disable();				//取消片选
}

//SPI_FLASH写使能
//将WEL置位
void SPI_FLASH_Write_Enable(void)
{
	SPI_FLASH_Enable();					//使能器件
	SPI_Send_Byte(W25X_WriteEnable);	//发送写使能
	SPI_FLASH_Disable();				//取消片选
}

//SPI_FLASH写禁止
//将WEL清零
void SPI_FLASH_Write_Disable(void)
{
	SPI_FLASH_Enable();					//使能器件
	SPI_Send_Byte(W25X_WriteDisable);	//发送写禁止指令
	SPI_FLASH_Disable();				//取消片选
}

//读取芯片ID W25X16的ID:0XEF14
U16 SPI_Flash_ReadID(void)
{
	U16 Temp = 0;

	SPI_FLASH_Enable();
	SPI_Send_Byte(W25X_ManufactDeviceID); //发送读取ID命令
	SPI_Send_Byte(0x00);
	SPI_Send_Byte(0x00);
	SPI_Send_Byte(0x00);
	Temp |= SPI_Send_Byte(0xFF) << 8;
	Temp |= SPI_Send_Byte(0xFF);
	SPI_FLASH_Disable();

	return Temp;
}

//读取SPI FLASH
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void SPI_Flash_Read(U8 *pBuffer, U32 ReadAddr, U16 NumByteToRead)
{
	U16 i;

	SPI_FLASH_Enable();						//使能器件
	SPI_Send_Byte(W25X_ReadData);			//发送读取命令
	SPI_Send_Byte((U8)((ReadAddr) >> 16));	//发送24bit地址
	SPI_Send_Byte((U8)((ReadAddr) >> 8));
	SPI_Send_Byte((U8)ReadAddr);
	for (i = 0; i < NumByteToRead; i++)
	{
		pBuffer[i] = SPI_Send_Byte(0XFF);	//循环读数
	}
	SPI_FLASH_Disable();					//取消片选
}

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
void SPI_Flash_Write_Page(U8 *pBuffer, U32 WriteAddr, U16 NumByteToWrite)
{
	U16 i;

	SPI_FLASH_Write_Enable();				//SET WEL
	SPI_FLASH_Enable();						//使能器件
	SPI_Send_Byte(W25X_PageProgram);		//发送写页命令
	SPI_Send_Byte((U8)((WriteAddr) >> 16));	//发送24bit地址
	SPI_Send_Byte((U8)((WriteAddr) >> 8));
	SPI_Send_Byte((U8)WriteAddr);
	for (i = 0; i < NumByteToWrite; i++)
	{
		SPI_Send_Byte(pBuffer[i]);			//循环写数
	}
	SPI_FLASH_Disable();					//取消片选
	SPI_Flash_Wait_Busy();					//等待写入结束
}

//无检验写SPI FLASH
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(U8 *pBuffer, U32 WriteAddr, U16 NumByteToWrite)
{
	U16 pageremain;

	pageremain = 256 - WriteAddr % 256;		//单页剩余的字节数
	if (NumByteToWrite <= pageremain)
	{
		pageremain = NumByteToWrite;		//不大于256个字节
	}
	while (1)
	{
		SPI_Flash_Write_Page(pBuffer, WriteAddr, pageremain);
		if (NumByteToWrite == pageremain)
		{
			break; //写入结束了
		}
		else //NumByteToWrite>pageremain
		{
			pBuffer += pageremain;
			WriteAddr += pageremain;

			NumByteToWrite -= pageremain;				//减去已经写入了的字节数
			if (NumByteToWrite > 256)
			{
				pageremain = 256;	//一次可以写入256个字节
			}
			else
			{
				pageremain = NumByteToWrite;			//不够256个字节了
			}
		}
	};
}

#if 0
//写SPI FLASH
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
U8 SPI_FLASH_BUF[4096];
void SPI_Flash_Write(U8 *pBuffer, U32 WriteAddr, U16 NumByteToWrite)
{
	U32 secpos;
	U16 secoff;
	U16 secremain;
	U16 i;

	secpos = WriteAddr / 4096;	//扇区地址 0~511 for w25x16
	secoff = WriteAddr % 4096;	//在扇区内的偏移
	secremain = 4096 - secoff;	//扇区剩余空间大小

	if (NumByteToWrite <= secremain)
	{
		secremain = NumByteToWrite;		//不大于4096个字节
	}
	while (1)
	{
		SPI_Flash_Read(SPI_FLASH_BUF, secpos * 4096, 4096);		//读出整个扇区的内容
		for (i = 0; i < secremain; i++)		//校验数据
		{
			if (SPI_FLASH_BUF[secoff + i] != 0XFF)
			{
				break;		//需要擦除
			}
		}
		if (i < secremain)	//需要擦除
		{
			SPI_Flash_Erase_Sector(secpos);		//擦除这个扇区
			for (i = 0; i < secremain; i++)		//复制
			{
				SPI_FLASH_BUF[i + secoff] = pBuffer[i];
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF, secpos * 4096, 4096);	//写入整个扇区
		}
		else
		{
			SPI_Flash_Write_NoCheck(pBuffer, WriteAddr, secremain);		//写已经擦除了的,直接写入扇区剩余区间.
		}
		if (NumByteToWrite == secremain)
		{
			break; //写入结束了
		}
		else //写入未结束
		{
			secpos++; //扇区地址增1
			secoff = 0; //偏移位置为0

			pBuffer += secremain;		//指针偏移
			WriteAddr += secremain;		//写地址偏移
			NumByteToWrite -= secremain;	//字节数递减
			if (NumByteToWrite > 4096)
			{
				secremain = 4096;		//下一个扇区还是写不完
			}
			else
			{
				secremain = NumByteToWrite;		//下一个扇区可以写完了
			}
		}
	}
}
#endif

//擦除整个芯片
//整片擦除时间:
//W25X16:25s
//W25X32:40s
//W25X64:40s
//等待时间超长...
void SPI_Flash_Erase_Chip(void)
{
	SPI_FLASH_Write_Enable();		//SET WEL
	SPI_Flash_Wait_Busy();
	SPI_FLASH_Enable();				//使能器件
	SPI_Send_Byte(W25X_ChipErase);	//发送片擦除命令
	SPI_FLASH_Disable();			//取消片选
	SPI_Flash_Wait_Busy();			//等待芯片擦除结束
}

//擦除一个扇区
//Dst_Addr:扇区地址 0~511 for w25x16
//擦除一个扇区的最少时间:150ms
void SPI_Flash_Erase_Sector(U32 Dst_Addr)
{
	Dst_Addr *= 4096;
	SPI_FLASH_Write_Enable();				//SET WEL
	SPI_Flash_Wait_Busy();
	SPI_FLASH_Enable();						//使能器件
	SPI_Send_Byte(W25X_SectorErase);		//发送扇区擦除指令
	SPI_Send_Byte((U8)((Dst_Addr) >> 16));	//发送24bit地址
	SPI_Send_Byte((U8)((Dst_Addr) >> 8));
	SPI_Send_Byte((U8)Dst_Addr);
	SPI_FLASH_Disable();					//取消片选
	SPI_Flash_Wait_Busy();					//等待擦除完成
}

//等待空闲
void SPI_Flash_Wait_Busy(void)
{
	while ((SPI_Flash_ReadSR() & 0x01) == 0x01);	// 等待BUSY位清空
}

//进入掉电模式
void SPI_Flash_PowerDown(void)
{
	SPI_FLASH_Enable();				//使能器件
	SPI_Send_Byte(W25X_PowerDown);	//发送掉电命令
	SPI_FLASH_Disable();			//取消片选
	delayNMicroSeconds(3);			//等待TPD
}

//唤醒
void SPI_Flash_WAKEUP(void)
{
	SPI_FLASH_Enable();						//使能器件
	SPI_Send_Byte(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB
	SPI_FLASH_Disable();					//取消片选
	delayNMicroSeconds(3);					//等待TRES1
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

//读取wifi信息
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
			if (num == 0)	//生产的wifi
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

//保存wifi信息
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

//读取注册信息：读取注册标志位MSG_SYS_REGISTER，同时把IMEI读出来
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

//保存注册信息：保存注册标志位MSG_SYS_REGISTER，同时保存IMEI
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

//读取域名信息
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

//保存注册信息：保存注册标志位MSG_SYS_REGISTER，同时保存IMEI
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

//恢复工厂设置
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


