/*
Queue - Linked List implementation

Author: Luis Gallet Zambrano 260583750
Last date modified: 03/10/2016
*/
#include "Queue_linkedList.h"

/*
Function to add a node to the queue. Each node has the same format;
"index- data"
*/
void enqueue(int source, int pages, struct Node** front, struct Node** rear){
	struct Node* temp = 
		(struct Node*)malloc(sizeof(struct Node)); 
	temp->source = source;
	temp->pages = pages;
	temp->next = NULL;
	if(*front == NULL && *rear == NULL){
		*front = *rear = temp;
		return;
	}
	(*rear)->next = temp;
	*rear = temp;
}

/*
This function erase the node that is in the front of the queue
*/
struct Node** dequeue(struct Node** front, struct Node** rear){
	puts("From dequeue()");
	struct Node* temp = 
		(struct Node*)malloc(sizeof(struct Node));
	
	if(*front == NULL) {
		puts("error source?");
		printf("Queue is Empty\n");
		return NULL;
	}
	if(*front == *rear) {
		*front = *rear = NULL;
	}
	else {
		temp = *front;
		*front = (*front)->next;
	}

	return &temp;
}

/*
This fucntion prints every node of the list with the following format:
"index- data"
*/
void printList(struct Node** front){
	struct Node* temp = *front;
	int count = 0;
	while(temp != NULL){
		printf("%d- ID = %d, pages = %d\n", count, temp->source, temp->pages);
		temp = temp->next;
		count++;
	}
}

/*
This function returns the number of nodes the list has
*/
int size(struct Node** front){
	struct Node* temp = *front;
	int count = 0;
	while(temp != NULL){
		count++;
		temp = temp->next;
	}
	return count;
}
 
