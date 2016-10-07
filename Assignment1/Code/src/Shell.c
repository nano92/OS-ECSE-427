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
of the Queue (Linked List)
*/
struct Node* list1_front = NULL;
struct Node** ref_l1f = &list1_front;

struct Node* list1_rear = NULL;
struct Node** ref_l1r = &list1_rear;

/*
Globl variables to handle the output redirection

is_out_redirection: check if command entered asks for an output redirection
command_redirect: command user wishes to redirect
filename: file destination of the entered command
*/
unsigned char is_out_redirection;
char *command_redirect[20];
char filename[20];

/*
Globl variables to handle the piping command

isPiping: check if command entered asks for piping
lhs_command: command sender
rhs_command: command receiver
*/
unsigned char isPiping;
char *lhs_command[20];
char *rhs_command[20];

/*
Struct used to store child process information
*/
struct process_info
{
	long id;
	char *str;
};

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
		is_out_redirection = 1;
	} else{
		is_out_redirection = 0;
	}

	// Check if piping is specified..
	if ((loc = index(line, '|')) != NULL) {
		isPiping = 1;
	} else{
		isPiping = 0;
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
		
		strcpy(*argument, getItem(ref_l1f, arg_index));
		
		//Handles the case when the last argument entered was asked to be run in bg, 
		//but not the new one
		if(*background == 1){
			strcat(*argument, " &");
		}
		
		//Make one temporary copy of the selected argument, because the char* will be
		//stripped while constructing char* args[] with instruction from list
		strcpy(temp_arg, getItem(ref_l1f, arg_index));
		
		i = 0;
		
		if ((loc = index(temp_arg, '&')) != NULL) {
			*loc = ' ';
			*background = 1;
		}
		
		if ((loc = index(temp_arg, '>')) != NULL) {
			is_out_redirection = 1;
		} else{
			is_out_redirection = 0;
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
		setPiping(args);

	}
	
	return i;
}

/*
This function creates a string of the entered argument 
*/
void setArgumentString(char *args[], int *sizeOfArgs, char** argument){
	
	int arg_length = *sizeOfArgs;
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
void addHistory(char *args[], int *sizeOfArgs, int *counter, char** argument, int *background){
	
	setArgumentString(args, sizeOfArgs, argument);
	
	char *arg_to_list = (char *)malloc(80 * sizeof(char));
	int list_size = size(ref_l1f);
	
	if(strcmp(*argument,"history") != 0 && strcmp(*argument,"exit") != 0){
		strcpy(arg_to_list, *argument);
		
		if(*background == 1){
			strcat(arg_to_list, " &");
		}
	
		//Check size of history list before adding a new item. Remove oldest item in the queue
		//when there are more than 10 items in the list
		if(list_size == 10){
		dequeue(ref_l1f, ref_l1r);
		enqueue(arg_to_list, counter, ref_l1f, ref_l1r);
		}else{
			enqueue(arg_to_list, counter, ref_l1f, ref_l1r);
		}
		/*char* arg_to_list cannot be free, 
		otherwise the the linked list will get wrong values*/		
	
	//Decrease argument counter by one when command history is chosen, since it is not registered
	//in the history list
	}else if(strcmp(*argument,"history") == 0){
		*counter = *counter - 1;
	}
}

void printHistory(){
	printList(ref_l1f);
}

/*
When a command from the history list is demanded, 
this function returns the index of that command as an integer
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

void setPiping(char *args[]){
	int i =0, j = 0;
	//Clear rhs and lhs commands
	memset(rhs_command, '\0', 20);
	memset(lhs_command, '\0', 20);
	
	while(strcmp(args[i],"|") != 0){
		lhs_command[i] = args[i];
		i++;
		break;
	}

	//To point to rhs command
	i++;

	while(i < strlen(args)){
		rhs_command[j] = args[i];
		i++;
		j++;
	}
}

int executePipeCommand(){
	int status;
	
	int fd[2], nbytes;
	pipe(fd);
	
	pid_t w;
	pid_t pid = fork();
}

int executeCommand(char *args[], int *background, char** argument)
{
	struct process_info *info_sent = (struct process_info*)malloc(sizeof(struct process_info)); 
		
	if(strcmp(*argument,"exit") == 0){
		exit(1);
	}

	if(isPiping){
		executePipeCommand();
	}
		
	/*
	Built-in commands section
	*/
	
	//Print the last 10 arguments entered by the user
	if(strcmp(*argument,"history") == 0){
		printHistory();
		free(info_sent);
		return SUCCESS;
	}
	
	//Sets active directory to the one entered by the user
	if(strcmp(args[0],"cd") == 0){
        chdir(args[1]);
		free(info_sent);
		return SUCCESS;
	}
	
	if(strcmp(args[0],"pwd") == 0){
		char s[100];
		printf("%s\n",getcwd(s,100));
		free(info_sent);
		return SUCCESS;
	}
	
	/*
	Built-in commands section
	*/
	
	int status;
	int f;
	
	int fd[2], nbytes;
	pipe(fd);
	
	pid_t w;
	pid_t pid = fork();
	
	if (pid == -1) { 
		perror("fork"); exit(EXIT_FAILURE);
	}	
	if(pid == 0){	
		
		info_sent->id  = (long)getpid();
		info_sent->str = "Child process finished\0\n";
		
		if(*background == 1){
			if(is_out_redirection){
				close(STDOUT_FILENO);
				if ((f = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
   					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1){
   					
					perror("Cannot open output file\n"); exit(1);
				}else{
					
					if(execvp(command_redirect[0],command_redirect) < 0)
						printf("Command \"%s\" could not be executed\n", *argument); 
						exit(-1);
					close(f);
				}			
			}
			if(execvp(args[0],args) < 0)
				printf("Command \"%s\" could not be executed\n", *argument); 
				exit(-1);
			
			exit(1);
		}else{
			//close(fd[0]);
			//write(fd[1], info_sent, sizeof(struct process_info));
			if(is_out_redirection){
				close(STDOUT_FILENO);
				if ((f = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
   					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1){
   					
					perror("Cannot open output file: \n"); exit(-1);
				}else{
			
					if(execvp(command_redirect[0],command_redirect) < 0)
						printf("Command \"%s\" could not be executed\n", *argument);
						exit(-1);	
					close(f);
				}
			}else{
				if(execvp(args[0],args) < 0)
					printf("Command \"%s\" could not be executed\n", *argument);
					exit(-1);
			}
			
		}
		
		exit(1);
		//Code never gets to here...why???
		
	}else{
		if(*background == 1){
			printf("background proccess selected\n");
			w = waitpid(-1, &status, WNOHANG);
			printf("child status: %d\n", w);
		
			free(info_sent);
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
			
			close(fd[1]);
			
			nbytes = read(fd[0], info_sent, sizeof(struct process_info));
			printf("From child process %d: %s\n",info_sent->id, info_sent->str);
			
			close(fd[0]);
		}
			
	printf("Parent process finished\n");        					
	}
	
	free(info_sent);
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
		
		exe = executeCommand(args, &bg, &argument);
		
	}
	return EXIT_SUCCESS;
}
