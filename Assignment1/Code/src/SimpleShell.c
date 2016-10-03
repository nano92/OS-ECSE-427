//  Arrese Sebastien, 260587463
//  SimpleShell.c
//  Assignment 1 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>
#define MAX_HISTORY 10

//implement history

// //adds into history and loop around to delete if too big
// int addToHistoryList(char *line){
//     int i, j;
//     //Looping to find free spot to add command
//     for(i=0;i<MAX_HISTORY;i++){
//       if(historyList[i] == NULL){
//       	//Making space
//         historyList[i] = malloc(sizeof(char) * sizeof(line));
//         //Gets set in available space
//         sprintf(historyList[i],"%d%s%s", historyCount,"-",line);
//         //Total count incremented
//         historyCount++;
//         return 1;
//       }
//     }
//     //If no free spot found, move full array by 1, so last spot is available
//     for(j=0;j<9;j++){
//         free(historyList[j]);
//         historyList[j] = NULL;
//         historyList[j] = malloc(sizeof(char) * sizeof(historyList[j+1]));
//         strcpy(historyList[j], historyList[j+1]);        
//     }
//     //Free contents of last command spot
//     free(historyList[9]);
//     historyList[9] = NULL;
//     //Allocate space
//     historyList[9] = malloc(sizeof(char) * sizeof(line));
//      //Gets set in available space
//     sprintf(historyList[9], "%d%s%s", historyCount, "-", line);
//     //Total count incremented
//     historyCount++;
//     return 1;
// }



// //search history to see if number inputed with line is present in history
// char * searchHistory(int index){ 
//       int difference,indextosearch;
//       char * line;
     
//       difference = historyCount - index;
//       if (difference > 10){
//       	fprintf(stderr, "%s\n", "number not in history");
//       	return NULL;
//       }
//       else{
//       	indextosearch = MAX_HISTORY - difference;
//       	line = historyList[indextosearch];
//   		return line;
//       }
// }



int shell_exit()
{
  printf("%s\n", "Exit");
  return 0;
}


int getcmd(char *prompt, char *args[], int *background)
{
    int length, i = 0, j;
    char *token, *loc;
    size_t linecap = 0;
    char *line;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);
    if (length <= 0) {
        exit(-1);
    }

 
   //add_History(line);
    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else
        *background = 0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)  
            args[i++] = token;
    }
    //check if redirect symbol
    args[i]=NULL;
    return i;
}
   
  
   

    // // If only contains 1 token
    // if (tokencount == 1){
    // 	token = strsep(&line, " \t\n");
    // 	// Checks if first char is !
    // 	if (token[0] == '!'){
    // 		// loop through rest of the string and checks if all digits
    // 		for (int f = 1; f <strlen(token); f++){
    // 			if (isdigit(token[f]) == 0){
    // 				invalid = 1;
    // 				break;
    // 			}
    // 		}
    // 		// all chars after ! are digits
    // 		if (invalid != 1){
    // 			int number = atoi(token);
    // 			// line gets overwritten by previous command
    // 			line = searchHistory(number);
    // 		}

    // 	}

    

    // }

    //addToHistoryList(line);


//For for Child/ParentProcess
int createFork(char **args, int *bg){
    int pid, wpid, i, redirectSymbol, redirect;
    int status;
    pid = fork();
    // Child Process
    if (pid == 0){
        if (execvp(args[0], args) == -1) {
            perror("fork");
        } 
        exit(EXIT_FAILURE);
    //Error forking
    }else if(pid < 0){
        fprintf(stderr, "Error: failure to fork.");
        shell_exit();
    //Parent process
    }else{
        if(*bg){       
        }
        else{
            //Wait for child to exit
            do {

              waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }  
    return 1;
}




//execute non-built else go to process
int exec(char *args[],int *bg)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  return createFork(args,bg);
}


int main (void){
	char *args[20];
	int bg, loop;


	do{
        getcmd("\nSimpleShell>> ", args, &bg);
		loop = exec(args, &bg);
		printf("\n");
		}while(loop);
		return 1;
}