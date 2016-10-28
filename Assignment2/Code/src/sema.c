#include <stdio.h>
#include <stdlib.h>         
#include <errno.h>         
#include "sema.h"

void init_semaphore(Semaphore *sem, int value)
{
  int n = sem_init(sem, 1, value);
  if (n != 0) puts("sem_init failed"); exit(EXIT_FAILURE);
}

void semaphore_wait(Semaphore *sem)
{
  int n = sem_wait(sem);
  if (n != 0) puts("sem_wait failed"); exit(EXIT_FAILURE);
}

void semaphore_signal(Semaphore *sem)
{
  int n = sem_post(sem);
  if (n != 0) puts("sem_post failed"); exit(EXIT_FAILURE);
}
