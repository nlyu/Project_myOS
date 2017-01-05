#include "mypaging.h"

/* This file contain the implementation of functions for 
   paging initialization */

/* 
 * page directory for the process. 
 */

/*
 * init_paging()
 * Called from kernel.c to initialize paging.
 *
 * Inputs: none
 * Outputs: none
 */
void init_paging(void)
{
	/*	paging structure 
	
		   PDT			   PT(4kb)
		|-------|		|-------|
		|	0   |---->	|	0	|
		|	1	|---|	|  184	|------>video memory 
		|  ..	|	|	| 1023 	|
		|  ..	|	|	|-------|
		|  1023	|	|	
		|-------|	---> 4MB Kernal Space	
	
		*the rest of PDT from 2 to 1023 is set not-present
	
	
	   Initialize page table 
	   Set the flags and fields in the paging table entries. 
	   There are totally 1024 paging table 
	   reference: intel manual 3.10 paging 
	*/
	
	int i;
	for( i = 0; i < PAGE_TABLE_SIZE; i++ ) {
		/*the 184th page table is for video memory*/
		page_table_shell[i].present = (i == 184)? 1 : 0; //entry of the video memory
		/*the rest if marked zero */
		page_table_shell[i].read_write = (i == 184)? 1 : 0;//entry of the video memory
		page_table_shell[i].user_supervisor = (i == 184)? 1 : 0;
		page_table_shell[i].write_through = 0;
		page_table_shell[i].cache_disabled = 0;
		page_table_shell[i].accessed = 0;
		page_table_shell[i].dirty = 0;
		page_table_shell[i].pat = 0;
		page_table_shell[i].global = 0;
		page_table_shell[i].available = 0;
		page_table_shell[i].base_addr = i;
	}

	/* Initializing  Non-present pages directory entry. 
	   The first page directory is set to present because
	   we want to save the page table in the first index
	   of this page directory */
	page_directories[0].pd_entries[0].KB.present = 1; //this is used to saves 4KB addr
	page_directories[0].pd_entries[0].KB.read_write = 1;
	page_directories[0].pd_entries[0].KB.user_supervisor = 1;
	page_directories[0].pd_entries[0].KB.write_through = 0;
	page_directories[0].pd_entries[0].KB.cache_disabled = 0;
	page_directories[0].pd_entries[0].KB.accessed = 0;
	page_directories[0].pd_entries[0].KB.reserved = 0;
	page_directories[0].pd_entries[0].KB.page_size = 0;
	page_directories[0].pd_entries[0].KB.global_page = 0;
	page_directories[0].pd_entries[0].KB.available = 0;
	/* send the addr for page table to page directory */
	page_directories[0].pd_entries[0].KB.base_addr = (int)page_table_shell >> ADDRESS_SHIFT;
	
	/* Initialize the kernel page directory entry.
	   this page directory is set to 4MB 
	   and we need to enable read/write 
	   
	   Initialize the rest of page directory entries
	   The rest of page directory is set to not-present
	   I use 4MB mode instead of 4kb for convience*/
	for( i = 1; i < PAGE_DIRECTORY_SIZE; i++ ) {
	page_directories[0].pd_entries[i].MB.present = (i == 1)? 1 : 0; //the first index is set to present, the rest is not present
	page_directories[0].pd_entries[i].MB.read_write = (i == 1)? 1 : 0;//enable the first index , disable the rest
	page_directories[0].pd_entries[i].MB.user_supervisor =  0;
	page_directories[0].pd_entries[i].MB.write_through = 0;
	page_directories[0].pd_entries[i].MB.cache_disabled = 0;
	page_directories[0].pd_entries[i].MB.accessed = 0;
	page_directories[0].pd_entries[i].MB.dirty = 0;
	page_directories[0].pd_entries[i].MB.page_size = (i == 1)? 1 : 0;//1->4mb  0->4kb
	page_directories[0].pd_entries[i].MB.global = (i == 1)? 1 : 0;//ignore
	page_directories[0].pd_entries[i].MB.available = 0;
	page_directories[0].pd_entries[i].MB.pat = 0;
	page_directories[0].pd_entries[i].MB.base_addr = i;
	}

	/* Set control registers to initialize paging. */
	/* reference: osdev->setting paging */
	asm volatile(
	//enable cr3, cr3 contain the physical addr for page-directory
	"movl $page_directories, %eax;"
	"andl $0xFFFFFFE7, %eax;"
	"movl %eax, %cr3;"
	);
	asm volatile(
	//set pages as read-only
	"movl %cr4, %eax;"
	"orl $0x00000010, %eax;"
	"movl %eax, %cr4;"
	);
	asm volatile(
	//set paging bit of cr0
	"movl %cr0, %eax;"
	"orl $0x80000000, %eax;"
	"movl %eax, %cr0;"
	);	
	
	return;
}

/*
 * create_paging()
 * Create a new paging(virtual memory) for new program
 * 4mb to 8mb in both virtual and physical memory is for kernal.
 * The new user program is loaded at 0x8048000 in virtual memory
 * the first user program(shell) is loaded at 8mb in physical memory
 * the second user program is loaded at 12mb, and so on.
 * Inputs: the Nth process been run
 * Outputs: none
 */
