#ifndef SHELL_H_   /* Include guard */
#define SHELL_H_

#define EXIT_PROCESS -1
#define SUCCESS 1

struct process_info;

int getcmd(char *prompt, char *args[], int *background,int *args_count, char** argument);
void setArgumentString(char *args[], int *sizeOfArgs, char** argument);
void addHistory(char *args[], int *sizeOfArgs, int *counter, char** argument, int *background);
void printHistory();
int getIndex(char* argument[]);
int executeCommand(char *args[], int *background, char** argument);


#endif // SHELL_H_