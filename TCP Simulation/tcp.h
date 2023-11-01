#ifndef TCP_H

#define TCP_H
// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <semaphore.h>

#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

// Macros
#define NETWORK_FIFO_PATH "net/network"
#define FILE_MODE (S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)
#define FIFO_RFLAG (O_RDONLY | O_NONBLOCK | O_EXCL)
#define FIFO_WFLAG (O_WRONLY | O_NONBLOCK | O_EXCL)


// TCP DATA OFFSET and FLAGS
#define SYN_PACKET ((5 << 12) | (1 << 1))
#define ACK_SYN_PACKET ((5 << 12) | (1 << 1) | (1 << 4))
#define ACK_PACKET ((5 << 12) | (1 << 4))
#define RST_PACKET ((5 << 12) | (1 << 2))
#define FIN_PACKET ((5 << 12 ) | (1))

// Structures
typedef enum {CLOSED, EST, FIN, LISTEN, REQ, RST, IDLE} connections;

// TCP Header Packet
typedef struct{
    uint16_t source_port;         // 16 bits: Source Port
    uint16_t dest_port;           // 16 bits: Destination Port
    uint32_t sequence_num;        // 32 bits: Sequence Number
    uint32_t ack_num;             // 32 bits: Acknowledgment Number
    uint16_t data_offset_flags;   // 16 bits: Data Offset (4 bits) and Control Flags (9 bits)
    uint16_t window_size;         // 16 bits: Window Size
    uint16_t checksum;            // 16 bits: Checksum
    uint16_t urgent_ptr;          // 16 bits: Urgent Pointer
    char data[BUFSIZ]; 
} tcp_header;

typedef struct connectionList{
    uint16_t source_port;
    uint16_t dest_port;
    connections connectionStatus;
    int portFd;     // Write end file descriptor
    struct connectionList *next;
} connectionList;

typedef struct{
    uint16_t source_port;
    uint16_t dest_port;
    bool isData;
    connections request;
    char data[BUFSIZ];
} clientReqPacket;

typedef struct{
    char data[BUFSIZ];
    bool isData;
    connections info;
} tcpClientPacket;



// Print funcitons
void printTcpHeader(const tcp_header *header);
void print_connectionList(connectionList* node);
void print_tcpClientPacket(tcpClientPacket* packet);
void print_clientReqPacket(clientReqPacket* packet);




// Wrapper Functions
// Wrapper for FIFO

void Mkfifo(char* _path, mode_t _mode);

void twoWayFifo(int *fd, char* _path, int _flags, bool isClient);

void closeTwoWayFifo(int *fd);

void createTwoWayFifo(char* _path);

// Wrapper for pipe
void Pipe(int *fd);

void Close(int fd);

void Write(int fd, const void* buf, size_t nbytes);

int Open(char *_path, int _oflag);


ssize_t Read(int fd, void* buf, size_t nbytes);

pid_t Fork();

pid_t Waitpid(pid_t childPid, int *status_ptr, int options);


// Connection -> Handshake

tcp_header create_tcp_header(
    uint16_t source_port, uint16_t dest_port, uint32_t sequence_num,
    uint32_t ack_num, uint16_t data_offset_flags, uint16_t window_size,
    uint16_t checksum, uint16_t urgent_ptr);

void send_packet(int fd, tcp_header *packet, size_t packetSize);
ssize_t receive_packet(int fd, tcp_header *packet, size_t packetSize);
void copyTcpHeader(tcp_header *dest, const tcp_header *src);
#endif

