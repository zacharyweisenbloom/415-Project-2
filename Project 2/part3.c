/*
 * lab1_skeleton.c
 *
 *  Created on: Nov 25, 2020
 *      Author: Guan, Xin, Monil
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define _GNU_SOURCE
FILE *stream = NULL;
int count_lines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        return -1;
    }

    int lines = 0;
    char buffer[1024]; // Adjust buffer size as needed

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        lines++;
    }
    fclose(file);
    return lines;
}
int current_process;
int commands;
pid_t *pid_array;


void waitForSignal(int signum) {
    sigset_t sigset;
    int sig;
    sigemptyset(&sigset);
    sigaddset(&sigset, signum);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
    sigwait(&sigset, &sig);
}

void sendSignalToAll(pid_t *p_array, int n, int signal) {
	for (int i = 0; i < n; i++) {
        kill(p_array[i], signal);
    }
}

void next_process(){
    kill(pid_array[current_process], SIGSTOP);
    current_process = (current_process+1)%commands;
    
    while(kill(pid_array[current_process],SIGCONT) == -1){
        current_process = (current_process+1)%commands;
    }
    printf("Scheduling process %d\n", pid_array[current_process]);
    alarm(1);
}
int main(int argc, char const *argv[])
{
	
	//checking for command line argument
	size_t len = 128;
    char* line_buf = malloc(len);
	
	bool ifFile = false;

	if (argc == 1) {
        write(STDOUT_FILENO, ">>> ", 4);
		stream = stdin;
    }
	else if (argc == 3 && strcmp(argv[1], "-f") == 0) {
		stream = fopen(argv[2], "r");
		if(stream == NULL){
			printf("Error file does not exist\n");
			return -1;
		}
		//freopen("output.txt", "w", stdout);
		ifFile = true;
    }else{
		printf("Error error invalid input\n");
		return -1;
	}
	
	command_line large_token_buffer;
	command_line small_token_buffer;
	commands = count_lines(argv[2]);
	//current_process = commands;
	int line_num = 0;
	int numChildren = 0;

	//loop until the file is over
	pid_array = (pid_t *) malloc(sizeof(*pid_array)*commands);
	while (getline(&line_buf, &len, stream) != -1)
	{		
		
		//printf("the line: %s\n", line_buf);
		large_token_buffer = str_filler (line_buf, ";");
		//printf(" large token %d", large_token_buffer.num_token);
		//iterate through each large token

			//tokenize large buffer
			//smaller token is seperated by " "(space bar)
			small_token_buffer = str_filler(large_token_buffer.command_list[0], " ");
	
			pid_array[line_num] = fork();
			//if(pid_array[line_num] > 1)printf("pid_array%d\n", pid_array[line_num]);
			//if(pid_array[line_num] == 0)printf("pid_arrayChild%d\n", pid_array[line_num]);
			if (pid_array[line_num] < 0)
			{
				printf("Error forking!!\n");
			}
			if (pid_array[line_num] == 0)
			{
				//waitForSignal(SIGALARM);
				//printf("process running!\n");
				//kill(pid_array[line_num], SIGSTOP);
				raise(SIGSTOP);
				if (execvp(small_token_buffer.command_list[0], small_token_buffer.command_list) == -1)
				{
					printf("Error, invalid command.\n");
				}
			//
			exit(-1);
			}
			
			numChildren++;
			line_num++;
			//printf("Child here: %d\n", numChildren);
			//printf("%d\n", line_num);

				//printf ("\t\tToken %d: %s\n", j + 1, small_token_buffer.command_list[j]);
	

		//free smaller tokens and reset variable
		free_command_line (&large_token_buffer);
		memset (&large_token_buffer, 0, 0);
	}

	current_process = commands-1; 
    signal(SIGALRM, next_process);
    alarm(1);

	int status;
    while (numChildren > 0) {
        wait(&status); // Wait for any child to terminate
        numChildren--;
    }
	free(pid_array);
	if(ifFile){fclose(stream);}
	//free line buffer
	free (line_buf);

}
