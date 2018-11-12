// Developed by Aditya Sasongko
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <fcntl.h>
#include "kufs.h"

int main(void)
{ 
	char * disk_name = "test.txt";

	/* creating a disk with 5 blocks */
	kufs_create_disk(disk_name, 5);

	/* mounting the disk */
	kufs_mount(disk_name);

	/* creating 3 files which are supposed to be 
	added to fat data structure */
	kufs_create("file1");
	kufs_create("file2");
	kufs_create("file3");

	/* checking the content of fat up to this point, it should show all the 3 files without block ids */
	kufs_dump_fat();

	// open file2
	int fd = kufs_open("file2");

	/* checking if file descriptor matches the position 
	of file entry in fat data structure */
	printf("fd: %d\n", fd);

	/* Following 2 lines write 2 strings to file2 such that the second string 
	should be written right after the first string.
	Since the first written file is file2, the first block in virtual hard disk
	should be allocated to file2, i.e. these 2 strings must be written in the first block. */
	kufs_write(fd, "written string1", 15);
	
	kufs_dump_fat();
	
	
	kufs_write(fd, "written string2", 15);


	// close file2
	kufs_close(fd);
	// open file1
	fd = kufs_open("file1");
	
	// The following line writes string to the block allocated to file1.	Since file1 is written after file2, the second block in virtual hard disk	should be allocated to file1. 
	kufs_write(fd, "the next written string", 15);

	char char_blocks[2048];
	int i;
	for(i = 0; i < 2048; i++)
	{
		char_blocks[i] = 'a';
	}

	// close file1
	kufs_close(fd);

	// file2 is reopened again, therefore its file_position should be zero.
	fd = kufs_open("file2");

	// file2 is written. The 2048 byte string is written into first block and third block in virtual hard disk, 	since first block is already allocated to file2,	second block is allocated to file1 and third block is unallocated.
	kufs_write(fd, char_blocks, 2048);

	// file position is set to 1022 
	kufs_seek(fd, 1022);

	// the word hello should be divided such that 'he' is written at the end of first block 	and 'llo' is written at the beginning of the third block
	kufs_write(fd, "hello", 5);

	//file position is repositioned to 1022nd byte in file2 again.
	kufs_seek(fd, 1022);
	char read_chars[10];
	
	// 5 bytes are read from file2 starting from current file position
	kufs_read(fd, read_chars, 5);

	// following line should print 'hello'
	printf("read_chars: %s\n", read_chars);

	// Content of FAT is checked again. First and third blocks should be shown in the same line as file2 
	kufs_dump_fat();

	// close file2
	kufs_close(fd);

	// file 1 is deleted
	kufs_delete("file1");
	kufs_dump_fat();

	// File system is unmounted.	The first block in the file should no longer display entry for file1.
	//kufs_umount();

	return 0;
}
