#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <stdbool.h>
#include <fcntl.h>

int fd[100][2];
char command_history[500][500]; 
pid_t pid_history[500];
long execution_time[500];
int indexx=0;
int scheduler_pid;
int shell_pid;

typedef struct {
    pid_t items[1000];
	bool up[1000];
    int front;
    int rear;
	int index[1000];
	int total_cnt[1000];
	int wait_cycle[1000];
} Queue;

void initializeQueue(Queue* q)
{
    q->front = -1;
    q->rear = 0;
}

bool isEmpty(Queue* q) { return (q->front == q->rear - 1); }

bool isFull(Queue* q) { return (q->rear == 1000); }

void enqueue(Queue* q, pid_t value, int indexxx)
{
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }
    q->items[q->rear] = value;
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
	return q->items[q->front + 1];
}

typedef struct shm_t
{
	int ncpu, tslice;
	Queue pids[5];
	sem_t queue_empty[5];
	long total_time[1000];
	long wait_time[1000];
} shm_t;

shm_t* shm;

char** split_input(char* input_string,char* delimeter)
{
    char* cpy = malloc(sizeof(char) * (500));
	strcpy(cpy, input_string);
    
    char** split_string=(char **) malloc(500 * sizeof(char *));
    int ind=0;
    char* str_split = strtok(cpy,delimeter);
    while (str_split!=NULL){
        split_string[ind]=str_split;
        str_split = strtok(NULL,delimeter);
        ind++;
    }split_string[ind]=NULL;

    return split_string;

}

long current_time(){
    struct timeval time;
    gettimeofday(&time,NULL);
    long return_time= time.tv_sec * 1000;
    
    return return_time + time.tv_usec / 1000;
}

char* find_path(char* elf) 
{
	int status, fd[2];
	pipe(fd);
	if((status = fork()) < 0) 
	{
		fprintf(stderr, "Fork Error");
		exit(1);
	}else if(status == 0)
	{
		if(dup2(fd[1], STDOUT_FILENO) == -1)
		{
			fprintf(stderr, "Dup Error!");
			exit(1);
		}
		close(fd[0]);
		close(fd[1]);
		char* args[] = {"which", elf, NULL};
		execv("/bin/which", args);
	}
	close(fd[1]);
	char* path = malloc(sizeof(char) * (500));
	read(fd[0], path, 500);
	close(fd[0]);
	for(int i=0; i<500; i++)
	{
		if(path[i] == '\n') 
		{
			path[i] = '\0';
			break;
		}
	}

	return path;
}


void run_scheduler_process(char** and_split,int priority, int index){
    int pid = fork();
	if(pid < 0) 
	{
		fprintf(stderr, "Fork Error");
		exit(1);
	}else if(pid == 0) 
	{   
		char* path=find_path(and_split[1]);
        char* args[2];
        args[0]=and_split[1];
        args[1]=NULL;
		execv(path, args);
	}
    kill(pid,SIGSTOP);
	sem_wait(&shm->queue_empty[priority]);
    enqueue(&shm->pids[priority], pid, index);
	sem_post(&shm->queue_empty[priority]);
	int status;
	waitpid(pid, &status, 0);
}

int and_supporter(char* command, int index) 
{   
	printf("command: %s\n", command);
	char** and_split = split_input(command, " ");
	int priority=1;
	char* path = find_path(and_split[1]);
	if (and_split[2]!=NULL){
		priority=atoi(and_split[2]);
	}
	if(and_split[1] == NULL) 
	{
		fprintf(stderr, "File not found! Try Again.\n");
		return 0;
	}
	else if(strcmp(path, "") == 0) 
	{
		fprintf(stderr, "File not found! Try Again.\n");
		return 0;
	}
	else if(strcmp(and_split[0], "submit") == 0){
		if(fork() == 0) run_scheduler_process(and_split,priority, index);
    }

	return 1;
}

