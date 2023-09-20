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

    srand(time(NULL));

    for(int i = 0; i < 5; i++){
        arr[i] = rand() % 100;
    }

    int fd = open("sum", O_WRONLY);
    if(fd == -1){
        printf("Unable to open fifo \"sum\"");
        return -1;
    }

    for(int i = 0; i < 5; i++){
        if(write(fd, &arr[i], sizeof(int)) == -1){
            printf("Unable to write to fifo");
            return -1;
        }
        printf("Written: %d\n", arr[i]);
    }

    close(fd);

    return 0;
}