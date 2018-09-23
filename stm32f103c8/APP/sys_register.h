
#ifndef _SYS_REGISTER_H_
#define _SYS_REGISTER_H_
#include "includes.h"
#include "Msg.h"
#include "nfc.h"
#include "spiflash.h"


extern void printk(char *Data, ...);


//±æµÿΩ‚√‹KEY
extern unsigned char encrypt_key;
extern unsigned char WifiStatusFlag;
extern char msg[64];
extern INT8U IMEI[6];
extern INT8U CpuID[12];
extern unsigned char DOMAIN[25];
//time
extern unsigned int UnixTime;
extern unsigned int Time_year;
extern unsigned char Time_month;
extern unsigned char Time_date;
extern unsigned char Time_hour;
extern unsigned char Time_min;
extern unsigned char Time_sec;
extern unsigned char data_buf[32];
extern unsigned char display_dat_buf[13];

extern void EncryptData(unsigned char *data, int length, unsigned char key);
extern unsigned char update_wifi_info(unsigned char wifi_no);
extern void update_machine_time(unsigned char *time_buf);
extern void send_cpuid_version(unsigned char cmd, unsigned short step, unsigned char *cpuid, unsigned char state, unsigned char key);
extern void send_cut_part(unsigned char cmd, unsigned char number, unsigned char part, unsigned char *buf, unsigned char len);
extern unsigned char send_chess_data(unsigned char *chess_buf, unsigned char historyflag);
extern void SendHeartbeat(unsigned char cmd, unsigned short step, unsigned int time_stamp, unsigned char percent, unsigned char state);
extern unsigned char receive_factory_produce_result(unsigned char key);
extern unsigned char update_domain_info(void);
extern unsigned char receive_imei_info(void);


#endif

