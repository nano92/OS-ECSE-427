#include "jobs_queue.h"

int fd;
struct Jobs *jobs_controller;


void setup_shared_memory(){
    fd = shm_open(SHD_REG, O_CREAT | O_RDWR, S_IRWXU);
    if(fd == -1){
        printf("shm_open() failed\n");
        exit(1);
    }
    ftruncate(fd, sizeof(struct Jobs));
        //printf("ftruncate() failed\n");
        //exit(1);
}

void attach_shared_memory(){
    jobs_controller = (struct Jobs*)  mmap(NULL, sizeof(struct Jobs), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    //jobs_controller->job_queue = (struct Job_info*) mmap(NULL, (MAX_LENGTH * sizeof(struct Job_info)), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    //jobs_controller->rear = (struct Node*) mmap(NULL, sizeof(struct Node), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(jobs_controller == MAP_FAILED){
        printf("mmap() failed\n");
        exit(1);
    }

}

void init_shared_memory() {
    jobs_controller->first_job = 1;
    jobs_controller->repeated_ID = -1;
    jobs_controller->queue_length = MAX_LENGTH;
    jobs_controller->job_in = jobs_controller->job_out = 0;
    jobs_controller->shutdown_server = 0;
    sem_init(&(jobs_controller->mutex), 1, 1);    
    sem_init(&(jobs_controller->items), 1, 0);
    sem_init(&(jobs_controller->spaces), 1, MAX_LENGTH-1);
    
}

void destroy_shared_memory(){
    int r = shm_unlink(SHD_REG);
    if (r != 0)
        printf("shm_unlink() failed\n");
        exit(1);
}


int main()
{
    //int job_duration;
    setup_shared_memory();
    puts("Server: memory set up");
    attach_shared_memory();
    puts("Server: memory shared");
    init_shared_memory();
    puts("Server: memory init");
    while(1){
        if(jobs_controller->shutdown_server){
            puts("Server: shuting down...");
            destroy_shared_memory(); 
        }else{
            struct Job_info *job = (struct Job_info *)malloc(sizeof(struct Job_info));
            //printf("server main: %ld\n",jobs_controller);

            if (take_job(jobs_controller, &job)){
                
                printf("Server: printing %d pages from client %d\n",job->pages , job->ID);
                sleep(job->pages + 10);
        
                puts("...\n");

                puts("Server: finished printing\n");

                free(job);
            }else{
                free(job);
            }   

        }
    }

    return 0;
}