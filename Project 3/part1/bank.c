#include "string_parser.h"
#include "account.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

account* account_list;
int accounts;

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

void transfer(char* account_1, char* password, char* account_2, double amount){
    for(int i = 0; i<accounts; i++){
        if((strcmp(account_list[i].account_number, account_1)==0) && (strcmp(account_list[i].password, password)==0)){
            for(int j = 0; j<accounts; j++){
                if(strcmp(account_list[j].account_number,account_2)==0){
                    account_list[i].balance -= amount;
                    account_list[j].balance += amount;
                    account_list[i].transaction_tracter += amount;
                }
            }
        }
    }
}

double check_balance(char* account_1, char* password){
    for(int i = 0; i<accounts; i++){
        if((strcmp(account_list[i].account_number, account_1)==0) && (strcmp(account_list[i].password, password)==0)){
            return account_list[i].balance;
        }
    }
    return -1;
}

void deposit(char* account_1, char* password, double amount){
    for(int i = 0; i<accounts; i++){
        if((strcmp(account_list[i].account_number, account_1)==0) && (strcmp(account_list[i].password, password)==0)){
            account_list[i].balance += amount;
            account_list[i].transaction_tracter += amount;
        }
    }
}

void withdraw(char* account_1, char* password, double amount){
        for(int i = 0; i<accounts; i++){
        if((strcmp(account_list[i].account_number, account_1)==0) && (strcmp(account_list[i].password, password)==0)){
            account_list[i].balance-= amount;
            account_list[i].transaction_tracter += amount;
        }
    }
}

void* process_transaction(command_line* token_buffer){ 

    if(strcmp(token_buffer->command_list[0], "T") ==0){
        transfer(token_buffer->command_list[1], token_buffer->command_list[2],token_buffer->command_list[3], strtod(token_buffer->command_list[4], NULL));
    }else if(strcmp(token_buffer->command_list[0], "C")==0){
        check_balance(token_buffer->command_list[1], token_buffer->command_list[2]);
    }else if(strcmp(token_buffer->command_list[0], "D")==0){
        deposit(token_buffer->command_list[1], token_buffer->command_list[2], strtod(token_buffer->command_list[3], NULL));
    }else if(strcmp(token_buffer->command_list[0], "W")==0){
        withdraw(token_buffer->command_list[1], token_buffer->command_list[2], strtod(token_buffer->command_list[3], NULL));
    }
}

void* update_balance(){ //critical section
    for(int i = 0; i<accounts; i++){
        account_list[i].balance  =  account_list[i].balance + (account_list[i].reward_rate * account_list[i].transaction_tracter);
        account_list[i].transaction_tracter = 0;
    }
}

int main(int argc, char const *argv[]){

    FILE *stream = NULL;
    size_t len = 128;
    char* line_buf = malloc(len);
    stream = fopen(argv[1], "r");
    freopen("output.txt", "w", stdout);
    command_line large_token_buffer;


    getline(&line_buf, &len, stream);
    large_token_buffer = str_filler(line_buf, " ");

    accounts  = atoi(large_token_buffer.command_list[0]);
    account_list = (account*)malloc(sizeof(account)*accounts);
    for (int i = 0; i < accounts; i++) {
        strcpy(account_list[i].account_number, ""); // Initialize to empty string
        strcpy(account_list[i].password, "");       // Initialize to empty string
        account_list[i].balance = 0.0;              // Initialize to zero
        account_list[i].reward_rate = 0.0;          // Initialize to zero
        account_list[i].transaction_tracter = 0.0;  // Initialize to zero
        strcpy(account_list[i].out_file, "");       // Initialize to empty string

        // Initialize mutex
        if (pthread_mutex_init(&account_list[i].ac_lock, NULL) != 0) {
            perror("Failed to initialize mutex");
            // Properly handle initialization failure
        }
    }

    int num_transactions = count_lines(argv[1]) - (1 + 5*accounts);
    int increment = num_transactions / 10;
    free_command_line(&large_token_buffer);

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
    while(getline(&line_buf, &len, stream) != -1){
        large_token_buffer = str_filler(line_buf, " ");
        process_transaction(&large_token_buffer);
        free_command_line(&large_token_buffer);
    }
    update_balance();
    
    for (int i = 0; i < accounts; i++) {
        printf("%d Balance %.2f\n",i, account_list[i].balance);
    }
    free(account_list);
    fclose(stream);
    free(line_buf);
}


