/*
Queue_linkedList.c header file

Author: Luis Gallet Zambrano 260583750
Last date modified: 03/10/2016
*/

#ifndef QUEUE_LINKEDLIST_H_   /* Include guard */
#define QUEUE_LINKEDLIST_H_

struct Node;

void enqueue(char* info, int* index, struct Node** front, struct Node** rear);
void dequeue(struct Node** front, struct Node** rear);
char* getItem(struct Node** front, int index);
void printList(struct Node** front);
int size(struct Node** front);
void getIDs(struct Node** front, int **IDs, int size);

#endif // QUEUE_LINKEDLIST_H_