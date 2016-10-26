/*
Queue - Linked List implementation

Author: Luis Gallet Zambrano 260583750
Last date modified: 03/10/2016
*/
#include "Queue_linkedList.h"

struct Node{
	int source;
	struct Node* next;
};

/*
Function to add a node to the queue. Each node has the same format;
"index- data"
*/
void enqueue(int* source, struct Node** front, struct Node** rear){
	struct Node* temp = 
		(struct Node*)malloc(sizeof(struct Node)); 
	temp->source = *source;
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
int dequeue(struct Node** front, struct Node** rear){
	
	struct Node* temp = NULL;
	
	if(*front == NULL) {
		printf("Queue is Empty\n");
		return NULL;
	}
	if(*front == *rear) {
		*front = *rear = NULL;
	}
	else {
		temp = (*front)->next;
		free(*front);
		*front = temp;
	}

	return (*front)->source;
}

/*
This fucntion prints every node of the list with the following format:
"index- data"
*/
void printList(struct Node** front){
	struct Node* temp = *front;
	int count = 0;
	while(temp != NULL){
		printf("%d- %d\n", count, temp->source);
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
 
