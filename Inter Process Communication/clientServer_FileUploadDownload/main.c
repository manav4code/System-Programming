#include "clientServer.h"

// Functions of Client and Server
void client(int, int), server(int, int);

int main(int argc, char **argv){

    // Create shared Structure for sharing semaphore
    size_t size = sizeof(struct Shared);
    int fd;

    fd = Shm_open("/shared", SHMFLAGS, SHMMODE);
    Ftruncate(fd, size);
    shared = (struct Shared*)Mmap(NULL, size, SHMPROT, MMAPFLAG, fd, 0);
    close(fd);

    // Create Unnamed Pipes for IPC
    int pipeFd1[2], pipeFd2[2];
    // pipeFd1: P -> C
    // pipeFd2: C -> P
    Pipe(pipeFd1);
    Pipe(pipeFd2);

    // Initializing Shared Semaphore with value 0
    Sem_init(&shared->mutex, 1, 0); 

    // Create Child Process for running Server Application
    pid_t childProcess;

    if((childProcess = Fork()) == 0){
        // Child Process
        /*  Closing Write end of Pipe 1
          & Closing Read end of Pipe 2   */
        Close(pipeFd1[1]);
        Close(pipeFd2[0]);

        server(pipeFd1[0], pipeFd2[1]);
        exit(EXIT_SUCCESS);
    }

    // Parent Process
    /*  Closing Read end of Pipe 1
     &  Closing Write end of Pipe 2     */
    Close(pipeFd1[0]);
    Close(pipeFd2[1]);

    client(pipeFd2[0], pipeFd1[1]);
    Waitpid(childProcess, NULL, 0);
    shm_unlink("/shared");
    return 0;
}

void client(int readfd, int writefd){
    size_t len;
    ssize_t n;
    char buffer[MAXLINE];

    // Get File Path from the user
    printf("Enter Path: ");
    fgets(buffer, MAXLINE, stdin);
    len = strlen(buffer);

    // Remove newline character
    if(buffer[len - 1] == '\n')
        len--;
    
    printf("Sending Path...\n");
    // Send path to Server
    Write(writefd, buffer, len);


    sem_wait(&shared->mutex);
    // Keep reading from Pipe
    printf("Content of file:\n");
    while((n = Read(readfd, buffer, MAXLINE)) > 0){
        // Writing to STDOUT
        Write(STDOUT_FILENO, buffer, n);
    }
}

void server(int readfd, int writefd){
    int filefd;
    ssize_t n;
    char buffer[MAXLINE + 1];

    // Read PATH from Pipe
    if( (n = Read(readfd, buffer, MAXLINE)) == 0){
        perror("EOF while reading the PATH");
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0';

    if( (filefd = open(buffer, O_RDONLY)) < 0){
        // Write back the error msg to client
        snprintf(buffer + n, sizeof(buffer) - n, ": can't open file, %s\n", strerror(errno));

        n = strlen(buffer);
        Write(writefd, buffer, n);
    }
    else{
        sem_post(&shared->mutex);
        // Read file till EOF and Write to Pipe
        while( (n = Read(filefd, buffer, MAXLINE)) > 0){
            Write(writefd, buffer, n);
        }
    }

    // Close file
    Close(filefd);

}