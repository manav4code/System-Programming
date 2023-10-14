#include "clientServer.h"

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