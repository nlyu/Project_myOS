#include "systemcall.h"
#define MAGIC_NUM_0 0x7f
#define MAGIC_NUM_1 0x45
#define MAGIC_NUM_2 0x4c
#define MAGIC_NUM_3 0x46
#define PROGRAM_STARTING_ADDR 0x08048000
#define PROGRAM_ENDING_ADDR 0x083ffffc
//#define BOTTOM_STACK 0x007fffff
#define BOTTOM_STACK 0x00800000
#define USER_PROG_START 0x08000000		//128 Mb
#define USER_PROG_END 0x08400000		//132 Mb
#define ARG_SIZE 1024
#define STACK_SIZE 0x2000
#define LAST_13BITS_MASK 0xffffe000
#define USE 1
#define NOT_USE 0
#define NULL 0
#define CODE_STARTING_POINT 24
#define STDIN 0
#define STDOUT 1
#define VIDEO_ADDR 0x8400000
//table for file operations
uint32_t stdin_t[4] = {
	0,(uint32_t)terminal_read, 0, 0
};

uint32_t stdout_t[4] = {
	0, 0, (uint32_t)terminal_write, 0
};

uint32_t rtc_t[4] = {
	(uint32_t)rtc_open, (uint32_t)rtc_read, (uint32_t)rtc_write, (uint32_t)rtc_close
};

uint32_t file_t[4] = {
	(uint32_t)file_open, (uint32_t)file_read, (uint32_t)file_write, (uint32_t)file_close
};

uint32_t directory_t[4] = {
	(uint32_t)directory_open, (uint32_t)directory_read, (uint32_t)directory_write, (uint32_t)directory_close
};

/*
 * execute(const uint8_t* command)
 * Description: Load a executable file into physical memory and execute it 
 *
 * Inputs: command
 * Outputs: 0 if program is terminated correctly
 * Side effects: Control is switched to the new process 
 */
