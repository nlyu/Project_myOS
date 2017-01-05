#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#define NUM_BYTES 4
#define NOT_USED 0
//test open and close 
/*void test_open_close()
{
	int32_t result;
	printf("starting test\n");
	printf("try open rtc\n");
	result = rtc_open();
	if(result==-1)
		printf("open fail, rtc_open is working\n");
	if(result==0)
		printf("open success, rtc_open is not working\n");
	result = rtc_close();
	if(result==-1)
		printf("close fail, rtc_close is not working\n");
	if(result==0)
		printf("close success, rtc_close is working\n");
	result = rtc_open();
	if(result==0)
		printf("rtc opens again successfully\n");
	if(result==-1)
		printf("rtc_open goes wrong\n");
}
//test write 
void test_write(int32_t frequency)
{
	rtc_write(NOT_USED,&frequency,NUM_BYTES);
}
//test read 
void test_read()
{
	sti();
	while(1)
	{
		//if(rtc_flag==0)
		int a;
		rtc_read(NULL,&a,NOT_USED,0);
		printf("1");
		//else 
		//	break;
	}
}
//test close 
void test_close()
{
	rtc_close();
}
//another test
void test()
{
	int a = 0;
	rtc_write(NOT_USED,&a,NUM_BYTES);
	a = 2;
	rtc_write(NOT_USED,&a,NUM_BYTES);
	a = 4;
	rtc_write(NOT_USED,&a,NUM_BYTES);
	 a = 1024;
	rtc_write(NOT_USED,&a,NUM_BYTES);
	//test_close();
}*/
