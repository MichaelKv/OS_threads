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

/*
	Queue implementation start----------------------------------------------------------->
*/
struct queue {
	char **v;
	int head;
	int tail;
	int size;
	int maxsize;
	int index;
};

struct queue *queue_create(int maxsize) {
	struct queue *q = malloc(sizeof(*q));
	if (q != NULL) {
		q->v = (char**)malloc(sizeof(char) * (maxsize + 1));
		if (q->v == NULL) {
			free(q);
			return NULL;
		}
		q->maxsize = maxsize;
		q->size = 0;
		q->index = 0;
		q->head = maxsize + 1;
		q->tail = 0;
	}
	return q;
};

void queue_free(struct queue *q) {
	free(q->v);
	free(q);
}

int queue_size(struct queue *q) {
	return q->size;
}

int queue_enqueue(struct queue *q, char *value) {
	if (q->head == q->tail + 1) {
		fprintf(stderr, "queue: Queue overflow\n");
		return -1;
	}
	*(q->v + q->index++) = (char*)malloc(sizeof(char) * MAX_LANG);
	int j;
	for (j = 0; j < MAX_LANG; j++) {
		(q->v[q->tail])[j] = value[j];
	}
	// printf("--- Queue get the value: {%s}\n", value);
	// printf("--- Into queue was set the value: {%s}\n", q->v[q->tail]);
	q->tail++;
	q->tail = q->tail % (q->maxsize + 1);
	q->size++;
	return 0;
}

char* queue_dequeue(struct queue *q) {
	if (q->head % (q->maxsize + 1) == q->tail) {
		fprintf(stderr, "queue: Queue is empty\n");
	}
	q->head = q->head % (q->maxsize + 1);
	q->size--;
	return q->v[q->head++];
}
/*
	Queue implementation end------------------------------------------------------------->
*/