/*
 * shelly interface program
 
KUSIS ID: PARTNER NAME:
KUSIS ID: PARTNER NAME:
 
 */
 
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>
 
#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */
int parseCommand(char inputBuffer[], char *args[], int *background);
void shouldrunmethod(int shouldrun, char *args[], int background );
 
int main(void)
{   char inputBuffer[MAX_LINE];             /* buffer to hold the command entered */
    int background;  
    char *args[MAX_LINE/2 + 1];            /* command line (of 80) has max of 40 arguments */
    int status;                   /* result from execv system call*/
    int shouldrun = 1;
    char *fileName;
    int i, j;
    int k=0;
    char pathtofile[20];
    int redirect1 = 0;
    int redirect2 = 0;
    int isOnScript=0;
    int saved_stdout;
    int out;
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    int len; //length of command line entered
    int bookmark, bookmarkcount, bookmarkindex =0;
    FILE *file;
    char bookmarks[20][255];
    char buff[255];
char *token;
 
    while (shouldrun){                    /* Program terminates normally inside setup */
        background = 0;

	
	//load bookmarks from the file	
	bookmarkcount=0;
   	file=fopen("/home/eda/mybookmarks","r");
	while(!feof(file)){fgets(buff, 255, file);
	 strcpy(bookmarks[bookmarkcount], buff);
	 bookmarkcount++; 
	} 
	bookmarkcount--;
	fclose(file);
	

	if(bookmark==0){shouldrun = parseCommand(inputBuffer,args,&background);   }  /* get next command */  
	    // determine length of command line entered
		for (int e=0;e<41;e++) {
		    if(args[e]==NULL) { 
		        len=e; 
		        break;}}
		if(bookmark==1){    
		printf("%s", bookmarks[bookmarkindex]);
		int a=0;
		args[a] = strtok(bookmarks[bookmarkindex], " ");
   
  	 /* walk through other tokens */

  		 while( args[a]!=args[a-1]) {
  	    args[a]= strtok(NULL, " ");
	
		a++;
   }
	
	len=a-1;
}  

	/* checks to see if it is a redirect1 > or redirect2 >> command*/
	if(args[len-2]!=NULL){
	if(strncmp(args[len-2], ">>", 2)==0){redirect2=1;}
	if(strncmp(args[len-2], ">", 1)==0&&redirect2!=1){redirect1=1;}}

        if (strncmp(args[0], "exit", 4) == 0){
		if(bookmark==1){shouldrun=1;bookmark=0;}
        	if(isOnScript==1){
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		printf ( " Script has ended. Current local time and date: %s", asctime (timeinfo) );
		isOnScript=0;
                dup2(saved_stdout, 1);
                close(saved_stdout);
                close(out);
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		printf ( " Script has ended. Current local time and date: %s", asctime (timeinfo) );
		}
       		else{
                shouldrun = 0;     /* Exiting from myshell*/
            	}
        }

	else if (strncmp(args[0], "script", 6) == 0){
		if(bookmark==1){shouldrun=1;bookmark=0;}
		printf("Script has started \n");
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		printf ( " Current local time and date: %s", asctime (timeinfo) );
		isOnScript=1;
		out = open(args[1], O_WRONLY | O_APPEND | O_CREAT);
		saved_stdout = dup(1);
		dup2(out, 1);  
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		printf ( "Current local time and date: %s", asctime (timeinfo) );

        }
	else if (strncmp(args[0], "wforecast", 9) == 0){
		if(bookmark==1){shouldrun=1;bookmark=0;}
		
		FILE *fp;
		fp = fopen("cronweather.sh", "a+");
		//fprintf(fp, "#!/bin/bash\n");
       		fprintf(fp, "\ncrontab -l; (echo \"");
		fprintf(fp, "21 10 * * * curl wttr.in/Istanbul.png");
		fprintf(fp, " > %s", args[1]);
		fprintf(fp, "\") | crontab -"); 
      		fclose(fp);
				
		
		int out = open(args[len-1], O_WRONLY | O_TRUNC | O_CREAT, 0777);
                int saved_stdout = dup(1);
                dup2(out, 1);  
       		system("/home/eda/cronweather.sh");
      		dup2(saved_stdout, 1);
     		close(saved_stdout);
      		close(out);  
        }
   
	else if (strncmp(args[0], "bookmark", 8) == 0){
		if (strncmp(args[1], "-r", 2) == 0){
		        int lno, ctr = 0;
			int MAX=256;
			char ch;
			FILE *fptr1, *fptr2;
			char str[MAX], temp[] = "temp.txt";
			fptr1 = fopen("/home/eda/mybookmarks", "r");
			fptr2 = fopen(temp, "w"); // open the temporary file in write mode 
			for(int z = 0; z < 20; z++)
			{
			int strl=strlen(args[2]);
			if(strncmp(bookmarks[z], args[2], strl)==0){
			lno=z;}
			}
			lno++;
			while (!feof(fptr1)){
			strcpy(str, "\0");
			 fgets(str, MAX, fptr1);
			  if (!feof(fptr1)){
			  ctr++;
			   if (ctr != lno){
			   fprintf(fptr2, "%s", str);}}}
			    fclose(fptr1);
			    fclose(fptr2);
			    remove("/home/eda/mybookmarks");
			    rename(temp, "/home/eda/mybookmarks");
			    fptr1=fopen("/home/eda/mybookmarks","r");
			    ch=fgetc(fptr1);}
        
		else{

		if(bookmark==1){shouldrun=1;bookmark=0;}

		int bookmarkallowed=1;
		
		for(int z = 0; z < 20; z++)
		{
		int strl=strlen(args[1]);
		if(strncmp(bookmarks[z], args[1], strl)==0){
		bookmarkallowed=0;
		printf("I am sorry but the bookmark %s already exists \n", args[1]);}
		}
		if (bookmarkallowed==1){
		FILE *fp;
		fp = fopen("/home/eda/mybookmarks", "a+");
  		 fprintf(fp, "\n%s", args[1]);
		for(int h=2;h<len; h++){
		
		fprintf(fp, " %s", args[h]);
		}
		fclose(fp); }
        }}
	  else if (strncmp(args[0], "at", 2) == 0){
		if(bookmark==1){shouldrun=1;bookmark=0;}
		//freopen("weather.png", "w", stdout);
		
		
		FILE *fp;
		fp = fopen("at.sh", "a+");
		//fprintf(fp, "#!/bin/bash\n");
       		fprintf(fp, "\ncrontab -l; (echo \"");
		fprintf(fp, "%s %s %s %s %s ", args[1],args[2],args[3],args[4],args[5]);
		for(int y=6; y<len-1; y++){
		fprintf(fp, "%s ", args[y]);}
		fprintf(fp, "%s", args[len-1]);
		fprintf(fp, "\") | crontab -"); 
      		fclose(fp);
		system("/home/eda/at.sh");

        }
   
        else if(redirect1==1&&shouldrun==1){  
		if(bookmark==1){shouldrun=1;bookmark=0; }
		fileName=args[len-1];
                int out = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0777);
                int saved_stdout = dup(1);
                dup2(out, 1);  
                args[len-1]=NULL;
             	args[len-2]=NULL;
    		shouldrunmethod(shouldrun, args, background );
      		dup2(saved_stdout, 1);
     		close(saved_stdout);
      		close(out);     
            redirect1=0;
        }
        
        else if(redirect2==1&&shouldrun==1){
		if(bookmark==1){shouldrun=1;bookmark=0;}
		fileName=args[len-1];
		int out = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0777);
		int saved_stdout = dup(1);
		dup2(out, 1);  
		args[len-1]=NULL;
		args[len-2]=NULL;
		shouldrunmethod(shouldrun, args, background );
		dup2(saved_stdout, 1);
		close(saved_stdout);
		close(out);
		redirect2=0;
       }
	

       else{ 
		if(bookmark==1){shouldrunmethod(shouldrun, args, background );shouldrun=1;bookmark=0;}else{
         	if(isOnScript==1){
                int j=0;
                while (inputBuffer[j] != '\n' && inputBuffer[j] != '\0'){
                 	printf("%c", inputBuffer[j]);
                        j++;
                 }
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		printf ( "       Current local time and date: %s", asctime (timeinfo) );
        
     		}
       		 shouldrunmethod(shouldrun, args, background );
           	 printf( "\n");

		for(int z = 0; z < 20; z++)
		{
		int strl=strlen(args[0]);
		if(strncmp(bookmarks[z], args[0], strl)==0){
		bookmark=1;bookmarkindex=z;}
		}


       }}

    }

    return 0;
}
 
