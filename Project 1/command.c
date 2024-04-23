#include <stdio.h>
#include <dirent.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>
extern FILE* stream;

void listDir(){
    DIR* dir = opendir(".");
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        int len = strlen(entry->d_name);
        write(STDOUT_FILENO, entry->d_name, len);
        write(STDOUT_FILENO, " ", 1);
    }
    write(STDOUT_FILENO, "\n", 1);
    closedir(dir);
    
} 

void showCurrentDir(){
    char* buffer;
    size_t size = 1024;
    buffer = malloc(size);
    getcwd(buffer, size);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n",1);
    free(buffer);
} 

void makeDir(char *dirName){
if (mkdir(dirName, 0755) == -1){
    switch (errno) {
        case EACCES:
            write(STDOUT_FILENO, "Permission denied\n", strlen("Permission denied\n"));
            break;
        case EEXIST:
            write(STDOUT_FILENO, "Directory already exists\n", strlen("Directory already exists\n"));
            break;
        case ENOENT:
            write(STDOUT_FILENO, "A component of the path does not exist\n", 40);
            break;
        default:
            write(STDOUT_FILENO, "An error occurred\n", 18);
        }
    }
}

void changeDir(char *dirName){
    if(chdir(dirName) == -1){
        char* message = "Error Directory not found \n";
        write(STDOUT_FILENO, message, strlen(message));
    }
} /*for the cd command*/

void copyFile(char *sourcePath, char *destinationPath){
    int src_fd, dst_fd;
    ssize_t num_read;
    char buffer[1024];
    struct stat statbuf;
    char* Dest;

    src_fd = open(sourcePath, O_RDONLY);
    if (src_fd == -1) {
        char* message = "Error opening source file \n";
        write(STDOUT_FILENO, message, strlen(message));
        return;
    }
    if (stat(destinationPath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        Dest = (char*)malloc(sizeof(char)*(strlen(sourcePath) + strlen(destinationPath) + 2));
        strcpy(Dest, destinationPath);
        strcat(Dest, "/");
        strcat(Dest, basename(sourcePath));
        Dest[strlen(sourcePath) + strlen(destinationPath)+1] = '\0';
    }else{
        Dest = (char*)malloc(sizeof(char)*(strlen(destinationPath))+1);
        strcpy(Dest, destinationPath);
        Dest[strlen(destinationPath)] = '\0';
        
       //Dest = strdup(destinationPath);
       
    }
    
    dst_fd = open(Dest, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (dst_fd == -1) {
            char* message = "Error opening destination file \n";
            write(STDOUT_FILENO, message, strlen(message));
            //close(src_fd);  // Close source file descriptor before returning
            return;
    }
    
    
    while((num_read = read(src_fd, buffer, sizeof(buffer))) > 0){
        if(write(dst_fd, buffer, num_read) != num_read){
            //"Error writing to destination file"
            break;
        }
    } 

    if(num_read == -1){
        char* message = "Error reading source file\n";
        write(STDOUT_FILENO, message, strlen(message));
    }
    close(src_fd);
    close(dst_fd);
    free(Dest);
}
void moveFile(char *sourcePath, char *destinationPath){
    copyFile(sourcePath, destinationPath);
    remove(sourcePath);
} /*for the mv command*/

void deleteFile(char *filename){
    if(remove(filename) == -1){
        char* message = "File not found \n";
        write(STDOUT_FILENO, message, strlen(message));
    }
} /*for the rm command*/

void displayFile(char *filename){
    int src_fd;
    ssize_t num_read;

    char buffer[1024];
    src_fd = open(filename, O_RDONLY);
    if (src_fd == -1) {
        char* message = "File not found \n";
        write(STDOUT_FILENO, message, strlen(message));
        return;
    }
    while((num_read = read(src_fd, buffer, sizeof(buffer)))  > 0){
    if(write(STDOUT_FILENO, buffer, num_read) != num_read){
        //perror("Error writing to destination file");
        close(src_fd);
        
        return;
    }
} 
} /*for the cat command*/
