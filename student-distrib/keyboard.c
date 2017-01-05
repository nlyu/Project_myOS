//24 2:47
//keyboard array
#include "lib.h"
#include "keyboard.h"
#include "i8259.h"
#include "files.h"
#include "rtc.h"
#include "test.h"
//flags
int caps_flag = 0;
int shift_left_flag = 0;
int shift_right_flag = 0;
int ctrl_left_flag = 0;
int ctrl_right_flag = 0;
int32_t frequency;
//int32_t prev_frequency;

//////////////////////////////////////////////////////////////////////////////////////////////      杜建霖
char buffer[BUF_SIZE];      //the buffer used to hold the characters entered                
                                        //BUF_SIZE = 1024
int storage_buffer[BUF_SIZE];   //the buffer content will be stored in this storage buffer after ENTER is pressed

int buffer_content_counter;         //store the current number of characters stored in the buffer    
int storage_buffer_counter;         //counter for the storage buffer 

int backspacable_counter;           //counts how many char can be backspaced

int readable_flag;


int count=0;
//int rtc_flag2;
int index_file;						//     LV
////////////////////////////////////////////////////////////////////////////////////////////////     杜建霖


void keyboard_init(){
    //init hte buffer
    int i = 0;
    for (i = 0; i < BUF_SIZE; i++){     //initialize the buffer
        buffer[i] = NULL;
        storage_buffer[i] = NULL;
    }
    buffer_content_counter = 0;
    readable_flag = 0;
    backspacable_counter = 0;
    init_screen_x_and_y();
    update_terminal_cursor();           //////////////
    index_file = 0;
	count = 1;
	enable_irq(KEYBOARD_IRQ);
    return;
}




//normal keyboard info
unsigned char norm[KEYBOARD_NUM] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'', '`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,'*',    0,  /* Alt */
    ' ',
    0,
    0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, 0, 0, 0, 0,  0, 0, 0, 0,  0,  0, //all others are undefined
};

//keyboard info with cap lock on
unsigned char cap_norm[KEYBOARD_NUM] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n',0,
    'A','S','D','F','G','H','J','K','L',':','\'', '`', 0,'\\',
    'Z','X','C','V','B','N','M',',','.','/',0,'*',    0,  /* Alt */
    ' ',
    0,
    0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, 0, 0, 0, 0,  0, 0, 0, 0,  0,  0, //all others are undefined
};

//keyboard info while shift is pressed
unsigned char shift_norm[KEYBOARD_NUM] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,
    'A','S','D','F','G','H','J','K','L',':','\"', '~', 0,'\\',
    'Z','X','C','V','B','N','M','<','>','?',0,'*',    0,  /* Alt */
    ' ',
    0,
    0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, 0, 0, 0, 0,  0, 0, 0, 0,  0,  0, //all others are undefined
};

//keyboard info when shift and caps lock are on
unsigned char cap_shift[KEYBOARD_NUM] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t',
    'q','w','e','r','t','y','u','i','o','p','{','}','\n',0,
    'a','s','d','f','g','h','j','k','l',':','\"', '~', 0,'\\',
    'z','x','c','v','b','n','m','<','>','?',0,'*',    0,  /* Alt */
    ' ',
    0,
    0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, 0, 0, 0, 0,  0, 0, 0, 0,  0,  0, //all others are undefined
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////    杜建霖


/*
    This function empty the buffer and reset the buffer_content_counter to 0
	INPUT: none 
	OUTPUT: none
*/
void clear_buf() {
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        buffer[i] = NULL;
    }
    buffer_content_counter = 0;

    return;
}



/*
    This function empty the storage_buffer and reset the storage_buffer_counter to 0
	INPUT: none
	OUTPUT: none
*/
void clear_storage_buf() {
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        storage_buffer[i] = NULL;
    }
    storage_buffer_counter = 0;

    return;
}



/*
    This function print the buffer on the terminal and clear the buffer
    usually called when ENTER is pressed or the buffer max size is reached
    
    output: the number of bytes printd on screen; 0 means nothing is printed
  */
