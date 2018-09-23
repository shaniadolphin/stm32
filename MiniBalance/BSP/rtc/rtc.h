#ifndef __RTC_H
#define __RTC_H

#include "stm32f10x.h"

u8 RTC_Get(u32 timecount);
u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec);
u8 RTC_Set_Unix(u32 unixtime);
void rtcirqprocess(void);
void RTC_Configuration(void);
#endif
