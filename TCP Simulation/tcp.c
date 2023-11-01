#include "tcp.h"


// Wrapper for pipe
void Pipe(int *fd){
    
    if(pipe(fd) == -1){
        perror("Error occured while creating Pipe");
        exit(EXIT_FAILURE);
    }
}

int Open(char *_path, int _oflag){
    int fd;

    if( (fd = open(_path, _oflag)) == -1){
        if(errno == 2 && ((_oflag & 1) == 0)){
            // If FILE does not exist and
            // attempt was made to Open for Read
            // Only then create FIFO
            Mkfifo(_path, FILE_MODE);
            // Again try to open it.
            return Open(_path, _oflag);
        }

        perror("Error opening file");
        printf("errno = %d\n", errno);
        exit(EXIT_FAILURE);
    }
    return fd;
}

void Close(int fd){
    if(close(fd) == -1){
        perror("Error occured while Close");
        exit(EXIT_FAILURE);
    }
}

void Write(int fd, const void* buf, size_t nbytes){
    if(write(fd, buf, nbytes) == -1){
        perror("Error occured while Writing");
        exit(EXIT_FAILURE);
    }
}

void Mkfifo(char *_path, mode_t _mode){
    if(mkfifo(_path, _mode) == -1){
        perror("Error while creating FIFO");
        exit(EXIT_FAILURE);
    }
}

void createTwoWayFifo(char* _path){
    char readFilePath[20];
    char writeFilePath[20]; 

    strcpy(readFilePath, _path);
    strcpy(writeFilePath, _path);

    printf("File paths: %s %s\n",readFilePath, writeFilePath);
    strcat(readFilePath,".r");
    strcat(writeFilePath,".w");

    printf("File paths: %s %s\n",readFilePath, writeFilePath);

    // Creating FIFO
    Mkfifo(readFilePath, FILE_MODE);
    Mkfifo(writeFilePath, FILE_MODE);

    printf("Created FIFOs\n");
}

void twoWayFifo(int *fd, char* _path, int _flags, bool isClient){
    char readFilePath[20];
    char writeFilePath[20]; 

    strcpy(readFilePath, _path);
    strcpy(writeFilePath, _path);

    strcat(readFilePath,".r");
    strcat(writeFilePath,".w");
    // Creating Read-Write ends and returning the descriptors

    /*
    -> .r FIFO is client -> read and server -> write
    -> .w FIFO is client -> write and server -> read
    */
    if(isClient){
        fd[0] = Open(readFilePath, (_flags | O_RDONLY));
        fd[1] = Open(writeFilePath, (_flags | O_WRONLY));
    }
    else{
        fd[1] = Open(readFilePath, (_flags | O_WRONLY));
        fd[0] = Open(writeFilePath, (_flags | O_RDONLY));
    }
}

void closeTwoWayFifo(int *fd){
    Close(fd[0]);
    Close(fd[1]);
}

ssize_t Read(int fd, void* buf, size_t nbytes){
    ssize_t n;

    if((n = read(fd, buf, nbytes)) == -1){
        perror("Error occured while Reading");
        printf("Error No -> %d", errno);
        exit(EXIT_FAILURE);
    }

    return n;
}

// Wrapper for Processes
pid_t Fork(){
    pid_t pid;
    if( (pid = fork()) == -1){
        perror("Error occured while Fork");
        exit(EXIT_FAILURE);
    }
    return pid;
}  

pid_t Waitpid(pid_t childPid, int *status_ptr, int options){
    pid_t returnId;

    if((returnId = (childPid, status_ptr, options)) == -1){
        perror("Error occured while Waitpid");
        exit(EXIT_FAILURE);
    }
    return returnId;
}



// TCP Connection -> 2-way handshake

tcp_header create_tcp_header(
    uint16_t source_port, uint16_t dest_port, uint32_t sequence_num,
    uint32_t ack_num, uint16_t data_offset_flags, uint16_t window_size,
    uint16_t checksum, uint16_t urgent_ptr) {
    tcp_header hdr;
    hdr.source_port = source_port;
    hdr.dest_port = dest_port;
    hdr.sequence_num = sequence_num;
    hdr.ack_num = ack_num;
    hdr.data_offset_flags = data_offset_flags;
    hdr.window_size = window_size;
    hdr.checksum = checksum;
    hdr.urgent_ptr = urgent_ptr;
    return hdr;
}

void send_packet(int fd, tcp_header *packet, size_t packetSize){
    Write(fd, packet, packetSize);
}

ssize_t receive_packet(int fd, tcp_header *packet, size_t packetSize){
    ssize_t receiveBytes;

    receiveBytes = Read(fd, packet, packetSize);

    return receiveBytes;
}

////////////////////////////////////////////////////////////

void printTcpHeader(const tcp_header *header) {
    printf("-------------------------------\n");
    printf("Source Port: %d\n", header->source_port);
    printf("Destination Port: %d\n", header->dest_port);
    printf("Sequence Number: %d\n", header->sequence_num);
    printf("Acknowledgment Number: %d\n", header->ack_num);
    printf("Data Offset and Control Flags: 0x%x\n", header->data_offset_flags);
    printf("Window Size: %u\n", header->window_size);
    printf("Checksum: 0x%d\n", header->checksum);
    printf("Urgent Pointer: %u\n", header->urgent_ptr);
    printf("-------------------------------\n");
}

void copyTcpHeader(tcp_header *dest, const tcp_header *src) {
    dest->source_port = src->source_port;
    dest->dest_port = src->dest_port;
    dest->sequence_num = src->sequence_num;
    dest->ack_num = src->ack_num;
    dest->data_offset_flags = src->data_offset_flags;
    dest->window_size = src->window_size;
    dest->checksum = src->checksum;
    dest->urgent_ptr = src->urgent_ptr;
}


// Print Functions
void print_connectionList(connectionList* node){
    printf("-----------------------------------\n");
    printf("Connection Record:\n");
    printf("Source Port: %d\n", node->source_port);
    printf("Destination Port: %d\n", node->dest_port);
    printf("port fd: %d\n",node->portFd);
    printf("Connection Request type: %d\n", node->connectionStatus);
    printf("-----------------------------------\n");
}

void print_tcpClientPacket(tcpClientPacket* packet){
    printf("-------------------------------\n");
    printf("Packet from Client_TCP\n");
    printf("isData: %d\n", packet->isData);
    printf("Connections Type: %d\n", packet->info);
    printf("Data -> %s\n", packet->data);
    printf("-------------------------------\n");
}

void print_clientReqPacket(clientReqPacket* packet){
    printf("-------------------------------\n");
    printf("Source Port: %d\n", packet->source_port);
    printf("Destination Port: %d\n", packet->dest_port);
    printf("Request: %d\n",packet->request);
    printf("-------------------------------\n");
}