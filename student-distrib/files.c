#include "files.h"
#define SBUFSIZE 33
uint32_t flag_open;
uint32_t flag_dir;
int entry_counter = 0;
/*
 * filesystem_init()
 * Discription: get information from the filesystem block,
 * Information include addr for different block, size, length 
 * These information are crucial for reading file
 * Output: 1 SUCCESS
 *	       0 FAIL
 */
int filesystem_init(const uint32_t fs_start, const uint32_t fs_end)
{
	//check valid
	if(fs_start == fs_end){
		return FAIL;
	}
	//set the stat for boot block
	memcpy(&my_boot_block, (void *)fs_start, SIZE_FS_DENTRIES+1);
	//get the starting addr for boot, inode and data block
	file_addr.st_boot_block = fs_start;
	file_addr.st_inode = (fs_start + FS_UNIT_SIZE);
	file_addr.st_data = fs_start + (my_boot_block.inodes_num + 1)*FS_UNIT_SIZE;
	//get addr for the first dentry and inode
	dentries = (dentry_t *)(fs_start + SIZE_FS_DENTRIES+1);
	inodes = (inode_t *)file_addr.st_inode;
	entry_counter = 0;
	return SUCCESS;
}

/*
 * read_dentry_by_name()
 * discription: giving the name of the file, find the dentry that saves the 
 * file.
 * Input: filename, dentry to be filled
 * Output: -1 failure 
 *  		0 success 
 */
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry)
{
	int length = strlen((int8_t *)fname);
	int sizefile, sizedentry, size;
	if(length <= 0){
		return FAIL;
	}
	int i;
	for(i = 0; i < SIZE_FS_DENTRIES; i++ ) 
	{
		sizefile = strlen((int8_t *)fname);
		sizedentry = strlen(dentries[i].filename);
		if(sizefile || sizedentry > 31)	size = 30;
		if(sizefile > sizedentry)	size = sizefile;
		else	size = sizedentry;

		if((strncmp(dentries[i].filename,(int8_t *)fname, size) == 0)) 
		{
			//get dentry
			dentry_copy(i, dentry);
			return SUCCESS;
		}
	}
	return FAIL;
}

/*
 * read_dentry_by_index()
 * Discription: giving the index for dentry, return the target dentry
 * Input: target index, dentry that saves the file
 * Output:
 * 			-1: failure
 * 			0: success
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry)
{
	/* Check for an invalid index. */
	if( index >= SIZE_FS_DENTRIES || index < 0 || index >= my_boot_block.inodes_num)
	{
		return FAIL;
	}
	/* Copy the data into 'dentry'. */
	dentry_copy(index, dentry);
	return SUCCESS;
}

/*
 * read_dentry_by_index()
 * Discription: giving the inode that hold the file, offset in the file, and a length,
 * Saves the information from the offset to length into the buffer
 * Input: target index, dentry that saves the file
 * Output:
 * 			-1: failure
 * 			0: success
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length)
{
	/* Local variables. */
	uint8_t * read_addr;
    uint32_t read_data_block = offset / FS_UNIT_SIZE;
    uint32_t read_data = offset % FS_UNIT_SIZE;
	/* Check for an invalid inode number. */
	if(offset >= inodes[inode].length)	return 0;//end of file reached
	/* check valid */
	if(inode >= my_boot_block.inodes_num || inodes[inode].data_blocks[read_data_block] >= my_boot_block.datablocks_num)
	{
		length_of_file = 0;
		return FAIL;
	}
	/* Calculate the address. */
	read_addr = (uint8_t *)(file_addr.st_data + 
				(inodes[inode].data_blocks[read_data_block])*FS_UNIT_SIZE + read_data);
	/* Read all the data. */
	int i = 0;
	for(i = 0; i < length; i++){
		//see if we reach the end of the datablock
		if(read_data >= FS_UNIT_SIZE )
		{
			read_data = 0;//clear the offset to the begining of next block
			read_data_block++;//move to the next datablock
			/* check valid inode*/
			if( inodes[inode].data_blocks[read_data_block] >= my_boot_block.datablocks_num)
			{
				return FAIL;
			}
			/* Find the start of the next data block. */
			read_addr = (uint8_t *)(file_addr.st_data + (inodes[inode].data_blocks[read_data_block])*FS_UNIT_SIZE);
		}	
		//end of file
		if( i + offset >= inodes[inode].length)
		{
			length_of_file = i;
			return length_of_file;
		}
		//read data into the buffer
		buf[i] = *read_addr;	
		//increment the offset
		read_data++;
		read_addr++;
	}
	//get the true length of file succeesfully readed
	length_of_file = i;
	return length_of_file;
}


/*
 * file_load()
 * load the user program to the pysical memory
 * input: file name pointer, target address to be loaded
 */
int32_t file_load(const int8_t * fname, uint32_t addr)
{
	//check input
	if(fname == NULL || strlen(fname) == 0)	    return -1;
	//get dentry of given file
	dentry_t d;//saves the needed dentry
	if(read_dentry_by_name((uint8_t *)fname, &d) == -1)		return -1;
	//load the information of the program to the given addr
	uint32_t length = inodes[d.inode].length;
	return read_data(d.inode, 0, (uint8_t *)addr, length);
}

/*
 * file_read()
 * read the data in the file from a given filename
 * input: buffer used to saves the file data, offset to start reading, length to be read, file name pointer
 */
