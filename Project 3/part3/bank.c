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
pthread_mutex_t account_mutex = PTHREAD_MUTEX_INITIALIZER, count_mutex = PTHREAD_MUTEX_INITIALIZER, file_mutex = PTHREAD_MUTEX_INITIALIZER;
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
int has_sig = 0;

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
    pthread_mutex_lock(&count_mutex);
    count++;
    lines_processed += 1;
    if(count == 5000){
        //write(STDOUT_FILENO, "counting\n", 9);
        is_bank = 1;
        //if(is_bank == 1){
            //pthread_mutex_lock(&account_mutex);
          //  pthread_cond_signal(&cond_bank);
            //pthread_cond_wait(&cond, &account_mutex);
            //pthread_mutex_unlock(&account_mutex);
        //}
        //write(STDOUT_FILENO, "count is 5000", 13);
    }
    pthread_mutex_unlock(&count_mutex);
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
            account_list[i].balance -= amount;
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
    int account1 = -1;
    int account2 = -1;

    while (current_line < indices->start && (getline(&line_buf, &len, stream) != -1)) {
        current_line++;
    }

    command_line token_buffer;
    while (current_line < indices->end && (getline(&line_buf, &len, stream)) != -1) {
        //printf("current line: %d\n", current_line);

        /*pthread_mutex_lock(&account_mutex);

        */
       
    
        token_buffer = str_filler(line_buf, " ");
        account1 = -1;
        account2 = -1;

        pthread_mutex_lock(&account_mutex);
        while(is_bank){
            printf("%ld number reached, pausing\n",pthread_self());
            pthread_cond_wait(&cond, &account_mutex); 
        }
        pthread_mutex_unlock(&account_mutex);

        if(strcmp(token_buffer.command_list[0], "T")==0){
            for(int i = 0; i<accounts; i++){
                if((strcmp(account_list[i].account_number, token_buffer.command_list[1])==0)){
                    
                    account1 = i;
                    
                }
            }
            for(int j = 0; j<accounts; j++){
                        if(strcmp(account_list[j].account_number, token_buffer.command_list[3])==0){
                            //printf("second accoutn is %s \n", token_buffer.command_list[1]);
                            account2 = j;
                        }
            }

            if(account1 < account2){
                if(account1 != -1){

                    pthread_mutex_lock(&account_list[account1].ac_lock);
                    //while(is_bank){
                    //}
                    pthread_mutex_lock(&account_list[account2].ac_lock);
                }else{
                    //continue;
                }

                //while(is_bank){
                //}

            }else{
                if(account1 != -1){
                    pthread_mutex_lock(&account_list[account2].ac_lock);
                    pthread_mutex_lock(&account_list[account1].ac_lock);
                }else{
                    //continue;
                }
            }
        }else{
            for(int i = 0; i<accounts; i++){
                if((strcmp(account_list[i].account_number, token_buffer.command_list[1])==0)){
                    account1 = i;
                }
            }
            if(account1 != -1){
                pthread_mutex_lock(&account_list[account1].ac_lock);
            }
        }

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

        pthread_mutex_unlock(&account_list[account1].ac_lock);
        if(account2 != -1){
            pthread_mutex_unlock(&account_list[account2].ac_lock);
        }

        pthread_mutex_lock(&account_mutex);
        if(!has_sig && is_bank == 1){
            has_sig = 1;
            pthread_cond_signal(&cond_bank);
            
            //printf("here!\n");
        }
        pthread_mutex_unlock(&account_mutex);
    }
    
    pthread_mutex_lock(&account_mutex);
    active_threads--;
    if(active_threads == 0){
        is_bank = 1;
        pthread_cond_signal(&cond_bank);
    }
    pthread_mutex_unlock(&account_mutex);
    
    
    free(line_buf);
    fclose(stream);
    printf("%ld is done\n",pthread_self());
}

