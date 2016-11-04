/*
Printer Spooler
Client implementation

Author: Luis Gallet Zambrano 260583750
Last date modified: 04/11/2016
*/

#include <time.h>
#include "jobs_queue.h"

int fd;
struct Jobs *jobs_controller;

/*
Function to gain access to the shared memory for reading and writing
Size of the shared memory must be able to contain 10 jobs plus other variables
*/
void setup_shared_memory(){
    fd = shm_open(SHD_REG, O_RDWR, S_IRWXU);
    if(fd == -1){
        printf("shm_open() failed\n");
        exit(0);
    }
}

/*
Get a pinter to the shared memory space in order to access it
*/
void attach_shared_memory(){
    jobs_controller = 
    (struct Jobs*)  mmap(NULL, 
                    sizeof(struct Jobs) + MAX_LENGTH*sizeof(struct Job_info), 
                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(jobs_controller == MAP_FAILED){
        printf("mmap() failed\n");
        exit(0);
    }

}

/*
Assign pages to a "randomly" chosen number between 1 and 10
*/
void set_pages(int *pages){        
    *pages = 1 + (rand() % 10); 
}

/*
Detached from shared memory so printer can access it
*/
void detach_shared_memory(){
    if(munmap(jobs_controller, 
        sizeof(struct Jobs) + MAX_LENGTH*sizeof(struct Job_info)) == -1){
        
        puts("munmap failed with error:");
        exit(0);
    }
    close(fd);
}

/*
This function set the semaphores to the value they were going to have
before the interrupt signal was raised
*/
void handler(int signo){
    int temp;
    
    sem_getvalue(&jobs_controller->mutex, &temp);
    if(temp != 1)
        sem_post(&jobs_controller->mutex);
    
    sem_getvalue(&jobs_controller->items, &temp);
    if(temp != 1)
        sem_post(&jobs_controller->items);
    
    sem_getvalue(&jobs_controller->spaces, &temp);
    if(temp != 1)
        sem_post(&jobs_controller->spaces);
    exit(0);
}

int main(int argc, char *argv[]){
    srand(time(NULL)); 
    int pages, client_id = 0;
    
    if(signal(SIGINT, handler) == SIG_ERR)
        printf("Signal Handler Failure ..\n");

    setup_shared_memory();
    attach_shared_memory();

    if(argc == 2){
        if(strcmp((argv[1]), "shutdown") == 0){
            //Raise shutdown flag
            jobs_controller->shutdown_server = true;
            
            //Printer will be waiting from an incoming job at take_job(), 
            //thus it needs to exit the method
            sem_post(&jobs_controller->items);

            detach_shared_memory();
            exit(0);
        }
        
        //Client ID should be greater than 0
        if(atoi(argv[1]) > 0){
            client_id = atoi(argv[1]);
        }else{
            puts("ID not valid!!");
            exit(0);
        }        
        
        set_pages(&pages);

        put_job(jobs_controller, client_id, pages);
        detach_shared_memory();
        exit(1);  
        
    }
    else if(argc > 2){
        printf("Too many arguments entered.\n");
        exit(0);
    }
    else{
        printf("At least one argument expected.\n");
        exit(0);
    }
}