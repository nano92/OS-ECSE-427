/*
Printer Spooler
Server implementation

Author: Luis Gallet Zambrano 260583750
Last date modified: 04/11/2016
*/

#include "jobs_queue.h"

int fd;
struct Jobs *jobs_controller;

/*
Function to create the shared memory and make it accessible 
reading and writing
Size of the shared memory must be able to contain 10 jobs plus other variables
*/
void setup_shared_memory(){
    fd = shm_open(SHD_REG, O_CREAT | O_RDWR, S_IRWXU);
    if(fd == -1){
        printf("shm_open() failed\n");
        exit(0);
    }
    ftruncate(fd, sizeof(struct Jobs) + MAX_LENGTH*sizeof(struct Job_info));
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
Initialize job array elements to default values (e.g. 0)
*/
void init_job_queue(){
    for(int i = 0; i < MAX_LENGTH; i++){
        jobs_controller->job_queue[i].ID = 0;
        jobs_controller->job_queue[i].pages = 0;
    }
}

/*
Initialize all the variables shared between processes
*/
void init_shared_memory(){
    init_job_queue();
    //To check if a job in the queue has the same ID as a n incoming job
    jobs_controller->repeated_ID = -1; 
    jobs_controller->queue_length = MAX_LENGTH;
    //Variables that are going to dictate the index of the incoming job and the
    //that will be out next
    jobs_controller->job_in = jobs_controller->job_out = 0;
    //To check if a new job is waiting to enter the queue
    jobs_controller->new_job = true;
    //To check if a new printer has been added
    jobs_controller->new_printer = false;
    //To keep track of the number of printers available
    jobs_controller->printers_counter = 1;
    //Flag raised when user wants to shutdown all printers
    jobs_controller->shutdown_server = false;

    //Semaphore mutex: for gaining privilage access to critical section
    sem_init(&(jobs_controller->mutex), 1, 1);
    //Semaphore items: for checking that the queque is not empty    
    sem_init(&(jobs_controller->items), 1, 0);
    //Semaphore  spaces: for checking that there are empty spaces in the queue
    sem_init(&(jobs_controller->spaces), 1, MAX_LENGTH-1);
}

/*
Function to unlink the shared memory, used when user wants 
to shutdown all printers
*/
void destroy_shared_memory(){
    int r = shm_unlink(SHD_REG);
    if (r != 0)
        printf("shm_unlink() failed\n");
        exit(0);
}


int main()
{
    setup_shared_memory();
    attach_shared_memory();
    
    //Only initialize the shared memory variables for the first printer
    if(!jobs_controller->new_printer){
        init_shared_memory();
    }else{
        //Increment printer counter since the shared memory was not initialize,
        //meaning there is a new printer
        jobs_controller->printers_counter++;
    }

    //In order to keep track of which job each printer takes
    int printerID = jobs_controller->printers_counter;
    //Variable used to get the items semaphore value
    int temp;

    printf("Printer %d activated...\n", printerID);

    while(1){
        //Variable is false by default but set to true by the first printer
        jobs_controller->new_printer = true;
        //Check if user wants to shutdown printers
        if(jobs_controller->shutdown_server){
            puts("Server: shutting down...");
            destroy_shared_memory(); 
        }else{
            struct Job_info *job = 
                (struct Job_info *)malloc(sizeof(struct Job_info));

            //Check if printer is waiting for an incoming job and the new job
            //flag is false. If condition is met, then it means that there are
            //no jobs in the queue
            sem_getvalue(&jobs_controller->items, &temp);
            if(!jobs_controller->new_job && temp == 0){
                printf("No more clients for printer %d ...sleeping\n", 
                    printerID);
            }

            //If take_job returns false it means that the user has asked 
            //to shutdown the server    
            if (take_job(jobs_controller, &job, &printerID)){
                
                printf("Printer %d: printing %d pages from client %d\n", 
                    printerID, job->pages ,job->ID);
                
                sleep(job->pages);
        
                puts("...\n");

                printf("Printer %d: finished printing job from Client %d\n\n",
                    printerID, job->ID);
            }
            //Always free this variable    
            free(job);
        }
    }

    return 0;
}