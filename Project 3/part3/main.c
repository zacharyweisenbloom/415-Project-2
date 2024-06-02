#include "string_parser.h"
#include "account.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_WORKERS 10

account* account_list;
int accounts;
pthread_mutex_t account_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER, cond_bank = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;

typedef struct {
    int start;
    int end;
} data;

data *indices;
const char* path;
int count=0,lines_processed=0, threads_created=0;
int is_bank = 0;
int num_transactions;
int active_threads = NUM_WORKERS;

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

void increment_count(){//pthread cond wait with while loop and condition varaible to make sure of missed signals 
    count++;
    lines_processed += 1;
     //write(STDOUT_FILENO, "counting\n", 9);
    if(count == 5000){
        is_bank = 1;
        //write(STDOUT_FILENO, "count is 5000", 13);
        pthread_cond_signal(&cond_bank);
    }
}

void transfer(char* account_1, char* password, char* account_2, double amount){
    //pthread_mutex_lock(&account_mutex);
    for(int i = 0; i<accounts; i++){
        if((strcmp(account_list[i].account_number, account_1)==0) && (strcmp(account_list[i].password, password)==0)){
            increment_count();
            for(int j = 0; j<accounts; j++){
                if(strcmp(account_list[j].account_number,account_2)==0){
                    account_list[i].balance -= amount;
                    account_list[j].balance += amount;
                    account_list[i].transaction_tracter += amount;
                }
            }
        }
    }
    //pthread_mutex_unlock(&account_mutex);
}


double check_balance(char* account_1, char* password){
    //pthread_mutex_lock(&account_mutex);
    for(int i = 0; i<accounts; i++){
        if((strcmp(account_list[i].account_number, account_1)==0) && (strcmp(account_list[i].password, password)==0)){
            
            //return account_list[i].balance;
        }
    }
    //pthread_mutex_unlock(&account_mutex);
}

void deposit(char* account_1, char* password, double amount){
    //pthread_mutex_lock(&account_mutex);
    for(int i = 0; i<accounts; i++){
        if((strcmp(account_list[i].account_number, account_1)==0) && (strcmp(account_list[i].password, password)==0)){
            increment_count();
            account_list[i].balance += amount;
            account_list[i].transaction_tracter += amount;
        }
    }
    //pthread_mutex_unlock(&account_mutex);
}

void withdraw(char* account_1, char* password, double amount){
    //pthread_mutex_lock(&account_mutex);
        for(int i = 0; i<accounts; i++){
        if((strcmp(account_list[i].account_number, account_1)==0) && (strcmp(account_list[i].password, password)==0)){
            increment_count();
            account_list[i].balance-= amount;
            account_list[i].transaction_tracter += amount;
        }
    }
    //pthread_mutex_unlock(&account_mutex);
}

void* process_transaction(void* args){ 
    pthread_barrier_wait(&barrier);
    //pthread_mutex_lock(&account_mutex);
    //pthread_cond_wait(&cond, &account_mutex);
    FILE *stream = NULL;
    size_t len = 128;
    char* line_buf = malloc(len);
    stream = fopen(path, "r");
    int current_line = 0;
    data* indices = (data*)args;

    while (current_line < indices->start && (getline(&line_buf, &len, stream) != -1)) {
        current_line++;
    }

    command_line token_buffer;
    while (current_line < indices->end && (getline(&line_buf, &len, stream)) != -1) {
        //printf("current line: %d\n", current_line);
        pthread_mutex_lock(&account_mutex);
        while(is_bank){
            pthread_cond_wait(&cond, &account_mutex);
        }
        
        token_buffer = str_filler(line_buf, " ");
        //printf("line: %s\n", token_buffer.command_list[0]);


        if(strcmp(token_buffer.command_list[0], "T") ==0){
            transfer(token_buffer.command_list[1], token_buffer.command_list[2],token_buffer.command_list[3], strtod(token_buffer.command_list[4], NULL));
        }else if(strcmp(token_buffer.command_list[0], "C")==0){
            check_balance(token_buffer.command_list[1], token_buffer.command_list[2]);
        }else if(strcmp(token_buffer.command_list[0], "D")==0){
            deposit(token_buffer.command_list[1], token_buffer.command_list[2], strtod(token_buffer.command_list[3], NULL));
        }else if(strcmp(token_buffer.command_list[0], "W")==0){
            withdraw(token_buffer.command_list[1], token_buffer.command_list[2], strtod(token_buffer.command_list[3], NULL));
        }
        current_line++;
        free_command_line(&token_buffer);
        pthread_mutex_unlock(&account_mutex);
    }

    //pthread_barrier_wait(&barrier);
    pthread_mutex_lock(&account_mutex);
    active_threads--;
    if(active_threads == 0){
        pthread_cond_signal(&cond_bank);
    }
    pthread_mutex_unlock(&account_mutex);
    
    free(line_buf);
    fclose(stream);
}

