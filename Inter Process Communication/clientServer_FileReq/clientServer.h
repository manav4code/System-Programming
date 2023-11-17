#ifndef CLIENTSERVER_H

#define CLIENTSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> 
#include <sys/mman.h>
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
    bool signal;
};

// Wrappers for Pipe
void Pipe(int *fd);
void Close(int fd);
void Write(int fd, const void* buf, size_t nbytes);
ssize_t Read(int fd, char* buf, size_t nbytes);

// Wrapper for Processes
pid_t Fork();
pid_t Waitpid(pid_t childPid, int *status_ptr, int options);

// Wrapper for Semaphore
void Sem_init(sem_t* sem, int pshared, int value);


// Wraper for shared memory objects
int Shm_open(char* name, int flag, mode_t mode);
void Ftruncate(int fd, off_t len);
void* Mmap(void* addr, size_t len, int protflag, int flag, int fd, off_t offset);
void Munmap(void* ptr, size_t len);



#endif
