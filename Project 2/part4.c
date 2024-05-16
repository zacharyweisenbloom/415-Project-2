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
int current_process = 0;
int commands;
pid_t *pid_array;

void display_info(){
	pid_t pid;
	FILE* proc_file;
	char path[1024];
	char buffer[1024];
	printf("PID - utime     stime       time     nice    virt mem\n");
	for(int i =0; i<commands; i++){
		pid = pid_array[i];
		if(kill(pid, 0) == -1){
			continue;
		}
		snprintf(path,sizeof(path), "/proc/%d/stat", pid);
		proc_file = fopen(path, "r");
		double utime, stime, time;
		long nice;
		long unsigned int virt;
        command_line token_buff;
		if (fgets(buffer, sizeof(buffer), proc_file) != NULL) {
			token_buff = str_filler(buffer, " ");
			
			printf("%s ", token_buff.command_list[0]);
			/*printf("%s ", token_buff.command_list[13]);
			printf("%s ", token_buff.command_list[14]);
			printf("%s ", token_buff.command_list[18]);
			printf("%s ", token_buff.command_list[22]);
			printf("\n");
			*/
			utime = (double)atoi(token_buff.command_list[13])/sysconf(_SC_CLK_TCK);
			stime = (double)atoi(token_buff.command_list[14])/sysconf(_SC_CLK_TCK);
			time = utime + stime;
			nice = strtol(token_buff.command_list[18], NULL, 10);
			virt = strtoul(token_buff.command_list[22], NULL, 10);

			printf(" %f ", utime);
			printf(" %f ", stime);
			printf("   %f ", time);
			printf("%ld", nice);
			printf("       %lu \n", virt);
		}
	fclose(proc_file);
    free_command_line(&token_buff);
	}
}
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
		//printf("sending!\n");
        kill(p_array[i], signal);
    }
}

void next_process(){
    kill(pid_array[current_process], SIGSTOP);
    current_process = (current_process+1)%commands;
    
    while(kill(pid_array[current_process],SIGCONT) == -1){
        current_process = (current_process+1)%commands;
    }
	display_info(pid_array[current_process]);
    printf("starting process %d\n", pid_array[current_process]);
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

			if (pid_array[line_num] < 0)
			{
				printf("Error forking!!\n");

			}
			if (pid_array[line_num] == 0)
			{
				waitForSignal(SIGUSR1);
				//printf("process running!\n");
				if (execvp(small_token_buffer.command_list[0], small_token_buffer.command_list) == -1)
				{
					printf("Error, invalid command.\n");
				}
			//
            free(pid_array);
            free(line_buf);
            free_command_line (&small_token_buffer);
		    memset (&small_token_buffer, 0, 0);
            free_command_line (&large_token_buffer);
		    memset (&large_token_buffer, 0, 0);
            fclose(stream);
			exit(-1);
			}
			numChildren++;
			line_num++;

		//free smaller tokens and reset variable
        free_command_line (&small_token_buffer);
		memset (&small_token_buffer, 0, 0);
		free_command_line(&large_token_buffer);
		memset (&large_token_buffer, 0, 0);
	}
	sleep(1);
	sendSignalToAll(pid_array, numChildren, SIGUSR1);
	sendSignalToAll(pid_array, numChildren, SIGSTOP);
	//sendSignalToAll(pid_array, numChildren, SIGCONT);

    //want to do round robin here
    
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
