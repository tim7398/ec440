#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
using namespace std;
#define MAX_LINE 256 


int length = 0, location=0;
int parser(int length, char **command, char**args1, char**args2);
int run_command(char** command, int length);
int get_input(char** command);
void run_pipe(char** args1, char** args2);
void run_redirect(char** args1, char** args2);
void interrupt_handler(int);
int no_append = 0, input_f=0, out = 0, background=0, counts = 1, child = 0;
char handler_message[50];

int main(void)
{
	
	int should_run = 1;
	

	while(should_run) // runs the shell while flag is 1
	{
		char *args1[MAX_LINE]={NULL}, *args2[MAX_LINE]={NULL};
		char *command[MAX_LINE]={NULL};
		int action =0;
		printf("\nTSH>");
		fflush(stdout);
		length = get_input(command);
		
		action = parser(length,command, args1, args2);

		if (action == 1) // pipe
		{
			run_pipe( args1, args2);

		}

		if (action == 2) // redirect
		{
			run_redirect(args1,args2);
		}

		if(action == 0) // neither
		{
			run_command(command, length);
		}
		
	
	}
	
	
}

int get_input(char** command)
{
	char* input;
	string commands;
	int size = 0; 

	while (true)
	{
		cin >> commands;
		input = new char[commands.size()+1];
		strcpy(input, commands.c_str());
		command[size] = input;
		signal(SIGINT, interrupt_handler);

		size++;
		if (cin.get() == '\n')
		{
			counts=1;
			break;
		}

	}

	return size;
	
}

int parser(int length, char **command, char** args1, char** args2)
{
	int found = 0, i = 0; 
	// found = 0 is neither pipe or redirect
	// found = 1 is a pipe. 
	// found = 2 is a redirect. 
	// found = 3 both
	// location is where these things are found. 
	int counter = 0, increment = 0, increment2=0, location = 0;

	
	
	while ( command[i] != NULL)
	{
		
		if( strcmp(command[i], "|")==0)
		{
			found = 1;				// code for pipe
			location = i; // location of two potential pipes
			counter++;			
		}
		
		
		if( strcmp(command[i], ">>")==0 || strcmp(command[i],">") == 0 || strcmp(command[i],"<") == 0 || strcmp(command[i],"2>")==0||strcmp(command[i],"2>>")==0 ||strcmp(command[i],"&>")==0 )
		{	
			found = 2;				// code for redirect
			if(strcmp(command[i],">") == 0)
			{
				no_append = 1;
				out=0;
				input_f = 0;
				background=0;
			}
			if(strcmp(command[i],"2>>")==0)
			{
				out = 1;
				no_append = 0;
				input_f=0;
				background=0;
			}
			if(strcmp(command[i],"&>")==0)
			{
				out = 0;
				no_append = 1;
				input_f=0;
				background = 1;
			}
			if(strcmp(command[i],"2>")==0)
			{
				out = 1;
				no_append=1;
				input_f=0;
				background=0;
			}


			if(strcmp(command[i], ">>")==0 )
			{
				no_append = 0;
				out = 0;
				input_f = 0;
			}

			if (strcmp(command[i],"<") == 0)
			{
				no_append = 0;
				input_f = 1;
				out = 0;

			}

			location = i; // location of redirect	
		}
		i++;
		
	}
	
	//NOW I NEED TO PARSE THROUGH ARRAY AND SPLIT COMMANDS UP INTO SECTIONS
	
	if(found == 1 || found == 2)
	{
		
		// only 1 pipe
		for (int x = 0; x<location; x++)// first half before pipe
		{
			args1[x]= command[x];

		}

		for (int x = location+1; command[x]!=NULL; x++)// half after pipe
		{
			args2[increment] = command[x];
			increment++;

		}
	}

	
	


	
	
	

	return found;
	
	
}


int run_command(char** command, int length)
{
	pid_t pid;
	int i = 0, x= 0;
	
	int exist_amp = 0; // flag for finding '&', intially set to 0
		
			int foo = 0;
		
	

	while(1)// parse through char by char
	{
		
		if( (isalpha(command[i][x])==false) && (isdigit(command[i][x])==false) && command[i][x] != '&') // makes sure it does not over shoot each string
		{
			i++;	// go to next string
			x=0;	// start from beginning of new string
		}

		if ( i == length) // end of array of strings
		{
			break;
		}

		if (command[i][x] == '&') // found an ampersand
		{		
			exist_amp = 1; // set flag to 1
			command[i][x] = ' ';
			printf("found it: %c\n", command[i][x]);
		}

		x++; // goes through each char
	}

	pid = fork(); // fork a new process
	if (pid < 0) // error
	{
		fprintf(stderr, "Fork Failed");
		return 1;
	}
	//parent process and no ampersand
	if (exist_amp == 0) 
	{
	child = pid;

		waitpid(pid, NULL, 0); // only if no ampersand
	}
	
	
	//child process
	if (pid == 0)
	{
		
		execvp(command[0], command);
		perror("execvp error");
		
	}
	
	return 0;
	
}

