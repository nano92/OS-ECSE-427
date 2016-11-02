#include "jobs_queue.h"


void put_job(struct Jobs *job_ctrl, int item, int pages) {
  	sem_wait(&job_ctrl->spaces);
 	sem_wait(&job_ctrl->mutex);

 	//Check if the incoming job is already in the buffer waiting for a printer
    job_ctrl->repeated_ID = isNewJob(job_ctrl, item);
    if(job_ctrl->repeated_ID != -1){
        printf("Client %d is already in the job queue at position %d\n", item, job_ctrl->repeated_ID);
 		sem_post(&job_ctrl->mutex);
 		sem_post(&job_ctrl->spaces);
        return;
    }
  	
  	puts("Client: adding job..");

    job_ctrl->job_queue[job_ctrl->job_in].ID = item;
    job_ctrl->job_queue[job_ctrl->job_in].pages = pages;

    printf("Client: job %d added\n", job_ctrl->job_queue[job_ctrl->job_in].ID);
    
    //Calculate the index of the incoming job
    job_ctrl->job_in = (job_ctrl->job_in + 1) % job_ctrl->queue_length;

    job_ctrl->new_job = true;

   	sem_post(&job_ctrl->mutex);
  	sem_post(&job_ctrl->items);
}

bool_t take_job(struct Jobs *job_ctrl, struct Job_info **job, int *printerID) {   
    
    if(!job_ctrl->new_job){
    	printf("No more clients for printer %d ...sleeping\n", *printerID);
    }
    
    sem_wait(&job_ctrl->items);
    
    if(job_ctrl->shutdown_server){
    	return false;
    }
  	
  	sem_wait(&job_ctrl->mutex);

  	memcpy(*job, &job_ctrl->job_queue[job_ctrl->job_out], sizeof(struct Job_info));
  	printf("Printer %d: Job ID: %d job pages: %d\n",*printerID, (*job)->ID, (*job)->pages);

  	//When the requested jobs has been already copied to another data structure to be
  	//"printed", its values are reset to the default one, in order to avoid the case where
  	//a future job may have the same ID as an old one
  	job_ctrl->job_queue[job_ctrl->job_out].ID = 0;
  	job_ctrl->job_queue[job_ctrl->job_out].pages = 0;

  	//Calculate next index of the job that will be served next
  	job_ctrl->job_out = (job_ctrl->job_out + 1) % job_ctrl->queue_length;

  	job_ctrl->new_job = false;

  	sem_post(&job_ctrl->mutex);
  	sem_post(&job_ctrl->spaces);

    printf("Printer %d: finished processing job...starting printing\n", *printerID);

   	return true;
}

/*
This function checks if an incoming job has the same ID as another one that it is already
waiting in the buffer. 
It returns the index of the job in the buffer with the same ID or -1 when no equal ID is found.
If a job with the same ID was already served in the past, the job is accepted.
*/
int isNewJob(struct Jobs *job_ctrl, int item){
	for(int i = job_ctrl->job_out; i < MAX_LENGTH; i++){
		if(job_ctrl->job_queue[i].ID == item){
			return i;
		}
	}

	return -1;
}