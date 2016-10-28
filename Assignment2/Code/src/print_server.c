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
    if(jobs_controller == MAP_FAILED){
        printf("mmap() failed\n");
        exit(1);
    }

}

void init_shared_memory() {

    jobs_controller->front = NULL;
    jobs_controller->rear = NULL;

    jobs_controller->length = MAX_LENGTH;
    sem_init(&(jobs_controller->mutex), 1, 1);    
    sem_init(&(jobs_controller->items), 1, 0);
    sem_init(&(jobs_controller->spaces), 1, MAX_LENGTH-1);
    
}

int main()
{
    int job_duration;
    setup_shared_memory();
    puts("memory set up");
    attach_shared_memory();
    puts("memory shared");
    init_shared_memory();
    puts("memory init");
    while(1){
        job_duration = take_job(&jobs_controller);

        printf("printing %d pages from client\n",job_duration);
        sleep(job_duration);
        puts("Finished printing\n");
    }

    return 0;
}