void* update_balance(){ //critical section
    int index_count = 0;
    pthread_barrier_wait(&barrier);
    pthread_mutex_lock(&file_mutex);
    char filename[512];
    FILE *file;
    for(int i = 0; i<accounts; i++){
        snprintf(filename, sizeof(filename), "./output/account_%d_balance.txt", i);
        file = fopen(filename, "w");
        fprintf(file, "account %d:\n", i);
        fclose(file);
    }
     pthread_mutex_unlock(&file_mutex);
    
    while(1){
        pthread_mutex_lock(&account_mutex);
        
        while(!is_bank){
            printf("bank %ld is waiting, update time is %d\n",pthread_self(), index_count);
            //printf("isbank\n");
            pthread_cond_wait(&cond_bank, &account_mutex);
        }
        index_count++;
        printf("bank %ld signal recieved\n", pthread_self());


        //printf("index_count: %d", index_count);
        for(int i = 0; i<accounts; i++){
            pthread_mutex_lock(&account_list[i].ac_lock);
            account_list[i].balance  =  account_list[i].balance + (account_list[i].reward_rate * account_list[i].transaction_tracter);
            account_list[i].transaction_tracter = 0;
            
            
            snprintf(filename, sizeof(filename), "./output/account_%d_balance.txt", i);
            file = fopen(filename, "a");
            fprintf(file, "Balance: %.2f\n", account_list[i].balance);
            fclose(file);
            
            
            pthread_mutex_unlock(&account_list[i].ac_lock);
        }
        pthread_mutex_lock(&count_mutex);
        count = 0;
        pthread_mutex_unlock(&count_mutex);

        is_bank = 0;
        has_sig = 0;
        pthread_cond_broadcast(&cond);
        

        if(active_threads == 0){
            break;
        }
        pthread_mutex_unlock(&account_mutex);
    }
    pthread_mutex_unlock(&account_mutex);
    printf("total updates %d\n", index_count);


}

int main(int argc, char const *argv[]){
    FILE *stream = NULL;
    size_t len = 128;
    char* line_buf = malloc(len);
    stream = fopen(argv[1], "r");
    path = argv[1];

    command_line large_token_buffer;
    getline(&line_buf, &len, stream);
    large_token_buffer = str_filler(line_buf, " ");

    accounts  = atoi(large_token_buffer.command_list[0]);
    free_command_line(&large_token_buffer);
    account_list = (account*)malloc(sizeof(account)*accounts);
    for (int i = 0; i < accounts; i++) {
        strcpy(account_list[i].account_number, ""); 
        strcpy(account_list[i].password, "");       
        account_list[i].balance = 0.0;              
        account_list[i].reward_rate = 0.0;          
        account_list[i].transaction_tracter = 0.0;  
        strcpy(account_list[i].out_file, "");       

        // Initialize mutex
        if (pthread_mutex_init(&account_list[i].ac_lock, NULL) != 0) {
            perror("Failed to initialize mutex");
            // Properly handle initialization failure
        }
    }

    num_transactions = count_lines(argv[1]) - (1 + 5*accounts);
    int increment = num_transactions / NUM_WORKERS;
    
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
    pthread_barrier_init(&barrier, NULL, NUM_WORKERS + 2);
    for(int i = 0; i < NUM_WORKERS; i++){

        indices[i].start = increment * i + (1+ 5*accounts);

        indices[i].end = increment * (i+1) + (1+ 5*accounts);

        pthread_create(&threads[i], NULL, process_transaction, &indices[i]);
    }
    
    pthread_create(&bank_thread, NULL, update_balance, NULL);

    pthread_barrier_wait(&barrier); //signal threads to start
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_join(bank_thread, NULL);

    FILE* output_file;
    output_file = fopen("./output/output.txt", "w");
    for (int i = 0; i < accounts; i++) {
       fprintf(output_file, "%d Balance %.2f\n",i,account_list[i].balance);
    }
    
    fclose(output_file);
    free(line_buf);
    pthread_barrier_destroy(&barrier);
    free(indices);
    free(account_list);
    fclose(stream);
}


