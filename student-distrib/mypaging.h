/* paging.h - this file takes charge of paging */
#ifndef PAGING_H
#define PAGING_H


/* necessary file included */
#include "x86_desc.h"
#include "types.h"
#include "lib.h"

#define	ADDRESS_SHIFT			12
#define MAX_NUM_OF_PROCESSES     6
#define PAGE_DIRECTORY_SIZE   1024
#define PAGE_TABLE_SIZE 	  1024
#define PROGRAM_IMG				32	
#define VIDEO_MEM		33//132 Mb
/* Called from kernel.c for paging initialization. */
void init_paging(void);
void create_paging(int process);
#endif 
