#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>

int running = 0,proc[1000];

typedef struct {
    pid_t items[1000];
    int front;
    int rear;
} Queue;

typedef struct shm_t{
	int ncpu, tslice;
    Queue pids[5];
    sem_t queue_empty;
} shm_t;

shm_t* shm;

void initializeQueue(Queue* q)
{
    q->front = -1;
    q->rear = 0;
}

bool isEmpty(Queue* q) { return (q->front == q->rear - 1); }

bool isFull(Queue* q) { return (q->rear == 1000); }

void enqueue(Queue* q, pid_t pid)
{
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }
    q->items[q->rear] = pid;
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

void cleanup(shm_t *shm,char* shm_name) { 

    if (munmap(shm, sizeof(shm_t)) == -1) {
        perror("munmap");
        exit(1);
    }

    int shm_fd = shm_open(shm_name, O_RDWR, 0666); 
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    if (close(shm_fd) == -1) {
        perror("close");
        exit(1);
    }

}

void printQueue(Queue* q)
{   
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }

    printf("Current Queue: ");
    for (int i = q->front + 1; i < q->rear; i++) {
        printf("%d   %d", q->items[i]);
    }printf("\n");
}

pid_t front(Queue* q)
{
	return q->items[q->front + 1];
}

int run_batch()
{
	int i = 0;
	while(i < running)
	{
		pid_t pid = proc[i];
		int status;
		pid_t result = waitpid(pid, &status, WNOHANG);
		if(result == 0)
		{
			kill(pid, SIGSTOP);
			sem_wait(&shm->queue_empty);
			enqueue(&shm->pids, pid);
			sem_post(&shm->queue_empty);
		}
		i++;
	}

	running = 0;
	while(running < shm->ncpu && !isEmpty(&(shm->pids))) 
	{
		sem_wait(&shm->queue_empty);
		pid_t pid = front(&(shm->pids));
		dequeue(&(shm->pids));
		sem_post(&shm->queue_empty);
		proc[running] = pid;
		running++;
		kill(pid, SIGCONT);
	}

	if(running == 0) return 0;
	return 1;
}

void init_scheduler()
{
	while(1)
	{
		int status = run_batch();
		if(status) sleep(shm->tslice);
	}
}

int main(int argc, char** argv) 
{ 
	if (argc<2){
      fprintf(stderr, "Less number of command line arguments\n");
      exit(1);
  }
  char* shm_name=argv[1];
  int shm_fd;

  shm_fd = shm_open(shm_name, O_RDWR, 0666);

    if (shm_fd == -1) {
        perror("can't open shared memory");
        exit(1);
    }

    shm = mmap(0, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

	init_scheduler();

  munmap(shm, sizeof(shm_t));
  close(shm_fd);

	return 0;
}