int counter=0; 
int shell_entry_addr;
int arg_size;
int32_t execute (const uint8_t* command){
//printf("   |----------------------|   \n");
	//printf("   |WELCOME TO RED PILL OS|   \n");
	//printf("   |----------------------|   \n");
	//printf("Step 1: Enter execution\n");
	if( counter >= 6)
	{
		printf("Can't execute more than 6 processes\n");
		return -1;
	}
	//some local variable
	int i;
	uint32_t curr_esp, curr_ebp;
	uint8_t buf[4];
	uint8_t arg_buf[ARG_SIZE];
	pcb_t* parent_pcb;
	
	//step 1: Parse arg 
	if(command == NULL)		
		return -1; 

	uint8_t command_name[32];
	for(i = 0; (i < 32) && (command[i] != ' '); i++){
		command_name[i] = command[i];
	}
	command_name[i] = '\0';
	//printf("Step 2: Command_name: %s\n",command_name);
	//get the arguments for system call /////////////////////////////////////newwwwwwwwwwwwwwwww
	i++;
	arg_size = 0;
	while(command[i]!='\0')
	{
		arg_buf[arg_size] = command[i];
		arg_size++;
		i++;
	}
	arg_buf[arg_size] = '\0';
	//step 2: Check file validity
	if(check_executable(command_name)==-1)
	{
		return -1;
	}
	//printf("        File is executable\n");
	
	//read entry address
	if(file_read(buf,CODE_STARTING_POINT,4,(int8_t *)command_name) == -1)  //CODE_STARTING_POINT is 24
	{
		return -1;
	}
	int entry_addr = buf[0] | buf[1]<<8 | buf[2]<<16 | buf[3]<<24;
	
	//increment counter for new process
	counter = counter + 1;
	if(counter == 1)
	{
		shell_entry_addr = entry_addr;
	}
	//printf("Step 3: Entry_addr: %x\n",entry_addr);
	
	//step 3: Set up paging
	create_paging(counter);
	//printf("        Paging working\n");
	
	//step 4: Load file into memory
	uint32_t a = file_load((int8_t *)command_name, PROGRAM_STARTING_ADDR);
	//printf("Step 4: file load: %x\n",a);
	if(a==-1)///PROGRAM_STARTING_ADDR = 0	x08048000
	{
		return -1;	
	}
	//printf("        File load working\n");
	
	//step 5: Create PCB/Open FDs 
	pcb_t* pcb = (pcb_t *) (BOTTOM_STACK - STACK_SIZE*counter);
	//printf("        Pcb addr:%x\n",pcb);
	pcb->pid = counter;
	asm volatile ("movl %%esp, %[ESP]\n\t"
				  "movl %%ebp, %[EBP]"
				  :[ESP]"=g"(curr_esp),
				   [EBP]"=g"(curr_ebp)
				  );
	//printf("Step 5: Curr_esp:%x Curr_ebp:%x\n", curr_esp,curr_ebp);
	
	//pid for parent of shell is set to 0
	if(counter == 1) {
		pcb->parent_pid = 0;								
	} else {
		parent_pcb = ((pcb_t * )(curr_esp & LAST_13BITS_MASK));
		pcb->parent_pid = parent_pcb-> pid;
	} 
	pcb->parent_kernel_sp = curr_esp;
	pcb->parent_kernel_bp = curr_ebp;
		//open FDS
	pcb->file_array[STDIN].file_operation = (file_op *)stdin_t;
	int8_t* inputstdin = (int8_t *)"STDIN";
	strcpy((int8_t*)(pcb->file_array[STDIN].file_name), inputstdin);
	//pcb->file_array[STDIN].file_name = (uint32_t)"STDIN";
	pcb->file_array[STDIN].inode = 0;
	pcb->file_array[STDIN].fileposition = 0;
	pcb->file_array[STDIN].flags = USE;
	pcb->file_array[STDOUT].file_operation =   (file_op *)stdout_t;
	int8_t* inputstdout = (int8_t *)"STDOUT";
	strcpy((int8_t*)(pcb->file_array[STDOUT].file_name), inputstdout);
	pcb->file_array[STDOUT].inode = 0;
	pcb->file_array[STDOUT].fileposition = 0;
	pcb->file_array[STDOUT].flags = USE;
	
	//initialize other file descriptors 
	for (i=2; i<8; i++)
	{
		pcb->file_array[i].file_operation =  NULL;
		strcpy((int8_t *)(pcb->file_array[i].file_name), (int8_t *)"");
		pcb->file_array[i].inode = 0;
		pcb->file_array[i].fileposition = 0;
		pcb->file_array[i].flags = NOT_USE;
	}
	
	//step 6: Prepare for context switch
	tss.esp0 = BOTTOM_STACK - (counter-1)*0x2000-4;
	//clear_screen();
	for(i=0; i<ARG_SIZE; i++)
	{
		pcb->arg[i] = '\0';
	}
	strncpy((int8_t*)pcb->arg , (int8_t*)arg_buf, arg_size); ////////////////////////////newwwwwwwwwwwwwwwwwwwwwwww 
	//step 7: Push IRET context to stack
	asm volatile (
				  "cli\n\t"
				  "movw $0x2B, %%ax\n\t"
				  "movw %%ax, %%ds"
				  :
				  :
				  :"%eax");
	asm volatile (
				  "cli\n\t"
				  "pushl %[SS]\n\t"
				  "pushl %[ESP]\n\t"
				  "pushf\n\t "
				  "pushl %[CS]\n\t"
				  "pushl %[EIP]\n\t"
				  :
				  :[SS]"g"(USER_DS),[ESP]"g"(PROGRAM_ENDING_ADDR),[CS]"g"(USER_CS),[EIP]"g"(entry_addr)
				  );
	//step 8: IRET
	asm volatile ("iret");
	asm volatile ("haltlabel:");
	asm volatile("leave");
  	asm volatile("ret");
	//step 9: Return
	return 0;
}



uint32_t page_dir_addr = 0;
/*
 * halt(uint8_t status)
 * Description: Will halt the current process and change the related registers into correct values
 *
 * Inputs: status
 * Outputs: 0 if program is terminated correctly
 * Side effects: halt a process
 */
