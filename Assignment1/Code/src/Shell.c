#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define EXIT_PROCESS -1
#define SUCCESS 1
// This code is given for illustration purposes. You need not include or follow this
// strictly. Feel free to writer better or bug free code. This example code block does not
// worry about deallocating memory. You need to ensure memory is allocated and deallocated
// properly so that your shell works without leaking memory.
//
struct process_info
{
	long id;
	char *str;
};

int getcmd(char *prompt, char *args[], int *background, char *argument)
{
	int length, i = 0;
	char *token, *loc;
	char *line = NULL;
	size_t linecap = 0;
	printf("\n%s>>", prompt);
	
	length = getline(&line, &linecap, stdin);
	
	if (length <= 0) {
		exit(-1);
	}
	// Check if background is specified..
	if ((loc = index(line, '&')) != NULL) {
		*background = 1;
		*loc = ' ';
	} else
	*background = 0;


	
	while ((token = strsep(&line, " \t\n")) != NULL) 
	{
		strcpy(argument,token);
		if(argument[0] != '\0')
			strcat(argument, " ");
		
		for (int j = 0; j < strlen(token); j++){
			if (token[j] <= 32){
				token[j] = '\0';
			}
			if (strlen(token) > 0){
				args[i++] = token;
				strcat(argument, token);
			}
		}	
				//printf("token: %s\n",token);
	}
	args[i] = NULL;
	return i;
}

int execute_command(char *args[], int *background, char *argument)
{
	struct process_info *info_sent = (struct process_info*)malloc(sizeof(struct process_info)); 
	//struct process_info *info_received = (*struct process_info)malloc(sizeof(struct process_info)); 
	//char argument2[80];
	int arg_length;
	printf("argument: %s\n",args[0]);
	
 	if(strcmp(args[0],"exit") == 0)
	{
		return EXIT_PROCESS;
		//exit(EXIT_FAILURE);(sizeof(args)/sizeof(args[0]))
	}
	
	int status;
	
	int fd[2], nbytes;
	pipe(fd);
	
	pid_t w;
	pid_t pid = fork();
	
	if (pid == -1) { perror("fork"); exit(EXIT_FAILURE); }	
	if(pid == 0)
	{	
		
		info_sent->id  = (long)getpid();
		info_sent->str = "Child process finished\0\n";
		
		if(*background == 1)
		{
			if(execvp(args[0],args) < 0)
				printf("Command %s could not be executed\n", argument);
				exit(EXIT_FAILURE);
			
		}else
		{
			close(fd[0]);
			write(fd[1], info_sent, sizeof(struct process_info));
			
			if(execvp(args[0],args) < 0)
				printf("Command %s could not be executed\n", argument);
				exit(EXIT_FAILURE);
			
			close(fd[1]);
		}
		
		exit(EXIT_SUCCESS);
		
	}else 
	{
		if(*background == 1)
		{
			printf("background proccess selected");
			return SUCCESS;
			
		}else{
			 do {
				w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
				if (w == -1) { perror("waitpid"); exit(EXIT_FAILURE); }
		
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
	char argument[80];
	char hostname[80];
	gethostname(hostname, sizeof(hostname));
	
	while(1) {
		bg = 0;
		int cnt = getcmd(hostname, args, &bg, argument);
		printf("bg: %d\n", bg);
		exe = execute_command(args, &bg, argument);
		
		if(exe == EXIT_PROCESS)
		{
			exit(EXIT_FAILURE);
		}
	}
	
	return 0;
}
