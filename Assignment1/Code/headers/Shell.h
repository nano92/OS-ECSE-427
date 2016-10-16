/*
Shell.c header file

Author: Luis Gallet Zambrano 260583750
Last date modified: 10/10/2016
*/

#ifndef SHELL_H_   /* Include guard */
#define SHELL_H_

#define EXIT_PROCESS -1
#define SUCCESS 1


struct process_info;

typedef enum { false , true } bool_t;

int getcmd(char *prompt, char *args[], int *background,int *args_count, char** argument);
int getIndex(char* argument[]);
void addHistory(char *args[], int *sizeOfArgs, int *counter, char** argument, int *background);
void setArgumentString(char *args[], int *sizeOfArgs, char** argument);
void setOutRedirection(char *args[]);
void printHistory();
void printJobs();
void setPiping(char *args[], int *args_size);
void updateJobs(int size);
int executePipeCommand();
int executeCommand(char *args[], int *background, char** argument);

#endif // SHELL_H_