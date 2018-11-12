#ifndef KUFS_H
#define KUFS_H

#define BLOCK_SIZE 1024

typedef enum{false,true} bool;

#define MAX_FILENAME_LENGTH 100
#define MAX_NUMBER_OF_BLOCKS 100

typedef struct
{
    bool used;	
    char name[MAX_FILENAME_LENGTH]; // file name
    int size;                    // file size
    int file_blocks[MAX_NUMBER_OF_BLOCKS]; //array of blocks allocated to file
    int fd;                // file descriptor of file
    int fd_offset;  //which character the cursor is at
} fat;



int kufs_create_disk(char *disk_name, int disk_size);
int kufs_mount(char *disk_name);
int kufs_umount();
int kufs_create(char* filename);
int kufs_open(char *filename);
int kufs_close(int fd);
int kufs_delete(char *filename);
int kufs_write(int fd,void* buf, int n);
int kufs_read(int fd,void *buf,int n);
int kufs_seek(int fd,int n);
void kufs_dump_fat();

#endif
