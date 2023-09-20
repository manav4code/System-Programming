#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>


int main(int argc, char* argv[]){
    
    int arr[5];
    int fd = open("sum", O_RDONLY);
    
    if(fd == -1){
        printf("Unable to open fifo \"sun\"");
        return -1;
    }

    for(int i = 0; i < 5; i++){
        if (read(fd, &arr[i], sizeof(int)) == -1){
            printf("Problem reading the fifo");
            return -1;
        }

        printf("Read: %d\n", arr[i]);
    }

    close(fd);
    return 0;
}