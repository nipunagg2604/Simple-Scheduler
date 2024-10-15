#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>

int fd[100][2];
char command_history[500][500]; 
long execution_time[500];
char start_time[500][500];
char end_time[500][500];
int indexx=0;
int ncpu, tslice;

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

void create_process_and_run(char* path, char* command)
{
	int status = fork();
	if(status < 0) 
	{
		fprintf(stderr, "Fork Error");
		exit(1);
	}else if(status == 0) 
	{
		char** args = split_input(command," ");
		execv(path, args);
	}
	int gett;
	waitpid(status, &gett, 0);
}

void run_pipe_commands (char* path, char** splitted,char** all_commands,int i){
    if (i==0){
		pipe(fd[0]);
        int pid;
        int status;
        pid=fork();
        if (pid<0){
            printf("Forking failed\n");
            exit(1);
        }else if (pid==0){
            close(fd[0][0]);
            if(dup2(fd[0][1],STDOUT_FILENO) == -1)
			{
				fprintf(stderr, "Dup Error!");
				exit(1);
			}

            execv(path, splitted);

        }close (fd[0][1]);
        waitpid(pid,&status,0);
    }else if (all_commands[i+1]==NULL){
		pipe(fd[i]);
        int pid;
        int status;
        pid=fork();
        if (pid<0){
            printf("Forking failed\n");
            exit(1);
        }else if (pid==0){
            close(fd[i][1]);
            if(dup2(fd[i-1][0],STDIN_FILENO) == -1)
			{
				fprintf(stderr, "Dup Error!");
				exit(1);
			}

            execv(path, splitted);

        }close (fd[i-1][1]);
        waitpid(pid,&status,0);
    }else{
		pipe(fd[i]);
        int pid;
        int status;
        pid=fork();
        if (pid<0){
            printf("Forking failed\n");
            exit(1);
        }else if (pid==0){
            if(dup2(fd[i][1],STDOUT_FILENO) == -1)
			{
				fprintf(stderr, "Dup Error!");
				exit(1);
			}

            if(dup2(fd[i-1][0],STDIN_FILENO) == -1)
			{
				fprintf(stderr, "Dup Error!");
				exit(1);
			}
            execv(path, splitted);

        }close (fd[i][1]);
        close (fd[i-1][0]);
        waitpid(pid,&status,0);
    }
}

void pipe_commands_execution(char* command)
{
    char** all_commands=split_input(command,"|");
    for(int i=0; all_commands[i] != (char *)NULL ;i++)
	{
        char** splitted=split_input(all_commands[i]," ");
        char* path= (char*) malloc (500*sizeof(char*));
        path=find_path(splitted[0]);
		run_pipe_commands(path, splitted, all_commands, i);
    }
}

void launch(char* command)
{
	char** args = split_input(command, " ");
	char* path = malloc(sizeof(char) * (500));
	path = find_path(args[0]);
	create_process_and_run(path, command);
}

void and_supporter(char* command) 
{
	int counter = 0;
	for(int i=0; command[i] != '\0'; i++)
	{
		if(command[i] == '&') counter++;
	}

	char** and_split = split_input(command, "&");
	int siz = 0;
	pid_t pids[1000];
	for(int i=0; and_split[i] != NULL; i++)
	{
		siz++;
		pids[i] = fork();
		if(pids[i] < 0)
		{
			fprintf(stderr, "Fork Error!");
			exit(1);
		}
		else if(pids[i] == 0)
		{
			char** splitted = split_input(and_split[i], "|");
			int siz = 0;
			for(int i=0; splitted[i] != (char *)NULL; i++) siz++;
			if(siz > 1) pipe_commands_execution(and_split[i]);
			else launch(and_split[i]);
			exit(0);
		}
	}

	if(counter < siz)
	{
		int status;
		waitpid(pids[siz-1], &status, 0);
	}
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
	char* cmpr = "history";
	do
	{
		printf("acer@FalleN:~$");
		command = read_user_input();
		strcpy(command_history[indexx], command);
        time_t st=time(NULL);
        struct tm *local_time = localtime(&st);
        strftime(start_time[indexx], sizeof(start_time[indexx]), "%Y-%m-%d %H:%M:%S", local_time);
        long start = current_time();
		if(strcmp(cmpr, command) == 0) show_history();
		else and_supporter(command);
        long end = current_time();
        time_t en=time(NULL);
        local_time = localtime(&en);
        strftime(end_time[indexx], sizeof(end_time[indexx]), "%Y-%m-%d %H:%M:%S", local_time);
        long total_time = (long)(end - start);
        execution_time[indexx]=total_time;
        indexx++;

		free(command);
	}while(1);
}

static void sig_handler(int signum)
{
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

			char buff2[13] = "Start time: ";
			write(STDOUT_FILENO, buff2, 13);
			char* st = start_time[i];
			char* et = end_time[i];
			size_t siz_st = strlen(st);
			write(STDOUT_FILENO, st, siz_st);
			write(STDOUT_FILENO, buffr, 3);

			char buff4[11] = "End time: ";
			write(STDOUT_FILENO, buff4, 11);
			size_t siz_et = strlen(et);
			write(STDOUT_FILENO, et, siz_et);
			write(STDOUT_FILENO, buffr, 3);

			char buff3[17] = "Execution time: ";
			write(STDOUT_FILENO, buff3, 17);
			char str[100];
			sprintf(str, "%ld", execution_time[i]);
			size_t sz = strlen(str);
			write(STDOUT_FILENO, str, sz);
			write(STDOUT_FILENO, " ms\n", 4);
		}
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

int main(int argc, char** argv)
{
	ncpu = argv[1];
	tslice = argv[2];
	init_sig_handler();
	shell_loop();


	return 0;
}