int print_buf() {
    int i = 0;
    while (i < buffer_content_counter) {
      putc(buffer[i]);
      i++;
    }
    //putc('\n');                     //NewLine
    update_terminal_cursor();
    backspacable_counter = 0;                   //the contents printed in this section are all non-backspacable
    return i;
}




/*
    This function copies the content in storage_buffer to arg buf, and clears the storage_buffer
    INPUT: buffer addr, number of bytes we want to read
    OUTPUT: the number of characters read
*/
int32_t terminal_read(unsigned char* buf, uint32_t offset, int32_t nbytes, const int8_t * name) {
    int bytse_read = 0;
    int i;
	while (!readable_flag);
    if (readable_flag) {
        for (i = 0; i < storage_buffer_counter && i < nbytes; i++) {      //copy buffer
            buf[i] = storage_buffer[i];
            bytse_read++;                    //increment the counter;
        }
        clear_storage_buf();                   //clear the                                              
    
        readable_flag = 0;
        return bytse_read;

    } else {
        return -1;
    }

}

/*
    
    INPUT: buffer addr, number of bytes we want to write
    OUTPUT: the number of characters written on screen
    Function: writes the content in buf to terminal buffer, and print_buf()
*/
int32_t terminal_write(int fd, unsigned char* buf, int32_t nbytes) {
	int j = fd;
	j = 0;
    int i;
    for (i = 0; i < nbytes; i++) {
        buffer[i] = buf[i]; 
        buffer_content_counter++;                                   
    }
    int bytes_written = print_buf();
    clear_buf();
    backspacable_counter = 0;
    return bytes_written;         //returns the number of bytes printed

}

