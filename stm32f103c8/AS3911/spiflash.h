#ifndef __FLASH_H
#define __FLASH_H

#include "ams_types.h"


//W25X16��д
#define FLASH_ID		0xEF14
//ָ���
#define W25X_WriteEnable		0x06
#define W25X_WriteDisable		0x04
#define W25X_ReadStatusReg		0x05
#define W25X_WriteStatusReg		0x01
#define W25X_ReadData			0x03
#define W25X_FastReadData		0x0B
#define W25X_FastReadDual		0x3B
#define W25X_PageProgram		0x02
#define W25X_BlockErase			0xD8
#define W25X_SectorErase		0x20
#define W25X_ChipErase			0xC7
#define W25X_PowerDown			0xB9
#define W25X_ReleasePowerDown	0xAB
#define W25X_DeviceID			0xAB
#define W25X_ManufactDeviceID	0x90
#define W25X_JedecDeviceID		0x9F


u16  SPI_Flash_ReadID(void);  	    //��ȡFLASH ID
u8	 SPI_Flash_ReadSR(void);        //��ȡ״̬�Ĵ���
void SPI_FLASH_Write_SR(U8 sr);  	//д״̬�Ĵ���
void SPI_FLASH_Write_Enable(void);  //дʹ��
void SPI_FLASH_Write_Disable(void);	//д����
void SPI_Flash_Read(U8* pBuffer, U32 ReadAddr, U16 NumByteToRead);   //��ȡflash
void SPI_Flash_Write(U8* pBuffer, U32 WriteAddr, U16 NumByteToWrite);//д��flash
void SPI_Flash_Erase_Chip(void);    	  //��Ƭ����
void SPI_Flash_Erase_Sector(U32 Dst_Addr);//��������
void SPI_Flash_Wait_Busy(void);           //�ȴ�����
void SPI_Flash_PowerDown(void);           //�������ģʽ
void SPI_Flash_WAKEUP(void);			  //����
void SPI_Flash_Write_Page(U8 *pBuffer, U32 WriteAddr, U16 NumByteToWrite);



#define		SPI_DATA_START_ADDR			0
#define		SPI_DATA_CAPACITY			0x1f0000
#define		BYTES_PER_GAME				(3*4096)
#define		DATA_SEND_LEN				16
#define		BYTES_PER_STEP				DATA_SEND_LEN
#define		MAX_STEPS_PER_GAME			(BYTES_PER_GAME/BYTES_PER_STEP)
#define		SPI_BYTES_PER_SECTOR		4096
#define		SPI_BYTES_PER_PAGE			256
#define		ENCRYPT_LEN					12


#define SYS_INFO_OFFSET					0x1f0000
#define SYS_INFO_LEN					256

#define REGISTER_INFO_OFFSET			0x1f1000
#define REGISTER_FLAG					0x5a
typedef struct{
	U8 register_flag;
	U8 imei[6];
}REGISTER_STRUCT;

#define ANTENNA_INFO_OFFSET				0x1f2000

#define DOMAIN_INFO_OFFSET				0x1f3000
#define DOMAIN_FLAG						0x5a
#define DOMAIN_MAX_LEN					25
typedef struct{
	U8 domain_flag;
	U8 domain[DOMAIN_MAX_LEN];
}DOMAIN_STRUCT;

#define WIFI1_OFFSET					0x1f5000
#define WIFI2_OFFSET					0x1f6000
#define WIFI3_OFFSET					0x1f7000
#define WIFI4_OFFSET					0x1f8000
#define WIFI5_OFFSET					0x1f9000
#define WIFI6_OFFSET					0x1fa000
#define WIFI7_OFFSET					0x1fb000
#define WIFI8_OFFSET					0x1fc000
#define WIFI9_OFFSET					0x1fd000
#define WIFI10_OFFSET					0x1fe000
#define WIFI_SPACE_LEN					0x1000
#define MAX_WIFI_NUM					5
#define WIFI_MAX_LEN					25
typedef struct{
	char SSID[WIFI_MAX_LEN];
	char KEY[WIFI_MAX_LEN];
	U8 checksum;
}WIFI_STRUCT;

#define SPI_TEST_OFFSET					0x1ff000


extern const char *tcp_ip;

extern void SysinfoRead(void);
extern void SysinfoSave(void);
extern U8 read_wifi(U32 num, U8* ssid, U8* key);
extern U8 save_wifi(U32 num, U8* ssid, U8* key);
extern void read_register_info(void);
extern void save_register_info(void);
extern U8 read_domain_info(unsigned char *domain);
extern void save_domain_info(unsigned char *domain);
extern U8 read_game_step(U32 game_no, U32 step_no, U8* step_buff);


extern void testSpiflash(void);

extern void factory_default(void);

#endif

