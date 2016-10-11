/*
Simple Shell implementation

Author: Luis Gallet Zambrano 260583750
Last date modified: 10/10/2016
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include "Shell.h"
#include "Queue_linkedList.h"

/*
Linked List global variables
Two global variables that represent the front and the rear
of the history list (Linked List)
*/
struct Node *hisList_front = NULL;
struct Node **ref_hisList_front = &hisList_front;

struct Node *hisList_rear = NULL;
struct Node **ref_hisList_rear = &hisList_rear;

/*
Linked List global variables
Two global variables that represent the front and the rear
of the jobs list (Linked List)
*/
struct Node	*jobList_front = NULL;
struct Node **ref_jobList_front = &jobList_front;

struct Node *jobList_rear = NULL;
struct Node **ref_jobList_rear = &jobList_rear;

/*
Globl variables to handle the output redirection

is_out_redirection: check if command entered asks for an output redirection
command_redirect: command user wishes to redirect
filename: file destination of the entered command
*/
bool_t is_out_redirection = false;
char *command_redirect[20];
char filename[20];

/*
Globl variables to handle the piping command

isPiping: check if command entered asks for piping
lhs_command: command sender
rhs_command: command receiver
*/
bool_t isPiping = false;
char *lhs_command[20];
char *rhs_command[20];

int getcmd(char *prompt, char *args[], int *background,int *args_count, char **argument)
{
	int length, i = 0;
	char *token, *loc;
	char *line = NULL;
	size_t linecap = 0;
	printf("\n%s>>", prompt);
	
	length = getline(&line, &linecap, stdin);
	
	if (length <= 0) {
		exit(-1);
	}else if(length <= 1){
		puts("No command entered, try again");
		return -1;
	}
	
	if(line[0] == ' '){
		puts("No command entered, try again");
		return -1;
	}
	
	// Check if background is specified..
	if ((loc = index(line, '&')) != NULL) {
		*background = 1;
		*loc = ' ';
	} else
	*background = 0;
	
	// Check if output redirection is specified..
	if ((loc = index(line, '>')) != NULL) {
		is_out_redirection = true;
	} else{
		is_out_redirection = false;
	}

	// Check if piping is specified..
	if ((loc = index(line, '|')) != NULL) {
		isPiping = true;
	} else{
		isPiping = false;
	}
	
	while ((token = strsep(&line, " \t\n")) != NULL) 
	{
		for (int j = 0; j < strlen(token); j++)
			if (token[j] <= 32)
				token[j] = '\0';
			if (strlen(token) > 0)
				args[i++] = token;
	}
		
	//Case where we want user chooses a command from the history list
	if(args[0][0] == '!'){
		//Returns history list index specified in argument entered	
		int arg_index = getIndex(args);
		
		char *temp_arg = (char *)malloc(80 * sizeof(char));
		
		strcpy(*argument, getItem(ref_hisList_front, arg_index));
		
		//Handles the case when the last argument entered was asked to be run in bg, 
		//but not the new one
		if(*background == 1){
			strcat(*argument, " &");
		}
		
		//Make one temporary copy of the selected argument, because the char* will be
		//stripped while constructing char* args[] with instruction from list
		strcpy(temp_arg, getItem(ref_hisList_front, arg_index));
		
		i = 0;
		
		if ((loc = index(temp_arg, '&')) != NULL) {
			*loc = ' ';
			*background = 1;
		}
		
		if ((loc = index(temp_arg, '>')) != NULL) {
			is_out_redirection = true;
		} else{
			is_out_redirection = false;
		}

		// Check if piping is specified..
		if ((loc = index(temp_arg, '|')) != NULL) {
			isPiping = true;
		} else{
			isPiping = false;
		}	
		
		while ((token = strsep(&temp_arg, " ")) != NULL) 
		{
			for (int j = 0; j < strlen(token); j++)
				if (token[j] <= 32)
					token[j] = '\0';
				if (strlen(token) > 0)
					args[i++] = token;
		}
		free(temp_arg);
		
		printf("Selected argument from history list: %s\n", *argument);
	}
	
	free(line);
	
	//for redirect symbol
	args[i] = NULL;
	
	//add argument to history list
	addHistory(args, &i, args_count, argument, background);

	if(is_out_redirection){
		setOutRedirection(args);
	}else if(isPiping){
		setPiping(args, &i);

	}
	
	return i;
}

/*
This function creates a string of the entered argument 
*/
void setArgumentString(char *args[], int *args_size, char **argument){
	
	int arg_length = *args_size;
	strcpy(*argument, args[0]);
	if(arg_length > 1){
		for(int i=1; i < arg_length ; i++)
		{
			strcat(*argument, " ");
			strcat(*argument, args[i]);
		}
	}
}