int32_t halt (uint8_t status)
{
    pcb_t* pcb = (pcb_t *)(BOTTOM_STACK - counter*0x2000  );
  //checking if we are now in shell or not
	uint8_t temp = status;
	if(pcb->pid == 1)
	{
		clear_screen();
		asm volatile (
			"cli\n\t"
			"movw $0x2B, %%ax\n\t"
			"movw %%ax, %%ds"
			:
			:
			:"%eax");
		asm volatile (
			"cli\n\t"
			"pushl %[SS]\n\t"
			"pushl %[ESP]\n\t"
			"pushf\n\t "
			"pushl %[CS]\n\t"
			"pushl %[EIP]\n\t"
			:
			:[SS]"g"(USER_DS),[ESP]"g"(PROGRAM_ENDING_ADDR),[CS]"g"(USER_CS),[EIP]"g"(shell_entry_addr)
			);
		asm volatile("iret");
	}
  	--counter;
  	page_dir_addr = (uint32_t) (&page_directories[pcb->parent_pid - 1]);
  	//refreshing cr3
	asm volatile(
	//enable cr3, cr3 contain the physical addr for page-directory
	"movl page_dir_addr, %eax;"
	"andl $0xFFFFFFE7, %eax;"
	"movl %eax, %cr3;"
	);
  	tss.esp0 = BOTTOM_STACK - (counter-1)*0x2000 - 4;	
  	//reloading esp and ebp of parents
  	asm volatile("movl %0,%%esp;"::"g"(pcb->parent_kernel_sp));
  	asm volatile("movl %0,%%ebp;"::"g"(pcb->parent_kernel_bp));
	asm volatile(
	"xor %%eax, %%eax;"
	"movb %0,%%al;"
	::"g"(temp));
	
	asm volatile ("jmp haltlabel");

  	//never goes to here
  	//to avoid warning
  	return 0;
}



/*
 * read(int32_t fd, void* buf, int32_t nbytes)
 * Description: read a file 
 *
 * Inputs: fd, buf, nbytes
 * Outputs: number of bytes successfully read
 * Side effects: call a right read function according to fd
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes)
{
	sti();
	int bytes;
  	//if fd is out of range , return -1
	if(fd < 0 || fd >7 || buf == NULL)	return -1;
    pcb_t* pcb = (pcb_t *)(BOTTOM_STACK - counter*STACK_SIZE);
    if(pcb->file_array[fd].flags == 0)	return -1;
  	//pushing para, call read function
	uint32_t offset = pcb->file_array[fd].fileposition;
	int8_t * filename = (int8_t*)((pcb->file_array[fd].file_name));
	asm volatile("pushl %[name]\n\t"
				 "pushl %[nbytes]\n\t"
				 "pushl %[fileposition]\n\t"
				 "pushl %[buff]\n\t"
				 "call *%[read_function]\n\t"
				 "addl $16,%%esp\n\t"
				 "movl %%eax,%[read_bytes]"
				 :[read_bytes]"=g"(bytes)
				 :[name]"g"(filename),[nbytes]"g"(nbytes),[buff]"g"(buf),[fileposition]"g"(offset),[read_function]"g"(pcb->file_array[fd].file_operation->read)
				 :"%eax","%esp");
	pcb->file_array[fd].fileposition = pcb->file_array[fd].fileposition + bytes;
    return bytes;
}



/*
 * write(int32_t fd, void* buf, int32_t nbytes)
 * Description: write to a file 
 *
 * Inputs: fd, buf, nbytes
 * Outputs: number of bytes successfully wrote
 * Side effects: call a right write function according to fd
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes)
{
	int bytes;
    	//if fd is out of range , return -1
	if(fd < 0 || fd >7 || buf == NULL)	return -1;
  	pcb_t* pcb = (pcb_t *)(BOTTOM_STACK - counter*STACK_SIZE);
  	if(pcb->file_array[fd].flags == 0) return -1;
  	//pushing para, call write function
	asm volatile("pushl %[nbytes]\n\t"
				 "pushl %[buff]\n\t"
				 "pushl %[fd]\n\t"
				 "call *%[write_function]\n\t"
				 "addl $12,%%esp\n\t"
				 "movl %%eax,%[bytes]"
				 :[bytes]"=g"(bytes)
				 :[nbytes]"g"(nbytes),[buff]"g"(buf),[fd]"g"(fd),[write_function]"g"(pcb->file_array[fd].file_operation->write)
				 :"%eax","%esp");
  	return bytes;
}


/*
 * open (const uint8_t* filename)
 * Description: open a file
 *
 * Inputs: filename string
 * Outputs: the index of the file descriptor we opened
 * Side effects: 
 */