void shouldrunmethod(int shouldrun, char *args[], int background ){
char pathtofile[20]="";
pid_t child;                    /* process id of the child process */
    if (shouldrun) {
         child = fork();                               /* create child process*/
        if(child < 0){
           perror("Could not fork! \n");
        }
        else if(child==0){                             /* child process */
            strcat(pathtofile, "/bin/");
            strcat(pathtofile, args[0]);
    //    printf("%s",pathtofile);
            int status = execv(pathtofile, args);
            if(status <= 0){
	
	   
          //    printf("Could not find a command!");
exit(0);
            }
        }
        else {                                         /* parent process */
        if(background == 0){                     /* the & is not included in command line */
        waitpid(child,&background,0);
           }}
        


    }   /* wait for the specified child process to terminate */
}

/** 
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */


int parseCommand(char inputBuffer[], char *args[],int *background)
{
    int length,        /* # of characters in the command line */
      i,        /* loop index for accessing inputBuffer array */
      start,        /* index where beginning of next command parameter is */
      ct,            /* index of where to place the next parameter into args[] */
      command_number;    /* index of requested command number */
      
    ct = 0;
    
    /* read what the user enters on the command line */
    do {
      printf("shelly>");
      fflush(stdout);
      length = read(STDIN_FILENO,inputBuffer,MAX_LINE); 
        
    }
    while (inputBuffer[0] == '\n'); /* swallow newline characters */
    
    /**
     *  0 is the system predefined file descriptor for stdin (standard input),
     *  which is the user's screen in this case. inputBuffer by itself is the
     *  same as &inputBuffer[0], i.e. the starting address of where to store
     *  the command that is read, and length holds the number of characters
     *  read in. inputBuffer is not a null terminated C-string. 
     */    
    start = -1;
    if (length == 0)
      exit(0);            /* ^d was entered, end of user command stream */
    
    /** 
     * the <control><d> signal interrupted the read system call 
     * if the process is in the read() system call, read returns -1
     * However, if this occurs, errno is set to EINTR. We can check this  value
     * and disregard the -1 value 
     */

    if ( (length < 0) && (errno != EINTR) ) {
      perror("error reading the command");
      exit(-1);           /* terminate with error code of -1 */
    }
    
    /**
     * Parse the contents of inputBuffer
     */
    
    for (i=0;i<length;i++) { 
      /* examine every character in the inputBuffer */
      
      switch (inputBuffer[i]){
      case ' ':
      case '\t' :               /* argument separators */
    if(start != -1){
      args[ct] = &inputBuffer[start];    /* set up pointer */
      ct++;
    }
    inputBuffer[i] = '\0'; /* add a null char; make a C string */
    start = -1;
    break;
    
      case '\n':                 /* should be the final char examined */
    if (start != -1){
      args[ct] = &inputBuffer[start];     
      ct++;
    }
    inputBuffer[i] = '\0';
    args[ct] = NULL; /* no more arguments to this command */
    break;
    
      default :             /* some other character */
    if (start == -1)
      start = i;
    if (inputBuffer[i] == '&') {
      *background  = 1;
      inputBuffer[i-1] = '\0';
    }
      } /* end of switch */
    }    /* end of for */
    
    /**
     * If we get &, don't enter it in the args array
     */
    
    if (*background)
      args[--ct] = NULL;
    
    args[ct] = NULL; /* just in case the input line was > 80 */
ct++;

    return 1;
    
} /* end of parseCommand routine */







