#ifndef JOBS_QUEUE_H_  /* Include guard */

#include "common.h"

#define JOBS_QUEUE_H_

struct Jobs {
	struct Node *front;
	struct Node *rear;
  	int length;
  	sem_t mutex;     
  	sem_t items;       
  	sem_t spaces;      
};

void put_job(struct Jobs **job_ctrl, int item, int pages);
int take_job(struct Jobs **job_ctrl);

#define MAX_LENGTH 10

#endif //JOBS_QUEUE