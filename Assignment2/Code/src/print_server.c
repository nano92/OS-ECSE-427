#include "jobs_queue.h"

int fd;
struct Jobs *jobs_controller;

void setup_shared_memory(){
    fd = shm_open(SHD_REG, O_RDWR, S_IRWXU);
    if(fd == -1){
        printf("shm_open() failed\n");
        exit(1);
    }
    if (ftruncate(fd, sizeof(struct Jobs)) == -1)
        printf("ftruncate() failed\n");
        exit(1);
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
    jobs_controller->ref_front = &jobs_controller->front;
    jobs_controller->ref_rear = &jobs_controller->rear;

    jobs_controller->length = MAX_LENGTH;
    jobs_controller->mutex = init_semaphore(1);
    jobs_controller->items = init_semaphore(0);
    jobs_controller->spaces = init_semaphore(MAX_LENGTH-1);
}

int main()
{
    
}