/*
It adds the entered argument to the history list
*/
void addHistory(char *args[], int *args_size, int *counter, char **argument, int *background){
	
	setArgumentString(args, args_size, argument);
	
	char *arg_to_list = (char *)malloc(80 * sizeof(char));
	int list_size = size(ref_hisList_front);
	
	if(strcmp(*argument,"history") != 0 && strcmp(*argument,"exit") != 0){
		strcpy(arg_to_list, *argument);
		
		if(*background == 1){
			strcat(arg_to_list, " &");
		}
	
		//Check size of history list before adding a new item. Remove oldest item in the queue
		//when there are 10 items in the list
		if(list_size == 10){
			dequeue(ref_hisList_front, ref_hisList_rear);
			enqueue(arg_to_list, counter, ref_hisList_front, ref_hisList_rear);
		}else{
			enqueue(arg_to_list, counter, ref_hisList_front, ref_hisList_rear);
		}
		/* char* arg_to_list cannot be free, 
		otherwise the linked list will get wrong values
		*/		
	
	//Decrease argument counter by one when command history is chosen, since it is not registered
	//in the history list
	}else if(strcmp(*argument,"history") == 0){
		*counter = *counter - 1;
	}
}

/*
Prints the history list in the following format:
index- command

Where index indicates when the command was entered, for example:
if index = 2, it means that the command at that index was the second
command entered by the user
*/
void printHistory(){
	printList(ref_hisList_front);
}

/*
Prints the job list in the following format:
PID- command
*/
void printJobs(){
	printList(ref_jobList_front);
}

/*
When a command from the history list is demanded (eg. !2), 
this function returns the index asked by the user as an integer
*/
int getIndex(char* argument[]){
	char *p = argument[0];
	int val;
	while(*p){
		if(isdigit(*p)){
			val = strtol(p, &p, 10);
		}else{
			p++;
		}
	}
	
	return val;
}

/*
This functions takes the command entered by the user and
separates the command from the destination file, into two 
separate variables
*/
void setOutRedirection(char *args[]){
	int i = 0;
	//Clear command to redirect array before setting new commands
	memset(command_redirect, '\0', 20);
	
	while(strcmp(args[i],">") != 0){
		command_redirect[i] = args[i];
		i++;
	}
	strcpy(filename,args[i+1]);
}

/*
Strips the pipe command entered by the user into two commands for
proper handling 
*/
void setPiping(char *args[], int *args_size){
	int i =0, j = 0;
	//Clear rhs and lhs commands variables
	memset(rhs_command, '\0', 20);
	memset(lhs_command, '\0', 20);
	
	while(strcmp(args[i],"|") != 0){
		lhs_command[i] = args[i];
		i++;
	}

	//To point to rhs command
	i++;

	while(i < *args_size){
		rhs_command[j] = args[i];
		i++;
		j++;
	}
}

/*
This functions executethe pipe command when the boolean variable
isPiping is set to true
*/
int executePipeCommand()
{
	int status_lhs, status_rhs;
	int fd[2];
	pid_t w_lhs,w_rhs;
	pid_t pid_lhs, pid_rhs;

	if(pipe(fd) == -1){
		perror("pipe"); exit(EXIT_FAILURE);
	}
	
	pid_lhs = fork();

	if ( pid_lhs < 0) { 
		perror("fork pid_lhs"); exit(EXIT_FAILURE);
	}

	if(pid_lhs == 0){
		close(1);
		if(dup(fd[1]) == -1){
			perror("dup(fd[1])"); exit(EXIT_FAILURE);
		}
		close(fd[0]);
		close(fd[1]);		
		if(execvp(lhs_command[0],lhs_command) < 0){
			perror("exec pid_lhs"); exit(EXIT_FAILURE);
		
		exit(-1);

		}
	}else{
		pid_rhs = fork();

		if (pid_rhs < 0) { 
		perror("fork pid_lrhs"); exit(EXIT_FAILURE);
		}
		if(pid_rhs == 0){
			close(0);
			if(dup(fd[0]) == -1){
				perror("dup(fd[0])"); exit(EXIT_FAILURE);
			}
			close(fd[0]);
			close(fd[1]);
			if(execvp(rhs_command[0],rhs_command) < 0){
				perror("exec pid_rhs"); exit(EXIT_FAILURE);
			}
			
			exit(-1);

		}else{
			close(fd[0]);
			close(fd[1]);

			do{
				w_lhs = waitpid(pid_lhs, &status_lhs, WUNTRACED);
				w_rhs = waitpid(pid_rhs, &status_rhs, WUNTRACED);

			}while(!WIFEXITED(status_lhs) && !WIFEXITED(status_rhs) &&
			 	   !WIFSIGNALED(status_lhs) && !WIFSIGNALED(status_rhs));
		}

	}
	return SUCCESS;		
}
	

