#include "tcp.h"

#define SERVER_PORT_PATH "serverSide/ports/"
#define SYSTEM_FIFO_PATH "serverSide/system.fifo"

int portFd;
int sysFd;

void listen(uint16_t destinationPort){
    // This shall keep on listening to PORT 
    tcpClientPacket rxPacket;
    ssize_t readBytes;

    char fifoPath[50];
    sprintf(fifoPath, "%s%d.fifo", SERVER_PORT_PATH, destinationPort);

    portFd = Open(fifoPath, (O_EXCL | O_RDONLY));
    printf("Listening on Port %d.....\n", destinationPort);
    while( (readBytes = Read(portFd, &rxPacket, sizeof(tcpClientPacket))) > 0){
        print_tcpClientPacket(&rxPacket);
        if(rxPacket.info == EST){
            return;
        }
        else{
            printf("idk...\n");
        }
    }
}

int main(){
    
    sysFd = Open(SYSTEM_FIFO_PATH, (O_EXCL | O_WRONLY));

    printf("File Server is live....\n");
    uint16_t portNum = 80;
    while(true){
        listen(portNum);
        printf("Connected...\n");
        while(true){
            tcpClientPacket* dataPacket = (tcpClientPacket*)malloc(sizeof(tcpClientPacket));
            ssize_t readBytes = Read(portFd, dataPacket, sizeof(tcpClientPacket));
            if(dataPacket->isData){
                printf("File Requested: %s\n", dataPacket->data);
                size_t len = strlen(dataPacket->data);
                
                if(dataPacket->data[len-1] == '\n'){
                    // Removing newline character from the path.
                    // Since fgets stops read at newline but buffers it. 
                    dataPacket->data[len-1] = '\0';
                }
                int filefd;
                tcpClientPacket* txPacket = (tcpClientPacket*)malloc(sizeof(tcpClientPacket));
                txPacket->isData = true;
                memset(txPacket->data, 0, BUFSIZ);
                if( (filefd = open(dataPacket->data, O_RDONLY)) < 0){
                    // No such file exists
                    strcpy(txPacket->data, "No File Exist");
                    Write(sysFd, txPacket, sizeof(tcpClientPacket));
                }
                else{
                    // File exist, read file content and send
                    Read(filefd, txPacket->data, BUFSIZ);
                    Write(sysFd, txPacket, sizeof(tcpClientPacket));
                }
                free(txPacket);
                close(filefd);
            }
            else{
                if(dataPacket->info == FIN){
                    // Connection Termination
                    // Get out of this loop, no further file request from same client.
                    break;
                }
            }
        }   
        close(portFd);
    }
    close(sysFd);
    return 0;
}