#include "jobs_queue.h"

void push_job(struct Jobs *job_ctrl, int *item) {
	semaphore_wait(job_ctrl->spaces);
  	semaphore_wait(job_ctrl->mutex);

  	enqueue(item, job_ctrl->ref_front, job_ctrl->ref_rear);

  	semaphore_signal(job_ctrl->mutex);
  	semaphore_signal(job_ctrl->items);
}

int pop_job(struct Jobs *job_ctrl) {
  	semaphore_wait(job_ctrl->items);
  	semaphore_wait(job_ctrl->mutex);
  
  	int source = dequeue(job_ctrl->ref_front, job_ctrl->ref_rear);

  	semaphore_signal(job_ctrl->mutex);
  	semaphore_signal(job_ctrl->spaces);

  	return source;
}
