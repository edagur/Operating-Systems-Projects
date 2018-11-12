#include <stddef.h>
#include <printf.h>
#include <memory.h>
#include <stdlib.h>
#include "kufs.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int global_disk_size;
bool disk_open=false;
int virtual_disk_fd;
fat* fat_pointer;
bool* unallocated_blocks_array;
int unallocated_blocks[1024];
int dir_length=0;
int dir_length_absolute=0; //index of the last file entry in the first fat array
int kufs_create_disk(char *disk_name, int disk_size){
  int f;
  char buf[BLOCK_SIZE];

  if ((f = open(disk_name, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0) {
    perror("Disk creation failed.\n");
    return -1;
  }

  memset(buf, 0, BLOCK_SIZE);
  for (int i = 0; i < disk_size; ++i){
    write(f, buf, BLOCK_SIZE);}
    
    global_disk_size=disk_size;
  close(f);
printf("Disk created successfully.\n");
return 0;
}


int kufs_mount(char *disk_name){
if (disk_open) {
    printf("open_disk: disk is already open\n");
    return -1;
}

if ((virtual_disk_fd = open(disk_name, O_RDWR, 0777)) < 0) {
    perror("open_disk: cannot open file");
    return -1;
  }
  
  disk_open=true;
  
   char buf[BLOCK_SIZE] = "";
   fat_pointer = (fat*)malloc(BLOCK_SIZE);
    memset(buf, 0, BLOCK_SIZE);
    lseek(virtual_disk_fd, 0, SEEK_SET);
    read(virtual_disk_fd, buf, BLOCK_SIZE) ;
    
    memcpy(fat_pointer, buf, sizeof(fat_pointer) * dir_length);
    
	//unallocated_blocks_array = (bool*)malloc(global_disk_size);
	for(int i=1; i<global_disk_size; i++){
	}

printf("Disk [%s] is mounted successfully.\n", disk_name);
  
return 0;}
int kufs_umount(){


free(unallocated_blocks_array);

 
  
    bool unmount_allowed=true; //	unmount shouldn't be allowed if there are file descriptors in use
     for (int i = 0; i < dir_length_absolute; ++i) {
        if(fat_pointer[i].fd != -1) {
        unmount_allowed=false;
        }
    }
    
	if(!unmount_allowed){printf("There are open files, unmount not allowed.\n");}
	else{


     char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    char* block_ptr = buffer;

    for (int i = 0; i < dir_length_absolute; ++i) {
        if(fat_pointer[i].used == true) {
            memcpy(block_ptr, &fat_pointer[i], sizeof(fat_pointer[i]));
            block_ptr += sizeof(fat);
        }
}
 

    
    printf("umount %s\n", buffer);

lseek(virtual_disk_fd, 0, SEEK_SET);

write(virtual_disk_fd, buffer, BLOCK_SIZE);

free(fat_pointer);

  if (!disk_open) {
    printf(" no open disk\n");
    return -1;
  }
  
  close(virtual_disk_fd);

disk_open=false;
virtual_disk_fd=0;

printf("Disk unmounted successfully.\n");
}
return 0;}

int kufs_create(char* filename){
  for(int i = 0; i < dir_length_absolute; i++) {
        if(strcmp(fat_pointer[i].name, filename) == 0) {
        printf("File with name [%s] already exists", filename);
        return -1;
        }}
        
        
        for(int i = 0; i < dir_length_absolute; i++) {
            if(fat_pointer[i].used == false) {
                /* Initialize file information */
                strcpy(fat_pointer[i].name, filename);
                fat_pointer[i].size = 0;
               	fat_pointer[i].fd = -1;
               	fat_pointer[i].used = true;
               	fat_pointer[i].fd_offset = 0;
               	for(int j =0; j<MAX_NUMBER_OF_BLOCKS; j++){
               	fat_pointer[i].file_blocks[j]=-1;
                }
                printf("File [%s] created successfully. \n", filename);
                dir_length++;
		dir_length_absolute++;
                return 0;
		}
	
        }
                fat_pointer[dir_length_absolute].used = true;
                strcpy(fat_pointer[dir_length_absolute].name, filename);
                fat_pointer[dir_length_absolute].size = 0;
               	fat_pointer[dir_length_absolute].fd = -1;
               	fat_pointer[dir_length_absolute].fd_offset = 0;
               	for(int j =0; j<MAX_NUMBER_OF_BLOCKS; j++){
               	fat_pointer[dir_length_absolute].file_blocks[j]=-1;
                }
                printf("File [%s] created successfully.\n", filename);
                dir_length++;
		dir_length_absolute++;
		
   return 0;}
   
   
int kufs_open(char *filename){
  for(int i = 0; i < dir_length_absolute; i++) {
  		
  	if(fat_pointer[i].fd!=-1){ 
  	printf("The file [%s] is already open. \n", filename); 
  	return fat_pointer[i].fd;}
  	else{
        if(strcmp(fat_pointer[i].name, filename) == 0) {
        fat_pointer[i].fd=i;
        fat_pointer[i].used=true;
        printf("The file [%s] is opened with fd %d. \n", filename, fat_pointer[i].fd);
        return fat_pointer[i].fd;
        }}
        }
        printf("The file [%s] is not found on the file system.\n", filename); 
return -1;}


int kufs_close(int fd){

 for(int i = 0; i < dir_length_absolute; i++) {
        if(fat_pointer[i].fd==fd) {
        
        printf("The file [%s] with fd %d is closed.\n", fat_pointer[i].name,fat_pointer[i].fd );
	fat_pointer[i].fd=-1;
	fat_pointer[i].fd_offset=0;
	 return 0;
        }}
        
        printf("The file you are trying to close is not open\n");
        return -1;}
        
        
int kufs_delete(char *filename){
for(int i = 0; i < dir_length_absolute; i++) {
  		
        if(strcmp(fat_pointer[i].name, filename) == 0) {
        
        printf("The file [%s] is deleted\n", filename);
        fat_pointer[i].fd=-1;
        fat_pointer[i].used=false;
        fat_pointer[i].size=0;
        strcpy(fat_pointer[i].name, "");
        fat_pointer[i].fd_offset=0;
        for(int j=0; j<MAX_NUMBER_OF_BLOCKS; j++){
        if(fat_pointer[i].file_blocks[j]!=-1){
        fat_pointer[i].file_blocks[j]=-1;}

        }
        
         return 0;
        }
}

return -1;}
int kufs_write(int fd,void* buf, int n){
  	
  	
  		
       

return -1;}
int kufs_read(int fd,void *buf,int n){return -1;}
int kufs_seek(int fd,int n){

 for(int i = 0; i < dir_length_absolute; i++) {
        if(fat_pointer[i].fd==fd&&fat_pointer[i].used) {
        
        if(n>fat_pointer[i].size){
	fat_pointer[i].fd_offset=fat_pointer[i].size;
	printf("The file [%s] with fd %d has now offset %d.\n", fat_pointer[i].name,fat_pointer[i].fd , fat_pointer[i].size );
	return fat_pointer[i].size;}
	else{
	fat_pointer[i].fd_offset=n;
		printf("The file [%s] with fd %d has now offset %d.\n", fat_pointer[i].name,fat_pointer[i].fd , fat_pointer[i].fd_offset );
	 return n;
        }}
        
        }
        

return -1;}
void kufs_dump_fat(){
    char buf[BLOCK_SIZE]="";
    
    for (int i = 0; i < dir_length_absolute; ++i) {
        if(fat_pointer[i].used == true) {
        strcat(buf, " ");
           strcat(buf, fat_pointer[i].name);
           
         int j=0;  
         while(fat_pointer[i].file_blocks[j]!=-1)
         {char numberbuffer[5]; 
         
         sprintf(numberbuffer, " %d ",fat_pointer[i].file_blocks[j] );
         strcat(buf, numberbuffer);
         	j++;}
         	
         	 strcat(buf, "\n");
         	
        }
    }
    printf("%s",buf);
    
return;}




