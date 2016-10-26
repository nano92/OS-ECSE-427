#ifndef SEMAPHORE_H_   /* Include guard */

#include "common.h"

#define SEMAPHORE_H_

typedef sem_t Semaphore;

Semaphore *init_semaphore(int value);
void semaphore_wait(Semaphore *sem);
void semaphore_signal(Semaphore *sem);

#endif // SEMAPHORE_H_