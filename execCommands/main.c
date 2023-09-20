#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>


int main(int argc, char* argv[]){
    
    pid_t server_pid = fork();

    if(server_pid == -1){
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if(server_pid == 0){
        printf("Executing Server program\n");

        execl("server", "server", NULL);

        if(errno){
            perror("Server execl failed");
            exit(EXIT_FAILURE);
        }
    }

    pid_t client_pid = fork();

    if(client_pid == -1){
        perror("Fork Failed");
        exit(EXIT_FAILURE);
    }
    if(client_pid == 0) {
        printf("Executing Client Program\n");

        execl("client", "client", NULL);

        if(errno){
            perror("Client execl failed");
            exit(EXIT_FAILURE);
        }
    }

    while(wait(NULL) != -1);

    printf("Parent Ended!\n");

    return 0;
}