#include "jobs_queue.h"

int fd;
struct Jobs *jobs_controller;

void setup_shared_memory(){
    fd = shm_open(SHD_REG, O_CREAT | O_RDWR, S_IRWXU);
    if(fd == -1){
        printf("shm_open() failed\n");
        exit(0);
    }
    ftruncate(fd, sizeof(struct Jobs) + MAX_LENGTH*sizeof(struct Job_info));
}

void attach_shared_memory(){
    jobs_controller = (struct Jobs*)  mmap(NULL, sizeof(struct Jobs) + MAX_LENGTH*sizeof(struct Job_info), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(jobs_controller == MAP_FAILED){
        printf("mmap() failed\n");
        exit(0);
    }
}

void init_job_queue(){
    for(int i = 0; i < MAX_LENGTH; i++){
        jobs_controller->job_queue[i].ID = 0;
        jobs_controller->job_queue[i].pages = 0;
    }
}

void init_shared_memory() {
    init_job_queue();
    jobs_controller->repeated_ID = -1;
    jobs_controller->queue_length = MAX_LENGTH;
    jobs_controller->job_in = jobs_controller->job_out = 0;
    jobs_controller->new_job = true;
    jobs_controller->new_printer = false;
    jobs_controller->printers_counter = 1;
    sem_init(&(jobs_controller->mutex), 1, 1);    
    sem_init(&(jobs_controller->items), 1, 0);
    sem_init(&(jobs_controller->spaces), 1, MAX_LENGTH-1);
}

void destroy_shared_memory(){
    int r = shm_unlink(SHD_REG);
    if (r != 0)
        printf("shm_unlink() failed\n");
        exit(0);
}


int main()
{
    setup_shared_memory();
    puts("Server: memory set up");
    attach_shared_memory();
    puts("Server: memory shared");
    
    if(!jobs_controller->new_printer){
        init_shared_memory();
        puts("Server: memory init");
    }else{
        jobs_controller->printers_counter++;
    }

    int printerID = jobs_controller->printers_counter;

    while(1){
                
        jobs_controller->new_printer = true;
        if(jobs_controller->shutdown_server){
            puts("Server: shutting down...");
            destroy_shared_memory(); 
        }else{
            struct Job_info *job = (struct Job_info *)malloc(sizeof(struct Job_info));

            if (take_job(jobs_controller, &job, &printerID)){
                
                printf("Printer %d: printing %d pages from client %d\n", printerID, job->pages ,job->ID);
                sleep(job->pages);
        
                puts("...\n");

                printf("Printer %d: finished printing\n",printerID);
            }
                
            free(job);
        }
    }

    return 0;
}