#include <time.h>
#include "jobs_queue.h"

int fd;
struct Jobs *jobs_controller;

void setup_shared_memory(){
    fd = shm_open(SHD_REG, O_RDWR, S_IRWXU);
    if(fd == -1){
        printf("shm_open() failed\n");
        exit(1);
    }
}

void attach_shared_memory(){
    jobs_controller = (struct Jobs*)  mmap(NULL, sizeof(struct Jobs), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    //jobs_controller->job_queue = (struct Job_info*) mmap(NULL, (MAX_LENGTH * sizeof(struct Job_info)), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(jobs_controller == MAP_FAILED){
        printf("mmap() failed\n");
        exit(1);
    }

}

void set_pages(int *pages){
    srand(time(NULL));
    *pages = 1 + (rand() % 10);
}

void detach_shared_memory(){
    if(munmap(jobs_controller, (sizeof(struct Jobs))) == -1){
       //munmap(jobs_controller->rear, (sizeof(struct Node))) ==-1){
        puts("munmap failed with error:");
        exit(1);
    }
    
    close(fd);
    
}

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
    int pages, client_id = 0;
    
    if(signal(SIGINT, handler) == SIG_ERR)
        printf("Signal Handler Failure ..\n");

        setup_shared_memory();
        puts("Client: memory set up");
        attach_shared_memory();
        puts("Client: memory shared");

    if(argc == 2){
        if(strcmp((argv[1]), "shutdown") == 0){
            jobs_controller->shutdown_server = 1;
            sem_post(&jobs_controller->items);
            detach_shared_memory();
            exit(0);
        }
        
        if(atoi(argv[1]) > 0){
            client_id = atoi(argv[1]);
        }else{
            puts("ID must be greater than zero!!");
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