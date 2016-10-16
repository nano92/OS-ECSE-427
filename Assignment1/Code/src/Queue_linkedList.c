/*
Queue - Linked List implementation

Author: Luis Gallet Zambrano 260583750
Last date modified: 03/10/2016
*/
#include <stdio.h>
#include <stdlib.h>
#include "Queue_linkedList.h"

struct Node{
	char* data;
	int index;
	struct Node* next;
};

/*
Function to add a node to the queue. Each node has the same format;
"index- data"
*/
void enqueue(char* info, int* index, struct Node** front, struct Node** rear){
	struct Node* temp = 
		(struct Node*)malloc(sizeof(struct Node));
	temp->data = info; 
	temp->index = *index;
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
void dequeue(struct Node** front, struct Node** rear){
	
	struct Node* temp = NULL;
	
	if(*front == NULL) {
		printf("Queue is Empty\n");
		return;
	}
	if(*front == *rear) {
		*front = *rear = NULL;
	}
	else {
		temp = (*front)->next;
		free(*front);
		*front = temp;
	}
}

/*
This function returns the data that is stored in the node that matches
the asked index, if no match is found then NULL is returned
*/
char* getItem(struct Node** front, int index){
	struct Node* temp = *front;
	while(temp != NULL){
		if(index == temp->index){
			return temp->data;
		}else{
			temp = temp->next;
		}
	}
	return NULL;
}

/*
This fucntion prints every node of the list with the following format:
"index- data"
*/
void printList(struct Node** front){
	struct Node* temp = *front;
	while(temp != NULL){
		printf("%d- %s\n",temp->index, temp->data);
		temp = temp->next;
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

void getIDs(struct Node** front, int** IDs, int size){
	struct Node* temp = *front;
	int i = 0;
	while(temp != NULL || i != size){
		(*IDs)[i] = temp->index;
		i++;
		temp = temp->next;
	}
}
