#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

// Queue header start----------------------------------------------------------->
struct queue;
struct queue *queue_create(int maxsize);
int queue_enqueue(struct queue *q, char *value);
char* queue_dequeue(struct queue *q);
int queue_size(struct queue *q);
void queue_free(struct queue *q);
// Queue header end------------------------------------------------------------->

int main() {

}