int32_t open (const uint8_t* filename)
{
	
    //printf("filename:%s\n",filename);
  	pcb_t* curr_pcb = (pcb_t *)(BOTTOM_STACK - counter*STACK_SIZE);			//get the current pcb
  	
  	if(strncmp((int8_t*)"stdin", (int8_t*) filename, 5) == 0) {	

      	curr_pcb->file_array[0].file_operation = (file_op *)stdin_t;

        curr_pcb->file_array[0].flags = USE;
      	return 0;
  	}
  	
  	if(strncmp((int8_t*)"stdout", (int8_t*) filename, 5) == 0) {
  		curr_pcb->file_array[1].file_operation = (file_op *)stdout_t;

        curr_pcb->file_array[1].flags = USE;
      	return 0;
  	}
	
	int i;						//counter
	dentry_t temp;				//temp dentry
  	
	if(read_dentry_by_name(filename, &temp) == -1){		//return -1 if read_dentry_by_name fails
  		return -1;
    }
	/*
	int8_t * temp0 = (int8_t*)(curr_pcb->file_array[0].file_name);
	int8_t * temp1 = (int8_t*)(curr_pcb->file_array[1].file_name);
	int8_t * temp2 = (int8_t*)(curr_pcb->file_array[2].file_name);
	int8_t * temp3 = (int8_t*)(curr_pcb->file_array[3].file_name);
	int8_t * temp4 = (int8_t*)(curr_pcb->file_array[4].file_name);
	int8_t * temp5 = (int8_t*)(curr_pcb->file_array[5].file_name);
	*/
  	//let us find the file descriptor that is not in use, store it in vacant_fd
  	int vacant_fd = -1;
  	i = 2;
  	while(i < 8) {
      	if(curr_pcb->file_array[i].flags == 0){		//if the file_descriptor here is not in use
          	vacant_fd = i;
          	break;
        }
      	i++;
    }
  
  	if(vacant_fd == -1) {		//if there is no vacant spot left
    	return -1;
    } else {
  
        //then we need to check file type and act accordingly
        if(temp.filetype == 2){							//if this file is regular file
            curr_pcb->file_array[vacant_fd].file_operation = (file_op *)file_t;
        } else if(temp.filetype == 1) {					//if this file is a directory
            curr_pcb->file_array[vacant_fd].file_operation = (file_op *)directory_t;         
        } else if(temp.filetype == 0) {					//if this file is rtc
            curr_pcb->file_array[vacant_fd].file_operation = (file_op *)rtc_t;            	
        }
  		
      	curr_pcb->file_array[vacant_fd].flags = USE;
      	curr_pcb->file_array[vacant_fd].inode = temp.inode;
		curr_pcb->file_array[vacant_fd].fileposition = 0;
		//strcpy((int8_t*)(pcb->file_array[STDOUT].file_name), inputstdout);
      	strcpy((int8_t*)(curr_pcb->file_array[vacant_fd].file_name), (int8_t *)filename);	
//curr_pcb->file_array[vacant_fd].file_name = *filename;		
      	//printf("filename:%s\n",curr_pcb->file_array[vacant_fd].file_name);
		//printf("filename:%s\n",filename);
  		return vacant_fd;
    }
  
}



