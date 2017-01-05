#ifndef FILES_H
#define FILES_H

#include "types.h"
#include "lib.h"

//magic numbers
#define FS_UNIT_SIZE    4096
#define SIZE_FS_DENTRIES  63
#define SUCCESS 0
#define FAIL -1

typedef struct {
	uint32_t st_boot_block;
	uint32_t st_inode;
	uint32_t st_data;
} file_sys;

/*
 * struct for dir_entries.
 */
typedef struct {
	int8_t   filename[32]; //special type for define by lib
	uint32_t filetype;
	uint32_t inode;
	uint8_t  reserved[24];
} dentry_t;

/* 
 * struct for boot block
 */
typedef struct {
	uint32_t dentries_num;
	uint32_t inodes_num;
	uint32_t datablocks_num;
	uint8_t  reserved[52];
} boot_block;

/*
 * struct for inode.
 */
typedef struct{
	uint32_t length;
	uint32_t data_blocks[1023];
} inode_t;

boot_block my_boot_block; //saves information for boot block
file_sys file_addr; //saves starting addr for: boot, inode, data
dentry_t * dentries;//saves starting addr for dentries
inode_t * inodes;//saves starting addr for inodes
int length_of_file;//saves the length of data readed

/* 
 *Functions for MP3.2
 */
int filesystem_init(const uint32_t fs_start, const uint32_t fs_end);
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);
int32_t file_read(uint8_t * buf, uint32_t offset, uint32_t length, const int8_t * name);
int32_t file_load(const int8_t * fname, uint32_t addr);
int32_t file_close();
int32_t file_open(uint32_t fs_start, uint32_t fs_end);
int32_t file_write();
int32_t directory_open();
int32_t directory_read(uint8_t * buf, uint32_t offset, uint32_t length, const int8_t * name);
int32_t directory_write();
int32_t directory_close();
void dentry_copy(uint32_t index, dentry_t * dentry);
void files_test(void);
void print_allfile_name(void);
void file_index(int a);
#endif

