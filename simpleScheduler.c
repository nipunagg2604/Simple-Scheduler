#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int running = 0, ncpu, tslice;

typedef struct {
    pid_t items[1000];
    int front;
    int rear;
} Queue;

void initializeQueue(Queue* q)
{
    q->front = -1;
    q->rear = 0;
}

bool isEmpty(Queue* q) { return (q->front == q->rear - 1); }

bool isFull(Queue* q) { return (q->rear == MAX_SIZE); }

void enqueue(Queue* q, pid_t value)
{
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }
    q->items[q->rear] = value;
    q->rear++;
}

void dequeue(Queue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }
    q->front++;
}

void printQueue(Queue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }

    printf("Current Queue: ");
    for (int i = q->front + 1; i < q->rear; i++) {
        printf("%d ", q->items[i]);
    }
    printf("\n");
}

pid_t front(Queue* q)
{
	return q->items[q->front];
}

Queue pids;

void run_batch()
{
	int i = 0;
	while(i < running)
	{
		pid_t pid = proc[i];
		if(kill(pid, 0) == 0) 
		{
			kill(pid, SIGSTOP);
			enqueue(pids, pid);
		}
		i++;
	}

	running = 0;
	while(running < ncpu && !isEmpty(pids)) 
	{
		pid_t pid = front(pids);
		kill(pid, SIGCONT);
		dequeue(pids);
		running++;
	}
}

void init_scheduler()
{
	while(1)
	{
		run_batch();
		sleep(tslice);
	}
}

int main(int argc, char** argv) 
{
	ncpu = atoi(argv[1]);
	tslice = atoi(argv[2]);
	init_scheduler();


	return 0;
}