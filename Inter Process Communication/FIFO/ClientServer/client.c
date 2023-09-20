#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

void send(char *msg, int size){
    int fd = open("chat", O_WRONLY);
    if(fd == -1){
        printf("Unable to open fifo\n");
        exit(0);
    }
    
    if (write(fd, msg, size) == -1){
        printf("Error while writing to fifo\n");
        exit(0);
    }
    close(fd);
}

void rec(char *msg, int size){
    int fd = open("chat", O_RDONLY);
    if(fd == -1){
        printf("Unable to open fifo\n");
        exit(0);
    }
    ssize_t readBytes;
    if (readBytes = read(fd, msg, size) == -1){
        printf("Error while reading from fifo\n");
        exit(0);
    }
    close(fd);
}

int main(int argc, char* argv[]){

    // Chat
    char clientMessage[1024] = "Connected to Client";
    char serverMessage[1024];

    memset(serverMessage, 0, sizeof(serverMessage));

    send(clientMessage, sizeof(clientMessage));

    ssize_t numBytes;
    while(true){
        printf("Server: ");
        rec(serverMessage, sizeof(serverMessage));
        printf("%s\n", serverMessage);
        memset(serverMessage, 0, sizeof(serverMessage));

        // Send from Client to server;
        printf("Client: ");
        fgets(clientMessage, sizeof(clientMessage), stdin);
        send(clientMessage, sizeof(clientMessage));
        memset(clientMessage, 0, sizeof(clientMessage));
    }

    return 0;
}