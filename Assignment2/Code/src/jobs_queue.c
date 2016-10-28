#include "jobs_queue.h"
#include "Queue_linkedList.h"

void put_job(struct Jobs **job_ctrl, int item, int pages) {
    puts("at put_job()");
    sem_wait(&(*job_ctrl)->spaces);
  	sem_wait(&(*job_ctrl)->mutex);

  	puts("adding job..");
    enqueue(item, pages, &(*job_ctrl)->front, &(*job_ctrl)->rear);

  	sem_post(&(*job_ctrl)->mutex);
  	sem_post(&(*job_ctrl)->items);
}

int take_job(struct Jobs **job_ctrl) {
    puts("From take_job()");
    struct Node **job = (struct Node **)malloc(sizeof(struct Node));
  	sem_wait(&(*job_ctrl)->items);
  	sem_wait(&(*job_ctrl)->mutex);
    
    puts("waiting for job..");
  	job = dequeue(&(*job_ctrl)->front, &(*job_ctrl)->rear);
    printf("Job ID: %d job pages: %d\n",(*job)->source, (*job)->pages);

  	sem_post(&(*job_ctrl)->mutex);
  	sem_post(&(*job_ctrl)->spaces);

    puts("finished processing job..");
    return (*job)->pages;
}
