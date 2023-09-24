#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAXLINE (4096)
#define SHMFLAGS (O_CREAT | O_RDWR | O_SYNC)
#define SHMMODE (S_IRUSR | S_IWUSR)
#define SHMPROT (PROT_READ | PROT_WRITE)
#define MMAPFLAG (MAP_SHARED)

#define msleep(n) usleep(n*1000)

struct Shared{
    sem_t mutex;
};
struct Shared *shared;
// Wrapper for pipe
void Pipe(int *fd){
    
    if(pipe(fd) == -1){
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void Close(int fd){
    if(close(fd) == -1){
        perror(strerror(errno));
        exit(errno);
    }
}

void Write(int fd, const void* buf, size_t nbytes){
    if(write(fd, buf, nbytes) == -1){
        perror(strerror(errno));
        exit(errno);
    }
}

ssize_t Read(int fd, char* buf, size_t nbytes){
    ssize_t n;

    if((n = read(fd, buf, nbytes)) == -1){
        perror(strerror(errno));
        exit(errno);
    }

    return n;
}

// Wrapper for Processes
pid_t Fork(){
    pid_t pid;
    if( (pid = fork()) == -1){
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    return pid;
}  

pid_t Waitpid(pid_t childPid, int *status_ptr, int options){
    pid_t returnId;

    if((returnId = (childPid, status_ptr, options)) == -1){
        perror(strerror(errno));
        exit(errno);
    }
    return returnId;
}

// Wrapper for Semaphore
void Sem_init(sem_t* sem, int pshared, int value){
    if(sem_init(sem, pshared, value) == -1){
        Write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
        exit(EXIT_FAILURE);
    }
}

// Wraper for shared memory objects

int Shm_open(char* name, int flag, mode_t mode){
    int fd;
    if((fd = shm_open(name, flag, mode)) == -1){
        perror("Cannot open Shared Memory Segment");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void Ftruncate(int fd, off_t len){
    if(ftruncate(fd, len) == -1){
        perror("Cannot truncate the file");
        exit(EXIT_SUCCESS);
    }
}

void* Mmap(void* addr, size_t len, int protflag, int flag, int fd, off_t offset){
    void *ptr;

    if((ptr = mmap(addr, len, protflag, flag, fd, offset)) == MAP_FAILED){
        perror("Map Failed");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

void Munmap(void* ptr, size_t len){
    if(munmap(ptr, len) == -1){
        perror("Unable to delete Mapping");
        exit(EXIT_FAILURE);
    }
}

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

//home/manav/System Programming/UNPv2_IPC/Pipes/clientServer_FileReq/temp.c