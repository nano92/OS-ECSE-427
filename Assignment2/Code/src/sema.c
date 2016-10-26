#include <stdio.h>
#include <stdlib.h>         
#include <errno.h>         
#include "sema.h"

Semaphore *init_semaphore(int value)
{
  Semaphore *sem = (Semaphore *)malloc(sizeof(Semaphore));
  int n = sem_init(sem, 0, value);
  if (n != 0) perror("sem_init failed"); exit(EXIT_FAILURE);
  return sem;
}

void semaphore_wait(Semaphore *sem)
{
  int n = sem_wait(sem);
  if (n != 0) perror("sem_wait failed"); exit(EXIT_FAILURE);
}

void semaphore_signal(Semaphore *sem)
{
  int n = sem_post(sem);
  if (n != 0) perror("sem_post failed"); exit(EXIT_FAILURE);
}