int32_t close (int32_t fd)
{
	if(fd < 2|| fd > 7)	return -1;
	pcb_t* curr_pcb = (pcb_t *)(BOTTOM_STACK - counter*STACK_SIZE);
	if(curr_pcb->file_array[fd].flags == USE){
		int ret;
		asm volatile(
				 "call *%[close_function]\n\t"
				 "movl %%eax,%[ret_val]"
				 :[ret_val]"=g"(ret)
				 :[close_function]"g"(curr_pcb->file_array[fd].file_operation->close));
	    curr_pcb->file_array[fd].inode = 0;
		curr_pcb->file_array[fd].flags = NOT_USE;
		curr_pcb->file_array[fd].fileposition = 0;
		curr_pcb->file_array[fd].file_operation = NULL;
		strcpy((int8_t*)(curr_pcb->file_array[fd].file_name), (int8_t *)"");
		return ret;
	}
	else{
		return -1;
	}
}

int32_t getargs (uint8_t* buf, int32_t nbytes)
{
	if(buf == NULL || nbytes<(arg_size))
		return -1;
	pcb_t* pcb = (pcb_t *)(BOTTOM_STACK - counter*STACK_SIZE  );
	strncpy((int8_t*) buf,(int8_t*)pcb->arg, nbytes);
	//printf("-----------------------------------------------:%s\n", pcb->arg);
	//printf("------------------------------------------------:%s\n", buf);
	return 0;
}

int32_t vidmap (uint8_t** screen_start)
{
	//check
	if ((uint32_t) screen_start < USER_PROG_START || (uint32_t) screen_start >= USER_PROG_END) {
		return -1;			//check if the screen_start is valid
	}
	//assign video memory, map the virtual memory 132mb to physical address 0xB8000
	int i;
	for( i = 0; i < PAGE_TABLE_SIZE; i++ ) {
		/*the 184th page table is for video memory*/
		page_table_video[i].present = (i == 0)? 1 : 0; //entry of the video memory
		/*the rest if marked zero */
		page_table_video[i].read_write = (i == 0)? 1 : 0;//entry of the video memory
		page_table_video[i].user_supervisor = (i == 0)? 1 : 0;
		page_table_video[i].write_through = 0;
		page_table_video[i].cache_disabled = 0;
		page_table_video[i].accessed = 0;
		page_table_video[i].dirty = 0;
		page_table_video[i].pat = 0;
		page_table_video[i].global = 0;
		page_table_video[i].available = 0;
		page_table_video[i].base_addr = 184;
	}
 
	/* Initializing  Non-present pages directory entry. 
	   The first page directory is set to present because
	   we want to save the page table in the first index
	   of this page directory */
	int process = counter;
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.present = 1; //this is used to saves 4KB addr
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.read_write = 1;
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.user_supervisor = 1; //for user-level
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.write_through = 0;
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.cache_disabled = 0;
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.accessed = 0;
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.reserved = 0;
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.page_size = 0;
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.global_page = 0;
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.available = 0;
	/* send the addr for page table to page directory */
	page_directories[process-1].pd_entries[VIDEO_MEM].KB.base_addr = (int)page_table_video >> ADDRESS_SHIFT;
	*screen_start = (uint8_t *) VIDEO_ADDR; //0xB8000
	return 0;
}

int32_t set_handler (int32_t signum, void* handler_address)
{
	return -1;
}
int32_t sigreturn (void)
{
	return -1;
}



/*
 * check_executable(uint8_t* command_name)
 * Description: Check whether file is executable 
 *
 * Inputs: command_name 
 * Outputs: 0 if file is executable 
 * 			-1 if file is not executable 
 */
int check_executable(uint8_t* command_name)
{
	uint8_t temp[4];
	if(file_read(temp,0,4,(int8_t *)command_name) == -1){                   // Bytes 4 to 0 are corresponding to four magic numbers
		return -1;
	}
	if(temp[0] != MAGIC_NUM_0 || temp[1]!=MAGIC_NUM_1 || temp[2] != MAGIC_NUM_2 || temp[3]!= MAGIC_NUM_3)
	{
		return -1;
	}
	return 0;
}
