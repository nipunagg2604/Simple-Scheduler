#include "simpleScheduler.h"

int running = 0, ncpu, tslice,fd[2];
void call_scheduler(int _ncpu,int _tslice,int _fd[]){
    ncpu=_ncpu;
    tslice=_tslice;
    fd[0]=_fd[0];
    fd[1]=_fd[1];
    init_scheduler();
}
void read_from_pipe(){
  char buffer[10];
  read(fd[0],buffer,10);
  char* endptr;
  errno=0;
  long pid_long=strtol(buffer,&endptr,10);
  
  if (errno != 0 || *endptr != '\0' || pid_long > INT_MAX || pid_long < INT_MIN) {
        perror("Cannot convert error");
        return 1;
  }
  pid_t pid=(pid_t) strtol(buffer,&endptr,10);
  enqueue(pids,pid);
}

pid_t proc[100];
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

bool isFull(Queue* q) { return (q->rear == 1000); }

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

Queue* pids;

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
    proc[running]=pid;
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
	
	return 0;
}

