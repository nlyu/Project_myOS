#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_IRQ 1
#define KEYBOARD_PORT 0x60
#define KEYBOARD_OFFSET 0x54
#define KEYBOARD_NUM 128

//max size of terminal buffer = 128 	杜建霖
#define BUF_SIZE 1024
//int rtc_flag;

//Input: None
//Output: None
//Description: We are going to initialize the keyboard device in this function
//Now we just enable the irq
void keyboard_init();


int32_t terminal_read(unsigned char* buf, uint32_t offset, int32_t nbytes, const int8_t * name);
int32_t terminal_write(int fd, unsigned char* buf, int32_t nbytes);
int32_t terminal_open(const uint8_t* filename);

//Input: Keyboard information
//output: None
//Description: the input will go through all of the condition cases
//in order to determine the correct input to
void process_keyboard_input(unsigned character_info);
//Input: None
//Output: None
//Description: This is the interrupt handler function which will be called whenever keyboard interrupt
//is called
void keyboard_interrupt_handler();
#endif

