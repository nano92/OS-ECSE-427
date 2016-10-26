#ifndef JOBS_QUEUE_H_  /* Include guard */

#include "sema.h"
#include "Queue_linkedList.h"

#define JOBS_QUEUE_H_

struct Jobs {
	struct Node *front;
	struct Node *rear;
	struct Node **ref_front;
  	struct Node **ref_rear;
  	int length;
  	Semaphore *mutex;     
  	Semaphore *items;       
  	Semaphore *spaces;      
};

void push_job(struct Jobs *job_ctrl, int *item);
int pop_job(struct Jobs *job_ctrl);

#define MAX_LENGTH 10

#endif //JOBS_QUEUE