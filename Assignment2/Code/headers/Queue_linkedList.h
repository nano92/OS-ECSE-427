/*
Queue_linkedList.c header file

Author: Luis Gallet Zambrano 260583750
Last date modified: 03/10/2016
*/

#ifndef QUEUE_LINKEDLIST_H_   /* Include guard */

#include "common.h"

#define QUEUE_LINKEDLIST_H_

struct Node;

void enqueue(int* source, struct Node** front, struct Node** rear);
int dequeue(struct Node** front, struct Node** rear);
void printList(struct Node** front);
int size(struct Node** front);

#endif // QUEUE_LINKEDLIST_H_