#include "tcp.h"

#define SYS_FIFO_PATH "clientSide/system.fifo"
#define CLIENT_PORT_PATH "clientSide/ports/"
#define FILEREQ_PATH "test.txt"

void connect(uint16_t srcPort, uint16_t dest_port, connections req);
int sysFd;
int portFd;

int main(int argc, char *argv[]){
    

    sysFd = Open(SYS_FIFO_PATH, (O_EXCL | O_WRONLY));
    uint16_t sourcePort = atoi(argv[2]);
    uint16_t destPort = 80;
    // printf("I am able to open\n");
    connect(sourcePort, destPort, REQ);

    // Sending File Address
    clientReqPacket dataPacket;
    dataPacket.isData = true;
    dataPacket.source_port = sourcePort;
    dataPacket.dest_port = destPort;
    if(argc == 0){
        printf("Enter file path: ");
        fgets(dataPacket.data, BUFSIZ, stdin);
    }
    else{
        strcpy(dataPacket.data, argv[1]);
    }

    Write(sysFd, &dataPacket, sizeof(clientReqPacket));

    // File Address Sent

    // Receive Content
    printf("Waiting for response....\n");
    ssize_t readBytes;
    tcpClientPacket *data = (tcpClientPacket*)malloc(sizeof(tcpClientPacket));
    readBytes = Read(portFd, data, sizeof(tcpClientPacket));

    if(data->isData == true){
        printf("Data Received\n");
        printf("%s\n", data->data);
    }

    // Closing PORT fifo
    Close(portFd);
    // Closing system.fifo 
    Close(sysFd);

    return 0;
}

void connect(uint16_t srcPort, uint16_t dest_port, connections req){
    clientReqPacket ctrlPacket;

    ctrlPacket.source_port = srcPort;
    ctrlPacket.dest_port = dest_port;
    ctrlPacket.request = req;
    write(sysFd, &ctrlPacket, sizeof(ctrlPacket));

    char fifoPath[50];
    sprintf(fifoPath, "%s%d.fifo", CLIENT_PORT_PATH, srcPort);
    printf("Opening port....\n");
    portFd = Open(fifoPath,(O_EXCL | O_RDONLY));
    printf("File Opened...\n");
    ssize_t readBytes;
    tcpClientPacket rxPacket;
    readBytes = Read(portFd, &rxPacket, sizeof(tcpClientPacket));
    printf("Bytes Read: %ld\n", readBytes);
    print_tcpClientPacket(&rxPacket);
    // Receiving Control packet
    if(rxPacket.isData == false){
        if(rxPacket.info == EST){
            printf("Connection Established at Port %d\n", dest_port);
        }
        else{
            printf("No server application on Port: %d", dest_port);
            exit(EXIT_FAILURE);
        }
    }
}