void* update_balance(){ //critical section
    pthread_barrier_wait(&barrier);
    //pthread_barrier_wait(&barrier);
    char filename[256];
    for(int i = 0; i<accounts; i++){
        snprintf(filename, sizeof(filename), "account_%d_balance.txt", i);
        FILE *file = fopen(filename, "w");
        fclose(file);
    }

    
    while(1){

        pthread_mutex_lock(&account_mutex);
        pthread_cond_wait(&cond_bank, &account_mutex);
        write(STDOUT_FILENO, "update\n", 7);

        for(int i = 0; i<accounts; i++){
            account_list[i].balance  =  account_list[i].balance + (account_list[i].reward_rate * account_list[i].transaction_tracter);
            account_list[i].transaction_tracter = 0;
            snprintf(filename, sizeof(filename), "account_%d_balance.txt", i);
            FILE *file = fopen(filename, "a");
            fprintf(file, "Balance: %.2f\n", account_list[i].balance);
            fclose(file);
        }



        count = 0;
        is_bank = 0;

        pthread_mutex_unlock(&account_mutex);
        pthread_cond_broadcast(&cond);
        if(active_threads == 0){
            break;
        }
    }
}

int main(int argc, char const *argv[]){
    FILE *stream = NULL;
    size_t len = 128;
    char* line_buf = malloc(len);
    stream = fopen(argv[1], "r");
    path = argv[1];
    freopen("output.txt", "w", stdout);
    command_line large_token_buffer;
    getline(&line_buf, &len, stream);
    large_token_buffer = str_filler(line_buf, " ");

    accounts  = atoi(large_token_buffer.command_list[0]);
    free_command_line(&large_token_buffer);
    account_list = (account*)malloc(sizeof(account)*accounts);

    num_transactions = count_lines(argv[1]) - (1 + 5*accounts);
    int increment = num_transactions / NUM_WORKERS;
    
    
    //printf("lines: %d\n", num_transactions);

    for(int i = 0; i<accounts; i++){
        getline(&line_buf, &len, stream);

        getline(&line_buf, &len, stream);
        large_token_buffer = str_filler(line_buf, " ");
        strcpy(account_list[i].account_number, large_token_buffer.command_list[0]);
        free_command_line(&large_token_buffer);

        getline(&line_buf, &len, stream);
        large_token_buffer = str_filler(line_buf, " ");
        strcpy(account_list[i].password, large_token_buffer.command_list[0]);
        free_command_line(&large_token_buffer);

        getline(&line_buf, &len, stream);
        large_token_buffer = str_filler(line_buf, " ");
        account_list[i].balance = strtod(large_token_buffer.command_list[0], NULL);
        free_command_line(&large_token_buffer);

        getline(&line_buf, &len, stream);
        large_token_buffer = str_filler(line_buf, " ");
        account_list[i].reward_rate = strtod(large_token_buffer.command_list[0], NULL);
        free_command_line(&large_token_buffer);
    }
    
    int count = 0;
    pthread_t threads[10], bank_thread;
    indices = (data*)malloc(sizeof(data)*NUM_WORKERS);
    pthread_barrier_init(&barrier, NULL, NUM_WORKERS + 1);
    for(int i = 0; i < NUM_WORKERS; i++){

        indices[i].start = increment * i + (1+ 5*accounts)+1;

        indices[i].end = increment * (i+1) + (1+ 5*accounts)+1;
        //void* args = (void*) indices;
        pthread_create(&threads[i], NULL, process_transaction, &indices[i]);
    }
    
    pthread_create(&bank_thread, NULL, update_balance, NULL);
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(bank_thread, NULL);
    
    for (int i = 0; i < accounts; i++) {
       printf("%d Balance %.2f\n",i,account_list[i].balance);
    }

    free(line_buf);
    pthread_barrier_destroy(&barrier);
    free(indices);
    free(account_list);
    fclose(stream);
}


