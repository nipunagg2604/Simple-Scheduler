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

int running[5], proc[5][1000], proc_ind[5][1000];
int count[5][1000], waitt[5][1000];
bool going_up[5][1000];

typedef struct {
    pid_t items[1000];
	bool up[1000];
    int front;
    int rear;
	int index[1000];
	int total_cnt[1000];
	int wait_cycle[1000];
} Queue;

typedef struct shm_t{
	int ncpu, tslice;
    Queue pids[5];
    sem_t queue_empty[5];
	long total_time[1000];
	long wait_time[1000];
} shm_t;

shm_t* shm;

void initializeQueue(Queue* q)
{
    q->front = -1;
    q->rear = 0;
}

bool isEmpty(Queue* q) { return (q->front == q->rear - 1); }

bool isFull(Queue* q) { return (q->rear == 1000); }

void enqueue(Queue* q, pid_t pid, int indexxx)
{
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }
    q->items[q->rear] = pid;
	q->index[q->rear] = indexxx;
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
        printf("%d ", q->items[i]);
    }printf("\n");
}

void increase_cycle(int priority)
{
	for(int i=priority; i<5; i++)
	{
		Queue* q = &shm->pids[i];
		for (int i=q->front;i<q->rear;i++)
		{
			q->wait_cycle[i]++;
			q->total_cnt[i]++;
		}
	}
}

pid_t front(Queue* q)
{
	return q->items[q->front + 1];
}

int front_ind(Queue* q)
{
	return q->index[q->front + 1];
}

void change_direction(Queue* q)
{
	q->up[q->rear - 1] = !q->up[q->rear - 1];
}

void add_total_count(Queue* q, int add)
{
	q->total_cnt[q->rear - 1] += add;
}

void save_wait_time(Queue* q, int add)
{
	q->wait_cycle[q->rear - 1] += add;
}

int front_cnt(Queue* q)
{
	return q->total_cnt[q->front + 1];
}

int front_wait(Queue* q)
{
	return q->wait_cycle[q->front + 1];
}

bool front_up(Queue* q)
{
	return q->up[q->front + 1];
}

int check_pid(int pid) {
    char path[40];
    sprintf(path, "/proc/%d/stat", pid);
    
    FILE *stat_file = fopen(path, "r");
    if (!stat_file) {
        return 0;  // Process has terminated (or doesn't exist)
    }
    
    // Read the contents of the /proc/<PID>/stat file to check process state
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stat_file) != NULL) {
        char state;
        sscanf(buffer, "%*d %*s %c", &state);  // The 3rd field in /proc/<PID>/stat is the state
        fclose(stat_file);
        
        if (state == 'Z') {
            return 0;  // Process is a zombie (terminated but not reaped)
        } else {
            return 1;  // Process is running
        }
    }

    fclose(stat_file);
    return 0;  // Default: Process not running
}

int run_batch()
{
	for(int priority = 1; priority < 5; priority++)
	{
		int i = 0;
		while(i < running[priority])
		{
			pid_t pid = proc[priority][i];
			int indexx = proc_ind[priority][i];
			//printf("pid: %d\n", pid);
			int status;
			//pid_t result = waitpid(pid, &status, WNOHANG);
			//int result = check_pid(pid);
			//printf("result: %d\n", result);
			//if(result == 1)
			printf("Hello\n");
			if(kill(pid, 0) == 0)
			{
				//printf("Hello\n");
				kill(pid, SIGSTOP);
				if(!going_up[priority][i])
				{
					sem_wait(&shm->queue_empty[priority + 1]);
					enqueue(&shm->pids[priority + 1], pid, indexx);
					add_total_count(&shm->pids[priority + 1], count[priority][i]);
					save_wait_time(&shm->pids[priority + 1], waitt[priority][i]);
					if(priority == 3) change_direction(&shm->pids[4]);
					//printf("%d\n", priority + 1);
					sem_post(&shm->queue_empty[priority + 1]);
				}else
				{
					if(count[priority][i] % 3 == 0)
					{
						sem_wait(&shm->queue_empty[priority - 1]);
						enqueue(&shm->pids[priority - 1], pid, indexx);
						add_total_count(&shm->pids[priority - 1], count[priority][i]);
						save_wait_time(&shm->pids[priority - 1], waitt[priority][i]);
						if(priority == 2) change_direction(&shm->pids[1]);
						//printf("%d\n", priority - 1);
						sem_post(&shm->queue_empty[priority - 1]);
					}else
					{
						sem_wait(&shm->queue_empty[priority]);
						enqueue(&shm->pids[priority], pid, indexx);
						add_total_count(&shm->pids[priority], 1);
						//add_wait_time(&shm->pids[priority], 1);
						//printf("%d\n", priority);
						sem_post(&shm->queue_empty[priority]);
					}
				}
			}
			else 
			{
				printf("count: %d\n", count[priority][i]);
				shm->total_time[indexx] = shm->tslice*count[priority][i];
				shm->wait_time[indexx] = shm->tslice*waitt[priority][i];
			}
			i++;
		}

		running[priority] = 0;
		while(running[priority] < shm->ncpu && !isEmpty(&(shm->pids[priority]))) 
		{
			sem_wait(&shm->queue_empty[priority]);
			pid_t pid = front(&(shm->pids[priority]));
			int indexx = front_ind(&(shm->pids[priority]));
			int ct = front_cnt(&(shm->pids[priority]));
			int wt = front_wait(&(shm->pids[priority]));
			bool gng_up = front_up(&(shm->pids[priority]));
			dequeue(&(shm->pids[priority]));
			sem_post(&shm->queue_empty[priority]);
			proc[priority][running[priority]] = pid;
			proc_ind[priority][running[priority]] = indexx;
			count[priority][running[priority]] = ct;
			waitt[priority][running[priority]] = wt;
			going_up[priority][running[priority]] = gng_up;

			running[priority]++;
			kill(pid, SIGCONT);
		}

		if(running[priority] != 0) 
		{
			increase_cycle(priority);
			return 1;
		}
	}

	return 0;
}

void init_scheduler()
{
	while(1)
	{
		int status = run_batch();
		if(status) sleep(shm->tslice);
	}
}

void init_cnt(Queue* q)
{
	for(int i=0; i<1000; i++) {q->total_cnt[i] = 1; q->wait_cycle[i] = 0;}
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

	for(int i=1; i<5; i++) init_cnt(&shm->pids[i]);

	init_scheduler();

  munmap(shm, sizeof(shm_t));
  close(shm_fd);

	return 0;
}
