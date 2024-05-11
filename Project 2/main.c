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
#define _GNU_SOURCE
FILE *stream = NULL;

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
		freopen("output.txt", "w", stdout);
		ifFile = true;
    }else{
		printf("Error error invalid input\n");
		return -1;
	}
	
	command_line large_token_buffer;
	command_line small_token_buffer;

	int line_num = 0;
	//loop until the file is over
	while (getline(&line_buf, &len, stream) != -1)
	{		
		//exit out of interactive mode 
		if ((strcmp(line_buf, "exit\n") == 0)) {
            break; 
        }
		
		//tokenize line buffer
		//large token is seperated by ";"
		large_token_buffer = str_filler (line_buf, ";");

		//iterate through each large token
		for (int i = 0; i < large_token_buffer.num_token; i++)
		{

			//tokenize large buffer
			//smaller token is seperated by " "(space bar)
			small_token_buffer = str_filler (large_token_buffer.command_list[i], " ");
			char * things[] = {"./iobound", "-seconds", "10", NULL};
		
			//iterate through each smaller token to print
			if(execvp(small_token_buffer.command_list[0], small_token_buffer.command_list) == -1){
				printf("fail!");
			}
			

				//printf ("\t\tToken %d: %s\n", j + 1, small_token_buffer.command_list[j]);
		}

		if (stream == stdin) {
            write(STDOUT_FILENO, ">>> ", 4);
		}

		//free smaller tokens and reset variable
		free_command_line (&large_token_buffer);
		memset (&large_token_buffer, 0, 0);
	}
	if(ifFile){fclose(stream);}
	//free line buffer
	free (line_buf);
}
