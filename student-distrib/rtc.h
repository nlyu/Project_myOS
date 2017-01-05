#ifndef RTC_H
#define RTC_H

#include "lib.h"

#define RTC_IRQ 8
#define RTC_PORT 0x70
#define CMOS_PORT 0x71
#define PIE_ENABLE 0x40
#define REGA 0x8A
#define REGB 0x8B
#define REGC 0x8C
#define RESET_RV 0xf0
#define SLAVE 2

int volatile rtc_interrupt_occurred;
void rtc_init();
void rtc_handler();
int32_t rtc_read(void* buf, uint32_t offset,int32_t nbytes,int8_t * name);
int32_t rtc_write(int32_t fd, const int* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close();
#endif

