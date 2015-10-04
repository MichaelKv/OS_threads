#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_LANG 20 //The maximum size of the input lines
#define ROWS_COUNT 3 //The maximum amount of rows in the input file

#define MAX_COUNT_OF_NUMBERS 4 //The maximum amount of numbers in a row
#define MAX_COUNT_OF_DIGITS 5 //The maximum amount of digits in the input numbers

#define WORKERS_COUNT 5// The maximum amount threads of worker

// Queue header start----------------------------------------------------------->
struct queue;
struct queue *queue_create(int maxsize);
int queue_enqueue(struct queue *q, char *value);
char* queue_dequeue(struct queue *q);
int queue_size(struct queue *q);
void queue_free(struct queue *q);
// Queue header end------------------------------------------------------------->

// Parse header start----------------------------------------------------------->
int* parse(int *out_arr, char *in_str);
// Parse header end------------------------------------------------------------->

// Threads header start--------------------------------------------------------->
void* read_thread(void *p);
void* write_thread(void *p);
void* work_thread(void *p);
// Threads header end----------------------------------------------------------->

// Parameters of threads start-------------------------------------------------------->
struct params_for_reader {
	pthread_mutex_t mutex;
	pthread_cond_t condvar;
	struct queue *q;
	int count;
};

struct params_for_writer {
	pthread_mutex_t mutex;
	pthread_cond_t condvar;
	int sums[ROWS_COUNT];
	int i;
};

struct params_for_workers {
	struct params_for_reader* params_reader;
	struct params_for_writer* params_writer;
};
// Parameters of threads end---------------------------------------------------------->

int main() {
	struct params_for_reader params_reader;
	pthread_mutex_init(&params_reader.mutex, NULL);
  	pthread_cond_init(&params_reader.condvar, NULL); 
  	params_reader.q = queue_create(ROWS_COUNT);
  	params_reader.count = 0;

	struct params_for_writer params_writer;
	pthread_mutex_init(&params_writer.mutex, NULL);
  	pthread_cond_init(&params_writer.condvar, NULL);

  	struct params_for_workers params_workers;
  	params_workers.params_writer = &params_writer;
  	params_workers.params_reader = &params_reader;

  	pthread_t reader, writer;
  	pthread_t workers[WORKERS_COUNT];

  	pthread_create(&reader, NULL, read_thread, &params_reader);
	pthread_create(&writer, NULL, write_thread, &params_writer);

	for (int i = 0; i < WORKERS_COUNT; i++) {
		 pthread_create(&workers[i], NULL, work_thread, &params_workers);
	}

	pthread_join(reader, NULL);
 	pthread_join(writer, NULL);

	pthread_mutex_destroy(&params_writer.mutex);
	pthread_cond_destroy(&params_writer.condvar);
	pthread_mutex_destroy(&params_reader.mutex);
	pthread_cond_destroy(&params_reader.condvar);
}

// Threads implementation start---------------------------------------------------------->
void* read_thread(void *p) {
	sleep(1);
	struct params_for_reader* params = (struct params_for_reader*) p;
	FILE *file;
	char str[MAX_LANG], *estr;

	printf("Reader #{%d} => Opening file\n", (int)pthread_self());
   	file = fopen ("input.txt","r");
	if (file == NULL) {
		printf("Reader #{%d} => Error\n", (int)pthread_self());
	} else 
		printf("Reader #{%d} => Done\n", (int)pthread_self());

	while(1) {
		struct timespec tw = {0,300000000};
		struct timespec tr;
		nanosleep (&tw, &tr);

		estr = fgets (str, sizeof(str), file);
		pthread_mutex_lock(&params->mutex);

		if (estr != NULL) {
			printf("Reader #{%d} => I have read next string: %s", (int)pthread_self(), str);

			params->count++;
			queue_enqueue(params->q, str);
			for (int i = 0; i < WORKERS_COUNT; i++)
				pthread_cond_signal(&params->condvar);

			pthread_mutex_unlock(&params->mutex);
		} else {
			if (feof(file) != 0) {
				printf("Reader #{%d} => Reading is complete\n", (int)pthread_self());
				pthread_mutex_unlock(&params->mutex);
				break;
			} else {
				printf("Reader #{%d} => Error reading the file\n", (int)pthread_self());
				pthread_mutex_unlock(&params->mutex);
				break;
			}
		}
 	}
}
// Threads implementation end------------------------------------------------------------>

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

/*
	Parse implementation start----------------------------------------------------------->
*/
int* parse(int *out_arr, char *in_str) {
	int index = 0, out_index = 0;
	while(*(in_str + index) != '\0') {
		if (isdigit(*(in_str + index)) || *(in_str + index) == '-') {
			char *digit_buf = (char*)malloc(sizeof(char) * (MAX_COUNT_OF_DIGITS + 2));//One char for '\0' & one char for minus
			int i = 0;
			while(isdigit(*(in_str + index)) || *(in_str + index) == '-') {
			    *(digit_buf + i) = *(in_str + index);
			    index++;
			    i++;
			}
			*(out_arr + out_index) = atoi(digit_buf);
			out_index++;
		}
		index++;
	}
	return out_arr;
}
/*
	Parse implementation end------------------------------------------------------------->
*/