int32_t terminal_open(const uint8_t* filename) {
	keyboard_init();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////    杜建霖



void process_keyboard_input(unsigned character_info){
    //first setting the flags for pressing
        if(character_info == 0x36){     //Right Shift
            shift_right_flag = 1;
        }
        if(character_info == 0x2A){     //Left Shift
            shift_left_flag = 1;
        }
        if(character_info == 0x1D){     //Left Control
            ctrl_left_flag = 1;
        }
        if(character_info == 0x3A && caps_flag == 0){       //Cap Lock
            caps_flag = 1;
        }
        else if (character_info == 0x3A && caps_flag == 1){
            caps_flag = 0;
        }
    //checking for the releasing button
        if(character_info == 0xB6){     //Right Shift released
            shift_right_flag = 0;
        }
        if(character_info == 0xAA){     //Left Shift released
            shift_left_flag = 0;
        }
        if(character_info == 0x9D){     //Left Control released
            ctrl_left_flag = 0;
        }
		
        //find out which character is typed             //CRTL-L                                 //////杜建霖
        if((ctrl_left_flag == 1 || ctrl_right_flag == 1) && character_info == 0x26){
            clear_screen();      //clears the screen and reset the screen_x and screen_y
            clear_buf();
            backspacable_counter = 0;
            clear_storage_buf();
			update_terminal_cursor();
            return ;
        }
		//wuningkai
		/*if((ctrl_left_flag == 1 || ctrl_right_flag == 1) && character_info == 0x5){
			count++;
			switch(count){
				case 1:
					frequency = 2;
					break;
				case 2:
					frequency = 4;
					break;
				case 3:
					frequency = 8;
					break;
				case 4:
					frequency = 16;
					break;
				case 5:
					frequency = 32;
					break;
				case 6:
					frequency = 64;
					break;
				case 7:
					frequency = 128;
					break;
				case 8:
					frequency = 256;
					break;
				case 9:
					frequency = 512;
					break;
				case 10:
					frequency = 1024;
					break;
				default:
					frequency = 1024;
					break;
			}
			//prev_frequency = frequency;
			send_eoi(KEYBOARD_IRQ);
			clear_screen();  
			test_write(frequency);
			
			test_read();
            return;
        }
		if((ctrl_left_flag == 1 || ctrl_right_flag == 1) && character_info == 0x6){
			test_close();
			//rtc_flag=1;
			clear_screen();  
            return;
			}*/	
		//
  		//nuochen lyu
		//ctrl + 1 see file by name
		if((ctrl_left_flag == 1 || ctrl_right_flag == 1) && character_info == 0x2){
			files_test();
            return;
			}	
        if((ctrl_left_flag == 1 || ctrl_right_flag == 1) && character_info == 0x3){
			 print_allfile_name();
            return;
			}
		if((ctrl_left_flag == 1 || ctrl_right_flag == 1) && character_info == 0x4){
			file_index(index_file);
			index_file++;
            return;
			}
		//nuochen lyu end
        //from 0x3A, we will ignore all of the stuffs 
        if(character_info > 0x3A){      //if a key(other than the above special keys) is released
            return;
        }

        // ENTER
        if( norm[character_info] == '\n' ) {        //ENTER                                       //////杜建霖
            putc('\n');
            update_terminal_cursor();
            backspacable_counter =0;
            readable_flag = 0;      //like a lock?
            clear_storage_buf();
            int i;
            for (i = 0; i < buffer_content_counter; i++) {              //copy the buffer into storage buffer
                storage_buffer[i] = buffer[i];
                storage_buffer_counter++;
            }
            storage_buffer[i] = '\n';
            storage_buffer_counter++;
            readable_flag = 1;
            clear_buf();                                                // and now we can clear the buffer without worries
            return;
        }

        //BACKSPACE
        if( (norm[character_info] == '\b') && (backspacable_counter > 0)) {     //BACKSPACE                                   //////杜建霖
            //we need to put a blank at this spot in terminal
            back_space();
            update_terminal_cursor(); 
            backspacable_counter--;
            //delete the last char in the buffer
            if (buffer_content_counter > 0) {
                buffer_content_counter--;
                buffer[buffer_content_counter] = NULL;
            } else {
                if (buffer_content_counter == 0) { 
                    //do nothing, correct situation
                } else {                                    //if buffer_content_counter < 0 ! which should never happen printf to notify the exceptino
                    printf("杜建霖：Exception: keyboard.c: process_keyboard_input() when BACKSPACE is pressed\n");
                }
            }
            //printf("back space is pressed\n");
            return;
        } else {
			if (norm[character_info] == '\b') return;
		}
		
//By Kankoooooooooooooooooooooooooo
//need to be revised
        if( norm[character_info] == '\t' ) {
            putc(' ');
            putc(' ');
            update_terminal_cursor();
            backspacable_counter += 2;
            return;
        }
        if(character_info == 0x36 || character_info == 0x2A || character_info == 0x1D){
            return;
        }
        if(character_info == 0x3A){
            return;
        }
    
        ///    IF A letter is pressed

        if (buffer_content_counter < 128) {         // if the buffer is not full (counter < 128), then we ehco the char and put the char into buffer
           // CAP SHIFT logic
            if (caps_flag == 1) {
                //shift pressing
                if ((shift_left_flag|shift_right_flag) == 1) {
                    putc(cap_shift[character_info]);
                    buffer[buffer_content_counter] = cap_shift[character_info]; //put into buffer
                    
                } else {
                    putc(cap_norm[character_info]);
                    buffer[buffer_content_counter] = cap_norm[character_info]; //put into buffer
                    
                }
            } 
			//not using cap
			else {
                if ((shift_left_flag|shift_right_flag) == 1) {
                    putc(shift_norm[character_info]);
                    buffer[buffer_content_counter] = shift_norm[character_info]; //put into buffer
                    
                } else {
                    putc(norm[character_info]);
                    buffer[buffer_content_counter] = norm[character_info]; //put into buffer
                    
                }
            }

            update_terminal_cursor();
            buffer_content_counter++;                   //increment the counter
            backspacable_counter += 1;
            return;
        } else {

            //do nothing if the buffer is full

        }
        

}

void keyboard_interrupt_handler(){
    //masking interrupt
    cli();
    //getting input info
    unsigned char character_info = inb(KEYBOARD_PORT);
    //unsigned char data = norm[character_info];
    //we will not echo only when pushed
    process_keyboard_input(character_info);
    //if(data=='1')
    //  test_interrupts();
    send_eoi(KEYBOARD_IRQ);
    sti();

}

