# OS-Assignment-3
# A Simple Scheduler
## Sections In the File
1. [Description](#Description)
2. [Features](#Features)
3. [Prerequisites](#Prerequisites)
4. [How to Run](#How-to-Run)
5. [Working of the scheduler with shell](#Working-of-the-scheduler-with-shell)
6. [Authors](#Authors)
7. [Working Of Functions and Data Structures](#Working-Of-Functions-and-Data-Structures)

## Description

This assignment is an implementation of a simple scheduler integrated with a simple shell built in C programming language. In this, we created a simple scheduler which would run programs in a certain order to ensure that all processes share equal time on cpu and no process is favoured over other and all processes compute their output. These scheduler is integrated with shell which would take user inputs of elf file they wish to run and run them as per th signals recieved from scheduler.



## Features

- The shell should be able to take user inputs of an elf they wish to run and compute its output using command `submit` with the name of elf file like `./a.out`.

- The shell takes command line arguements about the number of cpus (`ncpu`) and `tslice` the scheduler works upon respectively.

- The scheduler works on 4 priority queues `1`, `2`, `3` and `4` which the user can send according to his wish along with submitting the elf file like `submit ./a.out 2` if he wish to send it to thr second priority.

- The shell then run these commands using a scheduler which ensures that all the process share equal time on cpu and compute their output.

- The shell also maintains a history of all the commands run till now, which is printed on the screen when the shell is temrinated using ctrl+c with other information like `pid`, completion time, waiting time etc.

## Prerequisites

Before running the program, ensure you have the following installed on your system:

1. **Linux System :**
    - You must have a linux system which would support the commands like cat, grep, ls etc.

2. **GCC (GNU Compiler Collection) :**
    - This is needed to compile the C program.
    - To check if GCC is installed, run:
   ```
      gcc --version
   ```
    - If it is not installed, you can install it using:
        - On linux System
           ```
           sudo apt update
           sudo apt install build-essential
           ```
3. **Basic Knowledge of Command Line :**
    - Familiarity with terminal/command prompt usage is required to compile and run the C program.

4. **C Standard Library :**
    - Your system should have the C standard library (typically included by default in most systems with GCC installed).

Once these are installed, you should be able to compile and run the C program from the command line.

## How to Run

1. **Download the files from the zip file :**
   Download all the files included in the zip folder.

2. **Compile the Program:** Navigate to the project directory and run the following command:
   >gcc simpleScheduler.c -o simpleScheduler

   >gcc simpleShell.c -o simpleShell
3. **Run the Program:** After compilation, run the program with the command:
   >./simpleShell `ncpu` `tslice`


## Working of the scheduler with shell

The purpose of this assignment is to implement a scheduler integrated with a simple shell in C, that mimics the functionality of a Unix/Linux scheduler and shell in some way except the scheduling policy which was left open to us to implement. The shell allows users to:

1. **Users will be able to run their executables**: Run elf files according to their wish and run them in priority as per their requirement using `submit` command along with elf file name like `./a.out`and if they wish, they can provide priority between 1-4 as per requirement.

2. **Equal Share between processes :** The scheduler ensures that all processes share equal time on cpu and run `ncpu` number of processes for a fixed `tslice` time and decrease all the priority of processes whose execution wasn't completed in this time slice (as per basic `mlfq` implmenetation).

3. **Bring back the processes from lowest most priority :** To ensure that the processes in lowest priority gets stucked in the ready queue, they are promoted to upper priority whenever they run for 3 times on the same priority, but these promotion starts only when the process is at the lowest priority till it reaches to the top.

4. **Handle SIGINT Signal:** The shell captures and manages signal SIGINT (Ctrl+C) and prints the all the history of commands with pid, completion time and waiting time.


## Authors
#### Author-1
- Name: Nipun Aggarwal
- Email: nipun23349@iiitd.ac.in
- GitHub: [Github Account](https://github.com/nipunagg2604)

#### Author-2
- Name: Ashwin Singh
- Email: ashwin23156@iiitd.ac.in
- GitHub: [Github Account](https://github.com/Ashwin-996)

#### Assignment Link -
- [Assignment Link](https://github.com/nipunagg2604/Simple-Scheduler.git)

## Working Of Functions and Data Structures

### We made the following functions while implementing our shell :

1. **sig_handler() -**
    - ***Parameters -*** It takes signal number as a arguement.
    - ***Functioning -*** If the signal is `SIGINT`, it prints the details of all the commands executed since the start of the current invocation of the `simple-shell` program and terminates the shell. The following details are printed for each command:
    * **Command name -** The name of the commands.
    * **Pid of the process -**  It is the pid of the processes which ran on the cpu.
    * **Completion time -** It is time the process took to complete in the multiple of `Tslice`.
    * **Waiting time -** It is time the process waited to complete its execution till it completed its execution.

2. **init_sig_handler() -**
    - ***Functioning -*** It initializes the `sigaction` system call and assigns a custom signal handler (`sig_handler()`) to it. The `sig_handler()` function is called once the program encounters `SIGNINT` interrupt signal.


3. **shell_loop() -**
    - ***Functioning -*** It is the main shell which provides the shell interface when the code is run. It takes the user input using `read_user_input ()` function. Then we called `and_supporter ()` function and saved its return value in `status`. If `status` is not equal to -1, it means it was a valid input and then we saved it into the `command_history` array declared globally. This process runs in an infinite loop.



4. **read_user_input() -**
    - ***Functioning -*** It takes one line input from the user for the elf file whihc need to be executed and return it.


5. **find_path() -**
    - ***Parameters -*** It takes an elf file name as a arguement and returns the path where the file is located on the machine.
    - ***Functioning -*** It creates a child process using `fork()`, inside which `stdout` is redirected to the writing end of a pipe which is created between the child and the parent. The child's process image is replaced with a new one when `execv()` is called so that the new process runs the which command on the elf file and writes the path to the pipe due to the redirection of `stdout`. The parent reads the path from the reading end of the pipe and returns the same.



6. **and_supporter() -**
- ***Paramaters -*** It takes user input and index where we need to store the pid of the process as arguement.
- ***Functioning -*** It breaks the user input by the ' ' character and extracts the priority index if send by the user in the input and saved it in `priority` variable which set to deafult `1` if no priority is send by the user. it also checks the user sent invalid elf file and returns `-1` else it send the splitted array of `submit` command and elf file to `run_scheduler_process ()` function by forking a child to ensure that the processes doesn't get blocked and the shell continues executing.


7. **split_input() -**
- ***Parameters -*** It takes a string as an arguement.
- ***Functioning -*** It splits the string by a delimiter through the inbuilt C `strtok()` function. A copy of the string is made before so as to not alter the original string passed to the function. Finally, it returns an array of `null` terminated strings.



8. **run_scheduler_process() -**
    - ***Parameters -*** It takes splitted array of user command contaning the name of elf file at index `1`, the priority index the process need to the run and index of the array in which we need to store the pid of the process.
    - ***Functioning -*** This function calls `fork ()` system call to create a child process and runs the elf file by extracting its path in the directory using `find_path ()` function and runs it using `execv ()` command by sending the path and `NULL` terminated array in its paramters. Whereas as soon as the child process is created, the parent process stops the execution using `kill (pid,SIGSTOP) ` and ensures that the process continues only when the scheduler sends is `SIGCONT` signal. After that the parent process saved the pid in `pid_history` array declared globally and adds the process pid in the corresponding queue to the priority index in the shared memory between shell and scheduler ensuring no race condtion occurs (by using semaphore) and at last gets blocked by the `waitpid` call and waits for the child process to complete its execution.



9. **Creating Queue Data Structure -**
- To implement the scheduler and shell, we implemented queue data strcuture, to ensure that the processes are run according to the order of submition and once a process runs for a time slice, it is put at the end of the queue, to ensure that all processes gets cpu share. For this, we implemented convention queue methods like `enqueue ()`,`dequeue ()`, `initialise_queue ()`, `isEmpty ()`, `isFull ()`, `printQueue ()` and `front ()` to work with the queue.

10. **Creating a shared memory between shell and scheduler -**
- To develop a interfile communication between both the files which are `simpleShell` and `simpleScheduler`, we created a shared memory with needed data arrays, queue arrays, required semaphores. To set up this shared memory, we created two functions `setup ()` and `cleanup ()` to setup the shared memory and clean the allocated memory after use.


11. **main () -**
- The main function of the shell first handles command line inputs of `ncpu` and `tslice`. It setups the shared memory between both the files. It then creates a child process and runs scheduler executable file using `execv`. it then calls `init_sig_handler ()` function and finally calls `shell_loop ()` function to provide user interface for inputs.

### We made the following functions while implementing our scheduler :
1. **increase_cycle() -**
    - ***Parameters -*** It takes an integer "priority" as input which tells us which of the four queues we will need for the function.
    - ***Functioning -*** The function adds 1 tslice amount of time to the waiting time and completion time of all processes sitting in one any of the queues after and including the queue with priority number "priority" and not running currently.
  
2. **front() -**
    - ***Parameters -*** Takes a pointer to a Queue data structure as input
    - ***Functioning -*** Returns the pid of the element at the front of the queue
  
3. **save_total_count() -**
    - ***Parameters -*** Takes a pointer to a Queue data structure as input and an integer val
    - ***Functioning -*** Updates the total_time of the last process added to the queue to val.
  
4. **change_direction() -**
    - ***Parameters -*** Takes a pointer to a Queue data structure as input
    - ***Functioning -*** This function is called when a process reaches the first or the last priority queue. It changes the direction of the process that is when the priority of the process is changed the next time will its priority be increased (go up) or decreases (go down).
  
5. **front_cnt() -**
    - ***Parameters -*** Takes a pointer to a Queue data structure as input
    - ***Functioning -*** Returns the total_time the process in front of queue has been in the system (running + waiting) from the time it arrived till now.
  
6. **run_batch() -**
    - ***Functioning -*** The function first stops all currently running processes and adds them back to their respective ready queue if they havent terminated yet, else it updates their final completion and waiting time in the array stored in shared memory. The heurisitic for adding processes to queues is that initially any process goes down a queue once it has exhausted its tslice on this queue until it reaches queue 4. If the process is in queue 4 then it goes up till it reaches queue 1 in the following way - It is promoted by one priority if it executes 3 times on the same queue. This is done till it reaches queue 1 after which it starts to go down again and this process continues until the process terminates. Then it traverses the queues from highest to lowest priority and schedules processes until either there are no more processes left to be scheduled or ncpu number of processes have been scheduled. It returns a status of 0 or 1, 0 indicating that there was no process to run during this time slice and 1 indicating that some processes have been scheduled to run on the cpu. If the status is 1, it also calls the increase_cycle() function to increase the wait and total time of the processes not sent to run on the cpu.

8. **init_scheduler() -**
    - ***Functioning -*** Runs an infinite loop in which run batch is called and if it runs successfully, that is some processes were sent to run, then the scheduler sleeps for tslice seconds and continues in the infinite loop after waking up. If not, it checks if the shell has exited due to the SIGINT signal, if it has exited then scheduler calls cleanup and exits since it did not have any more processes to run, otherwise it continues to run remaining processes.
  
7. **main() -**
    - ***Parameters -*** Takes shared memory name as parameters
    - ***Functioning -*** Initially the SIGINT signal is blocked using sigprocmask(). Then it opens the shared memory initialized by the shell program using the shared memory name. It then calls init_scheduler() after which the scheduler begins its work and starts to schedule processes.    

## Contribution

- We both implemnted the assignemnt together by thinking of the implementation of the shared memory between the both files, running different processes in according to the priority, handling saving the history of commands like completion time, waiting time, pid of the commands, thinking of appropriate implementation of bringing lowest priority processes and promoting them to upper priorities. 