int executeCommand(char *args[], int *background, char** argument)
{
	char *arg_jobs = (char *)malloc(80 * sizeof(char));
	int child_id;
	int status;
	int f; //To open the file for output redirection
	pid_t w;

	//In order to add the bg command to the job list
	strcpy(arg_jobs, *argument);	
			
/* Built-in commands section
*************************************************************************** 
*/
	//Exits shell
	if(strcmp(*argument,"exit") == 0){
		exit(1);
	}
	
	//Print the last 10 arguments entered by the user
	if(strcmp(*argument,"history") == 0){
		printHistory();
		return SUCCESS;
	}
	
	//Sets active directory to the one entered by the user
	if(strcmp(args[0],"cd") == 0){
        chdir(args[1]);
		return SUCCESS;
	}
	
	//Returns the active directory
	if(strcmp(args[0],"pwd") == 0){
		char s[100];
		printf("%s\n",getcwd(s,100));
		return SUCCESS;
	}

	//Returns a list of bg commands with their corresponding id
	if(strcmp(*argument,"jobs") == 0){
		printJobs();
		return SUCCESS;
	}

	//Brings process to the foreground (makes parent wait for
	//selected child to finish)
	if(strcmp(args[0],"fg") == 0){
		
		do{
			//Checks if process id is in the job list
			if(getItem(ref_jobList_front, atoi(args[1])) != NULL){
				//Wait for process to finish
				puts("Waiting for process to finish");
				w = waitpid(atoi(args[1]), &status, WUNTRACED);
				if (w == -1){
					perror("waitpid"); 
					return EXIT_PROCESS;
				}
			}else{
				printf("Child process: %d, not found\n",atoi(args[1]) );
				return EXIT_PROCESS;
			}
		}while(!WIFEXITED(status) && !WIFSIGNALED(status)); 
		
		if(w == atoi(args[1])){
			printf("Child process: %d has exited\n",atoi(args[1]) );
		}
		return SUCCESS;
	}	

/* Built-in commands section
*************************************************************************** 
*/

	pid_t pid = fork();
	
	if (pid == -1) { 
		perror("fork"); exit(EXIT_FAILURE);
	}	
	if(pid == 0){			
		if(*background == 1){	
			if(is_out_redirection){
				//Reset global variable
				is_out_redirection = false;

				close(STDOUT_FILENO);
				if ((f = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
   					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1){
   					
					perror("Cannot open output file\n"); exit(1);
				}else{
					
					if(execvp(command_redirect[0],command_redirect) < 0)
						printf("Command \"%s\" could not be executed\n", *argument); 
						exit(-1);
					close(f);
					exit(-1);
				}			
			}						
			if(execvp(args[0],args) < 0)
				printf("Command \"%s\" could not be executed\n", *argument); 
				exit(-1);
			
			exit(1);

		}else{
			if(is_out_redirection){
				//Reset global variable
				is_out_redirection = false;

				close(STDOUT_FILENO);
				if ((f = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
   					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1){
   					
					perror("Cannot open output file: \n"); exit(-1);
				}else{
			
					if(execvp(command_redirect[0],command_redirect) < 0)
						printf("Command \"%s\" could not be executed\n", *argument);
						exit(-1);	
					close(f);
					exit(1);
				}
			}else{
				if(execvp(args[0],args) < 0)
					printf("Command \"%s\" could not be executed\n", *argument);
					exit(-1);
			}
			exit(1);
		}
		
	}else{
		if(*background == 1){
			printf("background proccess selected\n");
			child_id = pid;
			//Add command to job list with its corresponding id
			enqueue(arg_jobs, &child_id , ref_jobList_front, ref_jobList_rear);

			return SUCCESS;
			
		}else{
			 do {
				w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
				if (w == -1){
					perror("waitpid"); exit(EXIT_FAILURE);
				}
		
				if (WIFEXITED(status)) {
					printf("exited, status=%d\n", WEXITSTATUS(status));
				} else if (WIFSIGNALED(status)) {
					printf("killed by signal %d\n", WTERMSIG(status));
				} else if (WIFSTOPPED(status)) {
					printf("stopped by signal %d\n", WSTOPSIG(status));
				} else if (WIFCONTINUED(status)) {
					printf("continued\n");
				}
			} while (!WIFEXITED(status)&& !WIFSIGNALED(status));
		}
			
	printf("Parent process finished\n");        					
	}
	return SUCCESS;
}

int main(void)
{
	char *args[20];
	int bg;
	int exe;
	char hostname[80];
	gethostname(hostname, sizeof(hostname));
	int args_count = 0;
	char *argument = (char *)malloc(80 * sizeof(char));
	//No need to free this variable since it is always used 
	//and the exit() system call is used to terminate the program
	while(1) {
		bg = 0;
		args_count++;
		int cnt = getcmd(hostname, args, &bg, &args_count, &argument);
		
		//When no command was entered (eg. blank space)
		if(cnt == -1)
		{
			continue;
		}
		
		if(isPiping){
		//Reset global variable
			isPiping = false;
			exe = executePipeCommand();

		}else{
			exe = executeCommand(args, &bg, &argument);
		}

		//When user asked to fg a process that has already exited
		if(exe == -1)
		{
			continue;
		}
		
	}

	return EXIT_SUCCESS;
}