uint32_t page_dir_address; //used to save new PDBR
void create_paging(int process){
	//check validality
	if(process >= 7 || process < 0){
		//printf("checkpoint1");
		return;
	}
	
	int i;
	for( i = 0; i < PAGE_TABLE_SIZE; i++ ) {
		/*the 184th page table is for video memory*/
		page_table_shell[i].present = (i == 184)? 1 : 0; //entry of the video memory
		/*the rest if marked zero */
		page_table_shell[i].read_write = (i == 184)? 1 : 0;//entry of the video memory
		page_table_shell[i].user_supervisor = (i == 184)? 1 : 0;
		page_table_shell[i].write_through = 0;
		page_table_shell[i].cache_disabled = 0;
		page_table_shell[i].accessed = 0;
		page_table_shell[i].dirty = 0;
		page_table_shell[i].pat = 0;
		page_table_shell[i].global = 0;
		page_table_shell[i].available = 0;
		page_table_shell[i].base_addr = i;
	}
    //printf("checkpoint2");
	/* Initializing  Non-present pages directory entry. 
	   The first page directory is set to present because
	   we want to save the page table in the first index
	   of this page directory */
	page_directories[process-1].pd_entries[0].KB.present = 1; //this is used to saves 4KB addr
	page_directories[process-1].pd_entries[0].KB.read_write = 1;
	page_directories[process-1].pd_entries[0].KB.user_supervisor = 1; //for user-level
	page_directories[process-1].pd_entries[0].KB.write_through = 0;
	page_directories[process-1].pd_entries[0].KB.cache_disabled = 0;
	page_directories[process-1].pd_entries[0].KB.accessed = 0;
	page_directories[process-1].pd_entries[0].KB.reserved = 0;
	page_directories[process-1].pd_entries[0].KB.page_size = 0;
	page_directories[process-1].pd_entries[0].KB.global_page = 0;
	page_directories[process-1].pd_entries[0].KB.available = 0;
	/* send the addr for page table to page directory */
	page_directories[process-1].pd_entries[0].KB.base_addr = (int)page_table_shell >> ADDRESS_SHIFT;
	//printf("checkpoint3");
	/* Initialize the kernel page directory entry.
	   this page directory is set to 4MB 
	   and we need to enable read/write 
	*/
	page_directories[process-1].pd_entries[1].MB.present = 1; //the first index is set to present, the rest is not present
	page_directories[process-1].pd_entries[1].MB.read_write = 1;//enable the first index , disable the rest
	page_directories[process-1].pd_entries[1].MB.user_supervisor = 0;
	page_directories[process-1].pd_entries[1].MB.write_through = 0;
	page_directories[process-1].pd_entries[1].MB.cache_disabled = 0;
	page_directories[process-1].pd_entries[1].MB.accessed = 0;
	page_directories[process-1].pd_entries[1].MB.dirty = 0;
	page_directories[process-1].pd_entries[1].MB.page_size = 1;//1->4mb  0->4kb
	page_directories[process-1].pd_entries[1].MB.global = 1;//ignore
	page_directories[process-1].pd_entries[1].MB.available = 0;
	page_directories[process-1].pd_entries[1].MB.pat = 0;
	page_directories[process-1].pd_entries[1].MB.base_addr = 1;//kernal is at 4MB to 8MB
	//printf("checkpoint5");
	
	//disable the other unused virtual memory
	for( i = 2; i < PAGE_DIRECTORY_SIZE; i++ ) {
		page_directories[process-1].pd_entries[i].MB.present = 0; //the first index is set to present, the rest is not present
		page_directories[process-1].pd_entries[i].MB.read_write = 0;//enable the first index , disable the rest
		page_directories[process-1].pd_entries[i].MB.user_supervisor = 0;
		page_directories[process-1].pd_entries[i].MB.write_through = 0;
		page_directories[process-1].pd_entries[i].MB.cache_disabled = 0;
		page_directories[process-1].pd_entries[i].MB.accessed = 0;
		page_directories[process-1].pd_entries[i].MB.dirty = 0;
		page_directories[process-1].pd_entries[i].MB.page_size = 1;//1->4mb  0->4kb
		page_directories[process-1].pd_entries[i].MB.global = 0;//ignore
		page_directories[process-1].pd_entries[i].MB.available = 0;
		page_directories[process-1].pd_entries[i].MB.pat = 0;
		page_directories[process-1].pd_entries[i].MB.base_addr = 0;
	}
	
	/* Initialize the program image directory entry.
	   this page directory is set to 4MB 
	   program image starts at 128mb (actually 0x08048000)
	*/
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.present = 1; //the first index is set to present, the rest is not present
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.read_write = 1;//enable the first index , disable the rest
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.user_supervisor = 1;//user level
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.write_through = 0;
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.cache_disabled = 0;
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.accessed = 0;
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.dirty = 0;
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.page_size = 1;//1->4mb  0->4kb
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.global = 0;//ignore
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.available = 0;
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.pat = 0;
	page_directories[process-1].pd_entries[PROGRAM_IMG].MB.base_addr = process + 1;//user program is located at (8mb + Nth)
	
	/* get the new page directory address for new user program. */
	page_dir_address = (uint32_t)(&page_directories[process-1]);
	/* Set control registers to initialize paging. */
	/* reference: osdev->setting paging */
	asm volatile(
	//enable cr3, cr3 contain the physical addr for page-directory
	"movl page_dir_address, %eax;"
	"andl $0xFFFFFFE7, %eax;"
	"movl %eax, %cr3;"
	);
	asm volatile(
	//set pages as read-only
	"movl %cr4, %eax;"
	"orl $0x00000010, %eax;"
	"movl %eax, %cr4;"
	);
	asm volatile(
	//set paging bit of cr0
	"movl %cr0, %eax;"
	"orl $0x80000000, %eax;"
	"movl %eax, %cr0;"
	);	
	//printf("checkpoint6");	
	return;
}