char* read_user_input()
{
	char* str = NULL;
	size_t len = 0;
	size_t num = getline(&str, &len, stdin);

	if(str[num-1] == '\n') str[num-1] = '\0';

	return str;
}

void show_history()
{
	for(int i=0; i<indexx; i++) printf("%d. %s\n", i+1, command_history[i]);
}

void shell_loop() 
{
	char* command;
	//char* cmpr = "history";
	do
	{
		printf("acer@FalleN:~$");
		command = read_user_input();
		//if(strcmp(cmpr, command) == 0) show_history();
		pid_t status = and_supporter(command, indexx);
   //     long total_time = (long)(end - start);
   //     execution_time[indexx]=total_time;
        if(status != -1) 
		{
			strcpy(command_history[indexx], command);
			pid_history[indexx] = status;
			indexx++;
		}

		free(command);
	}while(1);
}

static void sig_handler(int signum)
{
	if(shell_pid != getpid()) {kill(getpid(), SIGKILL); return;}
	static int counter = 0;
	if(signum == SIGINT)
	{
		for(int i=0; i<indexx; i++)
		{
			char buffr[3] = "   ";
			char buff[10] = "Command: ";
			write(STDOUT_FILENO, buff, 10);
			size_t siz_ch = strlen(command_history[i]);
			write(STDOUT_FILENO, command_history[i], siz_ch);
			write(STDOUT_FILENO, buffr, 3);

			char buff1[17] = "Waiting time: ";
			write(STDOUT_FILENO, buff1, 14);
			char str1[100];
			sprintf(str1, "%ld", shm->wait_time[i]);
			size_t sz1 = strlen(str1);
			write(STDOUT_FILENO, str1, sz1);
			write(STDOUT_FILENO, " s   ", 5);

			char buff3[17] = "Completion time: ";
			write(STDOUT_FILENO, buff3, 17);
			char str[100];
			sprintf(str, "%ld", shm->total_time[i]);
			size_t sz = strlen(str);
			write(STDOUT_FILENO, str, sz);
			write(STDOUT_FILENO, " s\n", 3);

		}kill(scheduler_pid, SIGKILL);
		exit(0);
	}
}

void init_sig_handler()
{
	struct sigaction sig;
	memset(&sig, 0, sizeof(sig));
	sig.sa_handler = sig_handler;
	if(sigaction(SIGINT, &sig, NULL) == -1)
	{
		fprintf(stderr, "Sigaction Error!");
		exit(1);
	}
}

shm_t* setup()
{
	const char* shm_name = "/shared_mem";
	int shm_fd;
	shm_t *shm;

	shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if(ftruncate(shm_fd, sizeof(shm_t)) == -1) 
	{
        perror("ftruncate");
        close(shm_fd);
        exit(1);
    }

    shm = mmap(0, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm == MAP_FAILED) 
	{
        perror("mmap");
        close(shm_fd);
        exit(1);
    }

    // Optional: Initialize the shared memory region (if needed)
    memset(shm, 0, sizeof(shm_t)); // Initialize memory to zero

    return shm;
}

void cleanup(shm_t *shm) {
    const char *shm_name = "/shared_mem"; 

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

    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
        exit(1);
    }
}

int main(int argc, char** argv)
{   
	if (argc!=3){
        exit(1);
    }

	shell_pid = getpid();

	shm = setup();
	for(int i=1; i<5; i++) 
	{
		sem_init(&shm->queue_empty[i], 1, 1);
		initializeQueue(&shm->pids[i]);
	}


	shm->ncpu = atoi(argv[1]);
	shm->tslice = atoi(argv[2]);

    scheduler_pid=fork();
    if (scheduler_pid<0){
        fprintf(stderr, "Fork Error");
		exit(1);
    }
    else if (scheduler_pid==0){
        char* path=find_path("./simpleScheduler");
        char* arr[3];
        arr[0]="./simpleScheduler";
        arr[1] = "/shared_mem";
        execv("./simpleScheduler", arr);
    }

	init_sig_handler();
	shell_loop();

	cleanup(shm);

	return 0;
}
