#include "jobs_queue.h"

void put_job(struct Jobs *job_ctrl) {
    puts("Client: at put_job()");
    sem_wait(&job_ctrl->spaces);
  	sem_wait(&job_ctrl->mutex);
  	puts("Client: adding job..");
    
    

   	sem_post(&job_ctrl->mutex);
  	sem_post(&job_ctrl->items);
  	printf("Client: job %d added\n", job_ctrl->ID);
}

void take_job(struct Jobs *job_ctrl, struct Node **job) {
    puts("Server: from take_job()");
    //struct Node **job = (struct Node **)malloc(sizeof(struct Node));
  	sem_wait(&job_ctrl->items);
  	sem_wait(&job_ctrl->mutex);

  	enqueue(job_ctrl->ID, job_ctrl->pages, job_ctrl->ref_front, job_ctrl->ref_rear);
  	//printf("Server: struct size %d\n", sizeof(job_ctrl));
    puts("Server: waiting for job..");
  	
  	dequeue(job_ctrl->ref_front, job_ctrl->ref_rear, job);
    printf("Job ID: %d job pages: %d\n",(*job)->source, (*job)->pages);

  	sem_post(&job_ctrl->mutex);
  	sem_post(&job_ctrl->spaces);

    puts("Server: finished processing job..");
}