void run_pipe(char** args1, char** args2)
{

	pid_t pid1;
	int fd[2];
	int Read_end = 0;
	int Write_end = 1;

	if(pipe(fd) == -1) // check for error
	{
		fprintf(stderr, "Pipe Failed\n" );
		return;
	}




	pid1 = fork();

	if(pid1 < 0)
	{
		fprintf(stderr, "Fork Failed\n" );
		return;
	}

	if(pid1 > 0) // parent process
	{
		child = pid1;
		close(fd[Read_end]); // close the unused end of the pipe
		execvp(args1[0], args1); // execute commands and argument
		close(fd[Write_end]);
		perror("execvp Failed");
	}

	else // child process
	{
		child = pid1;
		printf("child process: %d", pid1);
		close(fd[Write_end]); // close the unused end of the pipe
		dup2(fd[Read_end], 0); // duplicate the read end into stdin for process
		execvp(args2[0], args2); // execute commands and argument
		perror("execvp Failed");
	}



	}



	void run_redirect(char** args1, char** args2)
	{

			long file_length;
			int end = 0, read_Pipe[2], write_Pipe[2]; 
			pid_t pid1, pid2;
			char from_pipe[MAX_LINE];
			char from_file[MAX_LINE];	//read from file or from pipe
			FILE *fileIn,*fileOut;// used for writing and reading from files
			
			//Initialize pipes
			if (pipe(read_Pipe) <0){perror("read pipe");}
			if (pipe(write_Pipe)<0){perror("write pipe");}
						
				// check to see which kind of redirect
				if (no_append == 0) // append in this case
				{
					fileOut = fopen(args2[0],"a");
					printf("This is filenameOutput %s\n",args2[0]);
				}
				else { // no appending
					fileOut = fopen(args2[0],"w");
					}
				

			
			if (input_f==1) // use file as command
			{
				fileIn = fopen(args2[0],"r");
				fseek(fileIn,0,SEEK_END);			//find size of file
				file_length=ftell(fileIn);		
				rewind(fileIn);				//rewinds to top
				
				
				fread(from_file,1,file_length,fileIn);	
				fclose(fileIn);		
				

				
			}			
			
			//child process
			if ((pid1=fork())==0)
			{	
				child=pid1;
				printf("this is child pid %d\n", child);
				close(STDIN_FILENO);
				close(STDOUT_FILENO);
				close(STDERR_FILENO);
				//Setup outputs
				if (out == 1 )
				{
					dup2(read_Pipe[1],STDERR_FILENO);
					close(read_Pipe[0]);		//close input side of pipe
				} 
				else if (background == 1) 
				{
					dup2(read_Pipe[1],STDERR_FILENO);
					dup2(read_Pipe[1],STDOUT_FILENO);
					close(read_Pipe[0]);		//close input side of pipe
				} 
				else if (input_f==1)// input from file
				{							
					dup2(write_Pipe[0],STDIN_FILENO);		//stdin is coming from writepipe
					dup2(read_Pipe[1],STDERR_FILENO);
					dup2(read_Pipe[1],STDOUT_FILENO);
					close(read_Pipe[1]);		//close output side of pipe.
					close(write_Pipe[1]);		//close input to line

				}  
				else 
				{ 
					dup2(read_Pipe[1],STDERR_FILENO); 
					dup2(read_Pipe[1],STDOUT_FILENO);
					close(read_Pipe[0]);
				}
				
				execvp(args1[0], args1); // execute program
				
				
				perror("Could not execute program");
			//Parent Process
			} 
			else {
				
				child = pid1;
				if (input_f==1) // input from file. 
				{
					close(write_Pipe[0]);// close input of write pipe
					write(write_Pipe[1],from_file,file_length);	//send from_file to write pipe
			
				}

				//Read from the child process
				close(read_Pipe[1]);				//close output side of pipe
				end=read(read_Pipe[0],from_pipe,MAX_LINE);		//read
				from_pipe[end]=0;					//null terminate


				if (input_f==0) // prints to file
				{
					fprintf(fileOut,"%s",from_pipe);
					printf("i came here\n");


					fclose(fileOut);


				
				} 
				else {	
					printf("%s",from_pipe); // prints to terminal
				}
				
				

				waitpid(pid1,NULL,0);
			}		
		}
		
	 	
	

	void interrupt_handler(int signo)
	
	{

		if(counts == 1)
		{
			// printf("killed the childs \n");
			kill(child, SIGTERM);
			counts--;
		}
		 if(counts==0)
		{
			// printf("everything gone\n");
			 signal(SIGINT,SIG_DFL);  // restore default handler

		}

	}

























