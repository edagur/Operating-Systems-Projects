#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/mman.h>

#define MAX_PIDS 32
int status;
 pid_t *pids; //to keep process ids

int main(int argc, char *argv)
{
    	char str[100];
	printf( "Enter a word:");

   	int N=strlen(fgets(str,sizeof(str),stdin));//number of players for whispers game


pids = mmap(0, MAX_PIDS*sizeof(pid_t), PROT_READ|PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
int counter=0;
for(int i=0;i<N;i++) // loop will run N times
    {

        if(fork() == 0)
        {
	pids[i]=getpid();
    printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid());
    	exit(0);
	}else{
	waitpid(*pids,&status,0);
        printf("parent pid is %d\n",getpid());
	}
  // putting process id's into an array
     
    }
//int n=sizeof(pids);
//sorting the process pids in array
        for (int i = 0; i < N; i++)  {
            for (int j = i=0; j < N; j++) {
		 if (pids[i] > pids[j]){
			 int a = pids[i];
			pids[i] =pids[j];
			pids[j] = a;

				}
				 	}
       				 

for (int i = 0; i < N; i++)  {
printf("pids[i] is %d\n",pids[i]);
}


 


	char *buf=fgets(str,sizeof str,stdin);
	buf = malloc(10*sizeof(char)); 
	printf("buf is %s",buf);
//10 char buffer to place the data that will be received from the queue by another process
 	size_t length=strlen(buf);
	mqd_t q2;
	q2 = mq_open("/test_q",O_CREAT,0666,NULL); 
	 //creating a message queue with mq_open

struct mq_attr *attr1;
attr1 = malloc(sizeof(struct mq_attr));
mq_getattr(q2, attr1);
//allacating memory for the structure and then pass the queue descriptor and the structure pointer to mq_getattr that fills the structure with relevant data.

//sending messages into the queue,we again open the queue,but no create it.
mqd_t q1;
q1 = mq_open("/test_q",O_RDWR); 


//sending data,1 represents the priority of the message being sent.
mq_send(q1,buf,length,1);


for(int i=0;i<N;i++) // loop will run N times
    {
mq_receive(q2,buf,attr1->mq_msgsize,&pids[i]); 
//receiving data from the queue

int randomBit = rand() % 2;
if(randomBit==0){
char harf=buf[i+1];
char nextHarf=(char)((int) harf + 1);
buf[i+1]=nextHarf;
}

if(&pids[i+1]==NULL){

mq_send(q1,buf,length,pids[i]);
mq_receive(q2,buf,attr1->mq_msgsize,&pids[0]);
 printf("Word is %s",buf);
}else{
mq_send(q1,buf,length,pids[i]);
printf("Word is %s",buf);
}
}

}

}

