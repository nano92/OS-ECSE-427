#ifndef JOBS_QUEUE_H_  /* Include guard */

#include "common.h"
//#include "Queue_linkedList.h"

#define JOBS_QUEUE_H_

#define MAX_LENGTH 3

struct Job_info{
	int ID;
	int pages;
};

struct Jobs {
  	struct Job_info job_queue[MAX_LENGTH];
  	int queue_length;
  	int job_in, job_out;
  	int repeated_ID;
  	int printers_counter;
  	bool_t new_job;
  	bool_t shutdown_server;
  	bool_t new_printer;
  	sem_t mutex;     
  	sem_t items;       
  	sem_t spaces;      
};


void put_job(struct Jobs *job_ctrl, int item, int pages);
bool_t take_job(struct Jobs *job_ctrl, struct Job_info **job, int *printerID);
int isNewJob(struct Jobs *job_ctrl, int item);
void reset_job_queue(struct Jobs *job_ctrl);

#endif //JOBS_QUEUE