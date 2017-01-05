#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H

#include "files.h"
#include "mypaging.h"
#include "keyboard.h"
#include "types.h"
#include "lib.h"
#include "handler.h"
#include "i8259.h"
#include "rtc.h"

//pcb_file_de done
typedef struct file_op{
		int32_t open;
		int32_t read;
		int32_t write;
		int32_t close;
}file_op;


typedef struct file_descriptor{
    file_op * file_operation; //open, read, write and close pointer
	uint8_t file_name[32]; 		//name for the file
  	uint32_t inode; 			//pointer to inode for file
    uint32_t fileposition; 		//keep track of current reading from file
    int32_t flags; 				//mark for in-use
}file_descriptor;

//pcb doen
typedef struct {
	uint8_t pid; 		 //process id
    uint8_t parent_pid;  //parent id
    uint32_t parent_kernel_sp; //parent stack pointer
    uint32_t parent_kernel_bp; //parent base pointer
    uint8_t status; 	 //current status for queue
    uint8_t priority; 	 //current priority
    file_descriptor file_array[8]; //resouce we need
	uint8_t arg[1024];
}pcb_t;

//mp3.3
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
//mp3.4
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
//mp3.6
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

//helper function
int check_executable(uint8_t* command_name);
#endif
