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
   //jobs_controller->front = (struct Node*) mmap(NULL, sizeof(struct Node), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    //jobs_controller->rear = (struct Node*) mmap(NULL, sizeof(struct Node), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(jobs_controller == MAP_FAILED){
        printf("mmap() failed\n");
        exit(1);
    }

}

void init_shared_memory() {
    jobs_controller->front = NULL;
    jobs_controller->rear = NULL;
    jobs_controller->ref_front = &jobs_controller->front;
    jobs_controller->ref_rear = &jobs_controller->rear;

    jobs_controller->length = MAX_LENGTH;
    sem_init(&(jobs_controller->mutex), 1, 1);    
    sem_init(&(jobs_controller->items), 1, 0);
    sem_init(&(jobs_controller->spaces), 1, MAX_LENGTH-1);
    
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
        struct Node *job = (struct Node *)malloc(sizeof(struct Node));
        //printf("server main: %ld\n",jobs_controller);

        take_job(jobs_controller, &job);

        printf("printing %d pages from client %d\n",job->pages, job->source);
        sleep(job->pages);
        
        puts("\n");

        puts("Finished printing\n");

        free(job);
    }

    return 0;
}