int32_t file_read(uint8_t * buf, uint32_t offset, uint32_t length, const int8_t * name)
{
	dentry_t d; //use to save the dentry of the given file
	if(name == NULL || buf == NULL || length <= 0 || offset < 0 || read_dentry_by_name((uint8_t *)name, &d) == -1)	return -1;
	return read_data(d.inode, offset, buf, length);
}

/*
 * file_open()
 * open the file and set the flag
 * input: the module of the filesystem
 * output: success / fail
 */
int32_t file_open(uint32_t fs_start, uint32_t fs_end)
{
	/* Return error if the file system is already open. */
	/*if( 1 == flag_open )
	{
		return FAIL;
	}
	*/
	/* Initialize the file system. */
	flag_open = 1;
	filesystem_init(fs_start, fs_end);
	return SUCCESS;
}

/*
 * file_close()
 * close the file and set the flag
 * input: none
 * output: success/fail
 */
int32_t file_close(){
	//if(flag_open == 0)	return FAIL;
	//flag_open = 0;
	return SUCCESS;
}

/*
 * file_write()
 * write the file
 * input: none
 * output: none
 */
int32_t file_write(){
	return FAIL;
}

/*
 * directory_read()
 * read the directory, this is the main body for the ls program
 * input: buffer that takes the filename
 * output: length of the buffer
 */
int32_t directory_read(uint8_t * buf, uint32_t offset, uint32_t length, const int8_t * name)
{	
	/*int num_node = my_boot_block.dentries_num;
	int i;
	
	for(i = 0; i < num_node; i++ ) 
	{
		int j;
			for(j = 0; j < 32; j++){
				if(dentries[i].filename[j] != '\0'){
					printf("%c", dentries[i].filename[j]);;
				}
				else{
					printf(" ");
				}
			}
		printf("\n");
	}
	return SUCCESS;*/
	int num_node = my_boot_block.dentries_num;
	int i = 0;
	while(i < SBUFSIZE){
		buf[i] = '\0';
		i++;
	}
	if( num_node<=entry_counter)
	{
		entry_counter = 0;
		return SUCCESS;
	}
	int size = strlen((int8_t*)dentries[entry_counter].filename);
	if(size >= SBUFSIZE-1){
		size = SBUFSIZE-1;
	}
	strncpy((int8_t *)buf,(int8_t*)dentries[entry_counter].filename, size);
	entry_counter++;
	return strlen((int8_t *)buf);
}

/*
 * directory_write()
 * write the directory
 * input: none
 * output: always fail
 */
int32_t directory_write()
{
	return FAIL;
}

/*
 * directory_close()
 * close the directory
 * input: none
 * output: always success
 */
int32_t directory_close()
{
	return SUCCESS;
}

/*
 * directory_open()
 * open the directory
 * input: none
 * output: always success
 */
int32_t directory_open()
{
	return SUCCESS;
}


/*
 * helper function to copy a dentry to another dentry
 */
void dentry_copy(uint32_t index, dentry_t * dentry){
	strcpy(dentry->filename, dentries[index].filename );
	dentry->filetype = dentries[index].filetype;
	dentry->inode = dentries[index].inode;
}

/*
 * Test that printout all the file in the computer
 */
void print_allfile_name(void)
{
	int num_node = my_boot_block.dentries_num;
	int i;
	clear();
	printf("-----------------------------------------.\n");
	printf(" Below is all the file in this computer.\n");
	printf("-----------------------------------------.\n");
	for(i = 1; i < num_node; i++ ) 
	{
		int j;
			for(j = 0; j < 32; j++){
				if(dentries[i].filename[j] != '\0'){
					printf("%c", dentries[i].filename[j]);;
				}
				else{
					printf(" ");
				}
			}
		if(dentries[i].filetype == 0){
			printf("   Filetype: RTC");
		} else if(dentries[i].filetype == 1){
			printf("   Filetype: Directory");
		} else if(dentries[i].filetype == 2){
			printf("   Filetype: Regular file");
		} else {
			printf("   Filetype: Invalid file type");
		}
		printf("\n");
	}
	return;
}

/*
 * Test that read a file and print the information in that file to screen
 */
void files_test(void)
{
	int i;
	int8_t * asdf = "frame0.txt";
	uint8_t buf[6000];
	dentry_t dentry;
	clear();
	//read file
	read_dentry_by_name((uint8_t *)asdf, &dentry); 	
	printf("-----------------------------------------.\n");
	printf("   Reading file by name: 'frame0.txt'.\n");
	printf("-----------------------------------------.\n");
	uint32_t length = inodes[dentry.inode].length;
	if(read_data(dentry.inode, 0, buf, length) == SUCCESS){
		for( i = 0; i < length_of_file; i++ )
		{
			printf("%c", buf[i]);
		}
	}
	else{
		printf("FAIL TO READ FILE.\n");
	}
	return;
}

void file_index(int a){
	clear();
	dentry_t dentry;
	printf("-----------------------------------------.\n");
	printf("       Reading file by index: %d.\n", a);
	printf("-----------------------------------------.\n");
	int i = 0;
	if(read_dentry_by_index(a, &dentry) == SUCCESS){
		if(strlen(dentry.filename) == 0){
			printf("READ FAIL: empty file.");
		}
		else{
			for(i = 0; i < 32; i++){
				if(dentry.filename[i] == '\0'){
					break;
				}
				printf("%c", dentry.filename[i]);
			}
		}
	}
	else{
		printf("Read fail: Index does not exist");
	}
	//printf("       %s", dentries[i].filetype);
	printf("\n");
	return;
}
