#include "rtc.h"
#include "lib.h"
#include "i8259.h"
int volatile rtc_open_close=0;
/*
Description: initialize real time clock
Input: none
Output: none
Side-effct: enable PIE and set frequency to 2hz
*/
void rtc_init()
{
	cli();
	//NMI_disable;
	outb(REGB,RTC_PORT); //select register B, and disable NMI
	char prev=inb(CMOS_PORT);
	outb(REGB,RTC_PORT);
	outb(prev|PIE_ENABLE,CMOS_PORT); //enable PIE by or with 0x40, set bit 6 in RegB to 1
	
	//set rate to 15 which is 2hz
	char rate=0x0f;
	outb(REGA,RTC_PORT);
	prev=inb(CMOS_PORT);
	outb(REGA,RTC_PORT);
	outb((prev & RESET_RV)|rate,CMOS_PORT);
	//NMI_enable;
	sti();
	enable_irq(SLAVE);
	enable_irq(RTC_IRQ);
}

/*
Description: rtc handler
Input: none
Output: none
Side-effct: read register C and reset its value
*/
void rtc_handler()
{
	cli();
	outb(REGC,RTC_PORT);
	inb(CMOS_PORT);  //not used so far
	//test_interrupts();
	rtc_interrupt_occurred=0;
	send_eoi(RTC_IRQ);
	sti();
}

/*
Description: rtc read
Input: none
Output: 0 if success
Side-effct: read until rtc handler clears the rtc_interrupt_occurred signal
*/
int32_t rtc_read(void* buf, uint32_t offset,int32_t nbytes,int8_t * name)
{
	rtc_interrupt_occurred=1;
	while(rtc_interrupt_occurred==1)
	{
		/* DO NOTHING */
	}
	//printf("1");
	return 0;
}


/*
Description: rtc write
Input: buf pointer
	   number of bytes
Output: number of bytes wrote if success, -1 if fail
Side-effct: set 4 RVs bits in register A
*/
int32_t rtc_write(int32_t fd, const int* buf, int32_t nbytes)
{
	if(nbytes!=4)
		return -1;
	int frequency = *buf;
	char rate;
	//1024 is the highest frequency we are allowed to set
	if(frequency>1024)
		return -1;
	//using the formulae rate = 32768 >> (rate-1)
	switch(frequency){
		case 1024:
			rate = 0x06;
			break;
		case 512:
			rate = 0x07;
			break;
		case 256:
			rate = 0x08;
			break;
		case 128:
			rate = 0x09;
			break;
		case 64:
			rate = 0x0A;
			break;
		case 32:
			rate = 0x0B;
			break;
		case 16:
			rate = 0x0C;
			break;
		case 8:
			rate = 0x0D;
			break;
		case 4:
			rate = 0x0E;
			break;
		case 2:
			rate = 0x0F;
			break;
		case 0:
			rate = 0x00;
			break;
		default:
			return -1;
	}
	outb(REGA,RTC_PORT);
	char prev=inb(CMOS_PORT);
	outb(REGA,RTC_PORT);
	outb((prev & RESET_RV)|rate,CMOS_PORT);
	return nbytes;
}

/*
Description: rtc open
Input: none
Output: 0 if success, -1 if fail
Side-effct: initialize rtc, set rtc_open_close flag
*/
int32_t rtc_open(const uint8_t* filename)
{
	/*if(rtc_open_close==1)
		return -1;
	rtc_init();
	rtc_open_close=1;*/
	return 0;
}

/*
Description: rtc close
Input: none
Output: 0 if success, -1 if fail
Side-effct: disable rtc, clear rtc_open_close flag 
*/
int32_t rtc_close()
{
	/*if(rtc_open_close==1)
	{
		disable_irq(RTC_IRQ);
		disable_irq(SLAVE);
		rtc_open_close=0;
		return 0;
	}
	return -1;*/
	return 0;
}


