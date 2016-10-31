#ifndef JOBS_QUEUE_H_  /* Include guard */

#include "common.h"
#include "Queue_linkedList.h"

#define JOBS_QUEUE_H_

struct Jobs {
	struct Node *front = NULL;
	struct Node *rear = NULL;
	struct Node **ref_front = &front;
	struct Node **ref_rear=&rear;
	
	int ID;
	int pages;
  	int length;
  	sem_t mutex;     
  	sem_t items;       
  	sem_t spaces;      
};

void put_job(struct Jobs *job_ctrl);
void take_job(struct Jobs *job_ctrl, struct Node **job);

#define MAX_LENGTH 10

#endif //JOBS_QUEUE