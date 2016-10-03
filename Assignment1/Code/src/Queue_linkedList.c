/*Queue - Linked List implementation*/
#include<stdio.h>
#include<stdlib.h>
#include "Queue_linkedList.h"

struct Node{
	char* data;
	int index;
	struct Node* next;
};
// Two glboal variables to store address of front and rear nodes. 
//struct Node* front = NULL;
//struct Node* rear = NULL;

// To enqueue 
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

// To dequeue 
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

void printList(struct Node** front){
	struct Node* temp = *front;
	while(temp != NULL){
		printf("%d- %s\n",temp->index, temp->data);
		temp = temp->next;
	}
}

int size(struct Node** front){
	struct Node* temp = *front;
	int count = 0;
	while(temp != NULL){
		count++;
		temp = temp->next;
	}
	return count;
}


/*int main(){
	//struct Node* list1 = 
		//(struct Node*)malloc(sizeof(struct Node));
	struct Node* list1_front = NULL;
	struct Node** ref_l1f = &list1_front;
	struct Node* list1_rear = NULL;
	struct Node** ref_l1r = &list1_rear;
	
	//Drive code to test the implementation. 
	// Printing elements in Queue after each Enqueue or Dequeue 
	
	enqueue("ls -1", ref_l1f, ref_l1r); print(ref_l1f);
	printf("%d\n",size(ref_l1f));
	//free(list1);
	enqueue("mkdir hola", ref_l1f, ref_l1r); print(ref_l1f);
	
	enqueue("command 2", ref_l1f, ref_l1r); print(ref_l1f);
	dequeue(ref_l1f, ref_l1r); print(ref_l1f);
	enqueue("rmdir hola", ref_l1f, ref_l1r); print(ref_l1f);
	printf("%d\n",size(ref_l1f));
}*/
